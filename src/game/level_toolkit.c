// Array Toolkit -> "Machine Shop"
//
// The input's numbers are blocks on a conveyor, tagged [0] to [n-1]. "Before:" (from
// printArray) lights each block with the printed value. Then the very same blocks ride
// through the doubleAll() machine and come back changed - the array is changed in place,
// because the function gets a pointer to it. "After:" shows their new values, and
// "Average:" feeds them through calculateAverage(), whose dial shows the printed average.
// A block that didn't change, or changed wrongly, glows red; integer-division averages
// are pointed out.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { TK_COUNT = 1, TK_BLOCK, TK_BEFORE, TK_DOUBLE, TK_AFTER, TK_AVG };
enum { TKF_BEFORE = 1, TKF_AFTER, TKF_AVG, TKF_GARBLED, TKF_EXTRA };

#define KMAX 12

// ---------------------------------------------------------------- the parser

static void ParseToolkit(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[1 + 100];
    int n = InputInts(run->input, in, 101);
    int count = n > 0 ? in[0] : 0;
    if (count < 0) count = 0;
    if (count > n - 1) count = n > 0 ? n - 1 : 0;
    EvSpawn(&p, TK_COUNT, count, 0, 0);
    for (int i = 0; i < count && i < KMAX; i++) EvSpawn(&p, TK_BLOCK, i, 0, in[1 + i]);
    if (ParsePreview(&p)) return;

    char note[140];
    int before[100];
    int bn = 0;
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want[100], wn = 0, got[100], gn = 0;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        double wantAvg, gotAvg;
        if (ScanAfterNumber(E, el, "Average:", &wantAvg)) {
            if (ParseTake(&p, e)) { EvAct(&p, TK_AVG, 0, 0, wantAvg); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, TKF_AVG, e, 0, 0, 0, NULL); break; }
            if (ScanAfterNumber(A, al, "Average:", &gotAvg)) {
                long isum = 0;
                for (int i = 0; i < count; i++) isum += 2L * in[1 + i];
                if (count && fabs(gotAvg - (double)(isum / count)) < 0.001 && fabs(gotAvg - wantAvg) > 0.001)
                    snprintf(note, sizeof(note), "Integer division lost the decimals: divide by (double)size");
                else if (fabs(gotAvg - wantAvg) < 0.006)
                    snprintf(note, sizeof(note), "Same value, wrong format: use %%.2f");
                else
                    snprintf(note, sizeof(note), "The average should be %.*s", el - 9, E + 9);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, TKF_AVG, e, 0, 0, gotAvg, note);
                if (f) ScanAfterText(A, al, "Average:", f->text, EV_TEXT);
            } else {
                ParseFail(&p, FAIL_GARBLED, TKF_GARBLED, e, 0, 0, 0, "Expected Average: and a number");
            }
            break;
        }
        bool isBefore = ScanAfterInts(E, el, "Before:", want, 100, &wn);
        const char *prefix = isBefore ? "Before:" : "After:";
        if (!isBefore) ScanAfterInts(E, el, "After:", want, 100, &wn);
        if (wn > 100) wn = 100;
        if (ParseTake(&p, e)) {
            if (!isBefore) EvAct(&p, TK_DOUBLE, 0, 0, 0);
            for (int k = 0; k < wn && k < KMAX; k++) EvAct(&p, isBefore ? TK_BEFORE : TK_AFTER, k, 0, want[k]);
            if (isBefore) { bn = wn; memcpy(before, want, sizeof(int) * (size_t)wn); }
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, isBefore ? TKF_BEFORE : TKF_AFTER, e, 0, 0, 0, NULL); break; }
        if (!ScanAfterInts(A, al, prefix, got, 100, &gn)) {
            ParseFail(&p, FAIL_GARBLED, TKF_GARBLED, e, 0, 0, 0, isBefore ? "Expected Before: and the numbers" : "Expected After: and the numbers");
            break;
        }
        if (gn > 100) gn = 100;
        ParseShow(&p);
        if (!isBefore) EvAct(&p, TK_DOUBLE, 0, 0, 0);
        int k = 0;
        while (k < wn && k < gn && got[k] == want[k]) { if (k < KMAX) EvAct(&p, isBefore ? TK_BEFORE : TK_AFTER, k, 0, got[k]); k++; }
        if (k < wn && k < gn) {
            if (!isBefore && k < bn && got[k] == before[k]) snprintf(note, sizeof(note), "arr[%d] didn't change: doubleAll must change arr[i] itself", k);
            else snprintf(note, sizeof(note), "arr[%d] should be %d", k, want[k]);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, isBefore ? TKF_BEFORE : TKF_AFTER, e, k, 0, got[k], note);
            if (f) snprintf(f->text, EV_TEXT, "%d", want[k]);
        } else if (gn != wn) {
            snprintf(note, sizeof(note), "%d numbers were printed - the array has %d", gn, wn);
            ParseFail(&p, FAIL_WRONG, isBefore ? TKF_BEFORE : TKF_AFTER, e, k, 1, 0, note);
        } else {
            ParseFail(&p, FAIL_WRONG, isBefore ? TKF_BEFORE : TKF_AFTER, e, -1, 2, 0, "Right numbers, wrong spacing: one space between them");
        }
        break;
    }
    ParseEnd(&p, TKF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_SHOW, DO_CHEER, DO_BAD, DO_RETURN };

static struct {
    const EventList *ev;
    Cues  cues;
    float time, scroll;
    int   count, shownCount;
    int   value[KMAX];             // what each block shows
    int   state[KMAX];             // 0 input, 1 printed, 2 wrong
    bool  doubled;                 // has been through doubleAll
    float trip;                    // the blocks' trip through the machine: 0 home .. 1 back
    float tripStart;
    bool  tripping;
    double avg;
    bool  avgShown, avgBad;
    char  avgText[EV_TEXT], want[EV_TEXT];
    int   badAt;
    float maxAbs;
} K;

static float Gap(void) { return K.shownCount > 8 ? 0.55f : 0.7f; }
static Vector3 Home(int i) { return V3(-4.4f + i * Gap(), BELT_Y, 0); }

static void ToolkitReset(const EventList *ev)
{
    memset(&K, 0, sizeof(K));
    K.ev = ev;
    K.badAt = -1;
    K.maxAbs = 1;
    ShotOrbit(V3(0, 1.0f, -0.2f), 10.0f, 25, 0, 36, 0);
}

static float ToolkitEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == TK_COUNT) { K.count = i; K.shownCount = i > KMAX ? KMAX : i; }
            if (e->code == TK_BLOCK && i >= 0 && i < KMAX) {
                K.value[i] = (int)e->value;
                if (fabsf((float)e->value) * 2 > K.maxAbs) K.maxAbs = fabsf((float)e->value) * 2;
            }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if ((e->code == TK_BEFORE || e->code == TK_AFTER) && i >= 0 && i < KMAX) {
                CueAt(&K.cues, K.time + 0.1f, DO_SHOW, i, (int)e->value, V3(0, 0, 0), 0);
                return 0.2f;
            }
            if (e->code == TK_DOUBLE) {           // the whole array goes through the machine first
                K.tripping = true;
                K.tripStart = K.time;
                return 1.95f;
            }
            if (e->code == TK_AVG) {
                K.avg = e->value;
                K.avgShown = true;
                snprintf(K.avgText, sizeof(K.avgText), "%.2f", e->value);
                FxSparkle(V3(3.6f, 1.8f, -0.6f), 14, (Color){ 255, 220, 120, 255 });
                return 1.6f;
            }
            return 0;
        case EV_FAIL:
            if (e->code == TKF_AVG) {
                K.avg = e->value;
                K.avgShown = K.avgBad = true;
                StrCopy(K.avgText, sizeof(K.avgText), e->text);
                CueAt(&K.cues, K.time + 0.6f, DO_BAD, 0, 0, V3(3.6f, 1.8f, -0.6f), 0);
                if (K.ev) { const char *x = K.ev->expected; StrCopy(K.want, sizeof(K.want), strncmp(x, "Average: ", 9) == 0 ? x + 9 : x); }
                return 2.6f;
            }
            if ((e->code == TKF_BEFORE || e->code == TKF_AFTER) && e->b == 0 && i >= 0 && i < KMAX) {
                K.badAt = i;
                StrCopy(K.want, sizeof(K.want), e->text);
                CueAt(&K.cues, K.time + 0.1f, DO_SHOW, i, (int)e->value, V3(1, 0, 0), 0);
                CueAt(&K.cues, K.time + 0.3f, DO_BAD, i, 0, Home(i), 0);
                return 3.0f;
            }
            CueAt(&K.cues, K.time + 0.3f, DO_BAD, -1, 0, V3(0, 1.4f, 0), 0);
            return 2.4f;
        case EV_CRASH:
            CueAt(&K.cues, K.time + 0.1f, DO_BAD, -1, 0, V3(0, 1.4f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&K.cues, K.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void ToolkitUpdate(float dt)
{
    K.time += dt;
    if (K.tripping) {
        K.trip = Clamp01((K.time - K.tripStart) / 1.8f);
        if (K.trip < 1) K.scroll += dt * 2;
        if (K.trip >= 0.5f) K.doubled = true;
    }
    Cue c;
    while (CueDue(&K.cues, K.time, &c)) {
        switch (c.what) {
            case DO_SHOW:
                if (c.a >= 0 && c.a < KMAX) {
                    K.value[c.a] = c.b;
                    K.state[c.a] = c.v.x > 0 ? 2 : 1;
                    FxSparkle(V3(Home(c.a).x, BELT_Y + 0.6f, 0), 4, (Color){ 140, 230, 255, 255 });
                }
                break;
            case DO_BAD:
                FxSparks(V3(c.v.x, c.v.y + 0.6f, c.v.z), 32, (Color){ 255, 100, 90, 255 });
                PlaybackShake(0.2f);
                break;
            case DO_CHEER:
                FxConfetti(V3(0, 2.0f, 0), PlaybackFinale() ? 150 : 70, 2.0f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

// Where block i is on its trip: home, into doubleAll() on the right, round the back, home
static Vector3 BlockAt(int i)
{
    Vector3 h = Home(i);
    if (!K.tripping || K.trip >= 1) return h;
    float t = K.trip;
    if (t < 0.45f) { float k = EaseInOutQuad(t / 0.45f); return V3(h.x + k * 4.6f, h.y, h.z); }   // along the belt into the machine
    if (t < 0.55f) return V3(h.x + 4.6f, h.y, -0.9f);                                               // inside, crossing to the return rail
    float k = EaseInOutQuad((t - 0.55f) / 0.45f);
    return V3(h.x + 4.6f * (1 - k), h.y + sinf(k * PI) * 0.5f, -0.9f * (1 - k));                     // back home, same slot
}

static void ToolkitDraw(void)
{
    KitStage(V3(0, 0, -0.3f), 5.8f, 2.3f);
    KitBelt(V3(-5.0f, 0, 0), 6.6f, K.scroll);
    KitMachine(V3(1.95f, 0, -0.45f), V3(1.4f, 1.8f, 1.9f), (Color){ 70, 120, 200, 255 }, (Color){ 120, 200, 255, 255 }, 0.7f, K.tripping && K.trip < 1 ? K.time : 0);
    // calculateAverage(): a machine with a dial on its face
    float range = K.maxAbs > 1 ? K.maxAbs : 1;
    KitMachine(V3(3.75f, 0, -0.75f), V3(1.3f, 1.2f, 1.1f), (Color){ 160, 110, 60, 255 }, (Color){ 255, 200, 90, 255 }, 0.6f, 0);
    KitGauge(V3(3.75f, 1.85f, -0.6f), 0.5f, K.avgShown ? (float)(K.avg / range) + (K.avgBad ? 0.2f : 0) : 0,
             K.avgBad ? (Color){ 255, 90, 90, 255 } : (Color){ 255, 200, 70, 255 }, K.avgShown ? 0.9f : 0.2f);
    for (int i = 0; i < K.shownCount; i++) {
        Color col = (Color){ 200, 170, 120, 255 };
        float glow = 0;
        if (K.state[i] == 1) { col = K.doubled ? (Color){ 120, 200, 255, 255 } : (Color){ 150, 190, 240, 255 }; glow = 0.25f; }
        if (K.state[i] == 2) { col = (Color){ 230, 80, 80, 255 }; glow = 0.5f; }
        KitCrate(BlockAt(i), 0.46f, col, glow, 0);
    }

    for (int i = 0; i < K.shownCount; i++) {
        Vector3 b = BlockAt(i);
        Color fg = K.state[i] == 2 ? WHITE : K.state[i] ? LBL_GOLD : LBL_SOFT, bg = K.state[i] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        Label3D(V3(b.x, b.y + 0.72f, b.z + 0.1f), TextFormat("%d", K.value[i]), 15, fg, bg, LABEL_CODE);
        if (!K.tripping || K.trip >= 1)
            Label3D(V3(b.x, BELT_Y - 0.15f, 0.55f), TextFormat("[%d]", i), 11, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    if (K.count > KMAX) Label3D(V3(Home(KMAX - 1).x + 0.6f, BELT_Y + 0.5f, 0), TextFormat("... %d in all", K.count), 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(1.95f, 2.35f, -0.45f), "doubleAll(arr, size)", 14, (Color){ 150, 210, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(3.75f, 2.6f, -0.6f), "calculateAverage()", 13, (Color){ 255, 214, 140, 255 }, LBL_DIM, LABEL_CODE);
    if (K.avgShown)
        Label3D(V3(3.75f, 1.2f, -0.1f), TextFormat("%s", K.avgText), 17, K.avgBad ? WHITE : LBL_GOLD,
                K.avgBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (K.want[0] && (K.avgBad || K.badAt >= 0)) {
        Vector3 at = K.avgBad ? V3(3.75f, 0.75f, 0.1f) : V3(Home(K.badAt).x, BELT_Y + 1.15f, 0.1f);
        Label3D(at, TextFormat("should be %s", K.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (K.tripping && K.trip > 0.05f && K.trip < 1)
        Label3D(V3(-1.2f, 2.0f, 0), "the same blocks go in and come back changed", 13, LBL_SOFT, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_TOOLKIT = {
    "c-functions-arrays", "Machine Shop", "Your functions are machines: the array goes through doubleAll() and comes back changed.",
    { 120, 200, 255, 255 },
    ParseToolkit, ToolkitReset, ToolkitEvent, ToolkitUpdate, ToolkitDraw, NULL,
};
