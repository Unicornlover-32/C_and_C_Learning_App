// Function Pointer Calculator -> "Operator Switchboard"
//
// Four machines stand in a row: add, subtract, multiply and divide. The function pointer
// operation is an arm on a pivot: for each sum in the input it swings to the machine the
// program chose, the two numbers fly in, and the printed result comes out. Dividing by
// zero throws up a shield (the program must check before calling divide); an unknown
// operator swings the arm to nothing at all. A dial counts the calls, and "Calls: N"
// is checked against it. A wrong result cracks red.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { SB_SUM = 1, SB_RESULT, SB_DIVZERO, SB_UNKNOWN, SB_CALLS };
enum { SBF_LINE = 1, SBF_CALLS, SBF_GARBLED, SBF_EXTRA };

#define OPS 10

static int OpIndex(char op) { return op == '+' ? 0 : op == '-' ? 1 : op == '*' ? 2 : op == '/' ? 3 : -1; }

// ---------------------------------------------------------------- the parser

static void ParseFuncptr(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int n = 0;
    const char *s = run->input ? run->input : "";
    while (*s && n < OPS) {
        int a, b, used = 0;
        char op;
        if (sscanf(s, " %d %c %d%n", &a, &op, &b, &used) != 3 || used <= 0) break;
        GameEvent *g = EvSpawn(&p, SB_SUM, a, b, (unsigned char)op);
        if (g) snprintf(g->text, EV_TEXT, "%d %c %d", a, op, b);
        n++;
        s += used;
    }
    if (ParsePreview(&p)) return;

    char note[140];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ScanAfter(E, el, "Calls:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, SB_CALLS, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, SBF_CALLS, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Calls:", &got)) {
                if (got > want) snprintf(note, sizeof(note), "Only real calls count: don't call divide when dividing by 0");
                else snprintf(note, sizeof(note), "Each of the four functions must add 1 to calls (should be %d)", want);
                ParseFail(&p, FAIL_WRONG, SBF_CALLS, e, 0, 0, got, note);
            } else {
                ParseFail(&p, FAIL_GARBLED, SBF_GARBLED, e, 0, 0, 0, "Expected Calls: and a number");
            }
            break;
        }
        int k = e < n ? e : n - 1;
        int code = LineIs(E, el, "Error: divide by zero") ? SB_DIVZERO : (el > 18 && memcmp(E, "Unknown operator: ", 18) == 0) ? SB_UNKNOWN : SB_RESULT;
        const char *eq = NULL;
        for (int i = 0; i + 2 < el && !eq; i++) if (E[i] == ' ' && E[i + 1] == '=' && E[i + 2] == ' ') eq = E + i + 3;
        if (ParseTake(&p, e)) {
            GameEvent *g = EvAct(&p, code, k, 0, eq ? strtod(eq, NULL) : 0);
            if (g && eq) { int len = el - (int)(eq - E); if (len > EV_TEXT - 1) len = EV_TEXT - 1; memcpy(g->text, eq, (size_t)len); g->text[len] = '\0'; }
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, SBF_LINE, e, k, code, 0, NULL); break; }
        if (code == SB_DIVZERO) snprintf(note, sizeof(note), "Dividing by 0: check b first, print Error: divide by zero and don't call divide");
        else if (code == SB_UNKNOWN) snprintf(note, sizeof(note), "Any other operator: print %.*s", el, E);
        else snprintf(note, sizeof(note), "%.*s - is operation pointing at the right function?", el, E);
        GameEvent *f = ParseFail(&p, FAIL_WRONG, SBF_LINE, e, k, code, 0, note);
        if (f) { const char *ae = NULL; for (int i = 0; i + 2 < al && !ae; i++) if (A[i] == ' ' && A[i + 1] == '=' && A[i + 2] == ' ') ae = A + i + 3;
                 int len = ae ? al - (int)(ae - A) : al; if (len > EV_TEXT - 1) len = EV_TEXT - 1; memcpy(f->text, ae ? ae : A, (size_t)len); f->text[len] = '\0'; }
        break;
    }
    ParseEnd(&p, SBF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_SHOOT, DO_OUT, DO_SHIELD, DO_DROP, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time;
    int   n, a[OPS], b[OPS];
    char  op[OPS];
    char  sums[OPS][EV_TEXT];
    int   cur;                    // the sum being worked out
    float angle, angleTo;         // the pointer arm (degrees, 0 = straight back)
    int   target;                 // the machine it points at (-1 none, 4 = nothing)
    Vector3 ca, cb, caTo, cbTo;   // the two number crates
    bool  crates;
    char  result[OPS][EV_TEXT];
    int   resultState[OPS];       // 0 none, 1 right, 2 wrong
    int   resultMachine[OPS];
    float shield;
    int   calls;                  // calls the scene has seen
    bool  callsShown, callsBad;
    int   callsPrinted;
    char  want[96];
    float busy[4];
} F;

static float MachineX(int i) { return -3.0f + i * 1.75f; }
static Vector3 Pivot(void) { return V3(0, 0.9f, 1.4f); }

static void FuncptrReset(const EventList *ev)
{
    memset(&F, 0, sizeof(F));
    F.ev = ev;
    F.target = -1;
    F.cur = -1;
    for (int i = 0; i < OPS; i++) F.resultMachine[i] = -1;
    ShotOrbit(V3(0.2f, 1.0f, 0.1f), 9.8f, 27, 0, 38, 0);
}

static void Aim(int machine)
{
    F.target = machine;
    Vector3 p = Pivot();
    float tx = machine >= 0 && machine < 4 ? MachineX(machine) : 4.4f, tz = -1.1f;
    F.angleTo = atan2f(tx - p.x, -(tz - p.z)) * RAD2DEG;
}

static float FuncptrEvent(const GameEvent *e)
{
    int k = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (F.n < OPS) { F.a[F.n] = e->a; F.b[F.n] = e->b; F.op[F.n] = (char)e->value; StrCopy(F.sums[F.n], EV_TEXT, e->text); F.n++; }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
        case EV_FAIL: {
            if (e->code == SB_CALLS || (e->type == EV_FAIL && e->code == SBF_CALLS)) {
                F.callsShown = true;
                F.callsPrinted = (int)e->value;
                F.callsBad = e->type == EV_FAIL;
                if (F.callsBad) CueAt(&F.cues, F.time + 0.4f, DO_BAD, 0, 0, V3(4.4f, 1.6f, 0.2f), 0);
                return F.callsBad ? 2.6f : 1.2f;
            }
            if (e->type == EV_FAIL && e->code != SBF_LINE) {
                CueAt(&F.cues, F.time + 0.3f, DO_BAD, 0, 0, Pivot(), 0);
                return 2.4f;
            }
            if (k < 0 || k >= F.n) return 0.2f;
            int kind = e->type == EV_FAIL ? e->b : e->code;
            bool wrong = e->type == EV_FAIL;
            F.cur = k;
            int m = OpIndex(F.op[k]);
            F.crates = true;
            F.ca = V3(-0.5f, 0.3f, 2.2f);
            F.cb = V3(0.5f, 0.3f, 2.2f);
            F.caTo = V3(-0.35f, 0.9f, 1.9f);
            F.cbTo = V3(0.35f, 0.9f, 1.9f);
            StrCopy(F.result[k], EV_TEXT, e->text);
            if (kind == SB_UNKNOWN || m < 0) {
                Aim(4);
                CueAt(&F.cues, F.time + 0.8f, DO_DROP, k, 0, V3(0, 0, 0), 0);
            } else if (kind == SB_DIVZERO) {
                Aim(3);
                CueAt(&F.cues, F.time + 0.8f, DO_SHIELD, k, 0, V3(0, 0, 0), 0);
            } else {
                Aim(m);
                CueAt(&F.cues, F.time + 0.7f, DO_SHOOT, k, m, V3(0, 0, 0), 0);
                CueAt(&F.cues, F.time + 1.4f, DO_OUT, k, m, V3(wrong ? 1 : 0, 0, 0), 0);
            }
            if (wrong) {
                if (F.ev) StrCopy(F.want, sizeof(F.want), F.ev->expected);
                CueAt(&F.cues, F.time + 1.6f, DO_BAD, k, 0, V3(m >= 0 && m < 4 ? MachineX(m) : 4.4f, 1.6f, -0.2f), 0);
                if (kind != SB_RESULT) { F.resultState[k] = 2; F.resultMachine[k] = m >= 0 ? m : 3; StrCopy(F.result[k], EV_TEXT, F.ev ? F.ev->actual : "?"); }
                return 3.2f;
            }
            return 2.1f;
        }
        case EV_CRASH:
            CueAt(&F.cues, F.time + 0.1f, DO_BAD, 0, 0, Pivot(), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&F.cues, F.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void FuncptrUpdate(float dt)
{
    F.time += dt;
    PopsUpdate(&F.pops, dt);
    F.angle = Smooth(F.angle, F.angleTo, 6, dt);
    F.ca = LerpV3(F.ca, F.caTo, 1 - expf(-7 * dt));
    F.cb = LerpV3(F.cb, F.cbTo, 1 - expf(-7 * dt));
    F.shield = Smooth(F.shield, 0, 1.2f, dt);
    for (int i = 0; i < 4; i++) F.busy[i] = fmaxf(0, F.busy[i] - dt);
    Cue c;
    while (CueDue(&F.cues, F.time, &c)) {
        switch (c.what) {
            case DO_SHOOT:
                F.caTo = V3(MachineX(c.b) - 0.2f, 1.1f, -1.1f);
                F.cbTo = V3(MachineX(c.b) + 0.2f, 1.1f, -1.1f);
                F.busy[c.b] = 0.9f;
                F.calls++;
                PopAdd(&F.pops, "calls++", V3(4.4f, 2.6f, 0.2f), (Color){ 160, 220, 255, 255 });
                break;
            case DO_OUT:
                F.crates = false;
                F.resultState[c.a] = c.v.x > 0 ? 2 : 1;
                F.resultMachine[c.a] = c.b;
                FxSparkle(V3(MachineX(c.b), 1.0f, -0.4f), 12, (Color){ 140, 255, 180, 255 });
                break;
            case DO_SHIELD:
                F.shield = 1;
                F.caTo = V3(MachineX(3) - 0.2f, 0.2f, 0.2f);
                F.cbTo = V3(MachineX(3) + 0.2f, 0.2f, 0.2f);
                F.resultState[c.a] = F.resultState[c.a] ? F.resultState[c.a] : 1;
                F.resultMachine[c.a] = 3;
                StrCopy(F.result[c.a], EV_TEXT, F.resultState[c.a] == 2 ? F.result[c.a] : "Error: divide by zero");
                FxSparks(V3(MachineX(3), 1.0f, -0.2f), 16, (Color){ 255, 140, 90, 255 });
                break;
            case DO_DROP:
                F.caTo = V3(3.8f, -0.6f, 0.4f);
                F.cbTo = V3(4.2f, -0.6f, 0.4f);
                F.resultState[c.a] = F.resultState[c.a] ? F.resultState[c.a] : 1;
                F.resultMachine[c.a] = 4;
                if (F.resultState[c.a] != 2) snprintf(F.result[c.a], EV_TEXT, "Unknown operator: %c", F.op[c.a]);
                break;
            case DO_BAD:
                FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 });
                PlaybackShake(0.2f);
                break;
            case DO_CHEER:
                FxConfetti(V3(0, 2.0f, 0), PlaybackFinale() ? 150 : 70, 3.0f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

static void FuncptrDraw(void)
{
    KitStage(V3(0.4f, 0, 0.3f), 5.2f, 2.2f);
    static const Color cols[4] = { { 80, 170, 120, 255 }, { 200, 110, 80, 255 }, { 120, 120, 220, 255 }, { 200, 160, 70, 255 } };
    for (int i = 0; i < 4; i++) {
        bool on = F.target == i;
        KitMachine(V3(MachineX(i), 0, -1.2f), V3(1.25f, 1.5f, 1.0f), cols[i], on ? (Color){ 255, 230, 140, 255 } : (Color){ 120, 140, 180, 255 },
                   on ? 0.8f : 0.2f, F.busy[i] > 0 ? F.time : 0);
    }
    // the divide machine's shield, and the empty spot an unknown operator points at
    if (F.shield > 0.02f) PartGlass(MESH_CUBE, V3(MachineX(3), 0.9f, -0.55f), V3(1.4f, 1.6f * F.shield, 0.05f), (Color){ 255, 90, 90, (unsigned char)(120 * F.shield) });
    Part(MESH_CYLINDER, V3(4.4f, 0.02f, -1.1f), V3(0.9f, 0.04f, 0.9f), (Color){ 50, 54, 70, 255 }, F.target == 4 ? 0.5f : 0);
    // the pointer: a pivot and an arm
    Vector3 p = Pivot();
    Part(MESH_CYLINDER, V3(p.x, p.y / 2, p.z), V3(0.3f, p.y, 0.3f), (Color){ 78, 86, 112, 255 }, 0);
    PartRot(MESH_CUBE, V3(p.x + sinf(F.angle * DEG2RAD) * 0.6f, p.y + 0.1f, p.z - cosf(F.angle * DEG2RAD) * 0.6f), V3(0.12f, 0.12f, 1.2f),
            V3(0, -F.angle, 0), (Color){ 200, 170, 255, 255 }, 0.4f);
    if (F.target >= 0) {
        float tx = F.target < 4 ? MachineX(F.target) : 4.4f;
        KitArc(V3(p.x + sinf(F.angle * DEG2RAD) * 1.2f, p.y + 0.1f, p.z - cosf(F.angle * DEG2RAD) * 1.2f), V3(tx, F.target < 4 ? 1.6f : 0.15f, -0.9f), 0.5f, 1,
               F.target == 4 ? (Color){ 255, 120, 100, 255 } : (Color){ 200, 170, 255, 255 }, 0.6f);
    }
    if (F.crates) {
        KitCrate(F.ca, 0.36f, (Color){ 120, 180, 255, 255 }, 0.2f, 0);
        KitCrate(F.cb, 0.36f, (Color){ 120, 180, 255, 255 }, 0.2f, 0);
    }
    // the calls counter
    KitGauge(V3(4.4f, 1.6f, 0.2f), 0.45f, (F.callsShown ? F.callsPrinted : F.calls) / 6.0f, F.callsBad ? (Color){ 255, 90, 90, 255 } : (Color){ 160, 220, 255, 255 },
             F.callsShown ? 0.9f : 0.4f);
    Part(MESH_CYLINDER, V3(4.4f, 0.55f, 0.15f), V3(0.08f, 1.1f, 0.08f), (Color){ 78, 86, 112, 255 }, 0);

    static const char *const names[4] = { "add", "subtract", "multiply", "divide" }, *const signs[4] = { "+", "-", "*", "/" };
    for (int i = 0; i < 4; i++) {
        Label3D(V3(MachineX(i), 2.25f, -1.2f), names[i], 13, LBL_SOFT, LBL_DIM, LABEL_CODE);
        Label3D(V3(MachineX(i), 0.95f, -0.6f), signs[i], 22, WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE | LABEL_BIG);
    }
    Label3D(V3(4.4f, 0.4f, -1.0f), "NULL", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(p.x - 1.0f, p.y + 0.2f, p.z), "operation", 13, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
    if (F.cur >= 0 && F.cur < F.n) Label3D(V3(0, 0.25f, 2.3f), F.sums[F.cur], 16, LBL_GOLD, LBL_DIM, LABEL_CODE);
    if (F.crates && F.cur >= 0) {
        Label3D(V3(F.ca.x, F.ca.y + 0.5f, F.ca.z), TextFormat("%d", F.a[F.cur]), 13, WHITE, LBL_DIM, LABEL_CODE);
        Label3D(V3(F.cb.x, F.cb.y + 0.5f, F.cb.z), TextFormat("%d", F.b[F.cur]), 13, WHITE, LBL_DIM, LABEL_CODE);
    }
    for (int k = 0; k < F.n; k++) {          // the latest answer from each machine
        if (!F.resultState[k] || F.resultMachine[k] < 0) continue;
        bool latest = true;
        for (int j = k + 1; j < F.n; j++) if (F.resultState[j] && F.resultMachine[j] == F.resultMachine[k]) latest = false;
        if (!latest) continue;
        float x = F.resultMachine[k] < 4 ? MachineX(F.resultMachine[k]) : 4.4f;
        bool bad = F.resultState[k] == 2;
        Label3D(V3(x, 2.7f, -1.2f), F.resultMachine[k] >= 3 && !bad && (F.result[k][0] == 'E' || F.result[k][0] == 'U') ? F.result[k] : TextFormat("= %s", F.result[k]),
                14, bad ? WHITE : LBL_GOOD, bad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(4.4f, 2.25f, 0.2f), F.callsShown ? TextFormat("Calls: %d", F.callsPrinted) : TextFormat("calls = %d", F.calls), 14,
            F.callsBad ? WHITE : LBL_GOLD, F.callsBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (F.want[0]) Label3D(V3(0, 3.2f, -1.2f), TextFormat("should be: %s", F.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    else if (F.callsBad) Label3D(V3(4.4f, 2.7f, 0.2f), TextFormat("the program made %d", F.calls), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    PopsDraw(&F.pops);
}

const LevelDef LEVEL_FUNCPTR = {
    "c-scope-funcptr", "Operator Switchboard", "The function pointer swings to the machine your program picks for each sum.",
    { 200, 170, 255, 255 },
    ParseFuncptr, FuncptrReset, FuncptrEvent, FuncptrUpdate, FuncptrDraw, NULL,
};
