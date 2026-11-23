// Build a List -> "Node Chain"
//
// Every list the program prints becomes a chain of nodes on the heap: a value and a next
// arrow to the following node, from the head pointer to nullptr. Between printed states
// the chain changes the way the program changed it: nodes that stay slide along, a node
// that was deleted unlinks and drops away, and a new one pops in at the head. "Value not
// found" sends a search pointer down the chain to nullptr; "List is empty" shows head
// pointing straight at nullptr. A wrong chain shows in red, with the right one underneath.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { LS_BEGIN = 1, LS_NODE, LS_END, LS_MSG };
enum { PH_LIST, PH_DELETE, PH_INSERT };
enum { MSG_NOTFOUND = 1, MSG_EMPTY };
enum { LSF_LIST = 1, LSF_MSG, LSF_GARBLED, LSF_EXTRA };

#define NMAX 12

// ---------------------------------------------------------------- the parser

// "After deleting 20: 10 30" -> phase, the value (20) and the list
static bool ScanState(const char *s, int len, int *phase, int *x, int *vals, int *n)
{
    char buf[200];
    int m = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1;
    memcpy(buf, s, (size_t)m);
    buf[m] = '\0';
    *x = 0;
    const char *rest;
    if (strncmp(buf, "List:", 5) == 0) { *phase = PH_LIST; rest = buf + 5; }
    else {
        int used = 0;
        if (sscanf(buf, "After deleting %d:%n", x, &used) == 1 && used > 0) *phase = PH_DELETE;
        else if (sscanf(buf, "After inserting %d:%n", x, &used) == 1 && used > 0) *phase = PH_INSERT;
        else return false;
        rest = buf + used;
    }
    return ScanIntList(rest, (int)strlen(rest), vals, 64, n);
}

static void EmitState(Parse *p, int phase, int x, const int *vals, int n)
{
    EvAct(p, LS_BEGIN, phase, n, x);
    for (int k = 0; k < n && k < NMAX; k++) EvAct(p, LS_NODE, k, 0, vals[k]);
    EvAct(p, LS_END, phase, n, x);
}

static void ParseList(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    EvSpawn(&p, 0, 0, 0, 0);
    if (ParsePreview(&p)) return;

    char note[140];
    int prev[64], pn = 0;
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int msg = LineIs(E, el, "Value not found in the list.") ? MSG_NOTFOUND : LineIs(E, el, "List is empty.") ? MSG_EMPTY : 0;
        if (msg) {
            if (ParseTake(&p, e)) { EvAct(&p, LS_MSG, msg, 0, 0); continue; }
            snprintf(note, sizeof(note), msg == MSG_NOTFOUND ? "The value isn't in the list: DeleteWithValue should say so"
                                                             : "The list is empty: DeleteWithValue should say so");
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, LSF_MSG, e, msg, 0, 0, note);
            break;
        }
        int phase, x, want[64], wn = 0, gphase, gx, got[64], gn = 0;
        if (!ScanState(E, el, &phase, &x, want, &wn)) { ParseTake(&p, e); continue; }
        if (wn > 64) wn = 64;
        if (ParseTake(&p, e)) {
            EmitState(&p, phase, x, want, wn);
            memcpy(prev, want, sizeof(int) * (size_t)wn);
            pn = wn;
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, LSF_LIST, e, phase, 0, x, NULL); break; }
        if (!ScanState(A, al, &gphase, &gx, got, &gn) || gphase != phase) {
            if (LineIs(A, al, "Value not found in the list.") || LineIs(A, al, "List is empty."))
                ParseFail(&p, FAIL_WRONG, LSF_MSG, e, 0, 0, 0, "That message only belongs when the value isn't there");
            else
                ParseFail(&p, FAIL_GARBLED, LSF_GARBLED, e, phase, 0, x, "The chain can't be read from this line");
            break;
        }
        if (gn > 64) gn = 64;
        ParseShow(&p);
        EmitState(&p, phase, gx, got, gn);
        bool same = gn == pn;
        for (int k = 0; same && k < gn; k++) same = got[k] == prev[k];
        if (phase == PH_DELETE && same && gn > 0) snprintf(note, sizeof(note), "Nothing was unlinked: DeleteWithValue must remove the node with %d", x);
        else if (phase == PH_INSERT && gn == wn && gn > 0 && got[gn - 1] == x && want[0] == x) snprintf(note, sizeof(note), "%d went on the end - InsertAtBeginning puts it first", x);
        else if (gn == wn) {
            int k = 0;
            while (k < gn && got[k] == want[k]) k++;
            if (k < gn) snprintf(note, sizeof(note), "Node %d should hold %d", k + 1, want[k]);
            else snprintf(note, sizeof(note), "Right values, wrong spacing: one space between them");
        } else snprintf(note, sizeof(note), "The chain should have %d node%s, not %d", wn, wn == 1 ? "" : "s", gn);
        GameEvent *f = ParseFail(&p, FAIL_WRONG, LSF_LIST, e, phase, gn, x, note);
        if (f) {
            int o = 0;
            for (int k = 0; k < wn && k < 8 && o < EV_TEXT - 8; k++) o += snprintf(f->text + o, (size_t)(EV_TEXT - o), k ? " %d" : "%d", want[k]);
        }
        break;
    }
    ParseEnd(&p, LSF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_SEARCH, DO_CHEER, DO_BAD };

typedef struct { int value; Vector3 pos, to; bool alive; float drop; float pop; bool bad; } Node;

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time;
    Node  nodes[2 * NMAX];
    int   order[NMAX];               // the chain: node slots in order
    int   len;
    int   pending[NMAX], pendingN, phase, x;
    bool  bad, empty, notFound, searching;
    float searchT;
    char  want[EV_TEXT], caption[48];
    bool  any;
} L;

static float Gap(int n) { float g = 7.2f / (n + 1.5f); return g > 1.35f ? 1.35f : g; }
static Vector3 Slot(int k, int n) { return V3(-3.0f + k * Gap(n), 0, 0.2f); }
static Vector3 NullPos(void) { return L.len > 0 ? V3(Slot(L.len, L.len).x, 0, 0.2f) : V3(-1.6f, 0, 0.2f); }
static Vector3 HeadPos(void) { return V3(-4.6f, 1.3f, 0.2f); }

static void ListReset(const EventList *ev)
{
    memset(&L, 0, sizeof(L));
    L.ev = ev;
    ShotOrbit(V3(0, 0.8f, 0.2f), 9.4f, 30, 0, 37, 0);
}

// Moves the chain to its new state: matched nodes slide, new ones pop in, the rest drop
static void Reconcile(void)
{
    bool used[2 * NMAX] = { false };
    int next[NMAX], cursor = 0, placed = 0;
    for (int k = 0; k < L.pendingN; k++) {
        int v = L.pending[k], found = -1;
        for (int j = cursor; j < L.len && found < 0; j++) {
            int s = L.order[j];
            if (!used[s] && L.nodes[s].value == v) { found = s; cursor = j + 1; }
        }
        if (found < 0) {                          // a new node
            for (int s = 0; s < 2 * NMAX; s++)
                if (!L.nodes[s].alive && L.nodes[s].drop <= 0) {
                    found = s;
                    L.nodes[s] = (Node){ v, V3(Slot(k, L.pendingN).x, 1.6f, 0.2f), Slot(k, L.pendingN), true, 0, 0, false };
                    break;
                }
            if (found < 0) continue;
        }
        used[found] = true;
        next[placed++] = found;
    }
    for (int j = 0; j < L.len; j++) {             // nodes no longer in the chain drop away
        int s = L.order[j];
        if (!used[s] && L.nodes[s].alive) { L.nodes[s].alive = false; L.nodes[s].drop = 1.0f; FxDust(L.nodes[s].pos, 8); }
    }
    L.len = placed;
    for (int k = 0; k < L.len; k++) {
        L.order[k] = next[k];
        L.nodes[next[k]].to = Slot(k, L.len);
        L.nodes[next[k]].bad = false;
    }
    L.empty = (L.len == 0);
}

static float ListEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN: return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == LS_BEGIN) { L.phase = e->a; L.x = (int)e->value; L.pendingN = 0; return 0; }
            if (e->code == LS_NODE) { if (L.pendingN < NMAX) L.pending[L.pendingN++] = (int)e->value; return 0; }
            if (e->code == LS_END) {
                L.any = true;
                L.notFound = false;
                Reconcile();
                if (L.phase == PH_LIST) snprintf(L.caption, sizeof(L.caption), "InsertAtEnd x %d", L.len);
                else if (L.phase == PH_DELETE) snprintf(L.caption, sizeof(L.caption), "DeleteWithValue(%d)", L.x);
                else snprintf(L.caption, sizeof(L.caption), "InsertAtBeginning(%d)", L.x);
                return 1.5f;
            }
            if (e->code == LS_MSG) {
                if (e->a == MSG_NOTFOUND) { L.notFound = true; L.searching = true; L.searchT = 0; }
                else L.empty = true;
                return 1.8f;
            }
            return 0;
        case EV_FAIL:
            L.bad = true;
            StrCopy(L.want, sizeof(L.want), e->text);
            for (int k = 0; k < L.len; k++) L.nodes[L.order[k]].bad = true;
            CueAt(&L.cues, L.time + 0.6f, DO_BAD, 0, 0, V3(0, 0.8f, 0.4f), 0);
            return 2.8f;
        case EV_CRASH:
            CueAt(&L.cues, L.time + 0.1f, DO_BAD, 0, 0, V3(0, 0.8f, 0.4f), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&L.cues, L.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void ListUpdate(float dt)
{
    L.time += dt;
    PopsUpdate(&L.pops, dt);
    if (L.searching) L.searchT += dt;
    for (int s = 0; s < 2 * NMAX; s++) {
        Node *n = &L.nodes[s];
        if (n->alive) { n->pos = LerpV3(n->pos, n->to, 1 - expf(-6 * dt)); n->pop = fminf(1, n->pop + dt * 3); }
        else if (n->drop > 0) { n->drop -= dt; n->pos.y -= dt * 2.5f * (1.2f - n->drop); n->pos.z += dt * 1.2f; }
    }
    Cue c;
    while (CueDue(&L.cues, L.time, &c)) {
        if (c.what == DO_BAD) { FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else if (c.what == DO_CHEER) { FxConfetti(V3(0, 1.6f, 0.3f), PlaybackFinale() ? 150 : 70, 2.5f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void DrawNode(const Node *n)
{
    float k = EaseOutBack(n->pop);
    Vector3 p = n->pos;
    Color body = n->bad ? (Color){ 220, 80, 80, 255 } : (Color){ 120, 170, 240, 255 };
    Part(MESH_CUBE, V3(p.x - 0.12f, p.y + 0.3f * k, p.z), V3(0.5f * k, 0.6f * k, 0.6f * k), body, n->bad ? 0.4f : 0.15f);
    Part(MESH_CUBE, V3(p.x + 0.26f, p.y + 0.3f * k, p.z), V3(0.24f * k, 0.6f * k, 0.6f * k), (Color){ 70, 80, 110, 255 }, 0);   // the next field
}

static void ListDraw(void)
{
    KitStage(V3(-0.4f, 0, 0.3f), 5.2f, 2.0f);
    KitLot(V3(0, 0, 0.2f), 8.6f, 2.4f, (Color){ 120, 255, 200, 255 }, 0.3f);
    Vector3 head = HeadPos();
    PartRot(MESH_CYLINDER, head, V3(0.45f, 0.1f, 0.45f), V3(90, 0, 0), (Color){ 200, 170, 255, 255 }, 0.6f);
    for (int s = 0; s < 2 * NMAX; s++) if (L.nodes[s].alive || L.nodes[s].drop > 0) DrawNode(&L.nodes[s]);
    // the arrows: head to the first node, each next to the following node, the last to nullptr
    Vector3 nul = NullPos();
    Part(MESH_CUBE, V3(nul.x, 0.2f, nul.z), V3(0.5f, 0.4f, 0.5f), (Color){ 60, 64, 84, 255 }, 0);
    Color arrow = L.bad ? (Color){ 255, 110, 100, 255 } : (Color){ 200, 170, 255, 255 };
    Vector3 first = L.len > 0 ? L.nodes[L.order[0]].pos : nul;
    KitArc(V3(head.x + 0.2f, head.y - 0.1f, head.z), V3(first.x - 0.12f, 0.7f, first.z), 0.4f, 1, arrow, L.empty ? 0.9f : 0.5f);
    for (int k = 0; k < L.len; k++) {
        Vector3 a = L.nodes[L.order[k]].pos;
        Vector3 b = (k + 1 < L.len) ? L.nodes[L.order[k + 1]].pos : nul;
        KitArc(V3(a.x + 0.26f, 0.62f, a.z), V3(b.x - 0.15f, 0.65f, b.z), 0.35f, 1, arrow, 0.45f);
    }
    // the search pointer walking to nullptr
    if (L.searching && L.notFound) {
        float t = Clamp01(L.searchT / 1.4f);
        float x = Lerpf(Slot(0, L.len).x, nul.x, t);
        Part(MESH_CONE, V3(x, 1.3f, 0.2f), V3(0.24f, 0.3f, 0.24f), (Color){ 255, 200, 90, 255 }, 0.8f);
    }

    Label3D(V3(head.x, head.y + 0.45f, head.z), "head", 14, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(nul.x, 0.55f, nul.z + 0.3f), "nullptr", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    for (int s = 0; s < 2 * NMAX; s++) {
        const Node *n = &L.nodes[s];
        if (!n->alive && n->drop <= 0) continue;
        Label3D(V3(n->pos.x - 0.12f, n->pos.y + 0.32f, n->pos.z + 0.35f), TextFormat("%d", n->value), 15, n->bad ? WHITE : LBL_GOLD,
                n->bad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    }
    if (L.caption[0]) Label3D(V3(0, 2.1f, 0.2f), L.caption, 15, (Color){ 140, 255, 210, 255 }, LBL_DIM, LABEL_CODE);
    if (L.notFound) Label3D(V3(nul.x, 1.55f, 0.2f), TextFormat("%d? not found", L.x), 13, LBL_GOLD, LBL_DIM, LABEL_CODE);
    if (L.empty && L.any) Label3D(V3(nul.x, 1.15f, 0.2f), "List is empty.", 13, LBL_GOLD, LBL_DIM, LABEL_CODE);
    if (L.bad && L.want[0]) Label3D(V3(0, 1.65f, 0.6f), TextFormat("should be: %s", L.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    PopsDraw(&L.pops);
}

const LevelDef LEVEL_LIST = {
    "cpp-linked-list", "Node Chain", "Each list your program prints becomes a chain of nodes, head to nullptr.",
    { 200, 170, 255, 255 },
    ParseList, ListReset, ListEvent, ListUpdate, ListDraw, NULL,
};
