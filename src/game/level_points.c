// Moving Points -> "Point Rover"
//
// The rover is a Point object on a coordinate floor, its (x,y) shown above it. "Start:
// (x,y)" puts it where the program says; each "Moved to: (x,y)" drives it there along the
// offset arrow from the input. A wrong point sends it off target while the right one
// blinks; "(4, 6)" with a space is pointed out, since print() must match exactly.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { PR_START = 1, PR_OFFSET, PR_AT };
enum { PRF_POINT = 1, PRF_GARBLED, PRF_EXTRA };

#define PMAX 24

static int Fixed(double v) { return (int)lround((v > 1e6 ? 1e6 : v < -1e6 ? -1e6 : v) * 1000); }
static double Unfix(int v) { return v / 1000.0; }

// The input's numbers (decimals allowed), up to max of them
static int InputNumbers(const char *in, double *out, int max)
{
    int n = 0;
    const char *s = in ? in : "";
    while (n < max) {
        while (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t') s++;
        if (!*s) break;
        char *end = NULL;
        double v = strtod(s, &end);
        if (end == s || !isfinite(v)) break;
        out[n++] = v;
        s = end;
    }
    return n;
}

// "Moved to: (4,6)" -> 4, 6 (spaces inside are allowed here; the exact text is checked elsewhere)
static bool ScanPoint(const char *s, int len, const char *prefix, double *x, double *y)
{
    char buf[96];
    if (!ScanAfterText(s, len, prefix, buf, sizeof(buf))) return false;
    char *p = buf;
    while (*p == ' ') p++;
    if (*p++ != '(') return false;
    char *end;
    *x = strtod(p, &end);
    if (end == p) return false;
    p = end;
    while (*p == ' ') p++;
    if (*p++ != ',') return false;
    *y = strtod(p, &end);
    if (end == p) return false;
    p = end;
    while (*p == ' ') p++;
    return *p == ')' && p[1] == '\0' && isfinite(*x) && isfinite(*y);
}

// ---------------------------------------------------------------- the parser

static void ParsePoints(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    double in[2 + 2 * PMAX];
    int n = InputNumbers(run->input, in, 2 + 2 * PMAX);
    if (n >= 2) {
        EvSpawn(&p, PR_START, Fixed(in[0]), Fixed(in[1]), 0);
        for (int i = 2; i + 1 < n; i += 2) EvSpawn(&p, PR_OFFSET, Fixed(in[i]), Fixed(in[i + 1]), (i - 2) / 2);
    }
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        const char *prefix = (el >= 6 && memcmp(E, "Start:", 6) == 0) ? "Start:" : "Moved to:";
        double x, y, gx, gy;
        if (!ScanPoint(E, el, prefix, &x, &y)) { x = y = 0; }
        if (ParseTake(&p, e)) { EvAct(&p, PR_AT, Fixed(x), Fixed(y), e); continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, PRF_POINT, e, Fixed(x), Fixed(y), e, NULL); break; }
        if (ScanPoint(A, al, prefix, &gx, &gy)) {
            if (fabs(gx - x) < 1e-9 && fabs(gy - y) < 1e-9)
                snprintf(note, sizeof(note), "Right point, wrong text: print() must write exactly %.*s", el - (int)strlen(prefix) - 1,
                         E + strlen(prefix) + 1);
            else
                snprintf(note, sizeof(note), "The point should be at %.*s", el - (int)strlen(prefix) - 1, E + strlen(prefix) + 1);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, PRF_POINT, e, Fixed(gx), Fixed(gy), e, note);
            if (f) { snprintf(f->text, EV_TEXT, "%d %d", Fixed(x), Fixed(y)); }
        } else {
            ParseFail(&p, FAIL_GARBLED, PRF_GARBLED, e, 0, 0, e, "The rover can't read this point");
        }
        break;
    }
    ParseEnd(&p, PRF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_ARRIVE };

static struct {
    const EventList *ev;
    Bot    bot;
    Cues   cues;
    Pops   pops;
    float  time;
    double sx, sy;                       // the start from the input
    double ox[PMAX], oy[PMAX];           // the offsets from the input
    int    moves;
    double path[PMAX + 1][2];            // where the points should be
    double minX, maxX, minY, maxY, scale;
    int    at;                           // printed points so far
    double shownX, shownY;               // where the program says the rover is
    char   label[48];
    bool   bad, garbled, placed;
    double wantX, wantY;
    double trail[PMAX + 2][2];
    int    trails;
    int    arrow;                        // the offset arrow being driven (-1 none)
    float  arrowT;
} P;

static Vector3 World(double x, double y)
{
    return V3((float)((x - (P.minX + P.maxX) / 2) * P.scale), 0, (float)(-(y - (P.minY + P.maxY) / 2) * P.scale));
}

static void Layout(void)
{
    P.path[0][0] = P.sx;
    P.path[0][1] = P.sy;
    for (int i = 0; i < P.moves; i++) {
        P.path[i + 1][0] = P.path[i][0] + P.ox[i];
        P.path[i + 1][1] = P.path[i][1] + P.oy[i];
    }
    P.minX = P.maxX = 0;
    P.minY = P.maxY = 0;
    for (int i = 0; i <= P.moves; i++) {
        P.minX = fmin(P.minX, P.path[i][0]); P.maxX = fmax(P.maxX, P.path[i][0]);
        P.minY = fmin(P.minY, P.path[i][1]); P.maxY = fmax(P.maxY, P.path[i][1]);
    }
    P.minX = floor(P.minX) - 1; P.maxX = ceil(P.maxX) + 1;
    P.minY = floor(P.minY) - 1; P.maxY = ceil(P.maxY) + 1;
    double w = P.maxX - P.minX, h = P.maxY - P.minY;
    P.scale = fmin(8.0 / w, 5.6 / h);
    if (P.scale > 1.6) P.scale = 1.6;
    if (P.scale < 0.25) P.scale = 0.25;
    BotInit(&P.bot, World(P.sx, P.sy), 0);
    P.bot.scale = 0.62f;
    P.bot.body = (Color){ 255, 214, 140, 255 };
    P.bot.accent = (Color){ 255, 150, 60, 255 };
    float ew = (float)(w * P.scale), eh = (float)(h * P.scale);
    ShotOrbit(V3(0, 0, 0.25f), fmaxf(7.5f, fmaxf(ew * 1.15f, eh * 1.55f)), 56, 0, 38, 0);
}

static void PointsReset(const EventList *ev)
{
    memset(&P, 0, sizeof(P));
    P.ev = ev;
    P.arrow = -1;
    P.scale = 1;
    P.maxX = P.maxY = 4;
    BotInit(&P.bot, V3(0, 0, 0), 0);
}

static float PointsEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == PR_START) { P.sx = Unfix(e->a); P.sy = Unfix(e->b); P.moves = 0; Layout(); }
            if (e->code == PR_OFFSET && P.moves < PMAX) { P.ox[P.moves] = Unfix(e->a); P.oy[P.moves] = Unfix(e->b); P.moves++; Layout(); }
            if (e->code == PR_OFFSET || e->code == PR_START) { P.shownX = P.sx; P.shownY = P.sy; }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
        case EV_FAIL: {
            if (e->type == EV_FAIL && e->code != PRF_POINT) {
                P.garbled = true;
                StrCopy(P.label, sizeof(P.label), P.ev->actual[0] ? P.ev->actual : "???");
                BotMood(&P.bot, MOOD_CONFUSED);
                BotShakeHead(&P.bot);
                PlaybackShake(0.15f);
                return 2.4f;
            }
            double x = Unfix(e->a), y = Unfix(e->b);
            int idx = (int)e->value;                 // 0 = start, then the moves
            P.bad = (e->type == EV_FAIL);
            if (P.bad) {
                int wx = 0, wy = 0;
                if (sscanf(e->text, "%d %d", &wx, &wy) == 2) { P.wantX = Unfix(wx); P.wantY = Unfix(wy); }
                if (P.ev->fail == FAIL_MISSING) { P.bad = false; P.garbled = true; StrCopy(P.label, sizeof(P.label), "(nothing printed)"); return 2.4f; }
            }
            // clamp far-away (wrong) points to just beyond the floor
            double cx = fmax(P.minX - 0.8, fmin(P.maxX + 0.8, x)), cy = fmax(P.minY - 0.8, fmin(P.maxY + 0.8, y));
            const char *txt = P.bad ? P.ev->actual : NULL;
            if (idx == 0) {
                P.bot.pos = World(cx, cy);
                BotMoveTo(&P.bot, World(cx, cy), 0.3f);
                P.placed = true;
                FxSparkle(P.bot.pos, 16, (Color){ 255, 214, 140, 255 });
                if (P.bad) CueAt(&P.cues, P.time + 0.3f, DO_CHEER, -1, 0, V3(0, 0, 0), 0);
            } else {
                P.arrow = idx - 1;
                P.arrowT = 0;
                CueAt(&P.cues, P.time + 0.45f, DO_ARRIVE, idx, 0, World(cx, cy), 0.9f);
            }
            P.shownX = x;
            P.shownY = y;
            if (txt) StrCopy(P.label, sizeof(P.label), strstr(txt, "(") ? strstr(txt, "(") : txt);
            else snprintf(P.label, sizeof(P.label), "(%g,%g)", x, y);
            if (P.trails < PMAX + 2) { P.trail[P.trails][0] = cx; P.trail[P.trails][1] = cy; P.trails++; }
            return P.bad ? 2.8f : (idx == 0 ? 0.9f : 1.6f);
        }
        case EV_CRASH:
            BotPowerDown(&P.bot);
            PlaybackShake(0.2f);
            return 2.4f;
        case EV_WIN:
            CueAt(&P.cues, P.time + 0.3f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void PointsUpdate(float dt)
{
    P.time += dt;
    P.arrowT += dt;
    BotUpdate(&P.bot, dt);
    PopsUpdate(&P.pops, dt);
    Cue c;
    while (CueDue(&P.cues, P.time, &c)) {
        if (c.what == DO_ARRIVE) {
            BotMoveTo(&P.bot, c.v, c.f);
            if (P.bad) {
                CueAt(&P.cues, P.time + c.f, DO_CHEER, -1, 0, V3(0, 0, 0), 0);
            }
        } else if (c.what == DO_CHEER && c.a < 0) {          // arrived at the wrong place
            BotMood(&P.bot, MOOD_SAD);
            BotShakeHead(&P.bot);
            FxSparks(V3(P.bot.pos.x, 0.4f, P.bot.pos.z), 30, (Color){ 255, 100, 90, 255 });
            PlaybackShake(0.2f);
        } else if (c.what == DO_CHEER) {
            BotFace(&P.bot, 0);
            BotCheer(&P.bot);
            FxConfetti(V3(P.bot.pos.x, 1.2f, P.bot.pos.z), PlaybackFinale() ? 150 : 70, 0.7f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static void PointsDraw(void)
{
    float w = (float)((P.maxX - P.minX) * P.scale), h = (float)((P.maxY - P.minY) * P.scale);
    KitStage(V3(0, 0, 0), w / 2 + 0.8f, h / 2 + 0.8f);
    // the grid: a line at every whole number, the axes brighter
    for (int x = (int)P.minX; x <= (int)P.maxX; x++) {
        Vector3 a = World(x, P.minY);
        bool axis = (x == 0);
        Part(MESH_CUBE, V3(a.x, 0.005f, 0), V3(axis ? 0.05f : 0.02f, 0.01f, h), axis ? (Color){ 255, 150, 60, 255 } : (Color){ 64, 72, 96, 255 }, axis ? 0.3f : 0);
    }
    for (int y = (int)P.minY; y <= (int)P.maxY; y++) {
        Vector3 a = World(P.minX, y);
        bool axis = (y == 0);
        Part(MESH_CUBE, V3(0, 0.005f, a.z), V3(w, 0.01f, axis ? 0.05f : 0.02f), axis ? (Color){ 255, 150, 60, 255 } : (Color){ 64, 72, 96, 255 }, axis ? 0.3f : 0);
    }
    // the offset arrows from the input (the one being driven glows)
    for (int i = 0; i < P.moves; i++) {
        Vector3 a = World(P.path[i][0], P.path[i][1]), b = World(P.path[i + 1][0], P.path[i + 1][1]);
        bool now = (i == P.arrow);
        float grow = now ? Clamp01(P.arrowT * 2.5f) : (i < P.arrow ? 1.0f : 0.0f);
        KitArc(V3(a.x, 0.08f, a.z), V3(b.x, 0.08f, b.z), 0.5f, grow, now ? (Color){ 255, 170, 70, 255 } : (Color){ 150, 120, 80, 255 }, now ? 0.8f : 0.1f);
    }
    for (int i = 0; i < P.trails; i++) {
        Vector3 t = World(P.trail[i][0], P.trail[i][1]);
        Part(MESH_CYLINDER, V3(t.x, 0.02f, t.z), V3(0.22f, 0.03f, 0.22f), (Color){ 255, 200, 120, 255 }, 0.6f);
    }
    if (P.bad) {          // where it should have gone
        Vector3 t = World(P.wantX, P.wantY);
        float k = Pulse(P.time, 6);
        Part(MESH_CYLINDER, V3(t.x, 0.03f, t.z), V3(0.5f + 0.15f * k, 0.03f, 0.5f + 0.15f * k), (Color){ 80, 220, 130, 255 }, 0.5f + 0.4f * k);
        Label3D(V3(t.x, 0.5f, t.z), TextFormat("should be (%g,%g)", P.wantX, P.wantY), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (P.placed || (P.ev && P.ev->preview)) BotDraw(&P.bot);

    // the axes' numbers and the rover's own (x,y)
    int step = (P.maxX - P.minX) > 12 ? 2 : 1;
    for (int x = (int)P.minX + 1; x < (int)P.maxX; x += step) {
        Vector3 a = World(x, 0);
        Label3D(V3(a.x, 0.05f, a.z + 0.25f), TextFormat("%d", x), 12, (Color){ 255, 190, 120, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    for (int y = (int)P.minY + 1; y < (int)P.maxY; y += step) {
        if (y == 0) continue;
        Vector3 a = World(0, y);
        Label3D(V3(a.x - 0.25f, 0.05f, a.z), TextFormat("%d", y), 12, (Color){ 255, 190, 120, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    Vector3 xe = World(P.maxX, 0), ye = World(0, P.maxY);
    Label3D(V3(xe.x - 0.2f, 0.1f, xe.z - 0.25f), "x", 16, (Color){ 255, 170, 80, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    Label3D(V3(ye.x + 0.25f, 0.1f, ye.z + 0.2f), "y", 16, (Color){ 255, 170, 80, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (P.placed || P.garbled) {
        Color fg = (P.bad || P.garbled) ? WHITE : LBL_GOLD, bg = (P.bad || P.garbled) ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        Label3D(V3(P.bot.pos.x, 1.25f, P.bot.pos.z), TextFormat("p = %s", P.label), 16, fg, bg, LABEL_CODE);
    } else if (P.ev && P.ev->preview) {
        Label3D(V3(P.bot.pos.x, 1.25f, P.bot.pos.z), TextFormat("Point p(%g, %g)", P.sx, P.sy), 15, LBL_SOFT, LBL_DIM, LABEL_CODE);
    }
    PopsDraw(&P.pops);
}

const LevelDef LEVEL_POINTS = {
    "cpp-objects", "Point Rover", "The rover is your Point object: it goes wherever print() says it is.",
    { 255, 170, 80, 255 },
    ParsePoints, PointsReset, PointsEvent, PointsUpdate, PointsDraw, NULL,
};
