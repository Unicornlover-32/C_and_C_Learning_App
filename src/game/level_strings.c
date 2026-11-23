// Word Reverser -> "Mirror Machine"
//
// The input word rides in as letter tiles, [0] [1] [2]..., closed off by a glowing '\0'
// tile: that is how C knows where a string ends. "Length: N" walks a counter along the
// tiles up to N - it should stop just before the '\0'. "Reversed: ..." sends the letters
// through the mirror machine; they come out the far side in the printed order. A wrong
// letter cracks red; a wrong length runs past the '\0' or stops short.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { ST_WORD = 1, ST_LEN, ST_OUT };
enum { STF_LEN = 1, STF_OUT, STF_GARBLED, STF_EXTRA };

#define SMAX 40

// ---------------------------------------------------------------- the parser

static void ParseStrings(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    char word[128] = "";
    if (run->input) sscanf(run->input, "%127s", word);
    int len = (int)strlen(word);
    GameEvent *w = EvSpawn(&p, ST_WORD, len, 0, 0);
    if (w) StrCopy(w->text, EV_TEXT, word);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ScanAfter(E, el, "Length:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, ST_LEN, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, STF_LEN, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Length:", &got)) {
                if (got == want + 1) snprintf(note, sizeof(note), "That counted the '\\0' too - stop before it");
                else if (got == want - 1) snprintf(note, sizeof(note), "One letter short - count from 0 up to the '\\0'");
                else snprintf(note, sizeof(note), "\"%.60s\" has %d letters", word, want);
                ParseFail(&p, FAIL_WRONG, STF_LEN, e, 0, 0, got, note);
            } else {
                ParseFail(&p, FAIL_GARBLED, STF_GARBLED, e, 0, 0, 0, "Expected Length: and a number");
            }
            break;
        }
        // the reversed word
        char wantText[128], gotText[128];
        if (!ScanAfterText(E, el, "Reversed:", wantText, sizeof(wantText))) wantText[0] = '\0';
        if (ParseTake(&p, e)) { GameEvent *g = EvAct(&p, ST_OUT, (int)strlen(wantText), 0, 0); if (g) StrCopy(g->text, EV_TEXT, wantText); continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, STF_OUT, e, 0, 0, 0, NULL); break; }
        if (!ScanAfterText(A, al, "Reversed:", gotText, sizeof(gotText))) {
            ParseFail(&p, FAIL_GARBLED, STF_GARBLED, e, 0, 0, 0, "Expected Reversed: and the word");
            break;
        }
        ParseShow(&p);
        int wl = (int)strlen(wantText), gl = (int)strlen(gotText), k = 0;
        while (k < wl && k < gl && wantText[k] == gotText[k]) k++;
        if (strcmp(gotText, word) == 0 && wl > 1 && strcmp(word, wantText) != 0) snprintf(note, sizeof(note), "That's the word forwards - start from the last letter");
        else if (k < wl && k < gl) snprintf(note, sizeof(note), "Letter %d should be '%c', not '%c'", k + 1, wantText[k], gotText[k]);
        else if (gl < wl) snprintf(note, sizeof(note), "Only %d of the %d letters came out", gl, wl);
        else snprintf(note, sizeof(note), "%d letters came out - the word has %d", gl, wl);
        GameEvent *f = ParseFail(&p, FAIL_WRONG, STF_OUT, e, k, gl, 0, note);
        if (f) StrCopy(f->text, EV_TEXT, gotText);
        break;
    }
    ParseEnd(&p, STF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_TAP, DO_CHEER, DO_BAD, DO_EMIT };

static struct {
    const EventList *ev;
    Cues  cues;
    float time, scroll;
    char  word[EV_TEXT];
    int   len;                 // letters in the word (shown: at most SMAX)
    int   counted, countGoal;  // the length counter
    bool  lenBad, outBad;
    int   lenShown;
    char  out[EV_TEXT];        // what came out of the mirror
    int   outCount, outGoal, badAt;
    float inT;                 // letters sliding into the mirror
    bool  mirror;
} S;

// The tiles squeeze together for long words, so the word and its '\0' always fit before the mirror
static float TileGap(void) { float g = 4.0f / (S.len + 2); return g > 0.56f ? 0.56f : g; }
static float TileSize(void) { float t = TileGap() * 0.8f; return t > 0.42f ? 0.42f : t; }
static float LetterSize(void) { float z = TileGap() * 34; return z < 11 ? 11 : z > 17 ? 17 : z; }
static Vector3 InTile(int i)  { return V3(-5.1f + i * TileGap(), BELT_Y + 0.22f, 0); }
static Vector3 OutTile(int i) { return V3(1.05f + i * TileGap(), BELT_Y + 0.22f, 0); }

static void StringsReset(const EventList *ev)
{
    memset(&S, 0, sizeof(S));
    S.ev = ev;
    S.badAt = -1;
    ShotOrbit(V3(0, 0.9f, 0), 10.2f, 26, 0, 36, 0);
}

static float StringsEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            StrCopy(S.word, sizeof(S.word), e->text);
            S.len = (int)strlen(S.word);
            if (S.len > SMAX) S.len = SMAX;
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == ST_LEN) {
                S.lenShown = (int)e->value;
                S.countGoal = S.len;
                S.counted = 0;
                for (int i = 0; i < S.countGoal; i++) CueAt(&S.cues, S.time + 0.2f + i * 0.16f, DO_TAP, i, 0, V3(0, 0, 0), 0);
                return 0.6f + S.countGoal * 0.16f;
            }
            if (e->code == ST_OUT) {
                StrCopy(S.out, sizeof(S.out), e->text);
                S.outGoal = (int)strlen(S.out);
                S.mirror = true;
                S.inT = 0;
                for (int i = 0; i < S.outGoal && i < SMAX; i++) CueAt(&S.cues, S.time + 0.5f + i * 0.2f, DO_EMIT, i, 0, V3(0, 0, 0), 0);
                return 1.0f + S.outGoal * 0.2f;
            }
            return 0;
        case EV_FAIL:
            if (e->code == STF_LEN) {
                int v = (int)e->value;
                S.lenShown = v;
                S.lenBad = true;
                S.countGoal = v < 0 ? 0 : v > S.len + 6 ? S.len + 6 : v;
                for (int i = 0; i < S.countGoal; i++) CueAt(&S.cues, S.time + 0.2f + i * 0.16f, DO_TAP, i, 0, V3(0, 0, 0), 0);
                CueAt(&S.cues, S.time + 0.4f + S.countGoal * 0.16f, DO_BAD, 0, 0, V3(0, 0, 0), 0);
                return 2.6f + S.countGoal * 0.16f;
            }
            if (e->code == STF_OUT) {
                StrCopy(S.out, sizeof(S.out), e->text);
                S.outGoal = (int)strlen(S.out);
                S.outBad = true;
                S.badAt = e->a;
                S.mirror = true;
                for (int i = 0; i < S.outGoal && i < SMAX; i++) CueAt(&S.cues, S.time + 0.5f + i * 0.2f, DO_EMIT, i, 0, V3(0, 0, 0), 0);
                CueAt(&S.cues, S.time + 0.6f + (S.badAt < S.outGoal ? S.badAt : S.outGoal) * 0.2f, DO_BAD, 1, 0, V3(0, 0, 0), 0);
                return 2.8f + S.outGoal * 0.2f;
            }
            CueAt(&S.cues, S.time + 0.2f, DO_BAD, 1, 0, V3(0, 0, 0), 0);
            return 2.4f;
        case EV_CRASH:
            CueAt(&S.cues, S.time + 0.1f, DO_BAD, 1, 0, V3(0, 0, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&S.cues, S.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void StringsUpdate(float dt)
{
    S.time += dt;
    S.inT += dt;
    if (S.mirror && S.outCount < S.outGoal) S.scroll += dt * 1.4f;
    Cue c;
    while (CueDue(&S.cues, S.time, &c)) {
        switch (c.what) {
            case DO_TAP:
                S.counted = c.a + 1;
                if (c.a < S.len) FxSparkle(V3(InTile(c.a).x, InTile(c.a).y + 0.4f, 0), 4, (Color){ 140, 230, 255, 255 });
                else FxSparks(V3(InTile(c.a).x, InTile(c.a).y + 0.3f, 0), 10, (Color){ 255, 120, 80, 255 });
                break;
            case DO_EMIT:
                S.outCount = c.a + 1;
                FxSparkle(V3(OutTile(c.a).x, OutTile(c.a).y + 0.3f, 0), 5, (Color){ 200, 160, 255, 255 });
                break;
            case DO_BAD:
                FxSparks(c.a ? V3(0, 1.4f, 0.4f) : V3(InTile(S.countGoal > 0 ? S.countGoal - 1 : 0).x, 1.2f, 0), 34, (Color){ 255, 100, 90, 255 });
                PlaybackShake(0.2f);
                break;
            case DO_CHEER:
                FxConfetti(V3(0, 1.8f, 0), PlaybackFinale() ? 150 : 70, 1.2f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

static void Letter(Vector3 at, char ch, Color col, float glow, float yaw)
{
    (void)ch;
    KitCrate(V3(at.x, at.y - 0.22f, at.z), TileSize(), col, glow, yaw);
}

static void StringsDraw(void)
{
    KitStage(V3(0, 0, 0), 6.0f, 2.0f);
    KitBelt(V3(-5.6f, 0, 0), 11.2f, S.scroll);
    // the mirror machine in the middle, with a glowing mirror pane on its face
    KitMachine(V3(-0.2f, 0, 0), V3(1.5f, 1.9f, 1.3f), (Color){ 110, 80, 160, 255 }, (Color){ 200, 160, 255, 255 }, 0.7f, S.mirror ? S.time : 0);
    Part(MESH_CUBE, V3(-0.2f, 1.35f, 0.67f), V3(0.9f, 0.6f, 0.02f), (Color){ 120, 150, 210, 255 }, 0.18f);

    // the word going in: each letter, then the '\0' that ends it
    int fed = S.mirror ? (int)fminf((float)S.len, S.inT / 0.2f) : 0;
    for (int i = 0; i < S.len; i++) {
        Vector3 t = InTile(i);
        if (i >= S.len - fed) continue;          // the last letters go into the mirror first
        bool counted = i < S.counted;
        Letter(t, S.word[i], counted ? (Color){ 110, 180, 255, 255 } : (Color){ 200, 170, 120, 255 }, counted ? 0.35f : 0, 0);
    }
    Vector3 nul = InTile(S.len);
    bool overrun = S.counted > S.len;
    Part(MESH_CUBE, V3(nul.x, nul.y - 0.02f, 0), V3(TileSize() * 0.7f, 0.4f, 0.42f), overrun ? (Color){ 235, 90, 90, 255 } : (Color){ 120, 255, 200, 255 }, 0.9f);
    for (int i = S.len + 1; i < S.counted; i++)        // counting past the end: empty, red slots
        Part(MESH_CUBE, V3(InTile(i).x, BELT_Y + 0.02f, 0), V3(TileSize(), 0.04f, 0.42f), (Color){ 235, 90, 90, 255 }, 0.7f);
    // the counter's arm hovers over the tile it is counting
    if (S.counted > 0) {
        Vector3 at = InTile(S.counted - 1);
        Part(MESH_CUBE, V3(at.x, 1.75f, 0), V3(0.12f, 0.5f, 0.12f), (Color){ 90, 98, 124, 255 }, 0);
        Part(MESH_CONE, V3(at.x, 1.36f, 0), V3(0.22f, 0.25f, 0.22f), S.lenBad ? (Color){ 235, 90, 90, 255 } : (Color){ 120, 220, 255, 255 }, 0.8f);
    }
    // the letters coming out of the mirror
    for (int i = 0; i < S.outCount && i < SMAX; i++) {
        bool bad = S.outBad && i >= S.badAt;
        Letter(OutTile(i), S.out[i], bad ? (Color){ 230, 80, 80, 255 } : (Color){ 190, 150, 255, 255 }, bad ? 0.5f : 0.3f, 0);
    }

    // labels
    for (int i = 0; i < S.len; i++) {
        if (i >= S.len - fed) continue;
        Vector3 t = InTile(i);
        Label3D(V3(t.x, t.y + 0.05f, 0.25f), TextFormat("%c", S.word[i]), LetterSize(), WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        if (S.len <= 14) Label3D(V3(t.x, BELT_Y - 0.08f, 0.5f), TextFormat("[%d]", i), 10, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    Label3D(V3(nul.x, nul.y + 0.05f, 0.25f), "\\0", 15, (Color){ 20, 60, 40, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    for (int i = 0; i < S.outCount && i < SMAX; i++) {
        Vector3 t = OutTile(i);
        Label3D(V3(t.x, t.y + 0.05f, 0.25f), TextFormat("%c", S.out[i]), LetterSize(), WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    if (S.counted > 0 || S.lenShown) {
        bool done = S.counted >= S.countGoal;
        bool bad = S.lenBad && done;
        Label3D(V3(InTile(S.counted > 0 ? S.counted - 1 : 0).x, 2.25f, 0), TextFormat("Length: %d", done ? S.lenShown : S.counted), 17,
                bad ? WHITE : LBL_GOLD, bad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        if (bad) Label3D(V3(InTile(S.len).x, 0.25f, 0.6f), TextFormat("'\\0' is at [%d]", S.len), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(-0.2f, 0.35f, 0.8f), "mirror", 13, (Color){ 210, 190, 255, 255 }, LBL_DIM, LABEL_CODE);
    if (S.outBad && S.outCount >= S.outGoal && S.ev)
        Label3D(V3(OutTile(0).x + 1.2f, 1.7f, 0.2f), TextFormat("should be: %.30s", S.ev->expected + (strncmp(S.ev->expected, "Reversed: ", 10) == 0 ? 10 : 0)),
                13, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_STRINGS = {
    "c-strings", "Mirror Machine", "Letters in, '\\0' at the end, and out through the mirror backwards.",
    { 190, 150, 255, 255 },
    ParseStrings, StringsReset, StringsEvent, StringsUpdate, StringsDraw, NULL,
};
