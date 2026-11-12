// Shopping List -> "Cargo Train"
//
// The vector is a train. "Items: N" couples a wagon for each word read - push_back adds
// one at the end - and the counter shows size(). Each "k. word" line paints that word
// on wagon [k-1], and "Longest:" puts a crown on its wagon (on a tie, the first). A wrong
// word or count shows in red; numbering from 0 and ties are named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { TR_WORD = 1, TR_ITEMS, TR_ITEM, TR_LONGEST };
enum { TRF_ITEMS = 1, TRF_ITEM, TRF_LONGEST, TRF_GARBLED, TRF_EXTRA };

#define WMAX 10

// ---------------------------------------------------------------- the parser

static void ParseVectors(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    char words[32][40];
    int n = 0, used;
    const char *s = run->input ? run->input : "";
    while (n < 32 && sscanf(s, " %39s%n", words[n], &used) == 1) { s += used; n++; }
    for (int i = 0; i < n && i < WMAX; i++) { GameEvent *g = EvSpawn(&p, TR_WORD, i, n, 0); if (g) StrCopy(g->text, EV_TEXT, words[i]); }
    if (ParsePreview(&p)) return;

    char note[140];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ScanAfter(E, el, "Items:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, TR_ITEMS, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, TRF_ITEMS, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Items:", &got)) {
                snprintf(note, sizeof(note), "%d words were read, so size() is %d", want, want);
                ParseFail(&p, FAIL_WRONG, TRF_ITEMS, e, 0, 0, got, note);
            } else ParseFail(&p, FAIL_GARBLED, TRF_GARBLED, e, 0, 0, 0, "Expected Items: and a number");
            break;
        }
        char wword[64], gword[64];
        if (ScanAfterText(E, el, "Longest:", wword, sizeof(wword))) {
            int at = -1;
            for (int i = 0; i < n && at < 0; i++) if (strcmp(words[i], wword) == 0) at = i;
            if (ParseTake(&p, e)) { EvAct(&p, TR_LONGEST, at, 0, 0); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, TRF_LONGEST, e, -1, at, 0, NULL); break; }
            if (ScanAfterText(A, al, "Longest:", gword, sizeof(gword))) {
                int gat = -1;
                for (int i = 0; i < n && gat < 0; i++) if (strcmp(words[i], gword) == 0) gat = i;
                if (gat > at && strlen(gword) == strlen(wword)) snprintf(note, sizeof(note), "A tie: the first longest word wins - only replace it when a word is longer (>)");
                else snprintf(note, sizeof(note), "The longest word is %s", wword);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, TRF_LONGEST, e, gat, at, 0, note);
                if (f) StrCopy(f->text, EV_TEXT, gword);
            } else ParseFail(&p, FAIL_GARBLED, TRF_GARBLED, e, -1, at, 0, "Expected Longest: and a word");
            break;
        }
        // "2. bread"
        int k = 0, k2 = 0, u1 = 0, u2 = 0;
        char tmp[96];
        int tl = el < 95 ? el : 95;
        memcpy(tmp, E, (size_t)tl); tmp[tl] = '\0';
        if (sscanf(tmp, "%d. %63s%n", &k, wword, &u1) != 2) { ParseTake(&p, e); continue; }
        if (ParseTake(&p, e)) { GameEvent *g = EvAct(&p, TR_ITEM, k - 1, 0, 0); if (g) StrCopy(g->text, EV_TEXT, wword); continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, TRF_ITEM, e, k - 1, 0, 0, NULL); break; }
        tl = al < 95 ? al : 95;
        memcpy(tmp, A, (size_t)tl); tmp[tl] = '\0';
        if (sscanf(tmp, "%d. %63s%n", &k2, gword, &u2) == 2) {
            if (k2 == k - 1) snprintf(note, sizeof(note), "Number the items from 1: print i + 1");
            else snprintf(note, sizeof(note), "Item %d should be %s", k, wword);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, TRF_ITEM, e, k - 1, k2, 0, note);
            if (f) StrCopy(f->text, EV_TEXT, gword);
        } else ParseFail(&p, FAIL_GARBLED, TRF_GARBLED, e, k - 1, 0, 0, "Expected a number, a dot and the word: 1. milk");
        (void)u1; (void)u2;
        break;
    }
    ParseEnd(&p, TRF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_COUPLE, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   n, total;
    char  word[WMAX][EV_TEXT];
    int   coupled;                  // wagons on the train
    float pop[WMAX];
    int   painted[WMAX];            // 0 blank, 1 painted, 2 wrong
    char  paint[WMAX][EV_TEXT];
    int   crown, crownWant;
    bool  crownBad, itemsShown, itemsBad;
    int   items;
    float coupleT;
    char  want[96];
} T;

static float Gap(void) { float g = 8.0f / (T.n + 1.5f); return g > 1.35f ? 1.35f : g; }
static float WagonX(int i) { return 3.0f - (i + 1) * Gap(); }

static void VectorsReset(const EventList *ev)
{
    memset(&T, 0, sizeof(T));
    T.ev = ev;
    T.crown = T.crownWant = -1;
    ShotOrbit(V3(-0.6f, 0.8f, 0.2f), 9.6f, 24, 0, 38, 0);
}

static float VectorsEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (i >= 0 && i < WMAX) { StrCopy(T.word[i], EV_TEXT, e->text); if (i + 1 > T.n) T.n = i + 1; T.total = e->b; }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
        case EV_FAIL:
            if (e->code == TR_ITEMS || e->code == TRF_ITEMS) {
                T.itemsShown = true;
                T.items = (int)e->value;
                T.itemsBad = e->type == EV_FAIL;
                for (int k = 0; k < T.n; k++) CueAt(&T.cues, T.time + 0.2f + k * 0.25f, DO_COUPLE, k, 0, V3(0, 0, 0), 0);
                if (T.itemsBad) CueAt(&T.cues, T.time + 0.5f + T.n * 0.25f, DO_BAD, 0, 0, V3(3.0f, 1.6f, 0), 0);
                return (T.itemsBad ? 2.4f : 0.5f) + T.n * 0.25f;
            }
            if ((e->code == TR_ITEM || e->code == TRF_ITEM) && i >= 0 && i < WMAX) {
                bool bad = e->type == EV_FAIL;
                T.painted[i] = bad ? 2 : 1;
                StrCopy(T.paint[i], EV_TEXT, e->text);
                FxSparkle(V3(WagonX(i), 1.0f, 0.4f), 8, bad ? (Color){ 255, 100, 90, 255 } : (Color){ 255, 220, 140, 255 });
                if (bad) { if (T.ev) StrCopy(T.want, sizeof(T.want), T.ev->expected); CueAt(&T.cues, T.time + 0.3f, DO_BAD, 0, 0, V3(WagonX(i), 1.2f, 0), 0); }
                return bad ? 2.6f : 0.6f;
            }
            if (e->code == TR_LONGEST || e->code == TRF_LONGEST) {
                T.crown = i;
                T.crownBad = e->type == EV_FAIL;
                T.crownWant = e->b;
                if (T.crownBad) { if (T.ev) StrCopy(T.want, sizeof(T.want), T.ev->expected); CueAt(&T.cues, T.time + 0.5f, DO_BAD, 0, 0, V3(i >= 0 ? WagonX(i) : 0, 1.6f, 0), 0); }
                return T.crownBad ? 2.6f : 1.4f;
            }
            CueAt(&T.cues, T.time + 0.3f, DO_BAD, 0, 0, V3(0, 1.2f, 0), 0);
            return 2.4f;
        case EV_CRASH:
            CueAt(&T.cues, T.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.2f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&T.cues, T.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void VectorsUpdate(float dt)
{
    T.time += dt;
    for (int k = 0; k < T.coupled && k < WMAX; k++) T.pop[k] = fminf(1, T.pop[k] + dt * 3);
    Cue c;
    while (CueDue(&T.cues, T.time, &c)) {
        if (c.what == DO_COUPLE) { T.coupled = c.a + 1; T.coupleT = T.time; FxDust(V3(WagonX(c.a), 0.2f, 0), 6); }
        else if (c.what == DO_BAD) { FxSparks(c.v, 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0, 2.0f, 0), PlaybackFinale() ? 150 : 70, 3.0f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void VectorsDraw(void)
{
    KitStage(V3(-0.8f, 0, 0.3f), 5.2f, 1.8f);
    // the rails
    for (int s = -1; s <= 1; s += 2) Part(MESH_CUBE, V3(-0.9f, 0.04f, s * 0.35f), V3(9.6f, 0.06f, 0.06f), (Color){ 150, 156, 176, 255 }, 0.1f);
    for (float x = -5.5f; x < 3.8f; x += 0.5f) Part(MESH_CUBE, V3(x, 0.02f, 0), V3(0.14f, 0.04f, 0.95f), (Color){ 90, 70, 55, 255 }, 0);
    // the engine
    Part(MESH_CUBE, V3(3.55f, 0.55f, 0), V3(1.2f, 0.7f, 0.8f), (Color){ 200, 70, 60, 255 }, 0);
    PartRot(MESH_CYLINDER, V3(3.45f, 0.95f, 0), V3(0.7f, 1.0f, 0.7f), V3(0, 0, 90), (Color){ 60, 64, 82, 255 }, 0);
    Part(MESH_CUBE, V3(3.05f, 1.15f, 0), V3(0.5f, 0.6f, 0.75f), (Color){ 200, 70, 60, 255 }, 0);
    Part(MESH_CYLINDER, V3(3.85f, 1.45f, 0), V3(0.2f, 0.4f, 0.2f), (Color){ 60, 64, 82, 255 }, 0);
    Part(MESH_SPHERE, V3(4.1f, 0.7f, 0), V3(0.18f, 0.18f, 0.18f), (Color){ 255, 230, 160, 255 }, 1.0f);
    // the wagons
    float g = Gap();
    for (int k = 0; k < T.coupled && k < WMAX; k++) {
        float s = EaseOutBack(T.pop[k]);
        Color body = T.painted[k] == 2 ? (Color){ 220, 80, 80, 255 } : T.painted[k] ? (Color){ 110, 170, 230, 255 } : (Color){ 90, 100, 130, 255 };
        Part(MESH_CUBE, V3(WagonX(k), 0.55f, 0), V3(g * 0.82f * s, 0.6f * s, 0.75f), body, T.painted[k] == 2 ? 0.4f : 0.1f);
        for (int w = -1; w <= 1; w += 2)
            PartRot(MESH_CYLINDER, V3(WagonX(k) + w * g * 0.28f, 0.2f, 0.4f), V3(0.26f, 0.06f, 0.26f), V3(90, 0, 0), (Color){ 40, 44, 58, 255 }, 0);
        Part(MESH_CUBE, V3(WagonX(k) + g * 0.46f, 0.4f, 0), V3(g * 0.12f, 0.06f, 0.06f), (Color){ 78, 86, 112, 255 }, 0);   // the coupling
        if (k == T.crown && !T.crownBad)
            for (int c = 0; c < 5; c++) {
                float a = c * 2 * PI / 5;
                Part(MESH_CONE, V3(WagonX(k) + cosf(a) * 0.16f, 1.02f, sinf(a) * 0.16f), V3(0.08f, 0.16f, 0.08f), (Color){ 255, 205, 70, 255 }, 0.8f);
            }
    }

    for (int k = 0; k < T.coupled && k < WMAX; k++) {
        Label3D(V3(WagonX(k), 0.62f, 0.42f), T.painted[k] ? T.paint[k] : "", 13, WHITE, T.painted[k] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        Label3D(V3(WagonX(k), 0.1f, 0.75f), TextFormat("[%d]", k), 11, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
    if (T.itemsShown) {
        bool counting = T.coupled < T.n;
        Label3D(V3(3.4f, 2.0f, 0), TextFormat("Items: %d", counting ? T.coupled : T.items), 15, T.itemsBad && !counting ? WHITE : LBL_GOLD,
                T.itemsBad && !counting ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        Label3D(V3(3.4f, 2.45f, 0), "size()", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    } else {
        for (int k = 0; k < T.n; k++) Label3D(V3(-5.0f + k * 0.0f, 1.8f - k * 0.3f, 0.6f), T.word[k], 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    }
    if (T.coupled > 0 && T.time - T.coupleT < 0.5f) Label3D(V3(WagonX(T.coupled - 1), 1.45f, 0), "push_back", 12, (Color){ 150, 210, 255, 255 }, LBL_DIM, LABEL_CODE);
    if (T.crown >= 0 || T.crownBad) {
        float x = T.crown >= 0 ? WagonX(T.crown) : 0;
        Label3D(V3(x, 1.6f, 0), T.crownBad && T.ev ? T.ev->actual : "Longest", 13, T.crownBad ? WHITE : LBL_GOLD, T.crownBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        if (T.crownBad && T.crownWant >= 0) Label3D(V3(WagonX(T.crownWant), 1.25f, 0.4f), "this one", 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
    if (T.want[0]) Label3D(V3(-0.8f, 2.6f, 0), TextFormat("should be: %s", T.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (T.total > WMAX) Label3D(V3(WagonX(WMAX - 1) - 0.9f, 0.7f, 0), TextFormat("+%d more", T.total - WMAX), 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_VECTORS = {
    "cpp-strings-vectors", "Cargo Train", "Each push_back couples a wagon; your loop paints the words on in order.",
    { 220, 110, 90, 255 },
    ParseVectors, VectorsReset, VectorsEvent, VectorsUpdate, VectorsDraw, NULL,
};
