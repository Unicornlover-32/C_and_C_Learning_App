// The Assembly Line: the 3D scene for the Factory Contracts and the Mega Factory boss,
// whose programs print all sorts of things, so this level reads any output line by line.
//
// The test's input is a crate that rides into the machine - the student's program. Every
// line the program prints comes out of the machine as a strip of tape and lands on the
// output board, lit green when it is exactly the line the test expects. The first line
// that differs lands red, with the line it should have been underneath; missing lines,
// extra lines, crashes (the machine smokes) and timeouts each look different.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { FC_INPUT = 1, FC_LINE };
enum { FCF_LINE = 1, FCF_EXTRA };

#define ROWS  6            // lines on the board at once (older ones scroll off the top)
#define KEEP  64

// ---------------------------------------------------------------- the parser

static void CopyLine(char *dst, const char *s, int len)
{
    int k = len < EV_TEXT - 1 ? len : EV_TEXT - 1;
    if (s && k > 0) memcpy(dst, s, (size_t)k);
    dst[k > 0 ? k : 0] = '\0';
}

static void ParseFactory(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    // the crate: the input's first line, and how many lines the test expects
    const char *in = run->input ? run->input : "";
    int n = 0, inLines = 0;
    for (const char *c = in; *c && inLines < 1000; c++) if (*c == '\n') inLines++;
    if (in[0] && in[strlen(in) - 1] != '\n') inLines++;
    GameEvent *g = EvSpawn(&p, FC_INPUT, p.exp.count, inLines, 0);
    if (g) {
        while (in[n] && in[n] != '\n' && in[n] != '\r' && n < 40) n++;
        char first[EV_TEXT];
        CopyLine(first, in, n);
        EvText(g, first[0] ? first : "(no input)");
    }
    if (ParsePreview(&p)) return;

    char note[FAIL_TEXT];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ParseTake(&p, e)) {
            GameEvent *a = EvAct(&p, FC_LINE, e, 0, 0);
            if (a) { char t[EV_TEXT]; CopyLine(t, E, el); EvText(a, t); }
            continue;
        }
        if (!A) snprintf(note, sizeof(note), "Your program stopped after %d line%s - line %d is missing", e, e == 1 ? "" : "s", e + 1);
        else snprintf(note, sizeof(note), "Line %d isn't what the test expects", e + 1);
        ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, FCF_LINE, e, e, 0, 0, note);
    }
    ParseEnd(&p, FCF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_LAND, DO_BAD, DO_SMOKE, DO_CHEER };
enum { ROW_GOOD = 1, ROW_BAD, ROW_STOP };

typedef struct { char text[EV_TEXT]; int state; float t; } Row;

static struct {
    const EventList *ev;
    Cues  cues;
    float time, scroll, busy, light;
    char  input[EV_TEXT];
    int   expLines;
    Row   rows[KEEP];
    int   count, good;
    float crate;              // the input crate riding in: 0 .. 1
    bool  started, smoke, won, failed;
    char  want[FAIL_TEXT];
} A;

#define BOARD_X   2.55f
#define BOARD_W   5.5f
#define BOARD_Z  -1.0f
#define ROW_TOP   3.25f
#define ROW_GAP   0.47f
#define MACHINE  (Vector3){ -1.75f, 0, 0.55f }

static void FactoryReset(const EventList *ev)
{
    memset(&A, 0, sizeof(A));
    A.ev = ev;
    A.light = 0.4f;
    ShotOrbit(V3(0.55f, 1.5f, -0.1f), 11.3f, 15, 0, 40, 0);
}

static float Lead(void)               // the crate rides in before the first line comes out
{
    if (A.started) return 0;
    A.started = true;
    return 1.0f;
}

static float LineTime(void)
{
    int lines = A.ev ? A.ev->printedLines : 0;
    return lines > 10 ? fmaxf(0.12f, 4.5f / lines) : 0.5f;
}

static Row *AddRow(const char *text, int state, float delay)
{
    if (A.count >= KEEP) {               // keep the newest: drop the oldest
        memmove(&A.rows[0], &A.rows[1], sizeof(Row) * (KEEP - 1));
        A.count--;
    }
    Row *r = &A.rows[A.count++];
    StrCopy(r->text, EV_TEXT, text);
    r->state = state;
    r->t = -delay;
    return r;
}

static float FactoryEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            StrCopy(A.input, sizeof(A.input), e->text);
            A.expLines = e->a;
            return 0;
        case EV_PRINT:
            return Lead();
        case EV_ACT: {
            float lead = Lead();
            AddRow(e->text, ROW_GOOD, lead);
            A.busy = lead + 0.4f;
            CueAt(&A.cues, A.time + lead + 0.3f, DO_LAND, A.count - 1, 0, V3(0, 0, 0), 0);
            return lead + LineTime();
        }
        case EV_FAIL: {
            float lead = Lead();
            const EventList *ev = A.ev;
            const char *got = ev && ev->actual[0] ? ev->actual : "(nothing more)";
            AddRow(got, ev && ev->actual[0] ? ROW_BAD : ROW_STOP, lead);
            if (ev) StrCopy(A.want, sizeof(A.want), ev->fail == FAIL_EXTRA ? "(the output should end here)" : ev->expected);
            A.busy = lead + 0.4f;
            A.failed = true;
            CueAt(&A.cues, A.time + lead + 0.35f, DO_BAD, 0, 0, V3(0, 0, 0), 0);
            return lead + 2.8f;
        }
        case EV_CRASH: {
            float lead = Lead();
            bool timeout = A.ev && A.ev->fail == FAIL_TIMEOUT;
            AddRow(timeout ? "(stopped - it ran too long)" : "(the program crashed)", ROW_STOP, lead);
            A.failed = true;
            CueAt(&A.cues, A.time + lead + 0.1f, DO_SMOKE, 0, 0, V3(0, 0, 0), 0);
            return lead + 2.6f;
        }
        case EV_WIN:
            A.won = true;
            CueAt(&A.cues, A.time + 0.25f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static float RowY(int i)              // where row i sits on the board (newest at the bottom)
{
    int first = A.count > ROWS ? A.count - ROWS : 0;
    return ROW_TOP - (i - first) * ROW_GAP;
}

static void FactoryUpdate(float dt)
{
    A.time += dt;
    A.busy = fmaxf(0, A.busy - dt);
    if (A.started) A.crate = fminf(1, A.crate + dt * 1.1f);
    if (A.busy > 0 || (A.started && A.crate < 1)) A.scroll += dt * 1.4f;
    for (int i = 0; i < A.count; i++) A.rows[i].t += dt;
    float want = A.won ? 1.0f : A.failed ? 0.9f : A.busy > 0 ? 0.8f : 0.4f;
    A.light += (want - A.light) * fminf(1, dt * 6);
    if (A.smoke && fmodf(A.time, 0.25f) < dt) FxDust(V3(MACHINE.x, 2.2f, MACHINE.z), 2);

    Cue c;
    while (CueDue(&A.cues, A.time, &c)) {
        if (c.what == DO_LAND) {
            if (c.a >= 0 && c.a < A.count) FxSparkle(V3(BOARD_X + BOARD_W / 2 - 0.35f, RowY(c.a), BOARD_Z + 0.2f), 6, COL3_GOOD);
        } else if (c.what == DO_BAD) {
            FxSparks(V3(BOARD_X + BOARD_W / 2 - 0.35f, RowY(A.count - 1), BOARD_Z + 0.25f), 34, COL3_BAD);
            PlaybackShake(0.22f);
        } else if (c.what == DO_SMOKE) {
            A.smoke = true;
            FxSparks(V3(MACHINE.x, 1.6f, MACHINE.z + 0.7f), 40, (Color){ 255, 150, 80, 255 });
            PlaybackShake(0.3f);
        } else if (c.what == DO_CHEER) {
            bool finale = PlaybackFinale();
            FxConfetti(V3(BOARD_X, 3.4f, BOARD_Z + 0.6f), finale ? 160 : 70, 2.6f);
            if (finale) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static Color RowColor(int state)
{
    if (state == ROW_GOOD) return (Color){ 34, 96, 62, 255 };
    if (state == ROW_BAD) return (Color){ 150, 38, 44, 255 };
    return (Color){ 120, 70, 30, 255 };
}

static Color LampColor(int state)
{
    return state == ROW_GOOD ? COL3_GOOD : state == ROW_BAD ? COL3_BAD : COL3_WARN;
}

static void FactoryDraw(void)
{
    const EventList *ev = A.ev;
    Vector3 m = MACHINE;
    KitStage(V3(0, 0, 0), 5.8f, 2.3f);

    // the input belt and its crate, then the machine
    KitBelt(V3(-5.5f, 0, m.z), 3.2f, A.scroll);
    KitBelt(V3(m.x + 0.7f, 0, m.z), 1.2f, A.scroll);
    Color lamp = A.smoke ? COL3_BAD : A.won ? COL3_GOOD : (Color){ 200, 170, 255, 255 };
    KitMachine(m, V3(1.5f, 2.0f, 1.3f), (Color){ 92, 78, 140, 255 }, lamp, A.light, A.busy > 0 ? A.time : 0);
    if (A.crate < 1) {
        float k = EaseInOutQuad(A.crate);
        KitCrate(V3(Lerpf(-4.7f, m.x, k), BELT_Y, m.z), 0.62f, (Color){ 190, 140, 90, 255 }, 0, 0);
    }

    // the output board
    float boardY = 1.95f, boardH = 3.3f;
    Part(MESH_CUBE, V3(BOARD_X, boardY, BOARD_Z - 0.06f), V3(BOARD_W + 0.2f, boardH + 0.2f, 0.12f), (Color){ 52, 58, 78, 255 }, 0);
    Part(MESH_CUBE, V3(BOARD_X, boardY, BOARD_Z), V3(BOARD_W, boardH, 0.02f), (Color){ 18, 20, 30, 255 }, 0);
    for (int s = -1; s <= 1; s += 2)
        Part(MESH_CYLINDER, V3(BOARD_X + s * (BOARD_W / 2 - 0.3f), 0.15f, BOARD_Z - 0.1f), V3(0.12f, 0.3f, 0.12f), (Color){ 78, 86, 112, 255 }, 0);
    Part(MESH_CUBE, V3(BOARD_X, boardY + boardH / 2 + 0.12f, BOARD_Z + 0.02f), V3(BOARD_W, 0.05f, 0.04f),
         A.won ? COL3_GOOD : A.failed ? COL3_BAD : (Color){ 86, 156, 255, 255 }, 0.7f);

    int first = A.count > ROWS ? A.count - ROWS : 0;
    float left = BOARD_X - BOARD_W / 2 + 0.15f, width = BOARD_W - 0.3f;
    for (int i = first; i < A.count; i++) {
        const Row *r = &A.rows[i];
        if (r->t < 0) continue;
        float y = RowY(i);
        // the strip comes out of the machine, flies to its row and stretches out
        float k = EaseOutCubic(Clamp01(r->t / 0.32f));
        Vector3 from = V3(m.x + 1.6f, BELT_Y + 0.1f, m.z), to = V3(left + width / 2, y, BOARD_Z + 0.04f);
        Vector3 at = LerpV3(from, to, k);
        at.y += sinf(k * PI) * 0.5f;
        float w = Lerpf(0.5f, width, k), h = Lerpf(0.06f, 0.38f, k);
        float glow = 0.15f + 0.5f * (1 - Clamp01((r->t - 0.3f) / 0.5f));
        Part(MESH_CUBE, at, V3(w, h, 0.03f), RowColor(r->state), glow);
        if (k >= 1) {
            Part(MESH_SPHERE, V3(left + width - 0.2f, y, BOARD_Z + 0.08f), V3(0.18f, 0.18f, 0.1f), LampColor(r->state), 0.9f);
            Label3D(V3(left + 0.12f, y, BOARD_Z + 0.1f), r->text, 13, WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE | LABEL_LEFT);
        }
    }

    // labels
    if (A.crate < 0.75f)
        Label3D(V3(Lerpf(-4.7f, m.x, EaseInOutQuad(A.crate)), BELT_Y + 1.05f, m.z + 0.2f), A.input, 13, LBL_GOLD, LBL_DIM, LABEL_CODE);
    Label3D(V3(m.x, 2.55f, m.z + 0.7f), "your program", 14, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
    char head[48];
    if (!ev || ev->preview) snprintf(head, sizeof(head), "OUTPUT  -  %d line%s expected", A.expLines, A.expLines == 1 ? "" : "s");
    else {
        int got = 0;
        for (int i = 0; i < A.count; i++) if (A.rows[i].state == ROW_GOOD && A.rows[i].t >= 0.32f) got++;
        snprintf(head, sizeof(head), "OUTPUT  -  %d of %d lines right", got, A.expLines);
    }
    Label3D(V3(BOARD_X, boardY + boardH / 2 + 0.38f, BOARD_Z + 0.1f), head, 13, A.won ? LBL_GOOD : A.failed ? LBL_BAD : LBL_SOFT, LBL_DIM, LABEL_CODE);
    if (ev && ev->preview)
        Label3D(V3(BOARD_X, boardY, BOARD_Z + 0.1f), "your program's output appears here", 13, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (A.want[0] && A.count > 0 && A.rows[A.count - 1].t > 0.4f) {
        float y = RowY(A.count - 1) - ROW_GAP;
        Label3D(V3(left + 0.12f, y, BOARD_Z + 0.1f), "should be:", 12, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE | LABEL_LEFT);
        Label3D(V3(left + 1.55f, y, BOARD_Z + 0.1f), A.want, 13, LBL_GOOD, LBL_DIM, LABEL_CODE | LABEL_LEFT);
    }
    if (A.won) Label3D(V3(m.x, 3.1f, m.z + 0.7f), "SHIPPED!", 16, LBL_GOOD, LBL_DIM, LABEL_CODE | LABEL_BIG);
}

const LevelDef LEVEL_FACTORY = {
    "", "Assembly Line", "Every test is a crate: your program reads it and prints its answer line by line.",
    { 200, 170, 255, 255 },
    ParseFactory, FactoryReset, FactoryEvent, FactoryUpdate, FactoryDraw, NULL,
};
