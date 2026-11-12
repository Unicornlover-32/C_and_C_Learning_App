// Shop Receipt -> "Price Dials"
//
// A shop console has a dial for each variable (int quantity, double price, subtotal, tax
// and total) and a locked plaque for the constant TAX_RATE. Each line the program prints
// spins its dial to the printed value and comes out of the receipt printer on the counter.
// A wrong value makes its dial overshoot and spark; the same value printed with the wrong
// number of decimals is pointed out as a formatting slip.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { RC_START = 1, RC_DIAL };
enum { RCF_DIAL = 1, RCF_ORDER, RCF_GARBLED, RCF_EXTRA };

#define DIALS 5
static const char *const KEYS[DIALS] = { "Quantity:", "Price:", "Subtotal:", "Tax:", "Total:" };
static const char *const NAMES[DIALS] = { "quantity", "price", "subtotal", "tax", "total" };
static const char *const TYPES[DIALS] = { "int", "double", "double", "double", "double" };
static const float RANGE[DIALS] = { 5, 25, 80, 8, 80 };

static int KeyOf(const char *s, int len)
{
    for (int i = 0; i < DIALS; i++) {
        int n = (int)strlen(KEYS[i]);
        if (s && len >= n && memcmp(s, KEYS[i], (size_t)n) == 0) return i;
    }
    return -1;
}

// ---------------------------------------------------------------- the parser

static void ParseReceipt(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    EvSpawn(&p, RC_START, 0, 0, 0);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int k = KeyOf(E, el);
        double want = 0, got = 0;
        if (k < 0 || !ScanAfterNumber(E, el, KEYS[k], &want)) k = -1;
        if (ParseTake(&p, e)) {
            if (k >= 0) {
                GameEvent *g = EvAct(&p, RC_DIAL, k, e, want);
                if (g) ScanAfterText(E, el, KEYS[k], g->text, EV_TEXT);
            }
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, RCF_DIAL, e, k, 0, 0, NULL); break; }
        int ak = KeyOf(A, al);
        if (k >= 0 && ak == k && ScanAfterNumber(A, al, KEYS[k], &got)) {
            if (fabs(got - want) < 0.006)
                snprintf(note, sizeof(note), "Same value, wrong format: money needs two decimal places (%%.2f)");
            else
                snprintf(note, sizeof(note), "%.*s should be %.*s", (int)strlen(KEYS[k]) - 1, KEYS[k], el - (int)strlen(KEYS[k]) - 1,
                         E + strlen(KEYS[k]) + 1);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, RCF_DIAL, e, k, 0, got, note);
            if (f) ScanAfterText(A, al, KEYS[k], f->text, EV_TEXT);
        } else if (ak >= 0) {
            ParseFail(&p, FAIL_WRONG, RCF_ORDER, e, k, ak, 0, "The lines are in the wrong order, or one is missing");
        } else {
            ParseFail(&p, FAIL_GARBLED, RCF_GARBLED, e, k, 0, 0, "The receipt printer can't read this line");
        }
        break;
    }
    ParseEnd(&p, RCF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_JAM, DO_CHEER };

static struct {
    const EventList *ev;
    Bot   bot;
    Cues  cues;
    float time;
    float dial[DIALS], target[DIALS];   // needle positions (0..1, more overshoots)
    float setT[DIALS];                  // when each dial was set (-1 never)
    int   state[DIALS];                 // 0 waiting, 1 right, 2 wrong
    char  shown[DIALS][EV_TEXT];        // the printed value
    char  want[DIALS][EV_TEXT];
    int   receipt;                      // lines on the receipt
    char  lines[8][48];
    bool  jam;
    int   items;                        // crates on the counter
    float itemT;
    int   focus;
} R;

static Vector3 DialPos(int i) { return V3(-2.7f + i * 1.35f, 1.55f, -1.2f); }
static Vector3 PrinterPos(void) { return V3(-1.75f, 0.89f, 0.5f); }

static void ReceiptReset(const EventList *ev)
{
    memset(&R, 0, sizeof(R));
    R.ev = ev;
    R.focus = -1;
    for (int i = 0; i < DIALS; i++) R.setT[i] = -1;
    BotInit(&R.bot, V3(2.75f, 0, 0.55f), -20);
    ShotOrbit(V3(0, 1.0f, -0.2f), 8.6f, 22, 0, 38, 0);
}

static float ReceiptEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN: return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code != RC_DIAL || i < 0 || i >= DIALS) return 0;
            R.state[i] = 1;
            R.target[i] = (float)(e->value / RANGE[i]);
            R.setT[i] = R.time;
            StrCopy(R.shown[i], EV_TEXT, e->text);
            if (R.receipt < 8) snprintf(R.lines[R.receipt++], 48, "%s %.36s", KEYS[i], e->text);
            R.focus = i;
            if (i == 0) { R.items = (int)e->value; R.itemT = 0; }
            FxSparkle(DialPos(i), 8, (Color){ 140, 230, 255, 255 });
            return 1.15f;
        case EV_FAIL:
            if (e->code == RCF_DIAL && i >= 0 && i < DIALS) {
                R.state[i] = 2;
                R.target[i] = (float)(e->value / RANGE[i]);
                R.setT[i] = R.time;
                StrCopy(R.shown[i], EV_TEXT, e->text[0] ? e->text : "?");
                {   // what it should be: the expected line after its label
                    const char *x = R.ev->expected;
                    int n = (int)strlen(KEYS[i]);
                    StrCopy(R.want[i], EV_TEXT, strncmp(x, KEYS[i], (size_t)n) == 0 ? x + n + 1 : x);
                }
                if (R.receipt < 8 && R.ev->actual[0]) snprintf(R.lines[R.receipt++], 48, "%.47s", R.ev->actual);
                R.focus = i;
            } else {
                CueAt(&R.cues, R.time + 0.3f, DO_JAM, 0, 0, V3(0, 0, 0), 0);
                if (R.receipt < 8) snprintf(R.lines[R.receipt++], 48, "%.47s", R.ev->actual[0] ? R.ev->actual : "???");
            }
            BotMood(&R.bot, MOOD_SAD);
            BotShakeHead(&R.bot);
            return 2.6f;
        case EV_CRASH:
            BotPowerDown(&R.bot);
            PlaybackShake(0.2f);
            return 2.4f;
        case EV_WIN:
            CueAt(&R.cues, R.time + 0.3f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void ReceiptUpdate(float dt)
{
    R.time += dt;
    BotUpdate(&R.bot, dt);
    R.itemT += dt;
    Cue c;
    while (CueDue(&R.cues, R.time, &c)) {
        if (c.what == DO_JAM) {
            R.jam = true;
            FxSparks(PrinterPos(), 30, (Color){ 255, 120, 80, 255 });
            PlaybackShake(0.2f);
        } else if (c.what == DO_CHEER) {
            BotFace(&R.bot, 0);
            BotCheer(&R.bot);
            FxConfetti(V3(0, 2.2f, -0.6f), PlaybackFinale() ? 150 : 70, 2.5f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
    for (int i = 0; i < DIALS; i++) {
        if (R.setT[i] < 0) continue;
        float t = (R.time - R.setT[i]) / 0.85f;
        if (R.state[i] == 2) {
            // overshoots past the end, sparks, and wobbles there
            float over = fmaxf(R.target[i], 1.0f) + 0.15f;
            R.dial[i] = t < 1 ? Lerpf(0, over, EaseOutCubic(t)) : over + sinf(R.time * 30) * 0.02f;
            if (t >= 1 && t - dt / 0.85f < 1) {
                FxSparks(DialPos(i), 34, (Color){ 255, 120, 80, 255 });
                PlaybackShake(0.2f);
            }
        } else {
            R.dial[i] = Lerpf(0, R.target[i], EaseOutBack(Clamp01(t)));
        }
    }
    if (R.focus >= 0) {          // the clerk looks at the dial being set
        Vector3 d = Vector3Subtract(DialPos(R.focus), R.bot.pos);
        BotFace(&R.bot, atan2f(d.x, d.z) * RAD2DEG);
    }
}

static void ReceiptDraw(void)
{
    KitStage(V3(0, 0, 0), 4.6f, 2.3f);
    // the console with its dials
    Part(MESH_CUBE, V3(0, 1.25f, -1.55f), V3(7.6f, 2.5f, 0.5f), (Color){ 56, 62, 84, 255 }, 0);
    Part(MESH_CUBE, V3(0, 2.52f, -1.5f), V3(7.8f, 0.08f, 0.6f), (Color){ 86, 156, 255, 255 }, 0.5f);
    for (int i = 0; i < DIALS; i++) {
        Color needle = R.state[i] == 2 ? (Color){ 255, 90, 90, 255 } : (Color){ 255, 200, 70, 255 };
        float glow = R.state[i] ? 0.9f : 0.3f;
        KitGauge(DialPos(i), 0.5f, R.dial[i], needle, glow);
        KitPanel(V3(DialPos(i).x, 0.75f, -1.28f), 1.15f, 0.24f, R.state[i] == 1 ? (Color){ 30, 70, 50, 255 } : (Color){ 28, 30, 42, 255 },
                 R.state[i] == 1 ? 0.3f : 0);
    }
    // the constant: a locked plaque
    KitPanel(V3(0, 2.36f, -1.28f), 2.6f, 0.2f, (Color){ 70, 50, 20, 255 }, 0.2f);
    Part(MESH_CUBE, V3(-1.5f, 2.33f, -1.22f), V3(0.18f, 0.15f, 0.06f), (Color){ 255, 200, 70, 255 }, 0.4f);
    Part(MESH_CUBE, V3(-1.5f, 2.43f, -1.22f), V3(0.11f, 0.11f, 0.03f), (Color){ 160, 170, 190, 255 }, 0);

    // the counter, the printer and its receipt
    Part(MESH_CUBE, V3(0, 0.42f, 0.45f), V3(4.2f, 0.84f, 0.9f), (Color){ 92, 70, 50, 255 }, 0);
    Part(MESH_CUBE, V3(0, 0.86f, 0.45f), V3(4.3f, 0.06f, 1.0f), (Color){ 130, 100, 72, 255 }, 0);
    Vector3 pr = PrinterPos();
    Part(MESH_CUBE, V3(pr.x, pr.y + 0.12f, pr.z), V3(0.6f, 0.24f, 0.45f), (Color){ 60, 64, 82, 255 }, 0);
    Part(MESH_CUBE, V3(pr.x, pr.y + 0.25f, pr.z + 0.12f), V3(0.4f, 0.03f, 0.08f), R.jam ? (Color){ 235, 90, 90, 255 } : (Color){ 80, 220, 130, 255 }, 0.8f);
    float paper = 0.12f + R.receipt * 0.17f;
    Part(MESH_CUBE, V3(pr.x, pr.y + 0.26f + paper / 2, pr.z - 0.1f), V3(0.42f, paper, 0.01f), (Color){ 235, 232, 220, 255 }, 0.05f);
    // the shopping: one crate for each item of the quantity
    for (int i = 0; i < R.items && i < 6; i++) {
        float k = EaseOutBack(Clamp01(R.itemT * 2.5f - i * 0.3f));
        if (k <= 0) continue;
        KitCrate(V3(0.1f + i * 0.5f - (1 - k) * 2, 0.89f, 0.45f), 0.36f, (Color){ 120, 180, 255, 255 }, 0.15f, 0);
    }
    if (R.items > 0) Label3D(V3(0.6f, 0.62f, 0.95f), TextFormat("%d x %s", R.items, R.shown[1][0] ? R.shown[1] : "19.99"), 13, LBL_GOLD, LBL_DIM, LABEL_CODE);
    BotDraw(&R.bot);

    // labels: the variables, the values, the constant, the receipt
    for (int i = 0; i < DIALS; i++) {
        Vector3 d = DialPos(i);
        Label3D(V3(d.x, 0.95f, -1.2f), TYPES[i], 11, (Color){ 130, 150, 200, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        Label3D(V3(d.x, 0.75f, -1.2f), NAMES[i], 14, R.state[i] == 1 ? LBL_GOOD : LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        if (R.state[i]) {
            Color fg = R.state[i] == 2 ? WHITE : LBL_GOLD, bg = R.state[i] == 2 ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
            Label3D(V3(d.x, d.y - 0.26f, d.z + 0.2f), R.shown[i], 16, fg, bg, LABEL_CODE);
            if (R.state[i] == 2 && R.want[i][0])
                Label3D(V3(d.x, d.y + 0.62f, d.z + 0.2f), TextFormat("should be %s", R.want[i]), 13, LBL_GOOD, LBL_DIM, LABEL_CODE);
        } else {
            Label3D(V3(d.x, d.y - 0.26f, d.z + 0.2f), "?", 15, LBL_SOFT, LBL_DIM, LABEL_CODE);
        }
    }
    Label3D(V3(0.15f, 2.36f, -1.15f), "const double TAX_RATE = 0.08", 12, (Color){ 255, 214, 140, 255 }, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    if (R.jam) Label3D(V3(pr.x, pr.y + 0.5f + paper, pr.z), "jammed!", 13, WHITE, (Color){ 200, 50, 55, 235 }, LABEL_CODE);
}

const LevelDef LEVEL_RECEIPT = {
    "c-variables-const", "Price Dials", "Every value your program prints spins the dial of its variable.",
    { 255, 200, 70, 255 },
    ParseReceipt, ReceiptReset, ReceiptEvent, ReceiptUpdate, ReceiptDraw, NULL,
};
