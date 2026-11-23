// Grade Report -> "Sorting Gate"
//
// A crate stamped with the score from the input rides a conveyor past five chutes, A, B,
// C, D and F, each marked with its range. "Grade: X" picks the chute a pusher shoves the
// crate into, and the comment line lights the big sign over the chutes. A wrong letter
// sends the crate down the wrong chute (it flashes red, the right one blinks green); a
// wrong comment shows on the sign in red.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { GR_SCORE = 1, GR_GRADE, GR_COMMENT };
enum { GRF_CHUTE = 1, GRF_COMMENT, GRF_GARBLED, GRF_EXTRA };

static const char LETTERS[5] = { 'A', 'B', 'C', 'D', 'F' };
static const char *const RANGES[5] = { "90+", "80-89", "70-79", "60-69", "0-59" };

static int Chute(char c)
{
    for (int i = 0; i < 5; i++) if (LETTERS[i] == c) return i;
    return -1;
}

// ---------------------------------------------------------------- the parser

static void ParseGrades(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int score = 0;
    InputInts(run->input, &score, 1);
    EvSpawn(&p, GR_SCORE, 0, 0, score);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        bool grade = el == 8 && memcmp(E, "Grade: ", 7) == 0;
        if (ParseTake(&p, e)) {
            if (grade) EvAct(&p, GR_GRADE, Chute(E[7]), 0, score);
            else { GameEvent *g = EvAct(&p, GR_COMMENT, 0, 0, 0); if (g) { int n = el < EV_TEXT - 1 ? el : EV_TEXT - 1; memcpy(g->text, E, (size_t)n); g->text[n] = '\0'; } }
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, grade ? GRF_CHUTE : GRF_COMMENT, e, -1, grade ? Chute(E[7]) : 0, 0, NULL); break; }
        if (grade) {
            if (al >= 8 && memcmp(A, "Grade: ", 7) == 0) {
                int got = al == 8 ? Chute(A[7]) : -1;
                int wantChute = Chute(E[7]);
                const char *range = (wantChute >= 0 && wantChute < 5) ? RANGES[wantChute] : "?";
                snprintf(note, sizeof(note), "A score of %d is a%s %c (%s)", score, E[7] == 'A' || E[7] == 'F' ? "n" : "", E[7], range);
                ParseFail(&p, FAIL_WRONG, GRF_CHUTE, e, got, Chute(E[7]), score, note);
            } else {
                ParseFail(&p, FAIL_GARBLED, GRF_GARBLED, e, -1, Chute(E[7]), score, "Expected Grade: and one letter");
            }
        } else {
            snprintf(note, sizeof(note), "The comment should be \"%.*s\"", el > 60 ? 60 : el, E);
            ParseFail(&p, FAIL_WRONG, GRF_COMMENT, e, 0, 0, 0, note);
        }
        break;
    }
    ParseEnd(&p, GRF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_PUSH, DO_LAND, DO_CHEER, DO_SIGN };

static struct {
    const EventList *ev;
    Cues  cues;
    float time, scroll;
    int   score;
    Vector3 crate;               // where the crate is
    Vector3 from, to;
    float moveT, moveDur;
    int   chute, want;           // the chute it went to (-1 none), the right one
    bool  wrong, landed, stuck;
    float push;                  // the pusher's stroke 0..1
    char  sign[EV_TEXT];
    bool  signBad;
    char  signWant[64];
    float flash;
} G;

static float ChuteX(int i) { return -1.3f + i * 1.3f; }
static Vector3 ChutePos(int i) { return V3(ChuteX(i), 0, -1.25f); }

static void GradesReset(const EventList *ev)
{
    memset(&G, 0, sizeof(G));
    G.ev = ev;
    G.chute = G.want = -1;
    G.crate = G.from = G.to = V3(-4.3f, BELT_Y, 0);
    ShotOrbit(V3(0.4f, 0.9f, -0.3f), 9.2f, 30, 0, 38, 0);
}

static void MoveCrate(Vector3 to, float seconds)
{
    G.from = G.crate;
    G.to = to;
    G.moveT = 0;
    G.moveDur = seconds;
}

static float GradesEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN: G.score = (int)e->value; return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == GR_GRADE && e->a >= 0) {
                G.chute = G.want = e->a;
                MoveCrate(V3(ChuteX(e->a), BELT_Y, 0), 0.9f + e->a * 0.18f);
                CueAt(&G.cues, G.time + 1.0f + e->a * 0.18f, DO_PUSH, e->a, 0, V3(0, 0, 0), 0);
                return 2.3f + e->a * 0.18f;
            }
            if (e->code == GR_COMMENT) {
                StrCopy(G.sign, sizeof(G.sign), e->text);
                CueAt(&G.cues, G.time + 0.1f, DO_SIGN, 0, 0, V3(0, 0, 0), 0);
                return 1.3f;
            }
            return 0;
        case EV_FAIL:
            if (e->code == GRF_CHUTE && e->a >= 0) {
                G.chute = e->a;
                G.want = e->b;
                G.wrong = true;
                MoveCrate(V3(ChuteX(e->a), BELT_Y, 0), 0.9f + e->a * 0.18f);
                CueAt(&G.cues, G.time + 1.0f + e->a * 0.18f, DO_PUSH, e->a, 0, V3(0, 0, 0), 0);
                return 3.2f;
            }
            if (e->code == GRF_COMMENT) {
                StrCopy(G.sign, sizeof(G.sign), G.ev->actual[0] ? G.ev->actual : "(nothing)");
                G.signBad = true;
                const char *x = G.ev->expected;
                snprintf(G.signWant, sizeof(G.signWant), "should be: %.48s", x);
                CueAt(&G.cues, G.time + 0.1f, DO_SIGN, 1, 0, V3(0, 0, 0), 0);
                return 2.6f;
            }
            // a grade the sorter can't read: the crate jams at the start of the chutes
            G.want = e->b;
            G.stuck = true;
            MoveCrate(V3(-2.4f, BELT_Y, 0), 0.7f);
            CueAt(&G.cues, G.time + 0.75f, DO_LAND, -1, 0, V3(0, 0, 0), 0);
            return 2.6f;
        case EV_CRASH:
            G.stuck = true;
            FxSparks(V3(G.crate.x, 1.0f, 0), 30, (Color){ 255, 160, 70, 255 });
            PlaybackShake(0.2f);
            return 2.4f;
        case EV_WIN:
            CueAt(&G.cues, G.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void GradesUpdate(float dt)
{
    G.time += dt;
    G.flash += dt;
    bool moving = G.moveDur > 0 && G.moveT < G.moveDur;
    if (G.moveDur > 0) {
        G.moveT += dt;
        float k = EaseInOutQuad(G.moveT / G.moveDur);
        G.crate = LerpV3(G.from, G.to, k);
        if (G.landed == false && G.crate.z < -0.3f) G.crate.y = G.from.y + (G.to.y - G.from.y) * k + sinf(k * PI) * 0.4f;
    }
    if (moving && fabsf(G.to.z) < 0.01f) G.scroll += dt * 1.6f;
    if (G.push > 0) G.push = fmaxf(0, G.push - dt * 1.4f);
    Cue c;
    while (CueDue(&G.cues, G.time, &c)) {
        switch (c.what) {
            case DO_PUSH:
                G.push = 1;
                MoveCrate(V3(ChuteX(c.a), 0.62f, -1.25f), 0.55f);
                CueAt(&G.cues, G.time + 0.55f, DO_LAND, c.a, 0, V3(0, 0, 0), 0);
                break;
            case DO_LAND:
                G.landed = true;
                G.flash = 0;
                if (c.a < 0 || G.wrong) {
                    FxSparks(V3(G.crate.x, 1.1f, G.crate.z), 34, (Color){ 255, 100, 90, 255 });
                    PlaybackShake(0.22f);
                } else {
                    FxSparkle(V3(G.crate.x, 1.3f, G.crate.z), 16, (Color){ 140, 255, 180, 255 });
                }
                break;
            case DO_SIGN:
                G.flash = 0;
                if (c.a) { FxSparks(V3(0.65f, 2.6f, -1.6f), 30, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.18f); }
                break;
            case DO_CHEER:
                FxConfetti(V3(ChuteX(G.chute >= 0 ? G.chute : 2), 1.6f, -1.25f), PlaybackFinale() ? 150 : 70, 0.5f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

static void GradesDraw(void)
{
    KitStage(V3(0.2f, 0, -0.3f), 5.4f, 2.4f);
    KitBelt(V3(-4.9f, 0, 0), 9.8f, G.scroll);
    // the back wall with the comment sign
    Part(MESH_CUBE, V3(0.6f, 1.6f, -2.2f), V3(7.4f, 3.2f, 0.3f), (Color){ 50, 56, 76, 255 }, 0);
    Color signCol = G.signBad ? (Color){ 120, 30, 34, 255 } : G.sign[0] ? (Color){ 30, 80, 60, 255 } : (Color){ 24, 26, 36, 255 };
    KitPanel(V3(0.6f, 2.6f, -2.02f), 5.6f, 0.62f, signCol, G.sign[0] ? 0.12f + 0.3f * Clamp01(1 - G.flash) : 0);
    // the chutes, and the pusher arms on the near side of the belt
    for (int i = 0; i < 5; i++) {
        Color col = (Color){ 60, 100, 170, 255 };
        float glow = 0.05f;
        if (G.landed && i == G.chute) { col = G.wrong ? (Color){ 235, 90, 90, 255 } : (Color){ 80, 220, 130, 255 }; glow = 0.9f; }
        if (G.landed && G.wrong && i == G.want) { col = (Color){ 80, 220, 130, 255 }; glow = 0.3f + 0.6f * Pulse(G.time, 7); }
        if (G.stuck && i == G.want) { col = (Color){ 80, 220, 130, 255 }; glow = 0.3f + 0.6f * Pulse(G.time, 7); }
        KitChute(ChutePos(i), col, glow);
        float stroke = (i == G.chute) ? EaseOutQuad(G.push) : 0;
        Part(MESH_CUBE, V3(ChuteX(i), BELT_Y + 0.25f, 0.95f - stroke * 0.55f), V3(0.5f, 0.3f, 0.16f), (Color){ 90, 98, 124, 255 }, 0);
        Part(MESH_CUBE, V3(ChuteX(i), BELT_Y + 0.25f, 1.25f - stroke * 0.3f), V3(0.08f, 0.08f, 0.5f), (Color){ 150, 160, 186, 255 }, 0);
    }
    // the crate, its score stamped on it
    KitCrate(G.crate, 0.55f, (Color){ 190, 140, 90, 255 }, 0.0f, 0);

    for (int i = 0; i < 5; i++) {
        Vector3 c = ChutePos(i);
        bool hi = G.landed && (i == G.chute || (G.wrong && i == G.want));
        Label3D(V3(c.x, 1.95f, c.z), TextFormat("%c", LETTERS[i]), 22, hi ? WHITE : LBL_GOLD,
                hi ? (G.wrong && i == G.chute ? (Color){ 200, 50, 55, 235 } : (Color){ 40, 150, 80, 235 }) : LBL_DIM, LABEL_CODE | LABEL_BIG);
        Label3D(V3(c.x, 0.3f, c.z + 0.45f), RANGES[i], 13, LBL_SOFT, LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(G.crate.x, G.crate.y + 0.85f, G.crate.z), TextFormat("score %d", G.score), 15, LBL_GOLD, LBL_DIM, LABEL_CODE);
    if (G.sign[0]) Label3D(V3(0.6f, 2.6f, -1.95f), G.sign, 19, WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (G.signBad) Label3D(V3(0.6f, 2.05f, -1.9f), G.signWant, 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (G.landed && G.wrong && G.want >= 0)
        Label3D(V3(ChuteX(G.want), 2.45f, -1.25f), TextFormat("%d goes here", G.score), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (G.stuck) Label3D(V3(G.crate.x, G.crate.y + 1.3f, 0), "which chute?", 14, LBL_BAD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_GRADES = {
    "c-decisions-switch", "Sorting Gate", "The grade your if / else prints picks the chute the score's crate drops into.",
    { 120, 200, 255, 255 },
    ParseGrades, GradesReset, GradesEvent, GradesUpdate, GradesDraw, NULL,
};
