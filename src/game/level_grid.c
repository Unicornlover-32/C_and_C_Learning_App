// Dynamic Grid -> "Heap Grid"
//
// grid is an int**: it points at a column of row pointers (made with new int*[rows]),
// and each row pointer points at its own block of ints out on the heap (new int[cols]).
// Each row the program prints builds that row's block, cell by cell, with an arrow from
// its row pointer; "Total:" adds up the cells. Wrong numbers light their cell red; a row
// too short or too long is pointed out.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { GD_SIZE = 1, GD_CELL, GD_TOTAL };
enum { GDF_CELL = 1, GDF_SHORT, GDF_SPACING, GDF_TOTAL, GDF_GARBLED, GDF_EXTRA };

#define GMAX 10

// ---------------------------------------------------------------- the parser

static void ParseGrid(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[2] = { 0, 0 };
    InputInts(run->input, in, 2);
    EvSpawn(&p, GD_SIZE, in[0], in[1], 0);
    if (ParsePreview(&p)) return;

    char note[120];
    int row = 0;
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int gl[64], gn = 0, wl[64], wn = 0;
        bool aInts = A && ScanIntList(A, al, gl, 64, &gn) && gn > 0;
        if (ScanAfter(E, el, "Total:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, GD_TOTAL, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, GDF_TOTAL, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Total:", &got)) {
                snprintf(note, sizeof(note), "The cells add up to %d", want);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, GDF_TOTAL, e, 0, 0, got, note);
                if (f) snprintf(f->text, EV_TEXT, "%d", want);
            } else if (aInts) {
                ParseFail(&p, FAIL_WRONG, GDF_SHORT, e, row, 0, 0, "One row too many");
            } else {
                ParseFail(&p, FAIL_GARBLED, GDF_GARBLED, e, 0, 0, 0, "Expected Total: and a number");
            }
            break;
        }
        if (!ScanIntList(E, el, wl, 64, &wn)) wn = 0;
        if (wn > 64) wn = 64;
        if (ParseTake(&p, e)) { for (int k = 0; k < wn && k < GMAX; k++) EvAct(&p, GD_CELL, row, k, wl[k]); row++; continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, GDF_SHORT, e, row, 0, 0, NULL); break; }
        if (!aInts) {
            ParseFail(&p, ScanAfter(A, al, "Total:", &got) ? FAIL_WRONG : FAIL_GARBLED, GDF_SHORT, e, row, 0, 0, "Expected the next row of numbers");
            break;
        }
        if (gn > 64) gn = 64;
        ParseShow(&p);
        int k = 0;
        while (k < wn && k < gn && gl[k] == wl[k]) { if (k < GMAX) EvAct(&p, GD_CELL, row, k, gl[k]); k++; }
        if (k < wn && k < gn) {
            snprintf(note, sizeof(note), "grid[%d][%d] should be %d", row, k, wl[k]);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, GDF_CELL, e, row, k, gl[k], note);
            if (f) snprintf(f->text, EV_TEXT, "%d", wl[k]);
        } else if (gn != wn) {
            snprintf(note, sizeof(note), "Row %d has %d numbers - each row has %d", row, gn, wn);
            ParseFail(&p, FAIL_WRONG, GDF_SHORT, e, row, gn, 0, note);
        } else {
            ParseFail(&p, FAIL_WRONG, GDF_SPACING, e, row, 0, 0, "Right numbers, wrong spacing: one space between them");
        }
        break;
    }
    ParseEnd(&p, GDF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   r, c, rr, cc;                 // the input, and what is drawn (clamped)
    int   cell[GMAX][GMAX];             // 0 not built, 1 right, 2 wrong
    int   value[GMAX][GMAX];
    float pop[GMAX][GMAX];
    bool  rowBuilt[GMAX];
    int   total;
    bool  totalShown, totalBad;
    char  want[EV_TEXT];
    int   wantI, wantJ;
    int   sum;
} D;

static float CellGap(void) { float g = 6.0f / (D.cc + 1); return g > 0.85f ? 0.85f : g; }
static float RowGap(void) { float g = 3.2f / (D.rr + 0.5f); return g > 0.9f ? 0.9f : g; }
static Vector3 Cell(int i, int j) { return V3(-1.2f + j * CellGap(), 0, -1.2f + i * RowGap()); }
static Vector3 RowPtr(int i) { return V3(-3.4f, 0.25f + 0.0f, -1.2f + i * RowGap()); }

static void GridReset(const EventList *ev)
{
    memset(&D, 0, sizeof(D));
    D.ev = ev;
    D.wantI = -1;
    D.rr = D.cc = 1;
}

static float GridEvent(const GameEvent *e)
{
    int i = e->a, j = e->b;
    switch (e->type) {
        case EV_SPAWN:
            D.r = i; D.c = j;
            D.rr = i < 1 ? 1 : i > GMAX ? GMAX : i;
            D.cc = j < 1 ? 1 : j > GMAX ? GMAX : j;
            ShotOrbit(V3(0.2f, 0.2f, 0.1f), 9.6f, 48, 0, 37, 0);
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == GD_CELL && i >= 0 && i < D.rr && j >= 0 && j < D.cc) {
                if (!D.rowBuilt[i]) { D.rowBuilt[i] = true; FxSparkle(RowPtr(i), 8, (Color){ 200, 170, 255, 255 }); }
                D.cell[i][j] = 1;
                D.value[i][j] = (int)e->value;
                D.sum += (int)e->value;
                return D.rr * D.cc > 30 ? 0.12f : 0.22f;
            }
            if (e->code == GD_TOTAL) { D.totalShown = true; D.total = (int)e->value; FxSparkle(V3(3.6f, 1.5f, 0), 16, (Color){ 255, 220, 120, 255 }); return 1.3f; }
            return 0;
        case EV_FAIL:
            StrCopy(D.want, sizeof(D.want), e->text);
            if (e->code == GDF_CELL && i >= 0 && i < D.rr && j >= 0 && j < D.cc) {
                if (!D.rowBuilt[i]) D.rowBuilt[i] = true;
                D.cell[i][j] = 2; D.value[i][j] = (int)e->value;
                D.wantI = i; D.wantJ = j;
            } else if (e->code == GDF_TOTAL) {
                D.totalShown = D.totalBad = true;
                D.total = (int)e->value;
            }
            CueAt(&D.cues, D.time + 0.3f, DO_BAD, 0, 0, i >= 0 && i < D.rr && j >= 0 && j < D.cc && e->code == GDF_CELL ? Cell(i, j) : V3(3.6f, 1.2f, 0), 0);
            return 2.6f;
        case EV_CRASH:
            CueAt(&D.cues, D.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.0f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&D.cues, D.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void GridUpdate(float dt)
{
    D.time += dt;
    for (int i = 0; i < GMAX; i++) for (int j = 0; j < GMAX; j++) if (D.cell[i][j]) D.pop[i][j] = fminf(1, D.pop[i][j] + dt * 4);
    Cue c;
    while (CueDue(&D.cues, D.time, &c)) {
        if (c.what == DO_BAD) { FxSparks(V3(c.v.x, c.v.y + 0.6f, c.v.z), 30, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0.5f, 1.6f, 0), PlaybackFinale() ? 150 : 70, 2.2f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void GridDraw(void)
{
    KitStage(V3(0, 0, 0.1f), 5.2f, 2.6f);
    // the stack side: grid itself, pointing at its row pointers
    Part(MESH_CUBE, V3(-4.5f, 0.02f, 0), V3(1.4f, 0.04f, 4.4f), (Color){ 40, 46, 64, 255 }, 0);
    Vector3 g = V3(-4.5f, 0.9f, -1.6f);
    PartRot(MESH_CYLINDER, g, V3(0.42f, 0.1f, 0.42f), V3(90, 0, 0), (Color){ 200, 170, 255, 255 }, 0.6f);
    KitArc(V3(g.x, g.y - 0.2f, g.z), V3(RowPtr(0).x, 0.5f, RowPtr(0).z), 0.4f, 1, (Color){ 200, 170, 255, 255 }, 0.5f);
    // the row pointers, and the heap with one block per row
    KitLot(V3(1.4f, 0, (Cell(0, 0).z + Cell(D.rr - 1, 0).z) / 2), CellGap() * D.cc + 2.0f, RowGap() * D.rr + 0.8f,
           (Color){ 120, 255, 200, 255 }, 0.35f);
    for (int i = 0; i < D.rr; i++) {
        Vector3 rp = RowPtr(i);
        Part(MESH_CUBE, rp, V3(0.5f, 0.5f, RowGap() * 0.8f), D.rowBuilt[i] ? (Color){ 150, 130, 220, 255 } : (Color){ 70, 76, 100, 255 }, D.rowBuilt[i] ? 0.08f : 0);
        if (D.rowBuilt[i]) KitArc(V3(rp.x + 0.25f, 0.5f, rp.z), V3(Cell(i, 0).x - 0.35f, 0.4f, rp.z), 0.5f, 1, (Color){ 200, 170, 255, 255 }, 0.5f);
        if (D.rowBuilt[i]) Part(MESH_CUBE, V3(Cell(i, 0).x + CellGap() * (D.cc - 1) / 2, 0.03f, Cell(i, 0).z), V3(CellGap() * D.cc + 0.1f, 0.06f, RowGap() * 0.85f),
                                (Color){ 50, 80, 76, 255 }, 0.1f);
        for (int j = 0; j < D.cc; j++) {
            if (!D.cell[i][j]) continue;
            float k = EaseOutBack(D.pop[i][j]);
            Color col = D.cell[i][j] == 2 ? (Color){ 230, 80, 80, 255 } : (Color){ 110, 200, 170, 255 };
            KitCrate(V3(Cell(i, j).x, 0.06f, Cell(i, j).z), fminf(CellGap(), RowGap()) * 0.62f * k, col, D.cell[i][j] == 2 ? 0.5f : 0.15f, 0);
        }
    }
    KitScreen(V3(3.9f, 0, -0.4f), 1.5f, 0.6f, D.totalBad ? (Color){ 235, 90, 90, 255 } : (Color){ 255, 200, 90, 255 }, D.totalShown ? 1 : 0.2f);

    for (int i = 0; i < D.rr; i++) {
        Vector3 rp = RowPtr(i);
        Label3D(V3(rp.x, 0.62f, rp.z), TextFormat("grid[%d]", i), 12, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
        if (D.rowBuilt[i] && i == 0) Label3D(V3(Cell(i, 0).x + 0.6f, 1.05f, rp.z - 0.1f), TextFormat("new int[%d]", D.c), 12, (Color){ 140, 255, 210, 255 }, LBL_DIM, LABEL_CODE);
        for (int j = 0; j < D.cc; j++) {
            if (!D.cell[i][j]) continue;
            Color fg = D.cell[i][j] == 2 ? WHITE : LBL_GOLD, bg = D.cell[i][j] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
            Label3D(V3(Cell(i, j).x, 0.62f, Cell(i, j).z + 0.1f), TextFormat("%d", D.value[i][j]), 14, fg, bg, LABEL_CODE);
        }
    }
    Label3D(V3(g.x, g.y + 0.45f, g.z), "int **grid", 13, (Color){ 220, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(1.4f, 0.05f, Cell(D.rr - 1, 0).z + RowGap() * 0.5f + 0.5f), "heap", 13, (Color){ 140, 255, 210, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    Vector3 sc = KitScreenCenter(V3(3.9f, 0, -0.4f), 0.6f);
    Label3D(sc, D.totalShown ? TextFormat("Total: %d", D.total) : TextFormat("sum: %d", D.sum), 16, D.totalBad ? WHITE : LBL_GOLD, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (D.totalBad) Label3D(V3(sc.x, sc.y - 0.6f, sc.z), TextFormat("should be %s", D.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (D.wantI >= 0) Label3D(V3(Cell(D.wantI, D.wantJ).x, 1.05f, Cell(D.wantI, D.wantJ).z), TextFormat("should be %s", D.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_GRID = {
    "cpp-new-delete", "Heap Grid", "new builds each row out on the heap; grid holds the pointers to them.",
    { 120, 255, 200, 255 },
    ParseGrid, GridReset, GridEvent, GridUpdate, GridDraw, NULL,
};
