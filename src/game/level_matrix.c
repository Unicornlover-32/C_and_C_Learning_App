// Matrix Sums -> "Warehouse Scanner"
//
// The input's matrix is a wall rack of crates, row i above row i+1, each tagged [i][j].
// As the program prints the matrix, a scanner head moves to each crate and lights it with
// the printed number. "Row sums:" fills a display at the end of each row while the scanner
// sweeps it; "Column sums:" fills the displays under the columns. A wrong number lights
// its crate (or display) red with the right value underneath.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { MX_SIZE = 1, MX_VALUE, MX_CELL, MX_ROWSUM, MX_COLSUM };
enum { MXF_CELL = 1, MXF_SPACING, MXF_SHORT, MXF_ROWSUM, MXF_COLSUM, MXF_GARBLED, MXF_EXTRA };

#define XMAX 10

// ---------------------------------------------------------------- the parser

// Compares a printed list with the expected one; fails at the first difference
static bool CheckList(Parse *p, int e, const int *want, int wn, const int *got, int gn, int code, int act, const char *what)
{
    char note[120];
    int k = 0;
    while (k < wn && k < gn && k < 16 && got[k] == want[k]) { EvAct(p, act, k, 0, got[k]); k++; }
    if (k == wn && k == gn) return true;
    if (k < wn && k < gn) {
        snprintf(note, sizeof(note), "%s %d should be %d", what, k + 1, want[k]);
        GameEvent *f = ParseFail(p, FAIL_WRONG, code, e, k, 0, got[k < 16 ? k : 15], note);
        if (f) snprintf(f->text, EV_TEXT, "%d", want[k]);
    } else if (gn < wn) {
        snprintf(note, sizeof(note), "Only %d of the %d %ss were printed", gn, wn, what);
        ParseFail(p, FAIL_WRONG, code, e, gn, 1, 0, note);
    } else {
        snprintf(note, sizeof(note), "%d %ss were printed - there should be %d", gn, what, wn);
        ParseFail(p, FAIL_WRONG, code, e, wn, 2, 0, note);
    }
    return false;
}

static void ParseMatrix(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[2 + XMAX * XMAX];
    int n = InputInts(run->input, in, 2 + XMAX * XMAX);
    int r = n > 0 ? in[0] : 0, c = n > 1 ? in[1] : 0;
    if (r < 0) r = 0;
    if (c < 0) c = 0;
    if (r > XMAX) r = XMAX;
    if (c > XMAX) c = XMAX;
    EvSpawn(&p, MX_SIZE, r, c, 0);
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            if (2 + i * c + j < n) EvSpawn(&p, MX_VALUE, i, j, in[2 + i * c + j]);
    if (ParsePreview(&p)) return;

    int row = 0;
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want[16], wn, got[16], gn = 0;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        bool rows = ScanAfterInts(E, el, "Row sums:", want, 16, &wn);
        bool cols = !rows && ScanAfterInts(E, el, "Column sums:", want, 16, &wn);
        if (rows || cols) {
            const char *prefix = rows ? "Row sums:" : "Column sums:";
            if (ParseTake(&p, e)) { for (int k = 0; k < wn && k < 16; k++) EvAct(&p, rows ? MX_ROWSUM : MX_COLSUM, k, 0, want[k]); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, rows ? MXF_ROWSUM : MXF_COLSUM, e, 0, 1, 0, NULL); break; }
            if (ScanAfterInts(A, al, prefix, got, 16, &gn)) {
                ParseShow(&p);
                if (CheckList(&p, e, want, wn, got, gn, rows ? MXF_ROWSUM : MXF_COLSUM, rows ? MX_ROWSUM : MX_COLSUM, rows ? "row sum" : "column sum"))
                    ParseFail(&p, FAIL_WRONG, MXF_SPACING, e, row, 0, 0, "Right numbers, wrong spacing: one space between them");
            } else if (ScanIntList(A, al, got, 16, &gn) && gn > 0) {
                ParseFail(&p, FAIL_WRONG, MXF_GARBLED, e, 0, 0, 0, rows ? "A row too many before the sums" : "Expected the column sums here");
            } else {
                ParseFail(&p, FAIL_GARBLED, MXF_GARBLED, e, 0, 0, 0, rows ? "Expected Row sums: and the numbers" : "Expected Column sums: and the numbers");
            }
            break;
        }
        // a row of the matrix
        if (!ScanIntList(E, el, want, 16, &wn)) wn = 0;
        if (ParseTake(&p, e)) { for (int k = 0; k < wn && k < 16; k++) EvAct(&p, MX_CELL, row, k, want[k]); row++; continue; }
        if (!A) { ParseFail(&p, FAIL_MISSING, MXF_SHORT, e, row, 0, 0, NULL); break; }
        if (!ScanIntList(A, al, got, 16, &gn) || gn == 0) {
            ParseFail(&p, FAIL_GARBLED, MXF_GARBLED, e, row, 0, 0, "Expected a row of the matrix here");
            break;
        }
        ParseShow(&p);
        int k = 0;
        while (k < gn && k < wn && got[k] == want[k]) { EvAct(&p, MX_CELL, row, k, got[k]); k++; }
        char note[120];
        if (k < gn && k < wn) {
            snprintf(note, sizeof(note), "[%d][%d] should be %d", row, k, want[k]);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, MXF_CELL, e, row, k, got[k], note);
            if (f) snprintf(f->text, EV_TEXT, "%d", want[k]);
        } else if (gn != wn) {
            snprintf(note, sizeof(note), "Row %d should have %d numbers, not %d", row, wn, gn);
            ParseFail(&p, FAIL_WRONG, MXF_SHORT, e, row, gn, 0, note);
        } else {
            ParseFail(&p, FAIL_WRONG, MXF_SPACING, e, row, 0, 0, "Right numbers, wrong spacing: print each one with %4d");
        }
        break;
    }
    ParseEnd(&p, MXF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { ST_IDLE, ST_GOOD, ST_BAD };
enum { DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    int   r, c;
    int   value[XMAX][XMAX];
    int   cell[XMAX][XMAX], shown[XMAX][XMAX];
    float lit[XMAX][XMAX];
    int   rowSum[XMAX], colSum[XMAX], rowState[XMAX], colState[XMAX];
    char  want[EV_TEXT];
    int   wantI, wantJ, wantKind;     // where the "should be" goes
    Vector3 head, headTo;              // the scanner
    bool  spacingRow[XMAX];
    float beam;
} X;

static float Gap(void) { return X.r > 5 ? 0.62f : 0.78f; }
static Vector3 Slot(int i, int j)
{
    float g = Gap();
    return V3((j - (X.c - 1) * 0.5f) * 0.92f, 1.2f + (X.r - 1 - i) * g, 0);
}
static Vector3 RowEnd(int i) { Vector3 s = Slot(i, X.c - 1); return V3(s.x + 1.25f, s.y, 0.1f); }
static Vector3 ColEnd(int j) { Vector3 s = Slot(X.r - 1, j); return V3(s.x, 0.42f, 0.25f); }

static void MatrixReset(const EventList *ev)
{
    memset(&X, 0, sizeof(X));
    X.ev = ev;
    X.wantI = -1;
}

static void Aim(Vector3 at) { X.headTo = V3(at.x, at.y + 0.45f, 0.75f); X.beam = 1; }

static float MatrixEvent(const GameEvent *e)
{
    int i = e->a, j = e->b;
    float quick = X.r * X.c > 30 ? 0.55f : 1.0f;
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == MX_SIZE) {
                X.r = i; X.c = j;
                float h = X.r * Gap() + 1.6f, w = X.c * 0.92f + 2.6f;
                ShotOrbit(V3(0.5f, h * 0.45f, 0), fmaxf(7.0f, fmaxf(w * 1.15f, h * 1.75f)), 14, 0, 38, 0);
                X.head = X.headTo = V3(0, h + 0.2f, 0.75f);
            }
            if (e->code == MX_VALUE && i >= 0 && i < XMAX && j >= 0 && j < XMAX) X.value[i][j] = (int)e->value;
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == MX_CELL && i >= 0 && i < X.r && j >= 0 && j < X.c) {
                X.cell[i][j] = ST_GOOD; X.shown[i][j] = (int)e->value; X.lit[i][j] = 1;
                Aim(Slot(i, j));
                return 0.32f * quick;
            }
            if (e->code == MX_ROWSUM && i >= 0 && i < X.r) {
                X.rowState[i] = ST_GOOD; X.rowSum[i] = (int)e->value;
                Aim(RowEnd(i));
                FxSparkle(RowEnd(i), 8, (Color){ 140, 255, 180, 255 });
                return 0.5f * quick;
            }
            if (e->code == MX_COLSUM && i >= 0 && i < X.c) {
                X.colState[i] = ST_GOOD; X.colSum[i] = (int)e->value;
                Aim(ColEnd(i));
                FxSparkle(ColEnd(i), 8, (Color){ 140, 255, 180, 255 });
                return 0.5f * quick;
            }
            return 0;
        case EV_FAIL:
            StrCopy(X.want, sizeof(X.want), e->text);
            if (e->code == MXF_CELL && i >= 0 && i < X.r && j >= 0 && j < X.c) {
                X.cell[i][j] = ST_BAD; X.shown[i][j] = (int)e->value;
                X.wantI = i; X.wantJ = j; X.wantKind = 0;
                Aim(Slot(i, j));
            } else if ((e->code == MXF_ROWSUM || e->code == MXF_COLSUM) && i >= 0) {
                bool rows = e->code == MXF_ROWSUM;
                int lim = rows ? X.r : X.c;
                if (i < lim) {
                    if (rows) { X.rowState[i] = ST_BAD; X.rowSum[i] = (int)e->value; } else { X.colState[i] = ST_BAD; X.colSum[i] = (int)e->value; }
                    X.wantI = i; X.wantKind = rows ? 1 : 2;
                    if (e->b != 0) X.want[0] = '\0';              // too few / too many: no single value to show
                    Aim(rows ? RowEnd(i) : ColEnd(i));
                }
            } else if (e->code == MXF_SPACING && i >= 0 && i < XMAX) {
                X.spacingRow[i] = true;
            }
            CueAt(&X.cues, X.time + 0.35f, DO_BAD, 0, 0, X.headTo, 0);
            return 2.6f;
        case EV_CRASH:
            CueAt(&X.cues, X.time + 0.1f, DO_BAD, 0, 0, X.head, 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&X.cues, X.time + 0.2f, DO_CHEER, 0, 0, X.head, 0);
            return 2.2f;
    }
    return 0;
}

static void MatrixUpdate(float dt)
{
    X.time += dt;
    X.head = LerpV3(X.head, X.headTo, 1 - expf(-14 * dt));
    X.beam = Smooth(X.beam, 0.25f, 3, dt);
    for (int i = 0; i < XMAX; i++) for (int j = 0; j < XMAX; j++) X.lit[i][j] = Smooth(X.lit[i][j], 0, 2.5f, dt);
    Cue c;
    while (CueDue(&X.cues, X.time, &c)) {
        if (c.what == DO_BAD) {
            FxSparks(V3(c.v.x, c.v.y - 0.3f, 0.2f), 32, (Color){ 255, 100, 90, 255 });
            PlaybackShake(0.2f);
        } else {
            FxConfetti(V3(0, Slot(0, 0).y + 0.8f, 0.4f), PlaybackFinale() ? 150 : 70, X.c * 0.45f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static void MatrixDraw(void)
{
    if (X.r <= 0 || X.c <= 0) { KitStage(V3(0, 0, 0), 4, 2); return; }
    float g = Gap();
    Vector3 tl = Slot(0, 0), br = Slot(X.r - 1, X.c - 1);
    float w = br.x - tl.x + 0.92f;
    KitStage(V3(0.6f, 0, 0.2f), fmaxf(4.0f, w / 2 + 2.2f), 2.2f);
    // the rack: a back panel, shelves under each row, and posts
    Part(MESH_CUBE, V3(0, (tl.y + br.y) / 2 + 0.1f, -0.42f), V3(w + 0.3f, tl.y - br.y + g + 0.2f, 0.08f), (Color){ 44, 50, 68, 255 }, 0);
    for (int i = 0; i < X.r; i++)
        Part(MESH_CUBE, V3(0, Slot(i, 0).y - 0.05f, -0.1f), V3(w + 0.3f, 0.06f, 0.7f), (Color){ 90, 98, 124, 255 }, 0);
    for (int s = -1; s <= 1; s += 2)
        Part(MESH_CUBE, V3(s * (w / 2 + 0.15f), (tl.y + 0.6f) / 2, -0.1f), V3(0.1f, tl.y + 0.6f, 0.7f), (Color){ 78, 86, 112, 255 }, 0);
    for (int i = 0; i < X.r; i++)
        for (int j = 0; j < X.c; j++) {
            Color col = { 170, 128, 82, 255 };
            float glow = X.lit[i][j] * 0.6f;
            if (X.cell[i][j] == ST_GOOD) col = (Color){ 120, 170, 240, 255 };
            if (X.cell[i][j] == ST_BAD) { col = (Color){ 230, 80, 80, 255 }; glow = 0.6f; }
            KitCrate(V3(Slot(i, j).x, Slot(i, j).y - 0.02f, -0.1f), 0.5f, col, glow, 0);
        }
    // the sum displays
    for (int i = 0; i < X.r; i++) {
        Color tint = X.rowState[i] == ST_BAD ? (Color){ 235, 90, 90, 255 } : X.rowState[i] ? (Color){ 80, 220, 130, 255 } : (Color){ 60, 70, 100, 255 };
        KitPanel(V3(RowEnd(i).x, RowEnd(i).y + 0.22f, -0.1f), 0.9f, 0.42f, (Color){ (unsigned char)(tint.r / 3), (unsigned char)(tint.g / 3), (unsigned char)(tint.b / 3), 255 }, X.rowState[i] ? 0.5f : 0);
    }
    for (int j = 0; j < X.c; j++) {
        Color tint = X.colState[j] == ST_BAD ? (Color){ 235, 90, 90, 255 } : X.colState[j] ? (Color){ 80, 220, 130, 255 } : (Color){ 60, 70, 100, 255 };
        KitPanel(V3(ColEnd(j).x, ColEnd(j).y, 0.2f), 0.78f, 0.38f, (Color){ (unsigned char)(tint.r / 3), (unsigned char)(tint.g / 3), (unsigned char)(tint.b / 3), 255 }, X.colState[j] ? 0.5f : 0);
    }
    // the scanner: a rail above the rack and a head with a beam
    float top = tl.y + 0.9f;
    Part(MESH_CUBE, V3(0.6f, top + 0.4f, 0.75f), V3(w + 3.2f, 0.08f, 0.08f), (Color){ 78, 86, 112, 255 }, 0);
    Part(MESH_CUBE, V3(X.head.x, top + 0.4f, 0.75f), V3(0.3f, 0.2f, 0.3f), (Color){ 60, 66, 90, 255 }, 0);
    Part(MESH_CYLINDER, V3(X.head.x, (top + 0.4f + X.head.y) / 2, 0.75f), V3(0.05f, top + 0.4f - X.head.y, 0.05f), (Color){ 120, 130, 160, 255 }, 0);
    Part(MESH_SPHERE, X.head, V3(0.2f, 0.2f, 0.2f), (Color){ 120, 220, 255, 255 }, 0.9f);
    PartGlass(MESH_CONE, V3(X.head.x, X.head.y - 0.12f, 0.5f), V3(0.3f, 0.26f, 0.3f), (Color){ 120, 220, 255, (unsigned char)(70 * X.beam) });

    // labels: indexes and values
    for (int i = 0; i < X.r; i++)
        for (int j = 0; j < X.c; j++) {
            Vector3 s = Slot(i, j);
            int st = X.cell[i][j];
            const char *t = st ? TextFormat("%d", X.shown[i][j]) : TextFormat("%d", X.value[i][j]);
            Color fg = st == ST_BAD ? WHITE : st ? LBL_GOLD : LBL_SOFT, bg = st == ST_BAD ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
            if (X.spacingRow[i]) { fg = (Color){ 255, 160, 60, 255 }; s.x += sinf(X.time * 40 + j) * 0.03f; }
            Label3D(V3(s.x, s.y + 0.27f, 0.25f), t, 15, fg, bg, LABEL_CODE);
            if (X.r * X.c <= 36) Label3D(V3(s.x, s.y - 0.02f, 0.3f), TextFormat("[%d][%d]", i, j), 10, (Color){ 140, 150, 180, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        }
    for (int i = 0; i < X.r; i++)
        Label3D(V3(RowEnd(i).x, RowEnd(i).y + 0.22f, 0.05f), X.rowState[i] ? TextFormat("%d", X.rowSum[i]) : "row sum", X.rowState[i] ? 16 : 10,
                X.rowState[i] == ST_BAD ? WHITE : X.rowState[i] ? LBL_GOOD : (Color){ 120, 130, 160, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    for (int j = 0; j < X.c; j++)
        Label3D(V3(ColEnd(j).x, ColEnd(j).y, 0.35f), X.colState[j] ? TextFormat("%d", X.colSum[j]) : "col", X.colState[j] ? 15 : 10,
                X.colState[j] == ST_BAD ? WHITE : X.colState[j] ? LBL_GOOD : (Color){ 120, 130, 160, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (X.want[0] && X.wantI >= 0) {
        Vector3 at = X.wantKind == 0 ? Slot(X.wantI, X.wantJ) : X.wantKind == 1 ? RowEnd(X.wantI) : ColEnd(X.wantI);
        Label3D(V3(at.x, at.y - 0.25f, 0.5f), TextFormat("should be %s", X.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    }
}

const LevelDef LEVEL_MATRIX = {
    "c-arrays-2d", "Warehouse Scanner", "The scanner reads your matrix crate by crate, then totals each row and column.",
    { 120, 170, 240, 255 },
    ParseMatrix, MatrixReset, MatrixEvent, MatrixUpdate, MatrixDraw, NULL,
};
