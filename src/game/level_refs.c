// Stats by Reference -> "Name Tags"
//
// main's variables min, max and average are boxes on the left; the numbers are crates on
// a belt. stats() has no boxes of its own: its parameters are references - name tags
// (&smallest, &largest, &average) clipped onto main's boxes, so each value it works out
// lands in main's box. Then swap(int &a, int &b) clips tags a and b onto the first and
// last crates and swaps them for real. A wrong value cracks red; a swap that changed
// nothing (copies instead of references) is named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { NT_VAL = 1, NT_STAT, NT_SWAP, NT_ORDER };
enum { NTF_STAT = 1, NTF_SWAP, NTF_GARBLED, NTF_EXTRA };

#define RMAX 12

static const char *const STAT_KEYS[3] = { "Min:", "Max:", "Average:" };

// ---------------------------------------------------------------- the parser

static void ParseRefs(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[101];
    int n = InputInts(run->input, in, 101), count = n > 0 ? in[0] : 0;
    if (count < 0) count = 0;
    if (count > n - 1) count = n > 0 ? n - 1 : 0;
    for (int i = 0; i < count && i < RMAX; i++) EvSpawn(&p, NT_VAL, i, count, in[1 + i]);
    if (ParsePreview(&p)) return;

    char note[140];
    long sum = 0;
    for (int i = 0; i < count; i++) sum += in[1 + i];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        double want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int k = -1;
        for (int i = 0; i < 3; i++) if (ScanAfterNumber(E, el, STAT_KEYS[i], &want)) k = i;
        if (k >= 0) {
            if (ParseTake(&p, e)) { GameEvent *g = EvAct(&p, NT_STAT, k, 0, want); if (g) ScanAfterText(E, el, STAT_KEYS[k], g->text, EV_TEXT); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, NTF_STAT, e, k, 0, 0, NULL); break; }
            if (ScanAfterNumber(A, al, STAT_KEYS[k], &got)) {
                if (k == 2 && count && fabs(got - (double)(sum / count)) < 0.001 && fabs(got - want) > 0.001)
                    snprintf(note, sizeof(note), "Integer division lost the decimals: divide by (double)size");
                else if (fabs(got - want) < 0.006) snprintf(note, sizeof(note), "Same value, wrong format: two decimal places");
                else snprintf(note, sizeof(note), "%s should be %.*s", k == 0 ? "The smallest" : k == 1 ? "The largest" : "The average", el - (int)strlen(STAT_KEYS[k]) - 1,
                              E + strlen(STAT_KEYS[k]) + 1);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, NTF_STAT, e, k, 0, got, note);
                if (f) ScanAfterText(A, al, STAT_KEYS[k], f->text, EV_TEXT);
            } else {
                ParseFail(&p, FAIL_GARBLED, NTF_GARBLED, e, k, 0, 0, "Expected the label and a number");
            }
            break;
        }
        int wl[101], wn = 0, gl[101], gn = 0;
        if (ScanAfterInts(E, el, "Swapped:", wl, 101, &wn)) {
            if (wn > 101) wn = 101;
            if (ParseTake(&p, e)) {
                EvAct(&p, NT_SWAP, 0, 0, 0);
                for (int i = 0; i < wn && i < RMAX; i++) EvAct(&p, NT_ORDER, i, 0, wl[i]);
                continue;
            }
            if (!A) { ParseFail(&p, FAIL_MISSING, NTF_SWAP, e, 0, 0, 0, NULL); break; }
            if (!ScanAfterInts(A, al, "Swapped:", gl, 101, &gn)) { ParseFail(&p, FAIL_GARBLED, NTF_GARBLED, e, 0, 0, 0, "Expected Swapped: and the numbers"); break; }
            if (gn > 101) gn = 101;
            ParseShow(&p);
            bool same = gn == count;
            for (int i = 0; same && i < gn; i++) same = gl[i] == in[1 + i];
            EvAct(&p, NT_SWAP, same, 0, 0);
            for (int i = 0; i < gn && i < RMAX; i++) EvAct(&p, NT_ORDER, i, 0, gl[i]);
            if (same && count > 1) snprintf(note, sizeof(note), "Nothing swapped: swap must take int& - with plain ints it only swaps copies");
            else snprintf(note, sizeof(note), "Only the first and last numbers trade places");
            ParseFail(&p, FAIL_WRONG, NTF_SWAP, e, 0, 0, 0, note);
            break;
        }
        ParseTake(&p, e);
    }
    ParseEnd(&p, NTF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_BAD, DO_LAND };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   n, count, value[RMAX], order[RMAX];
    int   statState[3];             // 0 empty, 1 set, 2 wrong
    char  statText[3][EV_TEXT];
    int   tagged;                   // the stat whose tag was clipped on last (-1)
    bool  swapping, swapped, noSwap, swapBad;
    float swapT;
    char  want[EV_TEXT];
} N;

static float CrateX(int i) { float g = 5.2f / (N.n + 1); if (g > 0.9f) g = 0.9f; return -0.6f + i * g; }
static Vector3 BoxPos(int k) { return V3(-4.0f + k * 1.15f, 0, -0.8f); }
static Vector3 Machine(void) { return V3(0.9f, 0, -1.3f); }

static void RefsReset(const EventList *ev)
{
    memset(&N, 0, sizeof(N));
    N.ev = ev;
    N.tagged = -1;
    for (int i = 0; i < RMAX; i++) N.order[i] = i;
    ShotOrbit(V3(-0.4f, 0.9f, 0), 9.6f, 27, 0, 38, 0);
}

static float RefsEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (i >= 0 && i < RMAX) { N.value[i] = (int)e->value; if (i + 1 > N.n) N.n = i + 1; N.count = e->b; }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == NT_STAT && i >= 0 && i < 3) {
                N.statState[i] = 1;
                StrCopy(N.statText[i], EV_TEXT, e->text);
                N.tagged = i;
                FxSparkle(V3(BoxPos(i).x, 1.2f, BoxPos(i).z), 12, (Color){ 140, 255, 180, 255 });
                return 1.2f;
            }
            if (e->code == NT_SWAP) {
                N.tagged = -1;
                N.swapping = true;
                N.noSwap = e->a != 0;
                N.swapT = 0;
                if (!N.noSwap) CueAt(&N.cues, N.time + 1.4f, DO_LAND, 0, 0, V3(0, 0, 0), 0);
                return 1.9f;
            }
            if (e->code == NT_ORDER) return 0.05f;
            return 0;
        case EV_FAIL:
            if (e->code == NTF_STAT && i >= 0 && i < 3) {
                N.statState[i] = 2;
                StrCopy(N.statText[i], EV_TEXT, e->text);
                N.tagged = i;
                if (N.ev) { const char *x = N.ev->expected; int kl = (int)strlen(STAT_KEYS[i]); StrCopy(N.want, sizeof(N.want), strncmp(x, STAT_KEYS[i], (size_t)kl) == 0 ? x + kl + 1 : x); }
                CueAt(&N.cues, N.time + 0.4f, DO_BAD, 0, 0, V3(BoxPos(i).x, 1.3f, BoxPos(i).z), 0);
            } else if (e->code == NTF_SWAP) {
                N.swapBad = true;
                if (N.ev) StrCopy(N.want, sizeof(N.want), N.ev->expected);
                CueAt(&N.cues, N.time + 0.3f, DO_BAD, 0, 0, V3(CrateX(0), 1.2f, 1.0f), 0);
            } else {
                CueAt(&N.cues, N.time + 0.3f, DO_BAD, 0, 0, V3(0, 1.2f, 0.4f), 0);
            }
            return 2.8f;
        case EV_CRASH:
            CueAt(&N.cues, N.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.2f, 0.4f), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&N.cues, N.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void RefsUpdate(float dt)
{
    N.time += dt;
    if (N.swapping) N.swapT += dt;
    Cue c;
    while (CueDue(&N.cues, N.time, &c)) {
        if (c.what == DO_LAND) { N.swapped = true; FxSparkle(V3(CrateX(0), 1.0f, 1.0f), 10, (Color){ 255, 220, 120, 255 }); FxSparkle(V3(CrateX(N.n - 1), 1.0f, 1.0f), 10, (Color){ 255, 220, 120, 255 }); }
        else if (c.what == DO_BAD) { FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(-0.5f, 2.0f, 0.4f), PlaybackFinale() ? 150 : 70, 2.5f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

// Where crate i is drawn: the first and last trade places over each other during the swap
static Vector3 CratePos(int i)
{
    Vector3 p = V3(CrateX(i), BELT_Y, 1.0f);
    if (!N.swapping || N.noSwap || N.n < 2 || (i != 0 && i != N.n - 1)) return p;
    float t = Clamp01((N.swapT - 0.5f) / 0.9f), k = EaseInOutQuad(t);
    int other = i == 0 ? N.n - 1 : 0;
    Vector3 q = LerpV3(p, V3(CrateX(other), BELT_Y, 1.0f), k);
    q.y += sinf(k * PI) * (i == 0 ? 1.0f : 0.5f);
    q.z += sinf(k * PI) * (i == 0 ? -0.4f : 0.4f);
    return q;
}

static void RefsDraw(void)
{
    KitStage(V3(-0.6f, 0, 0), 4.9f, 2.0f);
    KitBelt(V3(-1.1f, 0, 1.0f), 5.8f, 0);
    // main's variables
    Part(MESH_CUBE, V3(-2.85f, 0.02f, -0.8f), V3(3.6f, 0.04f, 1.2f), (Color){ 40, 46, 64, 255 }, 0);
    for (int k = 0; k < 3; k++) {
        Color col = N.statState[k] == 2 ? (Color){ 220, 80, 80, 255 } : N.statState[k] ? (Color){ 110, 200, 160, 255 } : (Color){ 90, 98, 124, 255 };
        KitCrate(BoxPos(k), 0.7f, col, N.statState[k] ? 0.25f : 0, 0);
    }
    // stats(): a machine whose parameters are tags clipped onto main's boxes
    KitMachine(Machine(), V3(1.3f, 1.4f, 0.9f), (Color){ 90, 120, 190, 255 }, (Color){ 150, 210, 255, 255 }, 0.6f, N.tagged >= 0 ? N.time : 0);
    for (int k = 0; k < 3; k++) {
        if (!N.statState[k]) continue;
        bool hot = k == N.tagged;
        KitArc(V3(Machine().x - 0.4f, 1.3f, Machine().z + 0.2f), V3(BoxPos(k).x, 0.95f, BoxPos(k).z), 0.6f, 1,
               N.statState[k] == 2 ? (Color){ 255, 110, 100, 255 } : (Color){ 255, 200, 120, 255 }, hot ? 0.8f : 0.3f);
    }
    // the numbers, and swap's two tags
    for (int i = 0; i < N.n; i++) {
        bool end = (i == 0 || i == N.n - 1) && N.swapping;
        Color col = N.swapBad && end ? (Color){ 220, 80, 80, 255 } : end ? (Color){ 255, 200, 120, 255 } : (Color){ 120, 170, 240, 255 };
        KitCrate(CratePos(i), 0.48f, col, end ? 0.3f : 0.1f, 0);
    }
    if (N.swapping && N.n >= 2) {
        Vector3 badge = V3((CrateX(0) + CrateX(N.n - 1)) / 2, 2.6f, 1.2f);
        PartRot(MESH_CYLINDER, badge, V3(0.4f, 0.08f, 0.4f), V3(90, 0, 0), (Color){ 255, 200, 120, 255 }, 0.4f);
        KitArc(V3(badge.x, badge.y - 0.15f, badge.z), V3(CratePos(0).x, CratePos(0).y + 0.6f, 1.0f), 0.2f, 1, (Color){ 255, 200, 120, 255 }, 0.6f);
        KitArc(V3(badge.x, badge.y - 0.15f, badge.z), V3(CratePos(N.n - 1).x, CratePos(N.n - 1).y + 0.6f, 1.0f), 0.2f, 1, (Color){ 255, 200, 120, 255 }, 0.6f);
    }

    static const char *const vars[3] = { "min", "max", "avg" }, *const tags[3] = { "&smallest", "&largest", "&average" };
    for (int k = 0; k < 3; k++) {
        Vector3 b = BoxPos(k);
        Label3D(V3(b.x, 0.2f, b.z + 0.55f), vars[k], 13, LBL_SOFT, LBL_DIM, LABEL_CODE);
        if (N.statState[k]) {
            Label3D(V3(b.x, 1.05f, b.z + 0.4f), N.statText[k], 15, N.statState[k] == 2 ? WHITE : LBL_GOLD, N.statState[k] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
            Label3D(V3(b.x, 1.5f, b.z), tags[k], 11, (Color){ 255, 214, 160, 255 }, LBL_DIM, LABEL_CODE);
        }
    }
    Label3D(V3(Machine().x, 2.0f, Machine().z), "stats(arr, size, ...)", 13, (Color){ 150, 210, 255, 255 }, LBL_DIM, LABEL_CODE);
    for (int i = 0; i < N.n; i++) {
        bool swappedEnd = N.swapped && N.n >= 2 && (i == 0 || i == N.n - 1);
        int shown = swappedEnd ? N.value[i == 0 ? N.n - 1 : 0] : N.value[i];
        Vector3 c = N.swapping && !N.swapped ? CratePos(i) : V3(CrateX(i), BELT_Y, 1.0f);
        if (N.swapping && !N.swapped && (i == 0 || i == N.n - 1)) shown = N.value[i];
        Label3D(V3(c.x, c.y + 0.75f, c.z + 0.1f), TextFormat("%d", shown), 14, LBL_GOLD, LBL_DIM, LABEL_CODE);
    }
    if (N.swapping && N.n >= 2) {
        Label3D(V3((CrateX(0) + CrateX(N.n - 1)) / 2, 3.05f, 1.2f), "swap(int &a, int &b)", 13, (Color){ 255, 214, 160, 255 }, LBL_DIM, LABEL_CODE);
        if (N.noSwap) Label3D(V3((CrateX(0) + CrateX(N.n - 1)) / 2, 1.9f, 1.4f), "nothing moved", 13, WHITE, (Color){ 200, 50, 55, 235 }, LABEL_CODE);
    }
    if (N.want[0]) Label3D(V3(-0.5f, 3.1f, 0), TextFormat("should be: %s", N.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_REFS = {
    "cpp-references", "Name Tags", "A reference is a second name tag on the same box: stats() fills main's boxes.",
    { 255, 200, 120, 255 },
    ParseRefs, RefsReset, RefsEvent, RefsUpdate, RefsDraw, NULL,
};
