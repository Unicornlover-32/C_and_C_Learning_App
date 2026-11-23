// Time Converter -> "Clock Tower"
//
// The input's seconds pour into a hopper as sand. The first printed line splits it into
// three glass tubes - hours (seconds / 3600), minutes (seconds % 3600 / 60) and seconds
// (seconds % 60) - each filling to the printed value, and a clock face turns its hands to
// match. "In minutes:" sets a dial to the decimal minutes. Input that isn't a number is
// spat back out of the hopper and "Invalid input" lights. A wrong part overflows its
// tube in red; integer division that lost the decimals is named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { CT_INPUT = 1, CT_SPLIT, CT_MINUTES, CT_INVALID };
enum { CTF_SPLIT = 1, CTF_MINUTES, CTF_INVALID, CTF_GARBLED, CTF_EXTRA };

static bool ScanSplit(const char *s, int len, int *n, int *h, int *m, int *sec)
{
    char buf[160];
    int k = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1, used = 0;
    memcpy(buf, s, (size_t)k);
    buf[k] = '\0';
    if (sscanf(buf, "%d seconds is %d hours, %d minutes and %d seconds%n", n, h, m, sec, &used) != 4) return false;
    return used == k;
}

// ---------------------------------------------------------------- the parser

static void ParseTime(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    char word[40] = "";
    if (run->input) sscanf(run->input, "%39s", word);
    char *end = NULL;
    long secs = strtol(word, &end, 10);
    bool valid = word[0] && end && end != word;
    GameEvent *g = EvSpawn(&p, CT_INPUT, valid, 0, valid ? (double)secs : 0);
    if (g) StrCopy(g->text, EV_TEXT, word);
    if (ParsePreview(&p)) return;

    char note[140];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, n, h, m, s, gn, gh, gm, gs;
        double want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (LineIs(E, el, "Invalid input")) {
            if (ParseTake(&p, e)) { EvAct(&p, CT_INVALID, 0, 0, 0); continue; }
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, CTF_INVALID, e, 0, 0, 0, "Check what scanf returns: if it isn't 1, print Invalid input and stop");
            break;
        }
        if (ScanSplit(E, el, &n, &h, &m, &s)) {
            if (ParseTake(&p, e)) { EvAct(&p, CT_SPLIT, h, m, s); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, CTF_SPLIT, e, -1, 0, 0, NULL); break; }
            if (!ScanSplit(A, al, &gn, &gh, &gm, &gs)) {
                ParseFail(&p, FAIL_GARBLED, CTF_GARBLED, e, 0, 0, 0, "Expected: N seconds is H hours, M minutes and S seconds");
                break;
            }
            ParseShow(&p);
            EvAct(&p, CT_SPLIT, gh, gm, gs);
            int part = gh != h ? 0 : gm != m ? 1 : gs != s ? 2 : -1;
            if (part == 1 && gm == n / 60) snprintf(note, sizeof(note), "Minutes are what's left after the hours: (seconds %% 3600) / 60");
            else if (part == 2 && gs == n) snprintf(note, sizeof(note), "Seconds are what's left after the minutes: seconds %% 60");
            else if (part == 0) snprintf(note, sizeof(note), "Hours are seconds / 3600 (integer division): %d", h);
            else if (part >= 0) snprintf(note, sizeof(note), "%s should be %d", part == 1 ? "Minutes" : "Seconds", part == 1 ? m : s);
            else snprintf(note, sizeof(note), "The wording must match exactly");
            GameEvent *f = ParseFail(&p, FAIL_WRONG, CTF_SPLIT, e, part, 0, part == 0 ? gh : part == 1 ? gm : gs, note);
            if (f) snprintf(f->text, EV_TEXT, "%d", part == 0 ? h : part == 1 ? m : s);
            break;
        }
        if (ScanAfterNumber(E, el, "In minutes:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, CT_MINUTES, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, CTF_MINUTES, e, 0, 0, 0, NULL); break; }
            if (ScanAfterNumber(A, al, "In minutes:", &got)) {
                if (valid && fabs(got - (double)(secs / 60)) < 0.001 && fabs(got - want) > 0.001)
                    snprintf(note, sizeof(note), "Integer division lost the decimals: divide by 60.0, or cast to double");
                else if (fabs(got - want) < 0.006) snprintf(note, sizeof(note), "Same value, wrong format: two decimal places (%%.2f)");
                else snprintf(note, sizeof(note), "In minutes should be %.*s", el - 12, E + 12);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, CTF_MINUTES, e, 0, 0, got, note);
                if (f) ScanAfterText(A, al, "In minutes:", f->text, EV_TEXT);
            } else {
                ParseFail(&p, FAIL_GARBLED, CTF_GARBLED, e, 0, 0, 0, "Expected In minutes: and a number");
            }
            break;
        }
        ParseTake(&p, e);
    }
    ParseEnd(&p, CTF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_FILL, DO_CHEER, DO_BAD, DO_SPIT };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    bool  valid;
    double secs;
    char  word[EV_TEXT];
    int   part[3];                // printed hours, minutes, seconds
    float fill[3], fillTo[3];
    int   badPart;
    bool  split, invalidShown, invalidBad, minShown, minBad;
    double minutes;
    char  minText[EV_TEXT], want[EV_TEXT];
    float pour;                   // sand pouring 1 -> 0
    float handH, handM;           // clock hands (degrees)
} T;

static float TubeX(int i) { return -2.4f + i * 1.25f; }
static float Cap(int i) { if (i > 0) return 60; return T.part[0] > 24 ? (float)T.part[0] * 1.15f : 24; }

static void TimeReset(const EventList *ev)
{
    memset(&T, 0, sizeof(T));
    T.ev = ev;
    T.badPart = -1;
    ShotOrbit(V3(0.2f, 1.4f, 0), 9.4f, 16, 0, 38, 0);
}

static float TimeEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            T.valid = e->a != 0;
            T.secs = e->value;
            StrCopy(T.word, sizeof(T.word), e->text);
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == CT_SPLIT) {
                T.split = true;
                T.part[0] = e->a; T.part[1] = e->b; T.part[2] = (int)e->value;
                T.pour = 1;
                for (int i = 0; i < 3; i++) CueAt(&T.cues, T.time + 0.5f + i * 0.45f, DO_FILL, i, 0, V3(0, 0, 0), 0);
                return 2.4f;
            }
            if (e->code == CT_MINUTES) {
                T.minShown = true;
                T.minutes = e->value;
                snprintf(T.minText, sizeof(T.minText), "%.2f", e->value);
                return 1.4f;
            }
            if (e->code == CT_INVALID) {
                T.invalidShown = true;
                CueAt(&T.cues, T.time + 0.2f, DO_SPIT, 0, 0, V3(0, 0, 0), 0);
                return 1.8f;
            }
            return 0;
        case EV_FAIL:
            StrCopy(T.want, sizeof(T.want), e->text);
            if (e->code == CTF_SPLIT && e->a >= 0 && e->a < 3) {
                T.badPart = e->a;
                T.fillTo[e->a] = 1.12f;              // the wrong part overflows its tube
                CueAt(&T.cues, T.time + 0.5f, DO_BAD, 0, 0, V3(TubeX(e->a), 2.6f, 0.3f), 0);
            } else if (e->code == CTF_MINUTES) {
                T.minShown = T.minBad = true;
                T.minutes = e->value;
                StrCopy(T.minText, sizeof(T.minText), e->text);
                if (T.ev) { const char *x = T.ev->expected; StrCopy(T.want, sizeof(T.want), strncmp(x, "In minutes: ", 12) == 0 ? x + 12 : x); }
                CueAt(&T.cues, T.time + 0.5f, DO_BAD, 0, 0, V3(2.9f, 1.7f, 0.2f), 0);
            } else if (e->code == CTF_INVALID) {
                T.invalidShown = T.invalidBad = true;
                CueAt(&T.cues, T.time + 0.3f, DO_BAD, 0, 0, V3(-3.6f, 2.6f, 0), 0);
            } else {
                CueAt(&T.cues, T.time + 0.3f, DO_BAD, 0, 0, V3(0, 2.0f, 0.3f), 0);
            }
            return 2.8f;
        case EV_CRASH:
            CueAt(&T.cues, T.time + 0.1f, DO_BAD, 0, 0, V3(0, 2.0f, 0.3f), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&T.cues, T.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void TimeUpdate(float dt)
{
    T.time += dt;
    T.pour = fmaxf(0, T.pour - dt * 0.7f);
    if (T.pour > 0.1f) FxDust(V3(-3.6f, 2.0f, 0), 1);
    for (int i = 0; i < 3; i++) T.fill[i] = Smooth(T.fill[i], T.fillTo[i], 4, dt);
    // the clock's hands follow the printed hours and minutes
    float wantH = T.split ? (T.part[0] % 12) * 30.0f + T.part[1] * 0.5f : 0, wantM = T.split ? T.part[1] * 6.0f : 0;
    T.handH = Smooth(T.handH, wantH, 3, dt);
    T.handM = Smooth(T.handM, wantM, 3, dt);
    Cue c;
    while (CueDue(&T.cues, T.time, &c)) {
        switch (c.what) {
            case DO_FILL: {
                int i = c.a;
                float k = T.part[i] / Cap(i);
                T.fillTo[i] = (i == T.badPart) ? 1.12f : (k < 0 ? 0 : k > 1 ? 1 : k);
                FxSparkle(V3(TubeX(i), 0.6f + T.fillTo[i] * 1.8f, 0.3f), 8, (Color){ 255, 220, 140, 255 });
                break;
            }
            case DO_SPIT:
                FxSparks(V3(-3.6f, 2.6f, 0.2f), 20, (Color){ 255, 160, 90, 255 });
                break;
            case DO_BAD:
                FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 });
                PlaybackShake(0.2f);
                break;
            case DO_CHEER:
                FxConfetti(V3(0, 2.6f, 0.3f), PlaybackFinale() ? 150 : 70, 2.2f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

static void TimeDraw(void)
{
    KitStage(V3(0, 0, 0), 4.8f, 1.8f);
    // the hopper the seconds pour into
    PartRot(MESH_CONE, V3(-3.6f, 2.35f, 0), V3(1.1f, 0.8f, 1.1f), V3(180, 0, 0), (Color){ 90, 98, 124, 255 }, 0);
    Part(MESH_CYLINDER, V3(-3.6f, 1.1f, 0), V3(0.14f, 2.0f, 0.14f), (Color){ 78, 86, 112, 255 }, 0);
    Part(MESH_CONE, V3(-3.6f, 2.25f, 0), V3(0.9f * (T.valid ? 0.6f + 0.4f * (1 - T.pour) : 0.3f), 0.4f, 0.9f), (Color){ 230, 200, 130, 255 }, 0.25f);
    // the three tubes
    for (int i = 0; i < 3; i++) {
        float x = TubeX(i);
        Part(MESH_CYLINDER, V3(x, 0.15f, 0.3f), V3(0.8f, 0.3f, 0.8f), (Color){ 70, 76, 100, 255 }, 0);
        float h = T.fill[i] * 1.8f;
        bool bad = (i == T.badPart && T.fill[i] > 0.9f);
        Color sand = bad ? (Color){ 235, 80, 80, 255 } : (Color){ 255, 200, 110, 255 };
        if (h > 0.01f) Part(MESH_CYLINDER, V3(x, 0.3f + h / 2, 0.3f), V3(0.52f, h, 0.52f), sand, bad ? 0.6f : 0.35f);
        PartGlass(MESH_CYLINDER, V3(x, 1.25f, 0.3f), V3(0.62f, 1.9f, 0.62f), (Color){ 160, 200, 255, 50 });
        Part(MESH_CYLINDER, V3(x, 2.22f, 0.3f), V3(0.7f, 0.06f, 0.7f), (Color){ 90, 98, 124, 255 }, 0);
    }
    // the clock face, hands set by the printed hours and minutes
    Vector3 c = V3(1.65f, 1.75f, -0.1f);
    PartRot(MESH_CYLINDER, c, V3(1.7f, 0.1f, 1.7f), V3(90, 0, 0), (Color){ 230, 225, 210, 255 }, 0.05f);
    PartRot(MESH_CYLINDER, V3(c.x, c.y, c.z - 0.03f), V3(1.85f, 0.08f, 1.85f), V3(90, 0, 0), (Color){ 120, 90, 60, 255 }, 0);
    for (int k = 0; k < 12; k++) {
        float a = k * 30 * DEG2RAD;
        Part(MESH_CUBE, V3(c.x + sinf(a) * 0.7f, c.y + cosf(a) * 0.7f, c.z + 0.06f), V3(0.05f, 0.12f, 0.02f), (Color){ 60, 60, 70, 255 }, 0);
    }
    PartRot(MESH_CUBE, V3(c.x + sinf(T.handH * DEG2RAD) * 0.22f, c.y + cosf(T.handH * DEG2RAD) * 0.22f, c.z + 0.08f), V3(0.07f, 0.45f, 0.02f),
            V3(0, 0, -T.handH), (Color){ 40, 40, 50, 255 }, 0);
    PartRot(MESH_CUBE, V3(c.x + sinf(T.handM * DEG2RAD) * 0.32f, c.y + cosf(T.handM * DEG2RAD) * 0.32f, c.z + 0.09f), V3(0.045f, 0.64f, 0.02f),
            V3(0, 0, -T.handM), (Color){ 200, 60, 50, 255 }, 0.2f);
    // the decimal minutes dial
    double range = T.valid && T.secs > 0 ? T.secs / 60.0 * 1.3 : 1;
    KitGauge(V3(3.55f, 1.55f, 0.1f), 0.48f, T.minShown ? (float)(T.minutes / range) + (T.minBad ? 0.25f : 0) : 0,
             T.minBad ? (Color){ 255, 90, 90, 255 } : (Color){ 255, 200, 70, 255 }, T.minShown ? 0.9f : 0.2f);
    Part(MESH_CYLINDER, V3(3.55f, 0.5f, 0.05f), V3(0.08f, 1.0f, 0.08f), (Color){ 78, 86, 112, 255 }, 0);

    // labels
    Label3D(V3(-3.6f, 3.05f, 0), T.valid ? TextFormat("%.0f seconds", T.secs) : TextFormat("\"%s\"", T.word), 15, T.valid ? LBL_GOLD : LBL_BAD, LBL_DIM, LABEL_CODE);
    static const char *const names[3] = { "hours", "minutes", "seconds" }, *const ops[3] = { "s / 3600", "s % 3600 / 60", "s % 60" };
    for (int i = 0; i < 3; i++) {
        float x = TubeX(i);
        Label3D(V3(x, 2.55f, 0.3f), names[i], 13, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        Label3D(V3(x, 0.1f, 0.8f), ops[i], 11, (Color){ 150, 170, 220, 255 }, LBL_DIM, LABEL_CODE);
        if (T.split && T.fill[i] > 0.02f) {
            bool bad = i == T.badPart;
            Label3D(V3(x, 0.4f + T.fill[i] * 1.8f, 0.7f), TextFormat("%d", T.part[i]), 16, bad ? WHITE : LBL_GOLD, bad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
            if (bad && T.want[0]) Label3D(V3(x, 2.9f, 0.3f), TextFormat("should be %s", T.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
        }
    }
    if (T.split) Label3D(V3(c.x, c.y - 1.15f, c.z + 0.2f), TextFormat("%d:%02d:%02d", T.part[0], T.part[1], T.part[2]), 15, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(3.55f, 2.3f, 0.1f), "In minutes", 12, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (T.minShown) Label3D(V3(3.55f, 0.85f, 0.4f), T.minText, 16, T.minBad ? WHITE : LBL_GOLD, T.minBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (T.minBad && T.want[0]) Label3D(V3(3.55f, 2.7f, 0.1f), TextFormat("should be %s", T.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (T.invalidShown) {
        Label3D(V3(-1.2f, 1.4f, 0.6f), T.invalidBad && T.ev ? (T.ev->actual[0] ? T.ev->actual : "(nothing printed)") : "Invalid input", 17,
                WHITE, T.invalidBad ? (Color){ 200, 50, 55, 235 } : (Color){ 150, 90, 40, 235 }, LABEL_CODE);
        if (T.invalidBad) Label3D(V3(-1.2f, 0.95f, 0.6f), "should be: Invalid input", 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
}

const LevelDef LEVEL_TIME = {
    "c-input-maths", "Clock Tower", "Integer division and % split your seconds into hours, minutes and seconds.",
    { 255, 200, 110, 255 },
    ParseTime, TimeReset, TimeEvent, TimeUpdate, TimeDraw, NULL,
};
