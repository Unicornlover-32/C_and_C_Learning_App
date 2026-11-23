// Deep Copy -> "Clone Lab"
//
// s1 and s2 are objects on the stack; each has an id and a name pointer. The names live
// out on the heap as letter tiles ending in '\0'. "s1: ..." lights s1 and its buffer. For
// "s2: ..." the copy constructor builds s2 its own buffer - the letters fly across - and
// then s2's first letter changes. If s1 changed too, both pointers swing to one shared
// buffer: the shallow-copy bug, visible.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { CL_INPUT = 1, CL_SHOW };
enum { CLF_SHALLOW = 1, CLF_ID, CLF_NAME, CLF_GARBLED, CLF_EXTRA };

#define NMAX 16

// "s1: 98 foo" -> id 98, name "foo"
static bool ScanStudent(const char *s, int len, const char *prefix, int *id, char *name, int cap)
{
    char rest[96];
    if (!ScanAfterText(s, len, prefix, rest, sizeof(rest))) return false;
    int used = 0;
    if (sscanf(rest, "%d %n", id, &used) != 1 || used <= 0) return false;
    StrCopy(name, cap, rest + used);
    return name[0] != '\0' && strchr(name, ' ') == NULL;
}

// ---------------------------------------------------------------- the parser

static void ParseDeep(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int id = 0;
    char name[64] = "", letter[8] = "";
    if (run->input) sscanf(run->input, "%d %63s %7s", &id, name, letter);
    GameEvent *g = EvSpawn(&p, CL_INPUT, id, (unsigned char)letter[0], 0);
    if (g) StrCopy(g->text, EV_TEXT, name);
    if (ParsePreview(&p)) return;

    char note[140], s2name[64];
    StrCopy(s2name, sizeof(s2name), name);
    if (s2name[0] && letter[0]) s2name[0] = letter[0];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, wid, gid;
        char wname[64], gname[64];
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int who = (el >= 3 && E[1] == '2') ? 2 : 1;
        const char *prefix = who == 1 ? "s1:" : "s2:";
        if (!ScanStudent(E, el, prefix, &wid, wname, sizeof(wname))) { ParseTake(&p, e); continue; }
        if (ParseTake(&p, e)) { GameEvent *a = EvAct(&p, CL_SHOW, who, 0, wid); if (a) StrCopy(a->text, EV_TEXT, wname); continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, CLF_NAME, e, who, 0, 0, NULL); break; }
        if (!ScanStudent(A, al, prefix, &gid, gname, sizeof(gname))) {
            ParseFail(&p, FAIL_GARBLED, CLF_GARBLED, e, who, 0, 0, "Expected the object's id and name");
            break;
        }
        GameEvent *f;
        if (who == 1 && strcmp(gname, s2name) == 0 && strcmp(name, s2name) != 0) {
            f = ParseFail(&p, FAIL_WRONG, CLF_SHALLOW, e, who, 0, gid,
                          "s1 changed too: both share one name. The copy constructor must new its own char array");
        } else if (gid != wid) {
            snprintf(note, sizeof(note), "The id should be %d", wid);
            f = ParseFail(&p, FAIL_WRONG, CLF_ID, e, who, 0, gid, note);
        } else {
            snprintf(note, sizeof(note), "The name should be %.40s", wname);
            f = ParseFail(&p, FAIL_WRONG, CLF_NAME, e, who, 0, gid, note);
        }
        if (f) StrCopy(f->text, EV_TEXT, gname);
        break;
    }
    ParseEnd(&p, CLF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_FLY, DO_CHANGE, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   id;
    char  name[EV_TEXT];
    char  letter;
    bool  shown[3];                     // s1, s2 printed
    char  buf[3][EV_TEXT];              // the buffers' text (1 and 2)
    int   letters[3];                   // letters placed in each buffer
    bool  bad[3], shallow, crashed;
    int   ids[3];
    float pointTo2;                     // s2's arrow: 0 to buffer 2, 1 to buffer 1 (shared)
    char  want[EV_TEXT];
    bool  changed, garbled;
} C;

static Vector3 ObjPos(int who) { return V3(-3.4f, 0, who == 1 ? -0.7f : 1.0f); }
static Vector3 Tile(int who, int i) { int n = (int)strlen(C.name) + 1; float g = n > 8 ? 4.4f / n : 0.55f; return V3(-0.6f + i * g, 0.25f, who == 1 ? -0.7f : 1.0f); }

static void DeepReset(const EventList *ev)
{
    memset(&C, 0, sizeof(C));
    C.ev = ev;
    ShotOrbit(V3(0, 0.6f, 0.2f), 9.4f, 38, 0, 37, 0);
}

static float DeepEvent(const GameEvent *e)
{
    int who = e->a;
    switch (e->type) {
        case EV_SPAWN:
            C.id = e->a;
            C.letter = (char)e->b;
            StrCopy(C.name, sizeof(C.name), e->text);
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == CL_SHOW && (who == 1 || who == 2)) {
                C.shown[who] = true;
                C.ids[who] = (int)e->value;
                if (who == 1) {
                    StrCopy(C.buf[1], EV_TEXT, e->text);
                    int n = (int)strlen(C.buf[1]);
                    for (int i = 0; i <= n && i < NMAX; i++) CueAt(&C.cues, C.time + 0.2f + i * 0.12f, DO_FLY, 1, i, V3(0, 0, 0), 0);
                    return 0.8f + n * 0.12f;
                }
                // the copy: letters fly from s1's buffer into a new one, then the first changes
                StrCopy(C.buf[2], EV_TEXT, C.buf[1][0] ? C.buf[1] : e->text);
                int n = (int)strlen(C.buf[2]);
                for (int i = 0; i <= n && i < NMAX; i++) CueAt(&C.cues, C.time + 0.3f + i * 0.14f, DO_FLY, 2, i, V3(0, 0, 0), 0);
                CueAt(&C.cues, C.time + 0.6f + n * 0.14f, DO_CHANGE, 2, 0, V3(0, 0, 0), 0);
                StrCopy(C.want, sizeof(C.want), e->text);
                return 1.4f + n * 0.14f;
            }
            return 0;
        case EV_FAIL:
            if (e->code == CLF_SHALLOW) {
                C.shallow = true;
                C.shown[1] = C.shown[2] = true;
                C.ids[1] = C.ids[2] = (int)e->value;
                StrCopy(C.buf[1], EV_TEXT, e->text);
                C.letters[1] = (int)strlen(C.buf[1]) + 1;
                C.bad[1] = true;
                CueAt(&C.cues, C.time + 0.5f, DO_BAD, 1, 0, Tile(1, 0), 0);
                return 3.0f;
            }
            if (e->code == CLF_GARBLED || e->code == CLF_EXTRA) {
                C.garbled = true;
                CueAt(&C.cues, C.time + 0.3f, DO_BAD, 2, 0, Tile(2, 0), 0);
                return 2.6f;
            }
            if (who == 1 || who == 2) {
                C.shown[who] = true;
                C.ids[who] = (int)e->value;
                StrCopy(C.buf[who], EV_TEXT, e->text);
                C.letters[who] = (int)strlen(C.buf[who]) + 1;
                C.bad[who] = true;
                if (C.ev) StrCopy(C.want, sizeof(C.want), C.ev->expected);
                CueAt(&C.cues, C.time + 0.4f, DO_BAD, who, 0, Tile(who, 0), 0);
            }
            return 2.8f;
        case EV_CRASH:
            C.crashed = true;
            CueAt(&C.cues, C.time + 0.2f, DO_BAD, 1, 0, Tile(1, 0), 0);
            return 2.6f;
        case EV_WIN:
            CueAt(&C.cues, C.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void DeepUpdate(float dt)
{
    C.time += dt;
    C.pointTo2 = Smooth(C.pointTo2, C.shallow ? 1.0f : 0.0f, 4, dt);
    Cue c;
    while (CueDue(&C.cues, C.time, &c)) {
        switch (c.what) {
            case DO_FLY:
                C.letters[c.a] = c.b + 1;
                FxSparkle(Tile(c.a, c.b), 4, c.a == 1 ? (Color){ 140, 230, 255, 255 } : (Color){ 140, 255, 190, 255 });
                break;
            case DO_CHANGE:
                C.changed = true;
                if (C.want[0]) StrCopy(C.buf[2], EV_TEXT, C.want);
                FxSparkle(V3(Tile(2, 0).x, 0.7f, Tile(2, 0).z), 14, (Color){ 255, 220, 120, 255 });
                break;
            case DO_BAD:
                FxSparks(V3(c.v.x, 0.8f, c.v.z), 34, (Color){ 255, 100, 90, 255 });
                PlaybackShake(0.22f);
                break;
            case DO_CHEER:
                FxConfetti(V3(-1.0f, 1.8f, 0.2f), PlaybackFinale() ? 150 : 70, 2.0f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

static void Object(int who)
{
    Vector3 o = ObjPos(who);
    bool on = C.shown[who];
    Color body = C.bad[who] || (C.shallow && on) ? (Color){ 200, 80, 80, 255 } : on ? (Color){ 120, 150, 220, 255 } : (Color){ 80, 88, 112, 255 };
    Part(MESH_CUBE, V3(o.x, 0.35f, o.z), V3(1.6f, 0.7f, 1.0f), body, on ? 0.15f : 0);
    Part(MESH_CUBE, V3(o.x - 0.4f, 0.72f, o.z), V3(0.6f, 0.06f, 0.7f), (Color){ 40, 44, 60, 255 }, 0);   // the id field
    Part(MESH_CUBE, V3(o.x + 0.4f, 0.72f, o.z), V3(0.6f, 0.06f, 0.7f), (Color){ 40, 44, 60, 255 }, 0);   // the name pointer
}

static void DeepDraw(void)
{
    KitStage(V3(-0.6f, 0, 0.2f), 4.8f, 2.2f);
    Part(MESH_CUBE, V3(-3.4f, 0.02f, 0.15f), V3(2.4f, 0.04f, 3.6f), (Color){ 40, 46, 64, 255 }, 0);
    KitLot(V3(1.3f, 0, 0.15f), 5.4f, 3.4f, (Color){ 120, 255, 200, 255 }, 0.35f);
    Object(1);
    Object(2);
    // the buffers: letters and the '\0' that ends them
    for (int who = 1; who <= 2; who++) {
        if (who == 2 && C.shallow) continue;
        int n = (int)strlen(C.buf[who]);
        for (int i = 0; i < C.letters[who] && i <= n && i < NMAX; i++) {
            bool nul = (i == n);
            bool changedLetter = (who == 2 && i == 0 && C.changed);
            Color col = nul ? (Color){ 120, 255, 200, 255 } : C.bad[who] || (C.shallow && who == 1) ? (Color){ 220, 80, 80, 255 } :
                        changedLetter ? (Color){ 255, 200, 90, 255 } : who == 1 ? (Color){ 120, 180, 255, 255 } : (Color){ 120, 220, 170, 255 };
            KitCrate(V3(Tile(who, i).x, 0.02f, Tile(who, i).z), nul ? 0.3f : 0.42f, col, nul ? 0.8f : 0.2f, 0);
        }
    }
    // the name pointers
    for (int who = 1; who <= 2; who++) {
        if (!C.shown[who] && !(C.shallow && who == 2)) continue;
        Vector3 from = V3(ObjPos(who).x + 0.4f, 0.8f, ObjPos(who).z);
        Vector3 to2 = V3(Tile(2, 0).x - 0.1f, 0.6f, Tile(2, 0).z), to1 = V3(Tile(1, 0).x - 0.1f, 0.6f, Tile(1, 0).z);
        Vector3 to = who == 1 ? to1 : LerpV3(to2, to1, C.pointTo2);
        Color col = (C.shallow) ? (Color){ 255, 100, 90, 255 } : (Color){ 200, 170, 255, 255 };
        KitArc(from, to, 0.7f, 1, col, 0.6f);
    }

    for (int who = 1; who <= 2; who++) {
        Vector3 o = ObjPos(who);
        Label3D(V3(o.x, 1.35f, o.z), who == 1 ? "s1" : "s2", 16, WHITE, LBL_DIM, LABEL_CODE);
        Label3D(V3(o.x - 0.4f, 0.85f, o.z + 0.25f), C.shown[who] ? TextFormat("%d", C.ids[who]) : "id", 13, C.shown[who] ? LBL_GOLD : LBL_SOFT, LBL_DIM, LABEL_CODE);
        Label3D(V3(o.x + 0.4f, 0.85f, o.z + 0.25f), "name", 12, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        int n = (int)strlen(C.buf[who]);
        if (who == 2 && C.shallow) continue;
        for (int i = 0; i < C.letters[who] && i <= n && i < NMAX; i++) {
            const char *t = (i == n) ? "\\0" : TextFormat("%c", C.buf[who][i]);
            Label3D(V3(Tile(who, i).x, 0.62f, Tile(who, i).z + 0.15f), t, i == n ? 12 : 16, WHITE, LBL_DIM, LABEL_CODE);
        }
    }
    if (C.shown[2] && !C.shallow && !C.bad[2]) Label3D(V3(Tile(2, 0).x + 1.0f, 1.2f, Tile(2, 0).z), "its own copy", 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (C.shallow) {
        Label3D(V3(Tile(1, 0).x + 1.4f, 1.95f, 0.15f), "s1 and s2 share one name!", 15, WHITE, (Color){ 200, 50, 55, 235 }, LABEL_CODE);
        Label3D(V3(Tile(1, 0).x + 1.4f, 1.55f, 0.15f), "should be: s1 keeps its own", 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (C.garbled && C.ev) Label3D(V3(Tile(1, 0).x + 1.0f, 1.5f, 0.15f), TextFormat("can't read: %.30s", C.ev->actual[0] ? C.ev->actual : "(nothing)"), 13, WHITE,
                                   (Color){ 200, 50, 55, 235 }, LABEL_CODE);
    if (C.crashed) Label3D(V3(Tile(1, 0).x + 1.0f, 1.5f, 0.15f), "crashed - the same memory deleted twice?", 13, WHITE, (Color){ 200, 50, 55, 235 }, LABEL_CODE);
    if (!C.shallow && (C.bad[1] || C.bad[2]) && C.want[0])
        Label3D(V3(Tile(C.bad[1] ? 1 : 2, 0).x + 1.0f, 1.25f, Tile(C.bad[1] ? 1 : 2, 0).z), TextFormat("should be: %s", C.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    Label3D(V3(-3.4f, 0.05f, 2.05f), "stack", 13, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    Label3D(V3(1.3f, 0.05f, 2.05f), "heap", 13, (Color){ 140, 255, 210, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
}

const LevelDef LEVEL_DEEPCOPY = {
    "cpp-object-memory", "Clone Lab", "A deep copy gets its own name on the heap; a shallow one shares - and breaks.",
    { 140, 255, 200, 255 },
    ParseDeep, DeepReset, DeepEvent, DeepUpdate, DeepDraw, NULL,
};
