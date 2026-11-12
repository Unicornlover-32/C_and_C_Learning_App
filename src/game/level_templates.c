// Template Toolkit -> "Mold Machine"
//
// Three lanes hold the input: three ints, three doubles, three words. One machine does
// all the work: for each printed line it snaps in a mold for T - int, double or string -
// and that lane's items go through. biggest<T> crowns the winner it printed; Adder<T>
// merges the three into the printed sum - which for strings means joining them. A wrong
// result cracks red; common template slips (an Adder that doesn't start at T(), words
// compared the wrong way) are named.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MD_INPUT = 1, MD_RESULT };
enum { MDF_RESULT = 1, MDF_GARBLED, MDF_EXTRA };

static const char *const KEYS[6] = { "Biggest int:", "Biggest double:", "Biggest word:", "Sum of ints:", "Sum of doubles:", "Joined words:" };
static const char *const TYPES[3] = { "int", "double", "string" };

// ---------------------------------------------------------------- the parser

static void ParseTemplates(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    char tok[9][24];
    int n = 0;
    const char *s = run->input ? run->input : "";
    int used;
    while (n < 9 && sscanf(s, " %23s%n", tok[n], &used) == 1) { s += used; n++; }
    for (int i = 0; i < n; i++) { GameEvent *g = EvSpawn(&p, MD_INPUT, i / 3, i % 3, 0); if (g) StrCopy(g->text, EV_TEXT, tok[i]); }
    if (ParsePreview(&p)) return;

    char note[140];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int k = -1;
        for (int i = 0; i < 6; i++) { int kl = (int)strlen(KEYS[i]); if (el >= kl && memcmp(E, KEYS[i], (size_t)kl) == 0) k = i; }
        if (k < 0) { ParseTake(&p, e); continue; }
        char want[64], got[64];
        ScanAfterText(E, el, KEYS[k], want, sizeof(want));
        if (ParseTake(&p, e)) { GameEvent *g = EvAct(&p, MD_RESULT, k, 0, 0); if (g) StrCopy(g->text, EV_TEXT, want); continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, MDF_RESULT, e, k, 0, 0, NULL); break; }
        if (!ScanAfterText(A, al, KEYS[k], got, sizeof(got))) { ParseFail(&p, FAIL_GARBLED, MDF_GARBLED, e, k, 0, 0, "The label doesn't match"); break; }
        if (k == 5 && (strcmp(got, "0") == 0 || got[0] == '\0')) snprintf(note, sizeof(note), "Start m_sum at T() - for a string that's \"\", and + joins words");
        else if (k == 2) snprintf(note, sizeof(note), "Words compare alphabetically with >: the biggest is %s", want);
        else if (k == 4 && strchr(want, '.') && !strchr(got, '.')) snprintf(note, sizeof(note), "Adder<double> must add doubles - is m_sum a double (T)?");
        else snprintf(note, sizeof(note), "%.*s should be %s", (int)strlen(KEYS[k]) - 1, KEYS[k], want);
        GameEvent *f = ParseFail(&p, FAIL_WRONG, MDF_RESULT, e, k, 0, 0, note);
        if (f) StrCopy(f->text, EV_TEXT, got);
        break;
    }
    ParseEnd(&p, MDF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_OUT, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    char  item[3][3][EV_TEXT];
    int   mold;                    // the type in the machine (-1 none)
    int   active;                  // the result being worked out (-1)
    char  result[6][EV_TEXT];
    int   state[6];                // 0 none, 1 right, 2 wrong
    int   crown[3];                // the item crowned in each lane (-1)
    float feed;
    char  want[96];
} M;

static float LaneZ(int lane) { return -1.0f + lane * 1.0f; }
static Vector3 ItemPos(int lane, int i) { return V3(-4.0f + i * 0.85f, BELT_Y, LaneZ(lane)); }
static Vector3 MachinePos(void) { return V3(0.3f, 0, 0); }
static Vector3 OutPos(int k) { int lane = k % 3; return V3(k < 3 ? 2.4f : 3.8f, BELT_Y, LaneZ(lane)); }

static void TemplatesReset(const EventList *ev)
{
    memset(&M, 0, sizeof(M));
    M.ev = ev;
    M.mold = M.active = -1;
    for (int i = 0; i < 3; i++) M.crown[i] = -1;
    ShotOrbit(V3(0, 0.9f, 0.2f), 9.6f, 34, 0, 38, 0);
}

static float TemplatesEvent(const GameEvent *e)
{
    int k = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (k >= 0 && k < 3 && e->b >= 0 && e->b < 3) StrCopy(M.item[k][e->b], EV_TEXT, e->text);
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
        case EV_FAIL:
            if (k >= 0 && k < 6 && (e->code == MD_RESULT || e->code == MDF_RESULT)) {
                M.mold = k % 3;
                M.active = k;
                M.feed = 0;
                StrCopy(M.result[k], EV_TEXT, e->text);
                bool bad = e->type == EV_FAIL;
                if (bad && M.ev) StrCopy(M.want, sizeof(M.want), M.ev->expected);
                CueAt(&M.cues, M.time + 0.9f, DO_OUT, k, bad, V3(0, 0, 0), 0);
                if (bad) CueAt(&M.cues, M.time + 1.1f, DO_BAD, 0, 0, OutPos(k), 0);
                return bad ? 3.0f : 1.6f;
            }
            CueAt(&M.cues, M.time + 0.3f, DO_BAD, 0, 0, V3(0.3f, 1.4f, 0), 0);
            return 2.4f;
        case EV_CRASH:
            CueAt(&M.cues, M.time + 0.1f, DO_BAD, 0, 0, V3(0.3f, 1.4f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&M.cues, M.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void TemplatesUpdate(float dt)
{
    M.time += dt;
    M.feed += dt;
    Cue c;
    while (CueDue(&M.cues, M.time, &c)) {
        if (c.what == DO_OUT) {
            int k = c.a;
            M.state[k] = c.b ? 2 : 1;
            if (k < 3 && !c.b) {                         // crown the item the program printed
                for (int i = 0; i < 3; i++) if (strcmp(M.item[k][i], M.result[k]) == 0 ||
                    (strtod(M.item[k][i], NULL) == strtod(M.result[k], NULL) && k < 2 && M.result[k][0])) { M.crown[k] = i; break; }
            }
            M.active = -1;
            FxSparkle(V3(OutPos(k).x, 1.2f, OutPos(k).z), 10, (Color){ 255, 220, 140, 255 });
        } else if (c.what == DO_BAD) { FxSparks(V3(c.v.x, c.v.y + 0.6f, c.v.z), 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0.3f, 2.2f, 0), PlaybackFinale() ? 150 : 70, 2.5f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void TemplatesDraw(void)
{
    KitStage(V3(0, 0, 0), 5.0f, 2.0f);
    static const Color laneCol[3] = { { 120, 170, 240, 255 }, { 150, 220, 170, 255 }, { 240, 190, 120, 255 } };
    for (int lane = 0; lane < 3; lane++) KitBelt(V3(-4.6f, 0, LaneZ(lane)), 4.4f, M.active % 3 == lane && M.active >= 0 ? M.feed * 1.6f : 0);
    // the machine, with the mold for T on its front
    Vector3 mp = MachinePos();
    KitMachine(mp, V3(1.5f, 2.0f, 3.0f), (Color){ 90, 96, 130, 255 }, M.mold >= 0 ? laneCol[M.mold] : (Color){ 120, 130, 160, 255 }, M.mold >= 0 ? 0.8f : 0.2f,
               M.active >= 0 ? M.time : 0);
    if (M.mold >= 0) Part(MESH_CUBE, V3(mp.x, 1.25f, mp.z + 1.52f), V3(0.9f, 0.4f, 0.04f), laneCol[M.mold], 0.5f);
    for (int lane = 0; lane < 3; lane++)
        for (int i = 0; i < 3; i++) {
            Vector3 p = ItemPos(lane, i);
            bool going = M.active >= 0 && M.active % 3 == lane;
            if (going) p.x += fminf(1, M.feed / 0.8f) * 0.4f;
            KitCrate(p, 0.42f, laneCol[lane], M.crown[lane] == i ? 0.6f : 0.1f, 0);
            if (M.crown[lane] == i)
                for (int c = 0; c < 5; c++) {
                    float a = c * 2 * PI / 5;
                    Part(MESH_CONE, V3(p.x + cosf(a) * 0.14f, p.y + 0.58f, p.z + sinf(a) * 0.14f), V3(0.07f, 0.14f, 0.07f), (Color){ 255, 205, 70, 255 }, 0.8f);
                }
        }
    for (int k = 3; k < 6; k++) if (M.state[k])
        KitCrate(OutPos(k), 0.55f, M.state[k] == 2 ? (Color){ 230, 80, 80, 255 } : laneCol[k % 3], M.state[k] == 2 ? 0.5f : 0.3f, 0);

    for (int lane = 0; lane < 3; lane++) {
        Label3D(V3(-4.9f, BELT_Y + 0.3f, LaneZ(lane) + 0.3f), TYPES[lane], 12, laneCol[lane], LBL_DIM, LABEL_CODE);
        for (int i = 0; i < 3; i++) {
            Vector3 p = ItemPos(lane, i);
            Label3D(V3(p.x, p.y + 0.62f, p.z + 0.15f), M.item[lane][i], 13, WHITE, LBL_DIM, LABEL_CODE);
        }
    }
    Label3D(V3(mp.x, 2.45f, mp.z), M.mold >= 0 ? TextFormat("T = %s", TYPES[M.mold]) : "template <typename T>", 15, M.mold >= 0 ? laneCol[M.mold] : LBL_SOFT, LBL_DIM, LABEL_CODE);
    for (int k = 0; k < 6; k++) {
        if (!M.state[k]) continue;
        Vector3 o = OutPos(k);
        bool bad = M.state[k] == 2;
        const char *t = k < 3 ? TextFormat("biggest: %s", M.result[k]) : TextFormat("%s", M.result[k]);
        Label3D(V3(o.x, o.y + (k < 3 ? 0.35f : 0.85f), o.z + 0.2f), t, 13, bad ? WHITE : LBL_GOLD, bad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    }
    Label3D(V3(2.4f, 0.1f, 1.8f), "biggest<T>", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(3.8f, 0.1f, 1.8f), "Adder<T>", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    if (M.want[0]) Label3D(V3(0.3f, 3.0f, 0), TextFormat("should be: %s", M.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_TEMPLATES = {
    "cpp-templates", "Mold Machine", "One template, many types: the machine snaps in a mold for T and does the job.",
    { 150, 220, 170, 255 },
    ParseTemplates, TemplatesReset, TemplatesEvent, TemplatesUpdate, TemplatesDraw, NULL,
};
