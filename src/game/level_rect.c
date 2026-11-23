// Rectangle -> "Build a Room"
//
// The input's width and height mark out a room. "Area: N" lays N floor tiles, one by one,
// with a counter; "Perimeter: N" runs N fence sections round the edge. Too few leaves
// gaps that flash; too many piles red tiles (or fence posts) outside the room. Common
// mix-ups (width + height, area for perimeter) are named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { RM_SIZE = 1, RM_AREA, RM_PERIM };
enum { RMF_AREA = 1, RMF_PERIM, RMF_GARBLED, RMF_EXTRA };

#define RMAX 12
#define T 0.8f

// ---------------------------------------------------------------- the parser

static void ParseRect(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[2] = { 0, 0 };
    InputInts(run->input, in, 2);
    int w = in[0], h = in[1];
    EvSpawn(&p, RM_SIZE, w, h, 0);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        bool area = ScanAfter(E, el, "Area:", &want);
        bool perim = !area && ScanAfter(E, el, "Perimeter:", &want);
        if (!area && !perim) { if (!ParseTake(&p, e)) ParseFail(&p, A ? FAIL_GARBLED : FAIL_MISSING, RMF_GARBLED, e, 0, 0, 0, NULL); continue; }
        if (ParseTake(&p, e)) { EvAct(&p, area ? RM_AREA : RM_PERIM, 0, 0, want); continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, area ? RMF_AREA : RMF_PERIM, e, 0, 0, 0, NULL); break; }
        if (ScanAfter(A, al, area ? "Area:" : "Perimeter:", &got)) {
            if (area && got == w + h) snprintf(note, sizeof(note), "That's width + height. The area is width * height");
            else if (area && got == 2 * (w + h)) snprintf(note, sizeof(note), "That's the perimeter. The area is width * height");
            else if (!area && got == w * h) snprintf(note, sizeof(note), "That's the area. The perimeter is 2 * (width + height)");
            else if (!area && got == w + h) snprintf(note, sizeof(note), "That's only two sides: the perimeter is 2 * (width + height)");
            else snprintf(note, sizeof(note), "%s should be %d", area ? "The area" : "The perimeter", want);
            ParseFail(&p, FAIL_WRONG, area ? RMF_AREA : RMF_PERIM, e, 0, 0, got, note);
        } else {
            ParseFail(&p, FAIL_GARBLED, RMF_GARBLED, e, 0, 0, 0, area ? "Expected Area: and a whole number" : "Expected Perimeter: and a whole number");
        }
        break;
    }
    ParseEnd(&p, RMF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Bot   bot;
    Cues  cues;
    float time;
    int   w, h, rw, rh;            // the input, and the room as drawn (clamped)
    int   tiles, tileGoal;         // tiles laid / to lay
    int   fence, fenceGoal;
    float tileT, fenceT;
    int   areaShown, perimShown;   // the printed values, -1 until printed
    bool  areaBad, perimBad, done;
} M;

static float X0(void) { return -M.rw * T / 2; }
static float Z0(void) { return -M.rh * T / 2; }
static Vector3 TileAt(int k)        // the k-th tile, row by row from the back; past the room: a pile in front
{
    int n = M.rw * M.rh;
    if (k < n) return V3(X0() + (k % M.rw + 0.5f) * T, 0, Z0() + (k / M.rw + 0.5f) * T);
    int j = k - n;
    return V3(X0() + (j % 8 + 0.5f) * T * 0.9f, 0.0f, -Z0() + 0.9f + (j / 8) * T * 0.8f);
}

// The k-th fence section: round the edge from the back-left corner, clockwise
static void FenceAt(int k, Vector3 *a, Vector3 *b)
{
    int w = M.rw, h = M.rh, per = 2 * (w + h);
    float x0 = X0(), z0 = Z0(), x1 = -X0(), z1 = -Z0();
    if (k >= per) {        // extra sections pile up beside the room
        int j = k - per;
        *a = V3(x1 + 0.7f + (j % 4) * 0.3f, 0, z1 - (j / 4) * 0.3f);
        *b = V3(a->x + 0.25f, 0, a->z);
        return;
    }
    if (k < w)               { *a = V3(x0 + k * T, 0, z0); *b = V3(x0 + (k + 1) * T, 0, z0); }
    else if (k < w + h)      { int i = k - w; *a = V3(x1, 0, z0 + i * T); *b = V3(x1, 0, z0 + (i + 1) * T); }
    else if (k < 2 * w + h)  { int i = k - w - h; *a = V3(x1 - i * T, 0, z1); *b = V3(x1 - (i + 1) * T, 0, z1); }
    else                     { int i = k - 2 * w - h; *a = V3(x0, 0, z1 - i * T); *b = V3(x0, 0, z1 - (i + 1) * T); }
}

static void RectReset(const EventList *ev)
{
    memset(&M, 0, sizeof(M));
    M.ev = ev;
    M.areaShown = M.perimShown = -1;
    M.rw = M.rh = 3;
}

static float RectEvent(const GameEvent *e)
{
    int per = 2 * (M.rw + M.rh);
    switch (e->type) {
        case EV_SPAWN:
            M.w = e->a;
            M.h = e->b;
            M.rw = e->a < 1 ? 1 : e->a > RMAX ? RMAX : e->a;
            M.rh = e->b < 1 ? 1 : e->b > RMAX ? RMAX : e->b;
            BotInit(&M.bot, V3(X0() - 1.1f, 0, -Z0() + 0.4f), 45);
            {
                float ext = fmaxf(M.rw * T, M.rh * T * 1.3f) + 3.0f;
                ShotOrbit(V3(0.2f, 0.2f, 0.4f), fmaxf(7.0f, ext * 1.25f), 46, 0, 38, 0);
            }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == RM_AREA) {
                M.areaShown = (int)e->value;
                M.tileGoal = M.rw * M.rh;
                return 0.7f + M.tileGoal * 0.04f;
            }
            if (e->code == RM_PERIM) {
                M.perimShown = (int)e->value;
                M.fenceGoal = per;
                return 0.7f + per * 0.06f;
            }
            return 0;
        case EV_FAIL:
            M.done = true;
            if (e->code == RMF_AREA) {
                M.areaShown = (int)e->value;
                M.areaBad = true;
                int v = (int)e->value;
                M.tileGoal = v < 0 ? 0 : v > M.rw * M.rh + 40 ? M.rw * M.rh + 40 : v;
                CueAt(&M.cues, M.time + 0.4f + M.tileGoal * 0.04f, DO_BAD, 0, 0, V3(0, 0, 0), 0);
            } else if (e->code == RMF_PERIM) {
                M.perimShown = (int)e->value;
                M.perimBad = true;
                int v = (int)e->value;
                M.fenceGoal = v < 0 ? 0 : v > per + 24 ? per + 24 : v;
                CueAt(&M.cues, M.time + 0.4f + M.fenceGoal * 0.06f, DO_BAD, 0, 0, V3(0, 0, 0), 0);
            } else {
                CueAt(&M.cues, M.time + 0.2f, DO_BAD, 0, 0, V3(0, 0, 0), 0);
            }
            return 2.6f + fmaxf(M.tileGoal * 0.04f, M.fenceGoal * 0.06f) * 0.3f;
        case EV_CRASH:
            M.done = true;
            BotPowerDown(&M.bot);
            PlaybackShake(0.2f);
            return 2.4f;
        case EV_WIN:
            M.done = true;
            CueAt(&M.cues, M.time + 0.3f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void RectUpdate(float dt)
{
    M.time += dt;
    BotUpdate(&M.bot, dt);
    Cue c;
    while (CueDue(&M.cues, M.time, &c)) {
        if (c.what == DO_CHEER) {
            BotFace(&M.bot, 0);
            BotCheer(&M.bot);
            FxConfetti(V3(0, 1.5f, 0), PlaybackFinale() ? 150 : 70, M.rw * T * 0.4f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
        } else {
            BotMood(&M.bot, MOOD_SAD);
            BotShakeHead(&M.bot);
            FxSparks(V3(0, 0.6f, 0), 30, (Color){ 255, 110, 80, 255 });
            PlaybackShake(0.22f);
        }
    }
    M.tileT += dt;
    while (M.tiles < M.tileGoal && M.tileT > 0.04f) {
        M.tileT -= 0.04f;
        FxDust(TileAt(M.tiles), 2);
        M.tiles++;
    }
    if (M.tiles >= M.tileGoal) M.tileT = 0;
    M.fenceT += dt;
    while (M.fence < M.fenceGoal && M.fenceT > 0.06f) { M.fenceT -= 0.06f; M.fence++; }
    if (M.fence >= M.fenceGoal) M.fenceT = 0;
}

static void RectDraw(void)
{
    float ext = fmaxf(M.rw, M.rh) * T / 2 + 2.2f;
    KitStage(V3(0, 0, 0.5f), ext + 0.6f, ext);
    int n = M.rw * M.rh;
    // the plot: a faint outline of every tile the room should have
    for (int k = 0; k < n; k++) {
        Vector3 t = TileAt(k);
        bool laid = k < M.tiles;
        Color top = laid ? (Color){ 196, 150, 100, 255 } : (Color){ 46, 52, 70, 255 };
        if (!laid && M.areaBad && M.done && M.tiles >= M.tileGoal)
            top = (Color){ 255, 160, 60, 255 };                  // tiles that should be there but weren't printed
        float pop = laid ? Clamp01((M.tiles - k) / 3.0f) : 1;
        if (laid) KitTile(t, T * 0.96f, pop, top, 0.08f);
        else Part(MESH_CUBE, V3(t.x, 0.01f, t.z), V3(T * 0.9f, 0.02f, T * 0.9f), top, M.areaBad && M.done ? 0.3f * Pulse(M.time, 6) : 0);
    }
    for (int k = n; k < M.tiles; k++)                           // too many: red tiles outside the room
        KitTile(TileAt(k), T * 0.8f, Clamp01((M.tiles - k) / 3.0f), (Color){ 220, 70, 70, 255 }, 0.4f);

    int per = 2 * (M.rw + M.rh);
    for (int k = 0; k < M.fence; k++) {
        Vector3 a, b;
        FenceAt(k, &a, &b);
        bool extra = k >= per;
        Color post = extra ? (Color){ 220, 70, 70, 255 } : (Color){ 230, 220, 200, 255 };
        Part(MESH_CUBE, V3(a.x, 0.3f, a.z), V3(0.08f, 0.6f, 0.08f), post, extra ? 0.4f : 0);
        if (!extra) {
            Vector3 c = Vector3Lerp(a, b, 0.5f);
            bool alongX = fabsf(b.x - a.x) > fabsf(b.z - a.z);
            Part(MESH_CUBE, V3(c.x, 0.42f, c.z), alongX ? V3(T, 0.06f, 0.04f) : V3(0.04f, 0.06f, T), (Color){ 200, 170, 120, 255 }, 0);
            Part(MESH_CUBE, V3(c.x, 0.22f, c.z), alongX ? V3(T, 0.06f, 0.04f) : V3(0.04f, 0.06f, T), (Color){ 200, 170, 120, 255 }, 0);
        }
    }
    BotDraw(&M.bot);

    // the measurements and the counters
    Label3D(V3(0, 0.15f, -Z0() + 0.45f), TextFormat("width = %d", M.w), 15, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(X0() - 0.55f, 0.15f, 0), TextFormat("height = %d", M.h), 15, LBL_SOFT, LBL_DIM, LABEL_CODE);
    if (M.areaShown >= 0) {
        bool counting = M.tiles < M.tileGoal;
        Color fg = M.areaBad && !counting ? WHITE : LBL_GOLD, bg = M.areaBad && !counting ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        Label3D(V3(0, 1.6f, 0), TextFormat("Area: %d", counting ? M.tiles : M.areaShown), 19, fg, bg, LABEL_CODE);
        if (M.areaBad && !counting) Label3D(V3(0, 1.15f, 0), TextFormat("the room has %d tiles", n), 14, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (M.perimShown >= 0) {
        bool counting = M.fence < M.fenceGoal;
        Color fg = M.perimBad && !counting ? WHITE : (Color){ 230, 220, 200, 255 }, bg = M.perimBad && !counting ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        Label3D(V3(-X0() + 0.2f, 1.1f, Z0() - 0.2f), TextFormat("Perimeter: %d", counting ? M.fence : M.perimShown), 17, fg, bg, LABEL_CODE);
        if (M.perimBad && !counting) Label3D(V3(-X0() + 0.2f, 0.7f, Z0() - 0.2f), TextFormat("the edge is %d sections", per), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
}

const LevelDef LEVEL_RECT = {
    "cpp-hello", "Build a Room", "Your area lays the floor tile by tile; your perimeter builds the fence.",
    { 200, 160, 110, 255 },
    ParseRect, RectReset, RectEvent, RectUpdate, RectDraw, NULL,
};
