// Student Records -> "Crew Roster"
//
// Each student in the input is a struct: a crew card with two fields, .name and .score,
// dealt onto a table, with a bar behind it as tall as the score. "Top: Name score" lifts
// that card up with a crown (on a tie the first card wins). "Average:" draws a glowing line
// across the bars at the printed average. The wrong card lifts in red; an average that
// lost its decimals to integer division is named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { CR_CARD = 1, CR_TOP, CR_AVG };
enum { CRF_TOP = 1, CRF_AVG, CRF_GARBLED, CRF_EXTRA };

#define CMAX 8

// ---------------------------------------------------------------- the parser

static void ParseStructs(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    char names[20][32];
    int scores[20], n = 0, count = 0;
    const char *s = run->input ? run->input : "";
    int used = 0;
    if (sscanf(s, "%d%n", &count, &used) == 1) {
        s += used;
        while (n < count && n < 20) {
            if (sscanf(s, " %31s %d%n", names[n], &scores[n], &used) != 2) break;
            s += used;
            n++;
        }
    }
    for (int i = 0; i < n && i < CMAX; i++) { GameEvent *g = EvSpawn(&p, CR_CARD, i, 0, scores[i]); if (g) StrCopy(g->text, EV_TEXT, names[i]); }
    if (ParsePreview(&p)) return;

    char note[140];
    long sum = 0;
    for (int i = 0; i < n; i++) sum += scores[i];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        double want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        char wname[64], gname[64];
        int wscore, gscore;
        if (ScanAfterNumber(E, el, "Average:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, CR_AVG, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, CRF_AVG, e, 0, 0, 0, NULL); break; }
            if (ScanAfterNumber(A, al, "Average:", &got)) {
                if (n && fabs(got - (double)(sum / n)) < 0.001 && fabs(got - want) > 0.001)
                    snprintf(note, sizeof(note), "Integer division lost the decimals: divide by (double)n");
                else if (fabs(got - want) < 0.051) snprintf(note, sizeof(note), "Same value, wrong format: one decimal place (%%.1f)");
                else snprintf(note, sizeof(note), "The average should be %.*s", el - 9, E + 9);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, CRF_AVG, e, 0, 0, got, note);
                if (f) ScanAfterText(A, al, "Average:", f->text, EV_TEXT);
            } else {
                ParseFail(&p, FAIL_GARBLED, CRF_GARBLED, e, 0, 0, 0, "Expected Average: and a number");
            }
            break;
        }
        char tmp[96];
        if (ScanAfterText(E, el, "Top:", tmp, sizeof(tmp)) && sscanf(tmp, "%63s %d", wname, &wscore) == 2) {
            int at = -1;
            for (int i = 0; i < n && at < 0; i++) if (strcmp(names[i], wname) == 0 && scores[i] == wscore) at = i;
            if (ParseTake(&p, e)) { EvAct(&p, CR_TOP, at, 0, wscore); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, CRF_TOP, e, -1, at, 0, NULL); break; }
            if (ScanAfterText(A, al, "Top:", tmp, sizeof(tmp)) && sscanf(tmp, "%63s %d", gname, &gscore) == 2) {
                int gat = -1;
                for (int i = 0; i < n && gat < 0; i++) if (strcmp(names[i], gname) == 0) gat = i;
                if (gscore == wscore && gat > at) snprintf(note, sizeof(note), "A tie: the first one wins - only replace the top on a strictly higher score (>)");
                else snprintf(note, sizeof(note), "The top score is %s with %d", wname, wscore);
                ParseFail(&p, FAIL_WRONG, CRF_TOP, e, gat, at, gscore, note);
            } else {
                ParseFail(&p, FAIL_GARBLED, CRF_GARBLED, e, -1, at, 0, "Expected Top: and a name and a score");
            }
            break;
        }
        ParseTake(&p, e);
    }
    ParseEnd(&p, CRF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   n;
    char  name[CMAX][EV_TEXT];
    int   score[CMAX];
    float deal;
    int   top, right;                 // the lifted card, and the one that should be (-1 none)
    bool  topBad, avgShown, avgBad;
    float lift;
    double avg;
    char  avgText[EV_TEXT], want[EV_TEXT];
    float avgLine;
    int   maxScore;
} R;

static float CardX(int i) { float g = 8.0f / (R.n + 1); if (g > 1.6f) g = 1.6f; return (i - (R.n - 1) * 0.5f) * g; }
static float BarH(int s) { return 0.2f + 2.0f * (s < 0 ? 0 : s) / (float)(R.maxScore > 0 ? R.maxScore : 100); }

static void StructsReset(const EventList *ev)
{
    memset(&R, 0, sizeof(R));
    R.ev = ev;
    R.top = R.right = -1;
    R.maxScore = 100;
    ShotOrbit(V3(0, 1.0f, 0.2f), 9.4f, 30, 0, 38, 0);
}

static float StructsEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (R.n < CMAX) {
                StrCopy(R.name[R.n], EV_TEXT, e->text);
                R.score[R.n] = (int)e->value;
                if (R.score[R.n] > R.maxScore) R.maxScore = R.score[R.n];
                R.n++;
            }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == CR_TOP) { R.top = e->a; R.lift = 0; FxSparkle(V3(e->a >= 0 ? CardX(e->a) : 0, 1.4f, 0.8f), 16, (Color){ 255, 220, 120, 255 }); return 1.6f; }
            if (e->code == CR_AVG) { R.avgShown = true; R.avg = e->value; snprintf(R.avgText, sizeof(R.avgText), "%.1f", e->value); return 1.6f; }
            return 0;
        case EV_FAIL:
            if (e->code == CRF_TOP) {
                R.top = e->a; R.right = e->b; R.topBad = true; R.lift = 0;
                CueAt(&R.cues, R.time + 0.6f, DO_BAD, 0, 0, V3(e->a >= 0 ? CardX(e->a) : 0, 1.6f, 0.8f), 0);
            } else if (e->code == CRF_AVG) {
                R.avgShown = R.avgBad = true;
                R.avg = e->value;
                StrCopy(R.avgText, sizeof(R.avgText), e->text);
                if (R.ev) { const char *x = R.ev->expected; StrCopy(R.want, sizeof(R.want), strncmp(x, "Average: ", 9) == 0 ? x + 9 : x); }
                CueAt(&R.cues, R.time + 0.6f, DO_BAD, 0, 0, V3(3.6f, 1.6f, -0.6f), 0);
            } else {
                CueAt(&R.cues, R.time + 0.3f, DO_BAD, 0, 0, V3(0, 1.2f, 0.6f), 0);
            }
            return 2.8f;
        case EV_CRASH:
            CueAt(&R.cues, R.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.2f, 0.6f), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&R.cues, R.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void StructsUpdate(float dt)
{
    R.time += dt;
    R.deal = fminf(R.deal + dt * 2.5f, (float)R.n + 1);
    R.lift = fminf(1, R.lift + dt * 2);
    float target = R.avgShown ? 0.2f + 2.0f * (float)R.avg / (float)(R.maxScore > 0 ? R.maxScore : 100) : 0;
    R.avgLine = Smooth(R.avgLine, target, 4, dt);
    Cue c;
    while (CueDue(&R.cues, R.time, &c)) {
        if (c.what == DO_BAD) { FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0, 2.2f, 0.4f), PlaybackFinale() ? 150 : 70, 2.5f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void StructsDraw(void)
{
    KitStage(V3(0, 0, 0.2f), 5.0f, 2.0f);
    // the table, and a back wall the score bars stand against
    Part(MESH_CUBE, V3(0, 0.4f, 0.8f), V3(8.6f, 0.08f, 1.4f), (Color){ 80, 110, 90, 255 }, 0);
    for (int s = -1; s <= 1; s += 2) Part(MESH_CUBE, V3(s * 4.0f, 0.2f, 0.8f), V3(0.12f, 0.4f, 1.2f), (Color){ 70, 60, 50, 255 }, 0);
    Part(MESH_CUBE, V3(0, 1.3f, -0.75f), V3(8.6f, 2.6f, 0.1f), (Color){ 40, 44, 62, 255 }, 0);
    for (int i = 0; i < R.n; i++) {
        float d = Clamp01(R.deal - i);
        if (d <= 0) continue;
        float x = CardX(i);
        bool lifted = (i == R.top);
        bool shouldBe = R.topBad && i == R.right;
        // the score bar
        Part(MESH_CUBE, V3(x, BarH(R.score[i]) / 2, -0.55f), V3(0.5f, BarH(R.score[i]), 0.2f), (Color){ 90, 140, 220, 255 }, 0.15f);
        // the card: dealt onto the table, lifted when it is the top
        float up = lifted ? EaseOutBack(R.lift) * 0.9f : 0;
        Vector3 c = V3(x + (1 - EaseOutCubic(d)) * -5.0f, 0.47f + up, 0.85f - up * 0.3f);
        Color face = lifted ? (R.topBad ? (Color){ 220, 90, 90, 255 } : (Color){ 255, 214, 120, 255 }) : shouldBe ? (Color){ 140, 230, 170, 255 } : (Color){ 230, 226, 214, 255 };
        PartRot(MESH_CUBE, c, V3(1.0f, 0.04f, 0.7f), V3(lifted ? -55 * R.lift : -8, 0, 0), face, lifted ? 0.35f : shouldBe ? 0.3f * Pulse(R.time, 6) : 0);
        if (lifted && !R.topBad) {
            for (int k = 0; k < 5; k++) {
                float a = k * 2 * PI / 5;
                Part(MESH_CONE, V3(c.x + cosf(a) * 0.16f, c.y + 0.62f, c.z - 0.2f + sinf(a) * 0.05f), V3(0.08f, 0.16f, 0.08f), (Color){ 255, 205, 70, 255 }, 0.8f);
            }
        }
    }
    // the average: a glowing line across the bars
    if (R.avgShown) {
        Color col = R.avgBad ? (Color){ 255, 90, 90, 255 } : (Color){ 120, 255, 180, 255 };
        Part(MESH_CUBE, V3(0, R.avgLine, -0.42f), V3(8.2f, 0.04f, 0.04f), col, 0.9f);
    }

    for (int i = 0; i < R.n; i++) {
        if (Clamp01(R.deal - i) < 0.6f) continue;
        float x = CardX(i);
        bool lifted = (i == R.top);
        float up = lifted ? EaseOutBack(R.lift) * 0.9f : 0;
        Color ink = lifted && R.topBad ? WHITE : (Color){ 40, 40, 50, 255 };
        Label3D(V3(x, 0.62f + up, 1.0f - up * 0.3f), TextFormat(".name  %s", R.name[i]), 12, ink, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        Label3D(V3(x, 0.5f + up, 1.12f - up * 0.3f), TextFormat(".score %d", R.score[i]), 12, ink, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        Label3D(V3(x, 0.3f, 1.65f), TextFormat("[%d]", i), 11, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    if (R.top >= 0 || R.topBad) {
        float x = R.top >= 0 ? CardX(R.top) : 0;
        const char *t = R.topBad && R.ev ? R.ev->actual : TextFormat("Top: %s %d", R.top >= 0 ? R.name[R.top] : "?", R.top >= 0 ? R.score[R.top] : 0);
        Label3D(V3(x, 2.1f, 0.6f), t, 15, R.topBad ? WHITE : LBL_GOLD, R.topBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        if (R.topBad && R.right >= 0) Label3D(V3(CardX(R.right), 1.2f, 1.2f), "this one", 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (R.avgShown) {
        Label3D(V3(4.3f, R.avgLine, -0.4f), TextFormat("Average: %s", R.avgText), 14, R.avgBad ? WHITE : LBL_GOOD, R.avgBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        if (R.avgBad && R.want[0]) Label3D(V3(4.3f, R.avgLine + 0.4f, -0.4f), TextFormat("should be %s", R.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(-3.9f, 2.45f, -0.6f), "struct Student", 13, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_STRUCTS = {
    "c-structs", "Crew Roster", "Every struct is a crew card: your program picks the top one and the average.",
    { 255, 214, 120, 255 },
    ParseStructs, StructsReset, StructsEvent, StructsUpdate, StructsDraw, NULL,
};
