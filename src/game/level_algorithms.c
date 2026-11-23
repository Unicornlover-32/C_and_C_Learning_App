// Score Cleaner -> "Score Sorter"
//
// The scores are crates on a belt. "Removed: N" opens trapdoors under the negative ones
// (erase + remove_if) and they drop away. "Sorted:" slides the rest into the printed
// order. "Highest" and "Lowest" light the two ends; "Passed:" lights every crate of 50
// or more behind a pass line. "No valid scores" leaves the belt empty. A crate in the
// wrong place, or a wrong count, shows in red; an ascending sort is named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { SS_VAL = 1, SS_REMOVED, SS_SORTED, SS_ORDER, SS_HIGH, SS_LOW, SS_PASSED, SS_NONE };
enum { SSF_REMOVED = 1, SSF_SORTED, SSF_HIGH, SSF_LOW, SSF_PASSED, SSF_NONE, SSF_GARBLED, SSF_EXTRA };

#define SMAX 12

// ---------------------------------------------------------------- the parser

static void ParseScores(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[101];
    int n = InputInts(run->input, in, 101), count = n > 0 ? in[0] : 0;
    if (count < 0) count = 0;
    if (count > n - 1) count = n > 0 ? n - 1 : 0;
    for (int i = 0; i < count && i < SMAX; i++) EvSpawn(&p, SS_VAL, i, 0, in[1 + i]);
    if (ParsePreview(&p)) return;

    char note[140];
    static const char *const keys[4] = { "Removed:", "Highest:", "Lowest:", "Passed:" };
    static const int acts[4] = { SS_REMOVED, SS_HIGH, SS_LOW, SS_PASSED }, fails[4] = { SSF_REMOVED, SSF_HIGH, SSF_LOW, SSF_PASSED };
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (LineIs(E, el, "No valid scores")) {
            if (ParseTake(&p, e)) { EvAct(&p, SS_NONE, 0, 0, 0); continue; }
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, SSF_NONE, e, 0, 0, 0, "Every score was negative: print No valid scores");
            break;
        }
        int k = -1;
        for (int i = 0; i < 4; i++) if (ScanAfter(E, el, keys[i], &want)) k = i;
        if (k >= 0) {
            if (ParseTake(&p, e)) { EvAct(&p, acts[k], 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, fails[k], e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, keys[k], &got)) {
                if (k == 0) snprintf(note, sizeof(note), "%d scores are negative: remove_if with isNegative, then erase", want);
                else if (k == 3) snprintf(note, sizeof(note), "Count the scores of 50 or more (>= 50): %d", want);
                else snprintf(note, sizeof(note), "%s should be %d", k == 1 ? "Highest" : "Lowest", want);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, fails[k], e, 0, 0, got, note);
                if (f) snprintf(f->text, EV_TEXT, "%d", want);
            } else ParseFail(&p, FAIL_GARBLED, SSF_GARBLED, e, 0, 0, 0, "The label doesn't match");
            break;
        }
        int wl[101], wn = 0, gl[101], gn = 0;
        if (ScanAfterInts(E, el, "Sorted:", wl, 101, &wn)) {
            if (wn > 101) wn = 101;
            if (ParseTake(&p, e)) { EvAct(&p, SS_SORTED, 0, 0, 0); for (int i = 0; i < wn && i < SMAX; i++) EvAct(&p, SS_ORDER, i, 0, wl[i]); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, SSF_SORTED, e, 0, 0, 0, NULL); break; }
            if (!ScanAfterInts(A, al, "Sorted:", gl, 101, &gn)) { ParseFail(&p, FAIL_GARBLED, SSF_GARBLED, e, 0, 0, 0, "Expected Sorted: and the scores"); break; }
            if (gn > 101) gn = 101;
            ParseShow(&p);
            EvAct(&p, SS_SORTED, 0, 0, 0);
            for (int i = 0; i < gn && i < SMAX; i++) EvAct(&p, SS_ORDER, i, 0, gl[i]);
            bool asc = gn == wn && gn > 1;
            for (int i = 0; asc && i < gn; i++) asc = gl[i] == wl[wn - 1 - i];
            int bad = 0;
            while (bad < gn && bad < wn && gl[bad] == wl[bad]) bad++;
            if (asc) snprintf(note, sizeof(note), "That's lowest first: the comparator descending should return a > b");
            else snprintf(note, sizeof(note), "Highest first, negatives removed: %.*s", el - 8, E + 8);
            ParseFail(&p, FAIL_WRONG, SSF_SORTED, e, bad, 0, 0, note);
            break;
        }
        ParseTake(&p, e);
    }
    ParseEnd(&p, SSF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_DROP, DO_CHEER, DO_BAD };

typedef struct { int value; Vector3 pos, to; bool gone; float fall; int slot; int mark; } Crate;   // mark: 0, 1 high, 2 low, 3 passed, 4 wrong

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    Crate c[SMAX];
    int   n;
    int   order[SMAX], orderN;      // the printed sort
    bool  sorted, none, noneBad;
    int   removed, passed, high, low;
    int   labelsBad;                // which readout is wrong (fail code)
    char  want[96];
    float doors;
} S;

static float SlotX(int i, int n) { float g = 8.0f / (n + 1); if (g > 0.8f) g = 0.8f; return (i - (n - 1) * 0.5f) * g; }

static void ScoresReset(const EventList *ev)
{
    memset(&S, 0, sizeof(S));
    S.ev = ev;
    S.high = S.low = S.removed = S.passed = -999;
    ShotOrbit(V3(0, 0.9f, 0.2f), 9.4f, 28, 0, 38, 0);
}

static void ApplyOrder(void)
{
    bool used[SMAX] = { false };
    for (int k = 0; k < S.orderN; k++)
        for (int i = 0; i < S.n; i++)
            if (!used[i] && !S.c[i].gone && S.c[i].value == S.order[k]) { used[i] = true; S.c[i].slot = k; S.c[i].to = V3(SlotX(k, S.orderN), BELT_Y, 0); break; }
}

static float ScoresEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (S.n < SMAX) {
                Crate *c = &S.c[S.n];
                c->value = (int)e->value;
                c->slot = S.n;
                S.n++;
                for (int i = 0; i < S.n; i++) S.c[i].pos = S.c[i].to = V3(SlotX(i, S.n), BELT_Y, 0);
            }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            switch (e->code) {
                case SS_REMOVED:
                    S.removed = (int)e->value;
                    S.doors = 1;
                    for (int i = 0; i < S.n; i++) if (S.c[i].value < 0) CueAt(&S.cues, S.time + 0.3f, DO_DROP, i, 0, V3(0, 0, 0), 0);
                    return 1.5f;
                case SS_SORTED: S.orderN = 0; S.sorted = true; return 0.05f;
                case SS_ORDER:
                    if (S.orderN < SMAX) S.order[S.orderN++] = (int)e->value;
                    ApplyOrder();
                    return 0.12f;
                case SS_HIGH: S.high = (int)e->value; if (S.orderN) for (int i = 0; i < S.n; i++) if (!S.c[i].gone && S.c[i].slot == 0) S.c[i].mark = 1; return 0.9f;
                case SS_LOW:  S.low = (int)e->value; if (S.orderN) for (int i = 0; i < S.n; i++) if (!S.c[i].gone && S.c[i].slot == S.orderN - 1) S.c[i].mark = 2; return 0.9f;
                case SS_PASSED:
                    S.passed = (int)e->value;
                    for (int i = 0; i < S.n; i++) if (!S.c[i].gone && S.c[i].value >= 50 && !S.c[i].mark) S.c[i].mark = 3;
                    return 1.2f;
                case SS_NONE: S.none = true; return 1.4f;
            }
            return 0;
        case EV_FAIL:
            S.labelsBad = e->code;
            StrCopy(S.want, sizeof(S.want), e->text);
            if (e->code == SSF_REMOVED) {
                S.removed = (int)e->value;
                for (int i = 0; i < S.n; i++) if (S.c[i].value < 0) CueAt(&S.cues, S.time + 0.3f, DO_DROP, i, 0, V3(0, 0, 0), 0);
            }
            if (e->code == SSF_HIGH) S.high = (int)e->value;
            if (e->code == SSF_LOW) S.low = (int)e->value;
            if (e->code == SSF_PASSED) S.passed = (int)e->value;
            if (e->code == SSF_NONE) S.noneBad = true;
            if (e->code == SSF_SORTED && S.ev) {
                StrCopy(S.want, sizeof(S.want), S.ev->expected);
                for (int i = 0; i < S.n; i++) if (!S.c[i].gone && S.c[i].slot >= e->a) S.c[i].mark = 4;
            }
            CueAt(&S.cues, S.time + 0.5f, DO_BAD, 0, 0, V3(0, 1.6f, 0), 0);
            return 2.8f;
        case EV_CRASH:
            CueAt(&S.cues, S.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.6f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&S.cues, S.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void ScoresUpdate(float dt)
{
    S.time += dt;
    S.doors = Smooth(S.doors, 0, 1.5f, dt);
    for (int i = 0; i < S.n; i++) {
        Crate *c = &S.c[i];
        if (c->gone) { c->fall += dt; c->pos.y -= dt * 3.0f * c->fall * 2; continue; }
        c->pos = LerpV3(c->pos, c->to, 1 - expf(-6 * dt));
    }
    Cue q;
    while (CueDue(&S.cues, S.time, &q)) {
        if (q.what == DO_DROP) { S.c[q.a].gone = true; FxDust(V3(S.c[q.a].pos.x, BELT_Y, 0), 6); }
        else if (q.what == DO_BAD) { FxSparks(q.v, 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0, 2.0f, 0), PlaybackFinale() ? 150 : 70, 3.0f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void ScoresDraw(void)
{
    KitStage(V3(0, 0, 0.2f), 5.0f, 1.8f);
    KitBelt(V3(-4.6f, 0, 0), 9.2f, 0);
    // trapdoors under the negative scores
    for (int i = 0; i < S.n; i++)
        if (S.c[i].value < 0 && !S.sorted) Part(MESH_CUBE, V3(SlotX(S.c[i].slot, S.n), BELT_Y + 0.01f, 0), V3(0.6f, 0.02f, 0.62f), (Color){ 120, 40, 40, 255 }, 0.3f * S.doors);
    // the pass line at 50
    Part(MESH_CUBE, V3(0, 0.02f, 1.1f), V3(9.0f, 0.03f, 0.05f), (Color){ 80, 220, 130, 255 }, 0.5f);
    for (int i = 0; i < S.n; i++) {
        const Crate *c = &S.c[i];
        if (c->gone && c->pos.y < -2) continue;
        Color col = c->value < 0 ? (Color){ 200, 90, 90, 255 } : (Color){ 120, 170, 240, 255 };
        float glow = 0.1f;
        if (c->mark == 1) { col = (Color){ 255, 205, 70, 255 }; glow = 0.6f; }
        if (c->mark == 2) { col = (Color){ 120, 220, 255, 255 }; glow = 0.5f; }
        if (c->mark == 3) { col = (Color){ 110, 220, 150, 255 }; glow = 0.4f; }
        if (c->mark == 4) { col = (Color){ 230, 80, 80, 255 }; glow = 0.5f; }
        KitCrate(c->pos, 0.5f, col, glow, 0);
    }

    for (int i = 0; i < S.n; i++) {
        const Crate *c = &S.c[i];
        if (c->gone && c->pos.y < 0) continue;
        Label3D(V3(c->pos.x, c->pos.y + 0.78f, 0.1f), TextFormat("%d", c->value), 14, c->mark == 4 ? WHITE : LBL_GOLD,
                c->mark == 4 ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    }
    float y = 2.5f;
    struct { const char *name; int v; int code; } rows[4] = { { "Removed", S.removed, SSF_REMOVED }, { "Highest", S.high, SSF_HIGH },
                                                              { "Lowest", S.low, SSF_LOW }, { "Passed", S.passed, SSF_PASSED } };
    for (int r = 0; r < 4; r++) {
        if (rows[r].v == -999) continue;
        bool bad = S.labelsBad == rows[r].code;
        Label3D(V3(-3.6f + r * 2.4f, y, -0.5f), TextFormat("%s: %d", rows[r].name, rows[r].v), 14, bad ? WHITE : LBL_GOOD,
                bad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        if (bad && S.want[0]) Label3D(V3(-3.6f + r * 2.4f, y + 0.42f, -0.5f), TextFormat("should be %s", S.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (S.sorted) Label3D(V3(0, 1.75f, 0), "sort(..., descending)", 13, (Color){ 150, 210, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(4.3f, 0.05f, 1.1f), ">= 50", 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (S.none || S.noneBad)
        Label3D(V3(0, 1.3f, 0.3f), S.noneBad && S.ev ? S.ev->actual : "No valid scores", 16, WHITE, S.noneBad ? (Color){ 200, 50, 55, 235 } : (Color){ 150, 90, 40, 235 }, LABEL_CODE);
    if (S.labelsBad == SSF_SORTED && S.want[0]) Label3D(V3(0, 3.1f, 0), TextFormat("should be: %s", S.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_ALGORITHMS = {
    "cpp-algorithms", "Score Sorter", "remove_if drops the bad scores, sort lines up the rest - all from your output.",
    { 120, 220, 255, 255 },
    ParseScores, ScoresReset, ScoresEvent, ScoresUpdate, ScoresDraw, NULL,
};
