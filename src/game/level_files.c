// Save and Load -> "Save Crystal"
//
// The file is a crystal on a pedestal. The input's numbers sit in the program's memory as
// glowing orbs; writing sends them into the crystal one by one (fprintf). Then the scene
// "restarts": the lights go down and memory is wiped - only the crystal still holds them.
// "Loaded N numbers" brings N orbs back out (fscanf), and "Total:" merges them into the
// sum. Too few orbs (the file wasn't closed before reading, say) leave the rest dark in
// the crystal; a wrong total cracks red.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { SC_NUM = 1, SC_COUNT, SC_LOADED, SC_TOTAL };
enum { SCF_LOADED = 1, SCF_TOTAL, SCF_GARBLED, SCF_EXTRA };

#define OMAX 12

static bool ScanLoaded(const char *s, int len, int *n)
{
    char buf[64];
    int k = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1, used = 0;
    memcpy(buf, s, (size_t)k);
    buf[k] = '\0';
    return sscanf(buf, "Loaded %d numbers%n", n, &used) == 1 && used == k;
}

// ---------------------------------------------------------------- the parser

static void ParseFiles(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[101];
    int n = InputInts(run->input, in, 101), count = n > 0 ? in[0] : 0;
    if (count < 0) count = 0;
    if (count > n - 1) count = n > 0 ? n - 1 : 0;
    for (int i = 0; i < count && i < OMAX; i++) EvSpawn(&p, SC_NUM, i, 0, in[1 + i]);
    EvSpawn(&p, SC_COUNT, count, 0, 0);
    if (ParsePreview(&p)) return;

    char note[140];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ScanLoaded(E, el, &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, SC_LOADED, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, SCF_LOADED, e, 0, 0, 0, NULL); break; }
            if (ScanLoaded(A, al, &got)) {
                if (got == 0) snprintf(note, sizeof(note), "Nothing came back: fclose the file after writing, then fopen it again to read");
                else if (got == want + 1) snprintf(note, sizeof(note), "One too many: count only while fscanf(...) == 1");
                else snprintf(note, sizeof(note), "%d numbers were saved, so %d should load", want, want);
                ParseFail(&p, FAIL_WRONG, SCF_LOADED, e, 0, 0, got, note);
            } else {
                ParseFail(&p, FAIL_GARBLED, SCF_GARBLED, e, 0, 0, 0, "Expected: Loaded N numbers");
            }
            break;
        }
        if (ScanAfter(E, el, "Total:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, SC_TOTAL, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, SCF_TOTAL, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Total:", &got)) {
                snprintf(note, sizeof(note), "The numbers in the file add up to %d", want);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, SCF_TOTAL, e, 0, 0, got, note);
                if (f) snprintf(f->text, EV_TEXT, "%d", want);
            } else {
                ParseFail(&p, FAIL_GARBLED, SCF_GARBLED, e, 0, 0, 0, "Expected Total: and a number");
            }
            break;
        }
        ParseTake(&p, e);
    }
    ParseEnd(&p, SCF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_WRITE, DO_RESTART, DO_LOAD, DO_MERGE, DO_CHEER, DO_BAD };
enum { ORB_MEMORY, ORB_FLYING_IN, ORB_STORED, ORB_FLYING_OUT, ORB_LOADED, ORB_MERGED };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   count, shown, value[OMAX];
    int   orb[OMAX];
    float orbT[OMAX];
    bool  started, restarted, loadedBad, totalShown, totalBad;
    int   loaded, total;
    float dark;                     // the restart's blackout
    char  want[EV_TEXT];
} S;

static Vector3 Crystal(void) { return V3(0, 1.4f, -0.4f); }
static Vector3 MemSlot(int i)  { return V3(-3.55f + (i % 6) * 0.46f, 1.0f + (i / 6) * 0.5f, 1.2f); }
static Vector3 LoadSlot(int i) { return V3(1.45f + (i % 6) * 0.46f, 1.0f + (i / 6) * 0.5f, 1.2f); }
static Vector3 TotalPos(void) { return V3(3.6f, 1.2f, 0.4f); }

static void FilesReset(const EventList *ev)
{
    memset(&S, 0, sizeof(S));
    S.ev = ev;
    ShotOrbit(V3(0, 1.2f, 0.4f), 9.2f, 22, 0, 38, 0);
}

static void Begin(void)
{
    if (S.started) return;
    S.started = true;
    for (int i = 0; i < S.shown; i++) CueAt(&S.cues, S.time + 0.2f + i * 0.2f, DO_WRITE, i, 0, V3(0, 0, 0), 0);
    CueAt(&S.cues, S.time + 0.6f + S.shown * 0.2f, DO_RESTART, 0, 0, V3(0, 0, 0), 0);
}

static float StartDelay(void) { return 1.6f + S.shown * 0.2f; }

static float FilesEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == SC_NUM && e->a >= 0 && e->a < OMAX) { S.value[e->a] = (int)e->value; if (e->a + 1 > S.shown) S.shown = e->a + 1; }
            if (e->code == SC_COUNT) S.count = e->a;
            return 0;
        case EV_PRINT:
            if (!S.started) { Begin(); return StartDelay(); }
            return 0.05f;
        case EV_ACT:
        case EV_FAIL:
            if (e->code == SC_LOADED || (e->type == EV_FAIL && e->code == SCF_LOADED)) {
                int v = (int)e->value;
                S.loaded = v;
                S.loadedBad = e->type == EV_FAIL;
                int k = v < S.shown ? v : S.shown;
                for (int i = 0; i < k; i++) CueAt(&S.cues, S.time + 0.2f + i * 0.2f, DO_LOAD, i, 0, V3(0, 0, 0), 0);
                if (S.loadedBad) CueAt(&S.cues, S.time + 0.4f + k * 0.2f, DO_BAD, 0, 0, Crystal(), 0);
                return (S.loadedBad ? 2.4f : 0.6f) + k * 0.2f;
            }
            if (e->code == SC_TOTAL || (e->type == EV_FAIL && e->code == SCF_TOTAL)) {
                S.totalShown = true;
                S.total = (int)e->value;
                S.totalBad = e->type == EV_FAIL;
                StrCopy(S.want, sizeof(S.want), e->type == EV_FAIL ? e->text : "");
                for (int i = 0; i < S.shown; i++) if (S.orb[i] == ORB_LOADED) CueAt(&S.cues, S.time + 0.1f + i * 0.08f, DO_MERGE, i, 0, V3(0, 0, 0), 0);
                if (S.totalBad) CueAt(&S.cues, S.time + 0.9f, DO_BAD, 0, 0, TotalPos(), 0);
                return S.totalBad ? 2.6f : 1.4f;
            }
            if (!S.started) Begin();
            CueAt(&S.cues, S.time + 0.3f, DO_BAD, 0, 0, Crystal(), 0);
            return 2.4f;
        case EV_CRASH:
            CueAt(&S.cues, S.time + 0.1f, DO_BAD, 0, 0, Crystal(), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&S.cues, S.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void FilesUpdate(float dt)
{
    S.time += dt;
    S.dark = Smooth(S.dark, 0, 1.6f, dt);
    for (int i = 0; i < OMAX; i++) S.orbT[i] += dt;
    Cue c;
    while (CueDue(&S.cues, S.time, &c)) {
        int i = c.a;
        switch (c.what) {
            case DO_WRITE: S.orb[i] = ORB_FLYING_IN; S.orbT[i] = 0; break;
            case DO_RESTART:
                S.restarted = true;
                S.dark = 1;
                for (int k = 0; k < S.shown; k++) S.orb[k] = ORB_STORED;
                break;
            case DO_LOAD: S.orb[i] = ORB_FLYING_OUT; S.orbT[i] = 0; break;
            case DO_MERGE: S.orb[i] = ORB_MERGED; S.orbT[i] = 0; break;
            case DO_BAD: FxSparks(c.v, 34, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); break;
            case DO_CHEER:
                FxConfetti(V3(0, 2.6f, 0), PlaybackFinale() ? 150 : 70, 2.0f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
}

static Vector3 OrbPos(int i)
{
    float t = Clamp01(S.orbT[i] / 0.5f), k = EaseInOutQuad(t);
    Vector3 cr = V3(Crystal().x + ((i % 4) - 1.5f) * 0.18f, Crystal().y - 0.2f + (i / 4) * 0.25f, Crystal().z);
    switch (S.orb[i]) {
        case ORB_MEMORY:     return MemSlot(i);
        case ORB_FLYING_IN:  { Vector3 p = LerpV3(MemSlot(i), cr, k); p.y += sinf(k * PI) * 0.8f; return p; }
        case ORB_STORED:     return cr;
        case ORB_FLYING_OUT: { Vector3 p = LerpV3(cr, LoadSlot(i), k); p.y += sinf(k * PI) * 0.8f; return p; }
        case ORB_LOADED:     return LoadSlot(i);
        default:             return LerpV3(LoadSlot(i), TotalPos(), k);
    }
}

static void FilesDraw(void)
{
    KitStage(V3(0.4f, 0, 0.3f), 4.8f, 1.8f);
    // the program's memory (left) and what it loaded (right): two trays
    Part(MESH_CUBE, V3(-2.4f, 0.4f, 1.2f), V3(2.8f, 0.8f, 0.9f), (Color){ 60, 66, 90, 255 }, 0);
    Part(MESH_CUBE, V3(2.6f, 0.4f, 1.2f), V3(3.0f, 0.8f, 0.9f), (Color){ 60, 66, 90, 255 }, 0);
    // the crystal: the file on its pedestal
    Vector3 c = Crystal();
    Part(MESH_CYLINDER, V3(c.x, 0.3f, c.z), V3(1.1f, 0.6f, 1.1f), (Color){ 70, 76, 100, 255 }, 0);
    int stored = 0;
    for (int i = 0; i < S.shown; i++) if (S.orb[i] == ORB_STORED) stored++;
    float glow = 0.25f + 0.08f * stored + 0.15f * Pulse(S.time, 2);
    Part(MESH_PYRAMID, V3(c.x, c.y + 0.45f, c.z), V3(0.9f, 0.9f, 0.9f), (Color){ 150, 210, 255, 255 }, glow * 0.8f);
    PartRot(MESH_PYRAMID, V3(c.x, c.y - 0.45f, c.z), V3(0.9f, 0.9f, 0.9f), V3(180, 0, 0), (Color){ 90, 140, 230, 255 }, glow);
    Part(MESH_SPHERE, V3(c.x, c.y, c.z), V3(0.35f, 0.35f, 0.35f), (Color){ 160, 220, 255, 255 }, glow);
    for (int i = 0; i < S.shown; i++) {
        if (S.orb[i] == ORB_MERGED && S.orbT[i] > 0.5f) continue;
        Color col = S.orb[i] == ORB_STORED ? (Color){ 140, 200, 255, 255 } : (Color){ 255, 214, 120, 255 };
        if (S.orb[i] == ORB_STORED && S.loadedBad && S.loaded >= 0 && i >= S.loaded) col = (Color){ 80, 90, 110, 255 };
        if (S.orb[i] == ORB_MEMORY && S.restarted) continue;
        Part(MESH_SPHERE, OrbPos(i), V3(0.26f, 0.26f, 0.26f), col, 0.8f);
    }
    if (S.totalShown) KitPanel(V3(TotalPos().x, TotalPos().y + 0.6f, TotalPos().z), 1.3f, 0.45f, S.totalBad ? (Color){ 120, 30, 34, 255 } : (Color){ 40, 90, 60, 255 }, 0.3f);

    for (int i = 0; i < S.shown; i++) {
        bool vis = S.orb[i] == ORB_MEMORY ? !S.restarted : S.orb[i] == ORB_LOADED || S.orb[i] == ORB_FLYING_OUT || S.orb[i] == ORB_FLYING_IN;
        if (!vis) continue;
        Vector3 p = OrbPos(i);
        Label3D(V3(p.x, p.y + 0.3f, p.z + 0.1f), TextFormat("%d", S.value[i]), 13, WHITE, LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(c.x, 2.55f, c.z), "scores.txt", 15, (Color){ 170, 220, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(-2.4f, 0.25f, 1.7f), S.restarted ? "memory (wiped)" : "memory", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(2.6f, 0.25f, 1.7f), "loaded back", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    if (S.started && !S.restarted) Label3D(V3(-1.2f, 2.2f, 0.4f), "fprintf", 14, LBL_GOLD, LBL_DIM, LABEL_CODE);
    if (S.restarted && S.dark > 0.2f) Label3D(V3(0, 2.0f, 1.0f), "restart: memory is gone, the file isn't", 14, WHITE, LBL_DIM, LABEL_CODE);
    if (S.loaded || S.loadedBad)
        Label3D(V3(2.6f, 2.3f, 1.0f), TextFormat("Loaded %d numbers", S.loaded), 15, S.loadedBad ? WHITE : LBL_GOOD,
                S.loadedBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (S.loadedBad) Label3D(V3(2.6f, 1.85f, 1.0f), TextFormat("the file holds %d", S.count), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (S.totalShown) {
        Label3D(V3(TotalPos().x, TotalPos().y + 0.6f, TotalPos().z + 0.1f), TextFormat("Total: %d", S.total), 15, S.totalBad ? WHITE : LBL_GOLD, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        if (S.totalBad && S.want[0]) Label3D(V3(TotalPos().x, TotalPos().y + 1.15f, TotalPos().z), TextFormat("should be %s", S.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
}

// The restart: a short blackout over the scene
static void FilesOverlay(Rectangle view)
{
    if (S.dark > 0.01f) DrawRectangleRec(view, (Color){ 0, 0, 0, (unsigned char)(200 * S.dark) });
}

const LevelDef LEVEL_FILES = {
    "c-files", "Save Crystal", "fprintf stores your numbers in the file; after a restart, fscanf brings them back.",
    { 140, 200, 255, 255 },
    ParseFiles, FilesReset, FilesEvent, FilesUpdate, FilesDraw, FilesOverlay,
};
