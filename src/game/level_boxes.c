// Box Arithmetic -> "Box Fusion"
//
// Box1 and Box2 are real boxes, sized by the sides each printed line shows (operator<<),
// with their volume (getVolume) on a tag. For Box3 both slide into the operator+ fusion
// machine and a bigger box comes out with the printed sides. "Same size:" is a balance
// scale: operator== levels it when all three sides match and tips it when they don't.
// A wrong box warps red, with the right one underneath; common slips are named.
#include "levels.h"
#include "stage.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BX_INPUT = 1, BX_BOX, BX_SAME };
enum { BXF_BOX = 1, BXF_SAME, BXF_GARBLED, BXF_EXTRA };

// "Box2: Box(12x13x10) volume 1560" -> 12, 13, 10, 1560
static bool ScanBox(const char *s, int len, int which, double *l, double *b, double *h, double *v)
{
    char buf[128], prefix[16];
    int k = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1, used = 0;
    memcpy(buf, s, (size_t)k);
    buf[k] = '\0';
    snprintf(prefix, sizeof(prefix), "Box%d: Box(", which);
    int pl = (int)strlen(prefix);
    if (strncmp(buf, prefix, (size_t)pl) != 0) return false;
    if (sscanf(buf + pl, "%lfx%lfx%lf) volume %lf%n", l, b, h, v, &used) != 4) return false;
    return pl + used == k;
}

// ---------------------------------------------------------------- the parser

static void ParseBoxes(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    double in[6] = { 0 };
    sscanf(run->input ? run->input : "", "%lf %lf %lf %lf %lf %lf", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5]);
    for (int k = 0; k < 2; k++) {
        GameEvent *g = EvSpawn(&p, BX_INPUT, k + 1, 0, 0);
        if (g) snprintf(g->text, EV_TEXT, "%.6g %.6g %.6g", in[3 * k], in[3 * k + 1], in[3 * k + 2]);
    }
    if (ParsePreview(&p)) return;

    char note[140];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (el >= 10 && memcmp(E, "Same size:", 10) == 0) {
            bool yes = strstr(E, "yes") != NULL;
            if (ParseTake(&p, e)) { EvAct(&p, BX_SAME, yes, 0, 0); continue; }
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, BXF_SAME, e, A && strstr(A, "yes") ? 1 : 0, yes, 0,
                      "operator== is true only when all three sides match");
            break;
        }
        int which = (el > 3 && E[0] == 'B') ? E[3] - '0' : 0;
        double l, b, h, v, gl, gb, gh, gv;
        if (which < 1 || which > 3 || !ScanBox(E, el, which, &l, &b, &h, &v)) { ParseTake(&p, e); continue; }
        if (ParseTake(&p, e)) {
            GameEvent *a = EvAct(&p, BX_BOX, which, 0, v);
            if (a) snprintf(a->text, EV_TEXT, "%g %g %g", l, b, h);
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, BXF_BOX, e, which, 0, 0, NULL); break; }
        if (!ScanBox(A, al, which, &gl, &gb, &gh, &gv)) {
            ParseFail(&p, FAIL_GARBLED, BXF_GARBLED, e, which, 0, 0, "operator<< prints a box as Box(LxBxH) - no spaces");
            break;
        }
        if (fabs(gv - gl * gb * gh) > 1e-6 && fabs(gl - l) < 1e-9 && fabs(gb - b) < 1e-9 && fabs(gh - h) < 1e-9)
            snprintf(note, sizeof(note), "getVolume is length * breadth * height = %g", v);
        else if (which == 3) snprintf(note, sizeof(note), "operator+ adds side to side: Box(%gx%gx%g)", l, b, h);
        else snprintf(note, sizeof(note), "Box%d should be Box(%gx%gx%g) volume %g", which, l, b, h, v);
        GameEvent *f = ParseFail(&p, FAIL_WRONG, BXF_BOX, e, which, 0, gv, note);
        if (f) snprintf(f->text, EV_TEXT, "%g %g %g", gl, gb, gh);
        break;
    }
    ParseEnd(&p, BXF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_FUSE, DO_CHEER, DO_BAD };

static struct {
    const EventList *ev;
    Cues  cues;
    float time;
    double side[4][3], vol[4];
    bool  shown[4], bad[4];
    float pop[4];
    float fuse;                   // boxes 1 and 2 sliding into the machine
    bool  fusing, sameShown, same, sameBad;
    float tilt;
    double scale;
    char  want[96];
} X;

static Vector3 BoxHome(int k) { return k == 1 ? V3(-3.0f, 0.55f, 0) : k == 2 ? V3(-1.0f, 0.55f, 0) : V3(2.6f, 0.55f, 0); }
static Vector3 Size(int k)
{
    float s = (float)X.scale;
    return V3((float)X.side[k][0] * s, (float)X.side[k][2] * s, (float)X.side[k][1] * s);   // length across, height up, breadth back
}

static void BoxesReset(const EventList *ev)
{
    memset(&X, 0, sizeof(X));
    X.ev = ev;
    X.scale = 0.08;
    ShotOrbit(V3(-0.2f, 1.0f, 0), 9.6f, 22, 0, 38, 0);
}

static void Rescale(void)
{
    double big = 0.1;
    for (int k = 1; k <= 3; k++) for (int j = 0; j < 3; j++) if (X.shown[k] && fabs(X.side[k][j]) > big) big = fabs(X.side[k][j]);
    X.scale = 1.7 / big;
    if (X.scale > 0.6) X.scale = 0.6;
}

static float BoxesEvent(const GameEvent *e)
{
    int k = e->a;
    switch (e->type) {
        case EV_SPAWN: {
            if (k < 1 || k > 2) return 0;
            double in[3] = { 0 };
            sscanf(e->text, "%lf %lf %lf", &in[0], &in[1], &in[2]);
            for (int j = 0; j < 3; j++) { X.side[k][j] = in[j]; X.side[3][j] = X.side[1][j] + X.side[2][j]; }
            X.shown[1] = X.shown[2] = X.shown[3] = true;     // for the size of the stage only
            Rescale();
            X.shown[1] = X.shown[2] = X.shown[3] = false;
            return 0;
        }
        case EV_PRINT: return 0.05f;
        case EV_ACT:
        case EV_FAIL:
            if ((e->code == BX_BOX || e->code == BXF_BOX) && k >= 1 && k <= 3) {
                double l, b, h;
                if (sscanf(e->text, "%lf %lf %lf", &l, &b, &h) == 3) { X.side[k][0] = l; X.side[k][1] = b; X.side[k][2] = h; }
                X.vol[k] = e->value;
                X.bad[k] = e->type == EV_FAIL;
                if (X.bad[k] && X.ev) StrCopy(X.want, sizeof(X.want), X.ev->expected);
                if (k == 3) {
                    X.fusing = true;
                    X.fuse = 0;
                    CueAt(&X.cues, X.time + 1.0f, DO_FUSE, 3, 0, V3(0, 0, 0), 0);
                } else {
                    X.shown[k] = true;
                    X.pop[k] = 0;
                    if (X.bad[k]) CueAt(&X.cues, X.time + 0.5f, DO_BAD, 0, 0, BoxHome(k), 0);
                }
                return X.bad[k] ? 2.8f : (k == 3 ? 2.2f : 1.2f);
            }
            if (e->code == BX_SAME || e->code == BXF_SAME) {
                X.sameShown = true;
                X.same = e->a != 0;
                X.sameBad = e->type == EV_FAIL;
                if (X.sameBad) { if (X.ev) StrCopy(X.want, sizeof(X.want), X.ev->expected); CueAt(&X.cues, X.time + 0.8f, DO_BAD, 0, 0, V3(-2.0f, 1.6f, 1.0f), 0); }
                return X.sameBad ? 2.8f : 1.6f;
            }
            CueAt(&X.cues, X.time + 0.3f, DO_BAD, 0, 0, V3(0, 1.4f, 0), 0);
            return 2.4f;
        case EV_CRASH:
            CueAt(&X.cues, X.time + 0.1f, DO_BAD, 0, 0, V3(0, 1.4f, 0), 0);
            return 2.4f;
        case EV_WIN:
            CueAt(&X.cues, X.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void BoxesUpdate(float dt)
{
    X.time += dt;
    for (int k = 1; k <= 3; k++) if (X.shown[k]) X.pop[k] = fminf(1, X.pop[k] + dt * 2.5f);
    if (X.fusing) X.fuse = fminf(1, X.fuse + dt * 1.1f);
    // the scale tips by the difference in volume (or sits level when the program says "yes")
    float want = 0;
    if (X.sameShown && !X.same) want = (X.vol[1] > X.vol[2] ? -1.0f : 1.0f) * 12;
    X.tilt = Smooth(X.tilt, want, 3, dt);
    Cue c;
    while (CueDue(&X.cues, X.time, &c)) {
        if (c.what == DO_FUSE) {
            X.shown[3] = true;
            X.pop[3] = 0;
            X.fusing = false;
            FxSparkle(V3(BoxHome(3).x, 1.2f, 0), 24, (Color){ 255, 220, 140, 255 });
            if (X.bad[3]) CueAt(&X.cues, X.time + 0.4f, DO_BAD, 0, 0, BoxHome(3), 0);
        } else if (c.what == DO_BAD) { FxSparks(V3(c.v.x, c.v.y + 0.8f, c.v.z), 32, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else { FxConfetti(V3(0, 2.2f, 0), PlaybackFinale() ? 150 : 70, 2.5f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void DrawBox(int k, Vector3 at, float s)
{
    Vector3 sz = Size(k);
    sz = V3(sz.x * s, sz.y * s, sz.z * s);
    Color col = X.bad[k] ? (Color){ 230, 80, 80, 255 } : k == 3 ? (Color){ 255, 200, 110, 255 } : (Color){ 150, 190, 240, 255 };
    float wobble = X.bad[k] ? sinf(X.time * 18) * 6 : 0;
    PartRot(MESH_CUBE, V3(at.x, at.y + sz.y / 2, at.z), sz, V3(0, 0, wobble), col, X.bad[k] ? 0.4f : 0.12f);
}

static void BoxesDraw(void)
{
    KitStage(V3(-0.2f, 0, 0.2f), 4.8f, 2.0f);
    // pedestals, the fusion machine, the balance scale
    for (int k = 1; k <= 3; k++) Part(MESH_CYLINDER, V3(BoxHome(k).x, 0.27f, 0), V3(1.6f, 0.54f, 1.6f), (Color){ 70, 76, 100, 255 }, 0);
    KitMachine(V3(0.7f, 0, -0.3f), V3(1.3f, 1.9f, 1.1f), (Color){ 150, 100, 200, 255 }, (Color){ 230, 180, 255, 255 }, 0.7f, X.fusing ? X.time : 0);
    Vector3 piv = V3(-2.0f, 0, 1.6f);
    Part(MESH_CYLINDER, V3(piv.x, 0.45f, piv.z), V3(0.12f, 0.9f, 0.12f), (Color){ 78, 86, 112, 255 }, 0);
    rlPushMatrix();
    rlTranslatef(piv.x, 0.95f, piv.z);
    rlRotatef(X.tilt, 0, 0, 1);
    Part(MESH_CUBE, V3(0, 0, 0), V3(1.8f, 0.06f, 0.1f), X.sameBad ? (Color){ 235, 90, 90, 255 } : (Color){ 200, 180, 120, 255 }, X.sameShown ? 0.4f : 0);
    for (int s = -1; s <= 1; s += 2) Part(MESH_CYLINDER, V3(s * 0.85f, -0.1f, 0), V3(0.5f, 0.04f, 0.5f), (Color){ 200, 180, 120, 255 }, 0);
    rlPopMatrix();

    for (int k = 1; k <= 2; k++) {
        if (!X.shown[k]) continue;
        Vector3 at = BoxHome(k);
        float s = EaseOutBack(X.pop[k]);
        if (X.fusing || X.shown[3]) {       // slide into the machine for operator+
            float f = X.shown[3] ? 1 : EaseInOutQuad(X.fuse);
            at = LerpV3(at, V3(0.7f, 0.55f, -0.3f), f);
            s *= 1 - f * 0.9f;
            if (X.shown[3]) continue;
        }
        if (s > 0.02f) DrawBox(k, at, s);
    }
    if (X.shown[3]) DrawBox(3, BoxHome(3), EaseOutBack(X.pop[3]));

    for (int k = 1; k <= 3; k++) {
        if (!X.shown[k] || (k < 3 && (X.fusing || X.shown[3]))) continue;
        Vector3 at = BoxHome(k);
        float top = Size(k).y + 0.8f;
        const char *t = TextFormat("Box%d: Box(%gx%gx%g)", k, X.side[k][0], X.side[k][1], X.side[k][2]);
        Label3D(V3(at.x, at.y + top, 0), t, 13, X.bad[k] ? WHITE : LBL_SOFT, X.bad[k] ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
        Label3D(V3(at.x, at.y + top - 0.38f, 0), TextFormat("volume %g", X.vol[k]), 13, LBL_GOLD, LBL_DIM, LABEL_CODE);
    }
    if (X.shown[3] || X.fusing) for (int k = 1; k <= 2; k++)
        Label3D(V3(BoxHome(k).x, 1.0f, 0.6f), TextFormat("Box%d", k), 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    Label3D(V3(0.7f, 2.55f, -0.3f), "operator+", 14, (Color){ 230, 200, 255, 255 }, LBL_DIM, LABEL_CODE);
    Label3D(V3(piv.x, 1.55f, piv.z), X.sameShown ? (X.same ? "operator==: yes" : "operator==: no") : "operator==", 13,
            X.sameBad ? WHITE : X.sameShown ? LBL_GOOD : LBL_SOFT, X.sameBad ? (Color){ 200, 50, 55, 235 } : LBL_DIM, LABEL_CODE);
    if (X.want[0]) Label3D(V3(-0.2f, 3.3f, 0), TextFormat("should be: %s", X.want), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
}

const LevelDef LEVEL_BOXES = {
    "cpp-overloading", "Box Fusion", "Your operators at work: + fuses two boxes, == weighs them, << labels them.",
    { 230, 180, 255, 255 },
    ParseBoxes, BoxesReset, BoxesEvent, BoxesUpdate, BoxesDraw, NULL,
};
