// Pointer Walk -> "Street Walk"
//
// The numbers read before the 0 are houses in a row at addresses four bytes apart (the
// array); the 0 is a dead-end sign, since it isn't stored. "Count: N" is checked at the
// dead end. For "Forward:" a pointer p hops house to house - p++ moves it one int along -
// lighting each printed value; for "Backward:" it walks back with p--. "Sum:" sends every
// house's loot into sum()'s bag. A wrong value lights the house p was on in red.
#include "levels.h"
#include "stage.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { PW_HOUSE = 1, PW_COUNT, PW_STEP, PW_SUM };
enum { PWF_COUNT = 1, PWF_STEP, PWF_SUM, PWF_GARBLED, PWF_EXTRA };

#define HMAX 12

// ---------------------------------------------------------------- the parser

static void ParseWalk(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[101];
    int n = InputInts(run->input, in, 101), count = 0;
    while (count < n && in[count] != 0) count++;
    for (int i = 0; i < count && i < HMAX; i++) EvSpawn(&p, PW_HOUSE, i, 0, in[i]);
    EvSpawn(&p, PW_COUNT, count, 0, 0);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        bool isCount = ScanAfter(E, el, "Count:", &want), isSum = !isCount && ScanAfter(E, el, "Sum:", &want);
        if (isCount || isSum) {
            const char *prefix = isCount ? "Count:" : "Sum:";
            if (ParseTake(&p, e)) { EvAct(&p, isCount ? PW_COUNT : PW_SUM, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, isCount ? PWF_COUNT : PWF_SUM, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, prefix, &got)) {
                if (isCount && got == want + 1) snprintf(note, sizeof(note), "That counted the 0 too - it ends the input, it isn't stored");
                else snprintf(note, sizeof(note), "%s should be %d", isCount ? "The count" : "The sum", want);
                ParseFail(&p, FAIL_WRONG, isCount ? PWF_COUNT : PWF_SUM, e, 0, 0, got, note);
            } else {
                ParseFail(&p, FAIL_GARBLED, PWF_GARBLED, e, 0, 0, 0, isCount ? "Expected Count: and a number" : "Expected Sum: and a number");
            }
            break;
        }
        bool back = el >= 9 && memcmp(E, "Backward:", 9) == 0;
        const char *prefix = back ? "Backward:" : "Forward:";
        int wl[101], wn = 0, gl[101], gn = 0;
        ScanAfterInts(E, el, prefix, wl, 101, &wn);
        if (wn > 101) wn = 101;
        if (ParseTake(&p, e)) {
            for (int k = 0; k < wn; k++) EvAct(&p, PW_STEP, back ? wn - 1 - k : k, back, wl[k]);
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, PWF_STEP, e, 0, back, 0, NULL); break; }
        if (!ScanAfterInts(A, al, prefix, gl, 101, &gn)) {
            ParseFail(&p, FAIL_GARBLED, PWF_GARBLED, e, 0, back, 0, back ? "Expected Backward: and the numbers" : "Expected Forward: and the numbers");
            break;
        }
        if (gn > 101) gn = 101;
        ParseShow(&p);
        int k = 0;
        while (k < wn && k < gn && gl[k] == wl[k]) { EvAct(&p, PW_STEP, back ? wn - 1 - k : k, back, gl[k]); k++; }
        if (k < wn && k < gn) {
            snprintf(note, sizeof(note), "Step %d should be %d - %s", k + 1, wl[k], back ? "start at the last number and use p--" : "start at the first number and use p++");
            GameEvent *f = ParseFail(&p, FAIL_WRONG, PWF_STEP, e, back ? wn - 1 - k : k, back, gl[k], note);
            if (f) snprintf(f->text, EV_TEXT, "%d", wl[k]);
        } else if (gn != wn) {
            snprintf(note, sizeof(note), "%d numbers were printed - there are %d", gn, wn);
            ParseFail(&p, FAIL_WRONG, PWF_STEP, e, gn < wn ? (back ? wn - 1 - gn : gn) : -1, back, 0, note);
        } else {
            ParseFail(&p, FAIL_WRONG, PWF_STEP, e, -1, back, 0, "Right numbers, wrong spacing: one space between them");
        }
        break;
    }
    ParseEnd(&p, PWF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_BAD, DO_COIN };

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time;
    int   value[HMAX], printed[HMAX], count, shown;
    int   lit[HMAX];               // 0 dark, 1 visited, 2 wrong
    float ptrX, ptrTo;             // the pointer's position along the street
    int   at;                      // the house p points at (-1: none yet)
    bool  back, counted, countBad, sumShown, sumBad, gone;
    int   countShown, sum;
    int   coins;                   // loot that has reached the bag
    char  want[EV_TEXT];
    int   wantAt;
} P;

static float Gap(void) { float g = 9.0f / (P.shown + 2); return g > 1.5f ? 1.5f : g; }
static float Size(void) { float s = Gap() / 1.35f; return s > 1 ? 1 : s; }
static float HouseX(int i) { return -4.6f + (i + 0.5f) * Gap(); }
static Vector3 BagPos(void) { return V3(4.7f, 0, 1.2f); }

static void WalkReset(const EventList *ev)
{
    memset(&P, 0, sizeof(P));
    P.ev = ev;
    P.at = -1;
    P.wantAt = -1;
    P.ptrX = P.ptrTo = -5.2f;
    ShotOrbit(V3(0, 1.0f, 0.6f), 10.2f, 24, 0, 37, 0);
}

static float WalkEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == PW_HOUSE && i >= 0 && i < HMAX) { P.value[i] = (int)e->value; if (i + 1 > P.shown) P.shown = i + 1; }
            if (e->code == PW_COUNT) P.count = i;
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == PW_COUNT) {
                P.counted = true;
                P.countShown = (int)e->value;
                FxSparkle(V3(HouseX(P.shown), 1.2f, 0.4f), 12, (Color){ 140, 255, 180, 255 });
                return 1.0f;
            }
            if (e->code == PW_STEP) {
                if (i >= 0 && i < HMAX) {
                    if (P.back != (e->b != 0)) { for (int k = 0; k < HMAX; k++) P.lit[k] = 0; P.back = e->b != 0; }
                    P.at = i;
                    P.ptrTo = HouseX(i);
                    P.lit[i] = 1;
                    PopAdd(&P.pops, e->b ? "p--" : "p++", V3(HouseX(i), 3.5f, 1.0f), (Color){ 200, 170, 255, 255 });
                }
                return P.shown > 8 ? 0.4f : 0.6f;
            }
            if (e->code == PW_SUM) {
                P.sumShown = true;
                P.sum = (int)e->value;
                P.coins = 0;
                for (int k = 0; k < P.shown; k++) CueAt(&P.cues, P.time + 0.1f + k * 0.12f, DO_COIN, k, 0, V3(0, 0, 0), 0);
                return 1.0f + P.shown * 0.12f;
            }
            return 0;
        case EV_FAIL:
            StrCopy(P.want, sizeof(P.want), e->text);
            if (e->code == PWF_COUNT) { P.counted = P.countBad = true; P.countShown = (int)e->value; }
            else if (e->code == PWF_SUM) {
                P.sumShown = P.sumBad = true;
                P.sum = (int)e->value;
                for (int k = 0; k < P.shown; k++) CueAt(&P.cues, P.time + 0.1f + k * 0.12f, DO_COIN, k, 0, V3(0, 0, 0), 0);
            } else if (e->code == PWF_STEP && i >= 0 && i < HMAX) {
                if (P.back != (e->b != 0)) { for (int k = 0; k < HMAX; k++) P.lit[k] = 0; P.back = e->b != 0; }
                P.at = i;
                P.ptrTo = HouseX(i);
                P.lit[i] = 2;
                P.printed[i] = (int)e->value;
                P.wantAt = i;
            } else if (e->code == PWF_STEP && i == -1) {
                P.gone = true;          // walked off the end of the street
                P.ptrTo = HouseX(P.shown) + 0.4f;
            }
            CueAt(&P.cues, P.time + 0.5f, DO_BAD, 0, 0, V3(P.ptrTo, 1.2f, 0.4f), 0);
            return 2.8f;
        case EV_CRASH:
            CueAt(&P.cues, P.time + 0.1f, DO_BAD, 0, 0, V3(P.ptrX, 1.2f, 0.4f), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&P.cues, P.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void WalkUpdate(float dt)
{
    P.time += dt;
    PopsUpdate(&P.pops, dt);
    P.ptrX = Lerpf(P.ptrX, P.ptrTo, 1 - expf(-10 * dt));
    Cue c;
    while (CueDue(&P.cues, P.time, &c)) {
        if (c.what == DO_COIN) {
            P.coins = c.a + 1;
            FxSparkle(V3(HouseX(c.a), 0.8f, 0.7f), 4, (Color){ 255, 220, 120, 255 });
        } else if (c.what == DO_BAD) {
            FxSparks(c.v, 30, (Color){ 255, 100, 90, 255 });
            PlaybackShake(0.2f);
        } else {
            FxConfetti(V3(0, 1.8f, 0.6f), PlaybackFinale() ? 150 : 70, 3.0f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static void WalkDraw(void)
{
    KitStage(V3(0, 0, 0.5f), 5.6f, 2.0f);
    KitRoad(V3(-5.4f, 0, 1.3f), 10.8f);
    float s = Size();
    for (int i = 0; i < P.shown; i++) {
        rlPushMatrix();
        rlTranslatef(HouseX(i), 0, 0);
        rlScalef(s, s, s);
        Color wall = P.lit[i] == 2 ? (Color){ 200, 70, 70, 255 } : P.lit[i] ? (Color){ 150, 140, 210, 255 } : (Color){ 120, 116, 150, 255 };
        KitHouse(V3(0, 0, 0), wall, (Color){ 90, 70, 140, 255 }, P.lit[i] ? 0.6f : 0);
        rlPopMatrix();
    }
    // the dead end: the 0 that stopped the reading isn't part of the array
    float dx = HouseX(P.shown);
    Part(MESH_CYLINDER, V3(dx, 0.5f, 0.3f), V3(0.08f, 1.0f, 0.08f), (Color){ 78, 86, 112, 255 }, 0);
    KitPanel(V3(dx, 1.05f, 0.3f), 0.7f, 0.35f, (Color){ 120, 40, 40, 255 }, 0.3f);
    // sum()'s bag at the end of the street, filling with loot
    KitBag(BagPos(), 0.4f + 0.04f * (P.coins > 8 ? 8 : P.coins), (Color){ 150, 110, 70, 255 });
    // the pointer: a badge over the street with an arrow down to the house it points at
    if (P.at >= 0 || P.gone) {
        Vector3 badge = V3(P.ptrX + 0.5f, 3.0f, 1.0f);
        Color col = (P.lit[P.at >= 0 ? P.at : 0] == 2 || P.gone) ? (Color){ 255, 100, 90, 255 } : (Color){ 200, 170, 255, 255 };
        PartRot(MESH_CYLINDER, badge, V3(0.42f, 0.08f, 0.42f), V3(90, 0, 0), col, 0.7f);
        KitArc(V3(badge.x, badge.y - 0.15f, badge.z), V3(P.ptrX, 1.45f * s + 0.1f, 0.1f), 0.2f, 1, col, 0.7f);
    }

    for (int i = 0; i < P.shown; i++) {
        Color fg = P.lit[i] == 2 ? WHITE : P.lit[i] ? LBL_GOLD : LBL_SOFT, bg = P.lit[i] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        Label3D(V3(HouseX(i), 0.6f * s, 0.7f), TextFormat("%d", P.lit[i] == 2 ? P.printed[i] : P.value[i]), 16, fg, bg, LABEL_CODE);
        if (P.shown <= 8) Label3D(V3(HouseX(i), 0.02f, 1.85f), TextFormat("0x20%02X", i * 4), 11, (Color){ 170, 150, 255, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    Label3D(V3(dx, 1.05f, 0.4f), "0", 14, WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    Label3D(V3(dx, 1.5f, 0.4f), "end", 11, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (P.counted)
        Label3D(V3(dx, 2.0f, 0.4f), TextFormat("Count: %d", P.countShown), 15, P.countBad ? WHITE : LBL_GOOD, P.countBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (P.countBad) Label3D(V3(dx, 2.45f, 0.4f), TextFormat("%d houses", P.shown), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (P.at >= 0 && P.at < HMAX)
        Label3D(V3(P.ptrX + 0.5f, 3.0f, 1.1f), "p", 15, WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (P.at >= 0 && P.at < HMAX && !P.sumShown)
        Label3D(V3(P.ptrX + 0.5f, 3.45f, 1.0f), TextFormat("p = 0x20%02X", P.at * 4), 12, (Color){ 200, 180, 255, 255 }, LBL_DIM, LABEL_CODE);
    if (P.sumShown) {
        Label3D(V3(BagPos().x, 1.25f, BagPos().z), TextFormat("Sum: %d", P.coins >= P.shown ? P.sum : 0), 16, P.sumBad && P.coins >= P.shown ? WHITE : LBL_GOLD,
                P.sumBad && P.coins >= P.shown ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        Label3D(V3(BagPos().x, 0.85f, BagPos().z + 0.2f), "sum(ptr, count)", 11, LBL_SOFT, LBL_DIM, LABEL_CODE);
    }
    if (P.want[0] && (P.wantAt >= 0 || P.sumBad || P.countBad)) {
        Vector3 at = P.wantAt >= 0 ? V3(HouseX(P.wantAt), 2.35f, 0.4f) : P.sumBad ? V3(BagPos().x, 1.7f, BagPos().z) : V3(dx, 2.9f, 0.4f);
        if (!P.countBad || P.wantAt >= 0 || P.sumBad) Label3D(at, TextFormat("should be %s", P.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (P.gone) Label3D(V3(dx + 0.6f, 2.4f, 0.4f), "past the end!", 13, LBL_BAD, LBL_DIM, LABEL_CODE);
    PopsDraw(&P.pops);
}

const LevelDef LEVEL_PTRWALK = {
    "c-pointer-arith", "Street Walk", "p++ walks your pointer along the street of houses, one int at a time.",
    { 200, 170, 255, 255 },
    ParseWalk, WalkReset, WalkEvent, WalkUpdate, WalkDraw, NULL,
};
