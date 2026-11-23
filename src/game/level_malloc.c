// How Many Numbers? -> "Heap Builder"
//
// On the left, the stack: n, and the pointer numbers. On the right, an empty plot of land:
// the heap. When the program starts printing, malloc builds exactly n houses on the plot
// (four bytes each) and the pointer's arrow reaches the first one. "You entered:" moves
// the values in, "Reversed:" walks a pointer back from the last house, and "Largest:" puts
// a crown on the biggest. With n = 0 nothing is built: the pointer stays NULL and the sign
// says "Nothing to store".
#include "levels.h"
#include "stage.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { ML_COUNT = 1, ML_VALUE, ML_BUILD, ML_STORE, ML_REV, ML_MAX, ML_NONE };
enum { MLF_STORE = 1, MLF_REV, MLF_MAX, MLF_NONE, MLF_GARBLED, MLF_EXTRA };

#define HMAX 12

// ---------------------------------------------------------------- the parser

static void ParseMalloc(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[101];
    int n = InputInts(run->input, in, 101);
    int count = n > 0 ? in[0] : 0;
    int stored = count < 0 ? 0 : (count > n - 1 ? (n > 0 ? n - 1 : 0) : count);
    EvSpawn(&p, ML_COUNT, count, 0, 0);
    for (int i = 0; i < stored && i < HMAX; i++) EvSpawn(&p, ML_VALUE, i, 0, in[1 + i]);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (LineIs(E, el, "Nothing to store")) {
            if (ParseTake(&p, e)) { EvAct(&p, ML_NONE, 0, 0, 0); continue; }
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, MLF_NONE, e, 0, 0, 0, "With nothing to store, print Nothing to store and don't call malloc");
            break;
        }
        int want, got;
        if (ScanAfter(E, el, "Largest:", &want)) {
            int at = 0;
            for (int i = 0; i < stored && i < HMAX; i++) if (in[1 + i] == want) { at = i; break; }
            if (ParseTake(&p, e)) { EvAct(&p, ML_MAX, at, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, MLF_MAX, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Largest:", &got)) {
                int gat = -1;
                for (int i = 0; i < stored && i < HMAX; i++) if (in[1 + i] == got) { gat = i; break; }
                snprintf(note, sizeof(note), "The largest is %d", want);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, MLF_MAX, e, gat, at, got, note);
                if (f) snprintf(f->text, EV_TEXT, "%d", want);
            } else {
                ParseFail(&p, FAIL_GARBLED, MLF_GARBLED, e, 0, 0, 0, "Expected Largest: and a number");
            }
            break;
        }
        bool rev = el >= 9 && memcmp(E, "Reversed:", 9) == 0;
        const char *prefix = rev ? "Reversed:" : "You entered:";
        int wl[101], wn = 0, gl[101], gn = 0;
        ScanAfterInts(E, el, prefix, wl, 101, &wn);
        if (wn > 101) wn = 101;
        if (ParseTake(&p, e)) {
            if (!rev) EvAct(&p, ML_BUILD, wn, 0, 0);
            for (int k = 0; k < wn; k++) EvAct(&p, rev ? ML_REV : ML_STORE, rev ? wn - 1 - k : k, 0, wl[k]);
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, rev ? MLF_REV : MLF_STORE, e, -1, 0, 0, NULL); break; }
        if (!ScanAfterInts(A, al, prefix, gl, 101, &gn)) {
            ParseFail(&p, FAIL_GARBLED, MLF_GARBLED, e, -1, 0, 0, rev ? "Expected Reversed: and the numbers" : "Expected You entered: and the numbers");
            break;
        }
        if (gn > 101) gn = 101;
        ParseShow(&p);
        if (!rev) EvAct(&p, ML_BUILD, wn, 0, 0);
        int k = 0;
        while (k < wn && k < gn && gl[k] == wl[k]) { EvAct(&p, rev ? ML_REV : ML_STORE, rev ? wn - 1 - k : k, 0, gl[k]); k++; }
        if (k < wn && k < gn) {
            snprintf(note, sizeof(note), "%s %d should be %d", rev ? "Going back, number" : "Number", k + 1, wl[k]);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, rev ? MLF_REV : MLF_STORE, e, rev ? wn - 1 - k : k, 0, gl[k], note);
            if (f) snprintf(f->text, EV_TEXT, "%d", wl[k]);
        } else if (gn != wn) {
            snprintf(note, sizeof(note), "%d numbers were printed - malloc made room for %d", gn, wn);
            ParseFail(&p, FAIL_WRONG, rev ? MLF_REV : MLF_STORE, e, -1, 1, 0, note);
        } else {
            ParseFail(&p, FAIL_WRONG, rev ? MLF_REV : MLF_STORE, e, -1, 2, 0, "Right numbers, wrong spacing: one space between them");
        }
        break;
    }
    ParseEnd(&p, MLF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_BUILD, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   count, value[HMAX];
    int   built;                   // houses built so far
    float buildT;
    bool  building, none, noneBad;
    int   state[HMAX];             // 0 empty, 1 stored, 2 wrong
    int   shown[HMAX];
    int   walker;                  // the Reversed pointer (-1 none)
    int   crown;                   // the house with the crown (-1 none)
    bool  crownBad;
    int   maxShown;
    char  want[EV_TEXT];
    int   wantAt;
} H;

static int Shown(void) { return H.count < 0 ? 0 : H.count > HMAX ? HMAX : H.count; }
static float Gap(void) { float g = 6.4f / (Shown() + 1); return g > 1.2f ? 1.2f : g; }
static float Size(void) { float s = Gap() / 1.2f; return s > 0.9f ? 0.9f : s; }
static float HouseX(int i) { return -1.5f + (i + 0.5f) * Gap() + 0.4f; }
static Vector3 PtrPos(void) { return V3(-4.2f, 1.2f, 0.2f); }

static void MallocReset(const EventList *ev)
{
    memset(&H, 0, sizeof(H));
    H.ev = ev;
    H.walker = H.crown = H.wantAt = -1;
    ShotOrbit(V3(0, 1.0f, 0.3f), 10.0f, 26, 0, 37, 0);
}

static float MallocEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == ML_COUNT) H.count = i;
            if (e->code == ML_VALUE && i >= 0 && i < HMAX) H.value[i] = (int)e->value;
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            switch (e->code) {
                case ML_BUILD:
                    H.building = true;
                    H.buildT = 0;
                    return 0.6f + Shown() * 0.15f;
                case ML_STORE:
                    if (i >= 0 && i < HMAX) { H.state[i] = 1; H.shown[i] = (int)e->value; FxSparkle(V3(HouseX(i), 0.8f, 0.6f), 6, (Color){ 255, 220, 120, 255 }); }
                    return 0.35f;
                case ML_REV:
                    if (i >= 0 && i < HMAX) H.walker = i;
                    return 0.45f;
                case ML_MAX:
                    H.walker = -1;
                    H.crown = i;
                    H.maxShown = (int)e->value;
                    FxSparkle(V3(HouseX(i), 1.6f, 0.2f), 18, (Color){ 255, 220, 120, 255 });
                    return 1.2f;
                case ML_NONE:
                    H.none = true;
                    return 1.4f;
            }
            return 0;
        case EV_FAIL:
            StrCopy(H.want, sizeof(H.want), e->text);
            if (e->code == MLF_NONE) { H.none = true; H.noneBad = true; }
            else if ((e->code == MLF_STORE || e->code == MLF_REV) && i >= 0 && i < HMAX) {
                H.state[i] = 2; H.shown[i] = (int)e->value; H.wantAt = i;
                if (e->code == MLF_REV) H.walker = i;
            } else if (e->code == MLF_MAX) {
                H.crownBad = true;
                H.crown = (i >= 0 && i < HMAX) ? i : -2;
                H.maxShown = (int)e->value;
                H.wantAt = e->b;
            }
            CueAt(&H.cues, H.time + 0.4f, DO_BAD, 0, 0, V3(i >= 0 && i < HMAX ? HouseX(i) : 0.8f, 1.3f, 0.4f), 0);
            return 2.8f;
        case EV_CRASH:
            CueAt(&H.cues, H.time + 0.1f, DO_BAD, 0, 0, V3(0.8f, 1.2f, 0.4f), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&H.cues, H.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void MallocUpdate(float dt)
{
    H.time += dt;
    if (H.building) {
        H.buildT += dt;
        int b = (int)(H.buildT / 0.15f) + 1;
        if (b > Shown()) b = Shown();
        while (H.built < b) { FxDust(V3(HouseX(H.built), 0, 0), 6); H.built++; }
    }
    Cue c;
    while (CueDue(&H.cues, H.time, &c)) {
        if (c.what == DO_BAD) { FxSparks(c.v, 30, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else if (c.what == DO_CHEER) {
            FxConfetti(V3(0.8f, 1.8f, 0.4f), PlaybackFinale() ? 150 : 70, 2.5f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static void MallocDraw(void)
{
    KitStage(V3(0, 0, 0.4f), 5.4f, 2.0f);
    // the stack: a little strip of ground for main's variables
    Part(MESH_CUBE, V3(-4.0f, 0.02f, 0.2f), V3(2.0f, 0.04f, 2.2f), (Color){ 40, 46, 64, 255 }, 0);
    Part(MESH_CUBE, V3(-4.4f, 0.4f, -0.4f), V3(0.7f, 0.8f, 0.6f), (Color){ 110, 120, 150, 255 }, 0);
    Vector3 ptr = PtrPos();
    PartRot(MESH_CYLINDER, ptr, V3(0.45f, 0.1f, 0.45f), V3(90, 0, 0), (Color){ 200, 170, 255, 255 }, 0.6f);
    // the heap: a plot that malloc builds on
    KitLot(V3(0.8f, 0, 0.1f), 7.0f, 2.4f, (Color){ 120, 255, 200, 255 }, H.built ? 0.6f : 0.25f);
    float s = Size();
    for (int i = 0; i < H.built; i++) {
        rlPushMatrix();
        rlTranslatef(HouseX(i), 0, 0);
        rlScalef(s, s, s);
        Color wall = H.state[i] == 2 ? (Color){ 200, 70, 70, 255 } : H.state[i] ? (Color){ 120, 200, 170, 255 } : (Color){ 100, 120, 130, 255 };
        KitHouse(V3(0, 0, 0), wall, (Color){ 60, 120, 110, 255 }, H.walker == i ? 0.8f : 0);
        rlPopMatrix();
    }
    if (H.built > 0) KitArc(V3(ptr.x, ptr.y + 0.2f, ptr.z), V3(HouseX(0), 1.4f * s + 0.1f, 0.1f), 0.8f, 1, (Color){ 200, 170, 255, 255 }, 0.6f);
    if (H.walker >= 0) KitArc(V3(HouseX(H.walker) + 0.3f, 2.9f, 0.9f), V3(HouseX(H.walker), 1.45f * s + 0.1f, 0.1f), 0.2f, 1, (Color){ 255, 200, 120, 255 }, 0.7f);
    if (H.crown >= 0 && H.crown < H.built) {
        Vector3 c = V3(HouseX(H.crown), 1.45f * s + 0.35f, 0);
        Color gold = H.crownBad ? (Color){ 235, 80, 80, 255 } : (Color){ 255, 205, 70, 255 };
        Part(MESH_CYLINDER, c, V3(0.45f, 0.18f, 0.45f), gold, 0.7f);
        for (int k = 0; k < 5; k++) {
            float a = k * 2 * PI / 5;
            Part(MESH_CONE, V3(c.x + cosf(a) * 0.18f, c.y + 0.12f, c.z + sinf(a) * 0.18f), V3(0.08f, 0.16f, 0.08f), gold, 0.7f);
        }
    }

    Label3D(V3(-4.4f, 1.05f, -0.1f), TextFormat("n = %d", H.count), 14, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(ptr.x, ptr.y + 0.55f, ptr.z), H.built ? "numbers" : "numbers = NULL", 13, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(-4.0f, 0.05f, 1.45f), "stack", 13, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    Label3D(V3(0.8f, 0.05f, 1.45f), "heap", 13, (Color){ 140, 255, 210, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (H.building && H.built > 0)
        Label3D(V3(0.8f, 2.85f, 0.1f), TextFormat("malloc(%d * sizeof(int))", H.count), 14, (Color){ 140, 255, 210, 255 }, LBL_DIM, LABEL_CODE);
    for (int i = 0; i < H.built; i++) {
        Color fg = H.state[i] == 2 ? WHITE : H.state[i] ? LBL_GOLD : LBL_SOFT, bg = H.state[i] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        Label3D(V3(HouseX(i), 0.55f * s, 0.6f), H.state[i] ? TextFormat("%d", H.shown[i]) : "?", 15, fg, bg, LABEL_CODE);
        if (Shown() <= 8) Label3D(V3(HouseX(i), 0.02f, 1.1f), TextFormat("[%d]", i), 11, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    if (H.crown != -1)
        Label3D(V3(H.crown >= 0 ? HouseX(H.crown) : 0.8f, 2.25f, 0.2f), TextFormat("Largest: %d", H.maxShown), 15,
                H.crownBad ? WHITE : LBL_GOLD, H.crownBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (H.none)
        Label3D(V3(0.8f, 1.0f, 0.1f), H.noneBad && H.ev ? H.ev->actual : "Nothing to store", 17, H.noneBad ? WHITE : LBL_GOOD,
                H.noneBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (H.want[0] && H.wantAt >= 0 && H.wantAt < H.built)
        Label3D(V3(HouseX(H.wantAt), 1.7f, 0.6f), TextFormat("should be %s", H.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (H.noneBad) Label3D(V3(0.8f, 0.55f, 0.3f), "should be: Nothing to store", 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_MALLOC = {
    "c-malloc", "Heap Builder", "malloc builds exactly as many houses as you ask for, out on the heap.",
    { 120, 255, 200, 255 },
    ParseMalloc, MallocReset, MallocEvent, MallocUpdate, MallocDraw, NULL,
};
