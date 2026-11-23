// Ticket Queue -> "Ticket Queue"
//
// The list<string> is a line of people at a ticket desk, front nearest the desk. The
// input's commands play in order: join walks someone to the back (push_back), vip cuts
// to the front (push_front), leave steps someone out wherever they are (remove). At each
// serve the program prints who is served: that person walks up to the desk ("Nobody
// waiting" buzzes an empty desk). "Still waiting:" tags everyone left. Serving the wrong
// person sends them up in red; serving from the back, or a vip left at the back, is named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { QU_JOIN = 1, QU_VIP, QU_LEAVE, QU_SERVE, QU_NOBODY, QU_WAITING };
enum { QUF_SERVE = 1, QUF_WAITING, QUF_GARBLED, QUF_EXTRA };

#define QMAX 12

// ---------------------------------------------------------------- the parser

static void ParseQueue(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    EvSpawn(&p, 0, 0, 0, 0);
    if (ParsePreview(&p)) return;

    char q[QMAX][32], note[140];
    bool vip[QMAX];
    int qn = 0, e = 0, used;
    const char *s = run->input ? run->input : "";
    char cmd[16], name[32];
    for (int i = 0; i < 40 && !p.stopped && sscanf(s, " %15s%n", cmd, &used) == 1; i++) {
        s += used;
        bool hasName = strcmp(cmd, "serve") != 0;
        if (hasName) { if (sscanf(s, " %31s%n", name, &used) != 1) break; s += used; }
        if (strcmp(cmd, "join") == 0 && qn < QMAX) { StrCopy(q[qn], 32, name); vip[qn] = false; qn++; GameEvent *g = EvAct(&p, QU_JOIN, i, 0, 0); if (g) StrCopy(g->text, EV_TEXT, name); }
        else if (strcmp(cmd, "vip") == 0 && qn < QMAX) {
            for (int k = qn; k > 0; k--) { memcpy(q[k], q[k - 1], 32); vip[k] = vip[k - 1]; }
            StrCopy(q[0], 32, name); vip[0] = true; qn++;
            GameEvent *g = EvAct(&p, QU_VIP, i, 0, 0); if (g) StrCopy(g->text, EV_TEXT, name);
        } else if (strcmp(cmd, "leave") == 0) {
            for (int k = 0; k < qn; k++) if (strcmp(q[k], name) == 0) { for (int j = k; j + 1 < qn; j++) { memcpy(q[j], q[j + 1], 32); vip[j] = vip[j + 1]; } qn--; k--; }
            GameEvent *g = EvAct(&p, QU_LEAVE, i, 0, 0); if (g) StrCopy(g->text, EV_TEXT, name);
        } else if (strcmp(cmd, "serve") == 0) {
            int el, al;
            const char *E = LineAt(&p.exp, e, &el);
            const char *A = ParsePeek(&p, &al);
            if (E && ParseTake(&p, e)) {
                e++;
                if (qn > 0) { GameEvent *g = EvAct(&p, QU_SERVE, i, 0, 0); if (g) StrCopy(g->text, EV_TEXT, q[0]); for (int j = 0; j + 1 < qn; j++) { memcpy(q[j], q[j + 1], 32); vip[j] = vip[j + 1]; } qn--; }
                else EvAct(&p, QU_NOBODY, i, 0, 0);
                continue;
            }
            char got[64] = "";
            if (A) { char tmp[96]; int tl = al < 95 ? al : 95; memcpy(tmp, A, (size_t)tl); tmp[tl] = '\0'; if (sscanf(tmp, "Serving %63s", got) != 1) StrCopy(got, sizeof(got), ""); }
            if (qn > 1 && strcmp(got, q[qn - 1]) == 0) snprintf(note, sizeof(note), "That's the back of the line: serve takes the front (front, then pop_front)");
            else if (qn > 0 && !vip[0] && got[0]) {
                int vk = -1;
                for (int k = 0; k < qn && vk < 0; k++) if (vip[k] && strcmp(q[k], got) == 0) vk = k;
                if (vk < 0) snprintf(note, sizeof(note), "%s is at the front, so %s is served", q[0], q[0]);
                else snprintf(note, sizeof(note), "vip goes to the front with push_front");
            } else if (qn == 0) snprintf(note, sizeof(note), "The line is empty: print Nobody waiting");
            else snprintf(note, sizeof(note), "%s is at the front, so %s is served", q[0], q[0]);
            GameEvent *f = ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, QUF_SERVE, e, i, 0, 0, note);
            if (f) StrCopy(f->text, EV_TEXT, got[0] ? got : (A && al ? "?" : ""));
            break;
        }
    }
    for (; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (el >= 14 && memcmp(E, "Still waiting:", 14) == 0) {
            if (ParseTake(&p, e)) { EvAct(&p, QU_WAITING, 0, 0, 0); continue; }
            snprintf(note, sizeof(note), "Print everyone left, front first: %.*s", el - 15, E + 15);
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, QUF_WAITING, e, 0, 0, 0, note);
            break;
        }
        if (!ParseTake(&p, e)) { ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, QUF_GARBLED, e, 0, 0, 0, "Every serve prints one line"); break; }
    }
    ParseEnd(&p, QUF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_EXIT, DO_CHEER, DO_BAD, DO_BUZZ };

typedef struct { char name[EV_TEXT]; Bot bot; bool in, leaving, served, bad; } Person;

static struct {
    const EventList *ev;
    Cues   cues;
    float  time;
    Person p[QMAX * 2];
    int    count;
    int    line[QMAX], lineN;        // people in the line, front first
    char   desk[EV_TEXT];
    bool   deskBad, nobody, waiting, waitingBad;
    float  buzz;
    char   want[96];
} Q;

static Vector3 Slot(int k) { return V3(2.1f - k * 0.85f, 0, 0.2f); }
static Vector3 Desk(void) { return V3(3.4f, 0, -0.5f); }

static Color NameColor(const char *s)
{
    unsigned h = 2166136261u;
    for (; *s; s++) h = (h ^ (unsigned char)*s) * 16777619u;
    static const Color cols[6] = { { 120, 190, 255, 255 }, { 255, 170, 90, 255 }, { 150, 230, 160, 255 }, { 230, 140, 220, 255 }, { 255, 220, 110, 255 }, { 160, 150, 255, 255 } };
    return cols[h % 6];
}

static void QueueReset(const EventList *ev)
{
    memset(&Q, 0, sizeof(Q));
    Q.ev = ev;
    ShotOrbit(V3(-0.2f, 0.8f, 0.2f), 9.0f, 26, 0, 38, 0);
}

static void Relayout(void)
{
    for (int k = 0; k < Q.lineN; k++) {
        Person *pp = &Q.p[Q.line[k]];
        BotMoveTo(&pp->bot, Slot(k), 0.6f);
    }
}

static int NewPerson(const char *name, Vector3 from)
{
    if (Q.count >= QMAX * 2) return -1;
    Person *pp = &Q.p[Q.count];
    memset(pp, 0, sizeof(*pp));
    StrCopy(pp->name, EV_TEXT, name);
    BotInit(&pp->bot, from, 90);
    pp->bot.scale = 0.5f;
    pp->bot.body = NameColor(name);
    pp->bot.accent = (Color){ 60, 64, 84, 255 };
    pp->in = true;
    return Q.count++;
}

static int FindInLine(const char *name)
{
    for (int k = 0; k < Q.lineN; k++) if (strcmp(Q.p[Q.line[k]].name, name) == 0) return k;
    return -1;
}

static void Serve(int k, bool bad)
{
    if (k < 0 || k >= Q.lineN) return;
    int id = Q.line[k];
    for (int j = k; j + 1 < Q.lineN; j++) Q.line[j] = Q.line[j + 1];
    Q.lineN--;
    Person *pp = &Q.p[id];
    pp->served = true;
    pp->bad = bad;
    BotMoveTo(&pp->bot, V3(Desk().x - 0.6f, 0, Desk().z + 0.2f), 0.7f);
    if (!bad) CueAt(&Q.cues, Q.time + 1.2f, DO_EXIT, id, 0, V3(0, 0, 0), 0);
    Relayout();
}

static float QueueEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN: return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            switch (e->code) {
                case QU_JOIN: { int id = NewPerson(e->text, V3(-5.0f, 0, 0.2f)); if (id >= 0 && Q.lineN < QMAX) { Q.line[Q.lineN++] = id; Relayout(); } return 0.9f; }
                case QU_VIP: {
                    int id = NewPerson(e->text, V3(1.0f, 0, 1.6f));
                    if (id >= 0 && Q.lineN < QMAX) { for (int k = Q.lineN; k > 0; k--) Q.line[k] = Q.line[k - 1]; Q.line[0] = id; Q.lineN++; Relayout(); }
                    return 1.0f;
                }
                case QU_LEAVE: {
                    int k = FindInLine(e->text);
                    if (k >= 0) {
                        Person *pp = &Q.p[Q.line[k]];
                        pp->leaving = true;
                        BotMoveTo(&pp->bot, V3(pp->bot.pos.x, 0, 2.6f), 0.9f);
                        for (int j = k; j + 1 < Q.lineN; j++) Q.line[j] = Q.line[j + 1];
                        Q.lineN--;
                        Relayout();
                    }
                    return 1.0f;
                }
                case QU_SERVE: StrCopy(Q.desk, EV_TEXT, e->text); Q.nobody = false; Serve(FindInLine(e->text), false); return 1.4f;
                case QU_NOBODY: Q.nobody = true; Q.desk[0] = '\0'; CueAt(&Q.cues, Q.time + 0.1f, DO_BUZZ, 0, 0, V3(0, 0, 0), 0); return 1.2f;
                case QU_WAITING: Q.waiting = true; return 1.2f;
            }
            return 0;
        case EV_FAIL:
            if (Q.ev) StrCopy(Q.want, sizeof(Q.want), Q.ev->expected);
            if (e->code == QUF_SERVE) {
                Q.deskBad = true;
                StrCopy(Q.desk, EV_TEXT, e->text[0] ? e->text : "(nothing)");
                int k = FindInLine(e->text);
                if (k >= 0) Serve(k, true);
                CueAt(&Q.cues, Q.time + 0.8f, DO_BAD, 0, 0, V3(Desk().x, 1.4f, Desk().z), 0);
            } else {
                Q.waiting = Q.waitingBad = true;
                CueAt(&Q.cues, Q.time + 0.4f, DO_BAD, 0, 0, V3(0, 1.4f, 0.2f), 0);
            }
            return 2.8f;
        case EV_CRASH:
            CueAt(&Q.cues, Q.time + 0.1f, DO_BAD, 0, 0, V3(Desk().x, 1.4f, Desk().z), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&Q.cues, Q.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void QueueUpdate(float dt)
{
    Q.time += dt;
    Q.buzz = fmaxf(0, Q.buzz - dt);
    for (int i = 0; i < Q.count; i++) BotUpdate(&Q.p[i].bot, dt);
    Cue c;
    while (CueDue(&Q.cues, Q.time, &c)) {
        if (c.what == DO_EXIT) { Person *pp = &Q.p[c.a]; BotMoveTo(&pp->bot, V3(5.6f, 0, -0.5f), 1.0f); }
        else if (c.what == DO_BUZZ) { Q.buzz = 1; FxSparks(V3(Desk().x, 1.5f, Desk().z), 10, (Color){ 255, 200, 90, 255 }); }
        else if (c.what == DO_BAD) { FxSparks(c.v, 30, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0, 2.0f, 0), PlaybackFinale() ? 150 : 70, 3.0f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); for (int i = 0; i < Q.lineN; i++) BotCheer(&Q.p[Q.line[i]].bot); }
    }
}

static void QueueDraw(void)
{
    KitStage(V3(-0.3f, 0, 0.4f), 5.2f, 1.9f);
    // the desk, with its light
    Vector3 d = Desk();
    Part(MESH_CUBE, V3(d.x, 0.45f, d.z), V3(1.0f, 0.9f, 1.2f), (Color){ 92, 80, 64, 255 }, 0);
    Part(MESH_CUBE, V3(d.x, 0.92f, d.z), V3(1.1f, 0.06f, 1.3f), (Color){ 130, 110, 90, 255 }, 0);
    Color lamp = Q.deskBad ? (Color){ 235, 90, 90, 255 } : Q.buzz > 0 ? (Color){ 255, 190, 70, 255 } : Q.desk[0] ? (Color){ 80, 220, 130, 255 } : (Color){ 90, 100, 130, 255 };
    KitLamp(V3(d.x + 0.3f, 0.95f, d.z - 0.3f), lamp, 0.8f);
    // the queue rope
    for (int k = 0; k < 8; k++) Part(MESH_CYLINDER, V3(2.5f - k * 0.85f, 0.25f, -0.4f), V3(0.08f, 0.5f, 0.08f), (Color){ 160, 140, 90, 255 }, 0);
    Part(MESH_CUBE, V3(-0.5f, 0.45f, -0.4f), V3(6.0f, 0.04f, 0.04f), (Color){ 200, 60, 60, 255 }, 0.2f);
    for (int i = 0; i < Q.count; i++) {
        Person *pp = &Q.p[i];
        if (pp->bot.pos.x > 5.4f || pp->bot.pos.z > 2.5f) continue;
        BotDraw(&pp->bot);
    }

    for (int i = 0; i < Q.count; i++) {
        Person *pp = &Q.p[i];
        if (pp->bot.pos.x > 5.4f || pp->bot.pos.z > 2.5f) continue;
        bool waitingNow = FindInLine(pp->name) >= 0;
        Color bg = pp->bad ? (Color){ 200, 50, 55, 235 } : (Q.waiting && waitingNow) ? (Q.waitingBad ? (Color){ 150, 60, 40, 235 } : (Color){ 40, 140, 80, 235 }) : LBL_DIM;
        Label3D(V3(pp->bot.pos.x, 1.1f, pp->bot.pos.z), pp->name, 13, WHITE, bg, LABEL_CODE);
    }
    for (int k = 0; k < Q.lineN; k++) Label3D(V3(Slot(k).x, 0.05f, 0.8f), k == 0 ? "front" : TextFormat("%d", k), 10, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (Q.desk[0] || Q.nobody)
        Label3D(V3(d.x, 1.9f, d.z), Q.nobody ? "Nobody waiting" : TextFormat("Serving %s", Q.desk), 15, Q.deskBad ? WHITE : LBL_GOLD,
                Q.deskBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    Label3D(V3(d.x, 0.2f, d.z + 0.75f), "desk", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    if (Q.want[0]) Label3D(V3(-0.5f, 2.5f, 0), TextFormat("should be: %s", Q.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (Q.waiting && !Q.waitingBad && Q.lineN == 0) Label3D(V3(0, 1.0f, 0.2f), "Still waiting: nobody", 14, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_QUEUE = {
    "cpp-lists", "Ticket Queue", "Your list is the line: push_back, push_front, remove - and serve from the front.",
    { 255, 190, 110, 255 },
    ParseQueue, QueueReset, QueueEvent, QueueUpdate, QueueDraw, NULL,
};
