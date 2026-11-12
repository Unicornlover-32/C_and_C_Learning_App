// Digit Counter -> "Digit Bins"
//
// The input number's digits sit on a belt. They are peeled off from the right (% 10,
// then / 10) and each drops into the bin for its name. The ten bins stand in the map's
// own order - alphabetical: eight, five, four, nine, one, seven, six, three, two, zero.
// Each "name = count" line lights its bin with the printed count, in the order printed.
// A wrong count cracks its bin red; a bin lit out of order means the program didn't just
// loop over the map.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { DG_DIGIT = 1, DG_ENTRY };
enum { DGF_ENTRY = 1, DGF_GARBLED, DGF_EXTRA };

#define DMAX 20

static const char *const BINS[10] = { "eight", "five", "four", "nine", "one", "seven", "six", "three", "two", "zero" };
static const int DIGIT_BIN[10] = { 9, 4, 8, 7, 2, 1, 6, 5, 0, 3 };     // digit -> its bin

static int BinOf(const char *name)
{
    for (int i = 0; i < 10; i++) if (strcmp(BINS[i], name) == 0) return i;
    return -1;
}

// "five = 1" -> bin 1, count 1
static bool ScanEntry(const char *s, int len, int *bin, int *count)
{
    char buf[64], name[32];
    int k = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1, used = 0;
    memcpy(buf, s, (size_t)k);
    buf[k] = '\0';
    if (sscanf(buf, "%31s = %d%n", name, count, &used) != 2 || used != k) return false;
    *bin = BinOf(name);
    return true;
}

// ---------------------------------------------------------------- the parser

static void ParseMaps(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    char num[32] = "";
    if (run->input) sscanf(run->input, "%31s", num);
    int n = 0;
    for (int i = 0; num[i] && n < DMAX; i++) if (num[i] >= '0' && num[i] <= '9') EvSpawn(&p, DG_DIGIT, n++, 0, num[i] - '0');
    if (ParsePreview(&p)) return;

    char note[140];
    int last = -1;
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, bin, count, gbin, gcount;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (!ScanEntry(E, el, &bin, &count)) { ParseTake(&p, e); continue; }
        if (ParseTake(&p, e)) { EvAct(&p, DG_ENTRY, bin, 0, count); last = bin; continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, DGF_ENTRY, e, -1, bin, 0, NULL); break; }
        if (!ScanEntry(A, al, &gbin, &gcount) || gbin < 0) {
            ParseFail(&p, FAIL_GARBLED, DGF_GARBLED, e, -1, bin, 0, "Expected a digit's name, = and a count, like five = 1");
            break;
        }
        if (gbin != bin && gbin < last) snprintf(note, sizeof(note), "Out of order: a map keeps its keys sorted - just loop over the map");
        else if (gbin != bin) snprintf(note, sizeof(note), "Next in the map's order is %.*s", el, E);
        else snprintf(note, sizeof(note), "%s appears %d time%s", BINS[bin >= 0 ? bin : 0], count, count == 1 ? "" : "s");
        ParseFail(&p, FAIL_WRONG, DGF_ENTRY, e, gbin, bin, gcount, note);
        break;
    }
    ParseEnd(&p, DGF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_PEEL, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   digit[DMAX], n;
    int   peeled;                    // digits taken off the right so far
    int   inBin[10];                 // digits that have landed in each bin
    int   lit[10];                   // 0 dark, 1 printed, 2 wrong
    int   shown[10];                 // the printed counts
    int   order[10], orderN;
    struct { int bin, digit; float t; bool live; } fly[DMAX];
    bool  started;
    char  want[96];
    int   wantBin;
} G;

static float BinX(int i) { return -4.05f + i * 0.9f; }
static Vector3 DigitPos(int i) { float g = 4.2f / (G.n + 1); if (g > 0.45f) g = 0.45f; return V3(-2.0f + (i - (G.n - 1) * 0.5f) * g, BELT_Y, 2.4f); }

static void MapsReset(const EventList *ev)
{
    memset(&G, 0, sizeof(G));
    G.ev = ev;
    G.wantBin = -1;
    ShotOrbit(V3(0, 0.8f, 1.0f), 10.4f, 34, 0, 38, 0);
}

static void Start(void)
{
    if (G.started) return;
    G.started = true;
    for (int k = 0; k < G.n; k++) CueAt(&G.cues, G.time + 0.2f + k * 0.22f, DO_PEEL, G.n - 1 - k, 0, V3(0, 0, 0), 0);
}

static float MapsEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (e->a >= 0 && e->a < DMAX) { G.digit[e->a] = (int)e->value; if (e->a + 1 > G.n) G.n = e->a + 1; }
            return 0;
        case EV_PRINT:
            if (!G.started) { Start(); return 0.8f + G.n * 0.22f; }
            return 0.05f;
        case EV_ACT:
            if (e->code == DG_ENTRY && e->a >= 0 && e->a < 10) {
                G.lit[e->a] = 1;
                G.shown[e->a] = (int)e->value;
                if (G.orderN < 10) G.order[G.orderN++] = e->a;
                FxSparkle(V3(BinX(e->a), 1.3f, 0), 10, (Color){ 140, 255, 180, 255 });
                return 0.6f;
            }
            return 0;
        case EV_FAIL:
            if (!G.started) Start();
            if (G.ev) StrCopy(G.want, sizeof(G.want), G.ev->expected);
            G.wantBin = e->b;
            if (e->a >= 0 && e->a < 10) { G.lit[e->a] = 2; G.shown[e->a] = (int)e->value; }
            CueAt(&G.cues, G.time + 0.4f, DO_BAD, 0, 0, V3(e->a >= 0 && e->a < 10 ? BinX(e->a) : 0, 1.4f, 0), 0);
            return 2.8f;
        case EV_CRASH:
            CueAt(&G.cues, G.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.4f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&G.cues, G.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void MapsUpdate(float dt)
{
    G.time += dt;
    for (int i = 0; i < DMAX; i++) {
        if (!G.fly[i].live) continue;
        G.fly[i].t += dt / 0.5f;
        if (G.fly[i].t >= 1) { G.fly[i].live = false; G.inBin[G.fly[i].bin]++; }
    }
    Cue c;
    while (CueDue(&G.cues, G.time, &c)) {
        if (c.what == DO_PEEL) {
            int i = c.a;
            G.peeled++;
            G.fly[i].bin = DIGIT_BIN[G.digit[i] % 10];
            G.fly[i].digit = G.digit[i];
            G.fly[i].t = 0;
            G.fly[i].live = true;
        } else if (c.what == DO_BAD) { FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0, 2.2f, 0), PlaybackFinale() ? 150 : 70, 3.5f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void MapsDraw(void)
{
    KitStage(V3(0, 0, 1.0f), 5.0f, 2.4f);
    KitBelt(V3(-4.4f, 0, 2.4f), 5.0f, 0);
    for (int b = 0; b < 10; b++) {
        Color col = G.lit[b] == 2 ? (Color){ 235, 90, 90, 255 } : G.lit[b] ? (Color){ 80, 220, 130, 255 } : (Color){ 70, 100, 150, 255 };
        if (G.wantBin == b && G.lit[b] != 1) col = (Color){ 80, 220, 130, 255 };
        float glow = G.lit[b] ? 0.6f : (G.wantBin == b ? 0.3f + 0.4f * Pulse(G.time, 6) : 0.05f);
        Part(MESH_CUBE, V3(BinX(b), 0.3f, 0), V3(0.72f, 0.6f, 0.72f), (Color){ 50, 56, 76, 255 }, 0);
        Part(MESH_CUBE, V3(BinX(b), 0.62f, 0), V3(0.78f, 0.05f, 0.78f), col, glow);
        for (int k = 0; k < G.inBin[b] && k < 6; k++)        // the digits that landed in it
            Part(MESH_CUBE, V3(BinX(b), 0.66f + k * 0.13f, 0), V3(0.3f, 0.11f, 0.3f), (Color){ 255, 200, 110, 255 }, 0.2f);
    }
    // the digits still on the belt (peeled from the right) and the ones flying
    for (int i = 0; i < G.n; i++) {
        bool gone = G.started && i >= G.n - G.peeled;
        if (!gone) KitCrate(DigitPos(i), 0.36f, (Color){ 255, 200, 110, 255 }, 0.15f, 0);
        if (G.fly[i].live) {
            float t = EaseInOutQuad(G.fly[i].t);
            Vector3 p = LerpV3(DigitPos(i), V3(BinX(G.fly[i].bin), 0.9f, 0), t);
            p.y += sinf(t * PI) * 1.2f;
            KitCrate(p, 0.3f, (Color){ 255, 200, 110, 255 }, 0.4f, t * 360);
        }
    }

    for (int b = 0; b < 10; b++) {
        Label3D(V3(BinX(b), 0.2f, 0.45f), BINS[b], 12, G.lit[b] ? WHITE : LBL_SOFT, G.lit[b] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        if (G.lit[b]) Label3D(V3(BinX(b), 1.65f, 0), TextFormat("= %d", G.shown[b]), 15, G.lit[b] == 2 ? WHITE : LBL_GOLD,
                              G.lit[b] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    }
    for (int k = 0; k < G.orderN; k++) Label3D(V3(BinX(G.order[k]), 2.05f, 0), TextFormat("%d", k + 1), 11, (Color){ 150, 170, 220, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    for (int i = 0; i < G.n; i++) {
        bool gone = G.started && i >= G.n - G.peeled;
        if (!gone) Label3D(V3(DigitPos(i).x, BELT_Y + 0.6f, 2.5f), TextFormat("%d", G.digit[i]), 13, WHITE, LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(-2.0f, 0.05f, 3.25f), "% 10 peels off the last digit", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(0.0f, 2.5f, -0.4f), "map<string, int> - always in key order", 13, (Color){ 150, 210, 255, 255 }, LBL_DIM, LABEL_CODE);
    if (G.want[0]) Label3D(V3(0, 3.0f, -0.4f), TextFormat("should be: %s", G.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_MAPS = {
    "cpp-maps", "Digit Bins", "Digits drop into bins in the map's own order - so print the map and they come out sorted.",
    { 120, 220, 170, 255 },
    ParseMaps, MapsReset, MapsEvent, MapsUpdate, MapsDraw, NULL,
};
