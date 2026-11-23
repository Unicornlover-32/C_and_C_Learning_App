// Times Table -> "Coin Grid"
//
// The input numbers queue up at the gate. Each "Please enter 1 to 12" the program prints
// turns one of them away; the first row of the table opens the gate for the valid one.
// The table is a grid of tiles with a face-down coin on each. Every number the program
// prints sends the robot to tile (row, column) to turn its coin over: the coin shows the
// printed number, then flies into the running total over the exit door. "Total: N" opens
// the door. A wrong number shows up on its coin, a short row leaves coins behind, and a
// number too many sends the robot into the wall.
#include "levels.h"
#include "kits.h"
#include "../ui.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { LP_TRY = 1, LP_GRID, LP_REJECT, LP_ACCEPT, LP_COIN, LP_ROWEND, LP_TOTAL };
enum { LPF_GATE = 1, LPF_LET_IN, LPF_COIN, LPF_LEFT, LPF_BUMP, LPF_SPACING, LPF_TOTAL, LPF_GARBLED, LPF_EXTRA };

#define GMAX    12
#define TRY_MAX 8
#define TILE    1.08f

// ---------------------------------------------------------------- the parser

static void ParseLoops(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);

    // The world: the input numbers wait at the gate; the first one from 1 to 12 is the table size
    int in[TRY_MAX], nIn = InputInts(run->input, in, TRY_MAX), n = 0;
    for (int i = 0; i < nIn; i++) {
        EvSpawn(&p, LP_TRY, i, 0, in[i]);
        if (n == 0 && in[i] >= 1 && in[i] <= GMAX) n = in[i];
    }
    EvSpawn(&p, LP_GRID, n, 0, n);
    if (ParsePreview(&p)) return;

    int tries = 0, row = 0;
    char note[96];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, t;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        int got[16], gn = 0;
        bool aInts = A && ScanIntList(A, al, got, 16, &gn) && gn > 0;

        if (LineIs(E, el, "Please enter 1 to 12")) {                 // the gate turns a number away
            double value = tries < nIn ? in[tries] : 0;
            if (ParseTake(&p, e)) { EvAct(&p, LP_REJECT, tries, 0, value); tries++; continue; }
            if (!A) ParseFail(&p, FAIL_MISSING, LPF_GATE, e, tries, 0, value, NULL);
            else if (aInts) ParseFail(&p, FAIL_WRONG, LPF_LET_IN, e, tries, 0, value, "Numbers outside 1 to 12 must be turned away");
            else ParseFail(&p, FAIL_WRONG, LPF_GATE, e, tries, 0, value, "The gate's message isn't right");
            break;
        }

        if (ScanAfter(E, el, "Total:", &want)) {                     // the exit door
            if (ParseTake(&p, e)) { EvAct(&p, LP_TOTAL, 0, 0, want); continue; }
            if (!A) {
                ParseFail(&p, FAIL_MISSING, LPF_TOTAL, e, 0, 0, 0, NULL);
            } else if (ScanAfter(A, al, "Total:", &t)) {
                snprintf(note, sizeof(note), "The coins add up to %d", want);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, LPF_TOTAL, e, 0, 0, t, note);
                if (f) snprintf(f->text, EV_TEXT, "%d", want);
            } else if (aInts) {
                ParseFail(&p, FAIL_WRONG, LPF_BUMP, e, row, 0, got[0], "One row too many");
            } else {
                ParseFail(&p, FAIL_GARBLED, LPF_GARBLED, e, 0, 0, 0, "The robot can't read this line");
            }
            break;
        }

        // a row of the table
        int wantRow[GMAX], wn = 0;
        if (!ScanIntList(E, el, wantRow, GMAX, &wn) || wn > GMAX) wn = 0;
        if (ParseTake(&p, e)) {
            if (row == 0) EvAct(&p, LP_ACCEPT, tries, 0, n);
            for (int c = 0; c < wn; c++) EvAct(&p, LP_COIN, row, c, wantRow[c]);
            EvAct(&p, LP_ROWEND, row, 0, 0);
            row++;
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, LPF_LEFT, e, row, 0, 0, NULL); break; }
        if (!aInts) {
            if (ScanAfter(A, al, "Total:", &t))
                ParseFail(&p, FAIL_WRONG, LPF_LEFT, e, row, 0, t, "The table stopped too soon");
            else if (LineIs(A, al, "Please enter 1 to 12"))
                ParseFail(&p, FAIL_WRONG, LPF_GATE, e, tries, 0, 0, "Only numbers outside 1 to 12 get that message");
            else
                ParseFail(&p, FAIL_GARBLED, LPF_GARBLED, e, row, 0, 0, "The robot can't read this line");
            break;
        }
        // the row is wrong: play the part of it that is right, then stop at the first problem
        ParseShow(&p);
        if (row == 0) EvAct(&p, LP_ACCEPT, tries, 0, n);
        int k = 0;
        while (k < gn && k < wn && got[k] == wantRow[k]) { EvAct(&p, LP_COIN, row, k, got[k]); k++; }
        if (k < gn && k < wn) {
            snprintf(note, sizeof(note), "Row %d, column %d should be %d", row + 1, k + 1, wantRow[k]);
            GameEvent *f = ParseFail(&p, FAIL_WRONG, LPF_COIN, e, row, k, got[k], note);
            if (f) snprintf(f->text, EV_TEXT, "%d", wantRow[k]);
        } else if (gn < wn) {
            ParseFail(&p, FAIL_WRONG, LPF_LEFT, e, row, gn, 0, "This row stopped too soon");
        } else if (gn > wn) {
            ParseFail(&p, FAIL_WRONG, LPF_BUMP, e, row, wn, got[wn], "One number too many on this row");
        } else {
            ParseFail(&p, FAIL_WRONG, LPF_SPACING, e, row, 0, 0, "Right numbers, wrong spacing: print each one with %4d");
        }
        break;
    }
    ParseEnd(&p, LPF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { COIN_HIDDEN, COIN_GOOD, COIN_BAD, COIN_LEFT };
enum { BLOCK_WAIT, BLOCK_IN, BLOCK_OUT, BLOCK_TAKEN };
enum { DO_MOVE, DO_REJECT, DO_ACCEPT, DO_GATE_TEXT, DO_LET_IN, DO_DOOR_OPEN, DO_DOOR_WRONG, DO_BUMP, DO_CHEER };

#define LATER_MAX 16
#define FLY_MAX   32
#define POP_MAX   8

typedef struct { int value, state; float t; Vector3 home, pos; float spin; } Block;
typedef struct { float at; int what, a; Vector3 v; float f; bool live; } Later;
typedef struct { Vector3 from; float t; int value; bool live; } Fly;
typedef struct { char text[16]; Vector3 at; float t; Color color; } PopText;

static struct {
    const EventList *ev;
    int     n;
    Block   blocks[TRY_MAX];
    int     blockCount;
    float   gate, gateTo, lampGlow;
    Color   lamp;
    char    gateMsg[48];
    Color   gateMsgColor;
    int     state[GMAX][GMAX];
    int     value[GMAX][GMAX];
    float   flip[GMAX][GMAX];        // -1 lying down; 0..1 turning over; then counts on
    float   flipAt[GMAX][GMAX];      // when its coin turns over (-1 = not yet)
    bool    flown[GMAX][GMAX];
    char    want[GMAX][GMAX][8];
    float   pop0;
    int     curRow, curCol;
    int     total;                   // the printed numbers that have reached the counter
    float   shown;
    Fly     fly[FLY_MAX];
    float   door, doorTo, doorGlow;
    Color   doorLight;
    char    doorMsg[32];
    Color   doorMsgColor;
    Bot     bot;
    Later   later[LATER_MAX];
    PopText pops[POP_MAX];
    int     popNext;
    char    botMsg[48];
    char    note[64];
    Vector3 noteAt;
    int     spacingRow;
    bool    ended;
    float   time;
} L;

static Color Orange(void) { return (Color){ 255, 160, 60, 255 }; }

// Copies text, cutting it short to fit
static void Copy(char *dst, int cap, const char *src)
{
    int n = 0;
    while (src && src[n] && n < cap - 1) { dst[n] = src[n]; n++; }
    dst[n] = '\0';
}

// ---- where things are
static float   Half(void)        { return (L.n > 0 ? L.n : 2) * TILE * 0.5f; }
static Vector3 TileAt(int r, int c) { float o = (L.n - 1) * 0.5f; return (Vector3){ (c - o) * TILE, 0, (r - o) * TILE }; }
static float   Row0Z(void)       { return L.n > 0 ? TileAt(0, 0).z : 0; }
static float   LastZ(void)       { return L.n > 0 ? TileAt(L.n - 1, 0).z : 0; }
static Vector3 GatePos(void)     { return (Vector3){ -Half() - 1.25f, 0, Row0Z() }; }
static Vector3 StartPos(void)    { Vector3 g = GatePos(); return (Vector3){ g.x - 1.3f, 0, g.z - 0.2f }; }
static Vector3 DoorPos(void)     { return (Vector3){ Half() + 1.2f, 0, LastZ() - 0.35f }; }
static Vector3 DoorFront(void)   { Vector3 d = DoorPos(); return (Vector3){ d.x, 0, d.z + 0.78f }; }
static Vector3 CounterPos(void)  { Vector3 d = DoorPos(); return (Vector3){ d.x, 2.3f, d.z }; }
static Vector3 Scanner(void)     { Vector3 g = GatePos(); return (Vector3){ g.x, 0.45f, g.z + 1.08f }; }
static Vector3 BlockHome(int i)  { Vector3 s = StartPos(); return (Vector3){ s.x + 0.2f, 0.18f, s.z + 1.25f + i * 0.58f }; }
static Vector3 RobotSpot(int r, int c) { Vector3 t = TileAt(r, c); return (Vector3){ t.x, 0.12f, t.z - 0.2f }; }
static Vector3 CoinSpot(int r, int c)  { Vector3 t = TileAt(r, c); return (Vector3){ t.x, 0.12f, t.z + 0.24f }; }
static Vector3 V3(float x, float y, float z) { return (Vector3){ x, y, z }; }
static Vector3 Add(Vector3 a, Vector3 b) { return (Vector3){ a.x + b.x, a.y + b.y, a.z + b.z }; }

static void Overview(Vector3 *pos, Vector3 *look)
{
    // everything from the input blocks on the left to the counter over the door on the right
    float half = Half() + 2.9f;
    float d = fmaxf(9.2f, half * 2.05f);
    Vector3 c = { (StartPos().x + DoorPos().x) * 0.5f + 0.45f, 0, (Row0Z() + LastZ()) * 0.5f + 0.35f };
    *look = V3(c.x, 0.4f, c.z);
    *pos = V3(c.x, d * 0.78f, c.z + d * 0.64f);
}

static void ShotOverview(float seconds)
{
    Vector3 p, l;
    Overview(&p, &l);
    CamMove(p, l, 38, seconds);
}

static void ShotGate(float seconds)
{
    Vector3 l = Add(GatePos(), V3(0.5f, 0.7f, 0.6f));
    CamMove(Add(l, V3(1.0f, 3.1f, 5.0f)), l, 38, seconds);
}

static void ShotDoor(float seconds)
{
    Vector3 l = Add(DoorPos(), V3(-0.4f, 1.0f, 0.7f));
    CamMove(Add(l, V3(-1.2f, 3.0f, 5.2f)), l, 38, seconds);
}

// ---- things that happen a moment later (when the robot gets there, when a block lands)
static void Soon(float delay, int what, int a, Vector3 v, float f)
{
    for (int i = 0; i < LATER_MAX; i++)
        if (!L.later[i].live) { L.later[i] = (Later){ L.time + delay, what, a, v, f, true }; return; }
}

static void PopUp(const char *text, Vector3 at, Color color)
{
    PopText *p = &L.pops[L.popNext++ % POP_MAX];
    Copy(p->text, sizeof(p->text), text);
    p->at = at;
    p->t = 0;
    p->color = color;
}

static Block *BlockAt(int i) { return (i >= 0 && i < L.blockCount) ? &L.blocks[i] : NULL; }

static void SendBlock(int i, float delayToReact, int reaction)
{
    Block *b = BlockAt(i);
    if (b) { b->state = BLOCK_IN; b->t = 0; }
    Soon(delayToReact, reaction, i, V3(0, 0, 0), 0);
}

static void LeftBehind(int fromRow, int fromCol)
{
    bool first = true;
    for (int r = 0; r < L.n; r++)
        for (int c = 0; c < L.n; c++)
            if ((r > fromRow || (r == fromRow && c >= fromCol)) && L.state[r][c] == COIN_HIDDEN) {
                L.state[r][c] = COIN_LEFT;
                if (first) {
                    first = false;
                    snprintf(L.note, sizeof(L.note), "left behind");
                    L.noteAt = Add(CoinSpot(r, c), V3(0, 0.75f, 0));
                }
            }
}

static void LoopsReset(const EventList *ev)
{
    memset(&L, 0, sizeof(L));
    L.ev = ev;
    L.curRow = L.curCol = -1;
    L.spacingRow = -1;
    L.lamp = (Color){ 120, 130, 160, 255 };
    L.lampGlow = 0.3f;
    L.doorLight = (Color){ 86, 156, 255, 255 };
    L.doorGlow = 0.6f;
    for (int r = 0; r < GMAX; r++)
        for (int c = 0; c < GMAX; c++) { L.flip[r][c] = -1; L.flipAt[r][c] = -1; }
    BotInit(&L.bot, StartPos(), 90);
}

static void Place(void)
{
    for (int i = 0; i < L.blockCount; i++) L.blocks[i].home = L.blocks[i].pos = BlockHome(i);
    BotInit(&L.bot, StartPos(), 90);
    Vector3 p, l;
    Overview(&p, &l);
    if (L.ev && L.ev->preview) CamSet(p, l, 38);
    else ShotGate(0);
}

// ---- the events
static float Act(const GameEvent *e, float k)
{
    int r = e->a, c = e->b;
    switch (e->code) {
        case LP_REJECT:
            SendBlock(e->a, 0.45f, DO_REJECT);
            return 1.15f * k;
        case LP_ACCEPT:
            SendBlock(e->a, 0.45f, DO_ACCEPT);
            Soon(0.8f, DO_MOVE, 0, Add(GatePos(), V3(0.95f, 0.12f, -0.2f)), 0.5f);
            ShotOverview(1.4f);
            return 1.3f * k;
        case LP_COIN: {
            if (r < 0 || r >= L.n || c < 0 || c >= L.n) return 0.05f;
            L.state[r][c] = COIN_GOOD;
            L.value[r][c] = (int)e->value;
            Vector3 to = RobotSpot(r, c);
            float dur = (0.16f + Vector3Distance(L.bot.moveDur > 0 ? L.bot.to : L.bot.pos, to) * 0.15f) * k;
            if (dur > 0.75f) dur = 0.75f;
            for (int i = 0; i < LATER_MAX; i++) if (L.later[i].what == DO_MOVE) L.later[i].live = false;
            BotMoveTo(&L.bot, to, dur);
            L.flipAt[r][c] = L.time + dur;
            L.curRow = r;
            L.curCol = c;
            return dur + 0.1f * k;
        }
        case LP_ROWEND:
            if (r >= 0 && r < L.n) PopUp("\\n", Add(TileAt(r, L.n - 1), V3(0.78f, 0.45f, 0)), (Color){ 160, 200, 255, 255 });
            return 0.12f * k;
        case LP_TOTAL:
            BotMoveTo(&L.bot, DoorFront(), 0.7f);
            Soon(0.6f, DO_DOOR_OPEN, (int)e->value, V3(0, 0, 0), 0);
            ShotDoor(1.0f);
            return 1.6f;
    }
    return 0;
}

static float Fail(const GameEvent *e)
{
    L.ended = true;
    int r = e->a, c = e->b;
    switch (e->code) {
        case LPF_GATE:
            SendBlock(e->a, 0.45f, DO_GATE_TEXT);
            BotMood(&L.bot, MOOD_CONFUSED);
            break;
        case LPF_LET_IN:
            SendBlock(e->a, 0.45f, DO_LET_IN);
            break;
        case LPF_COIN:
            if (r < 0 || r >= L.n || c < 0 || c >= L.n) break;
            L.state[r][c] = COIN_BAD;
            L.value[r][c] = (int)e->value;
            Copy(L.want[r][c], sizeof(L.want[r][c]), e->text);
            BotMoveTo(&L.bot, RobotSpot(r, c), 0.35f);
            L.flipAt[r][c] = L.time + 0.35f;
            L.curRow = r;
            L.curCol = c;
            break;
        case LPF_LEFT:
            LeftBehind(r, c);
            BotLookAround(&L.bot);
            break;
        case LPF_BUMP: {
            Vector3 to = (r >= L.n && L.n > 0) ? Add(RobotSpot(L.n - 1, 0), V3(0, 0, TILE * 0.62f))
                                               : (L.n > 0 ? Add(RobotSpot(r < L.n ? r : L.n - 1, L.n - 1), V3(TILE * 0.6f, 0, 0)) : L.bot.pos);
            BotMoveTo(&L.bot, to, 0.4f);
            Soon(0.4f, DO_BUMP, (int)e->value, to, 0);
            break;
        }
        case LPF_SPACING:
            L.spacingRow = r;
            if (r >= 0 && r < L.n) {
                snprintf(L.note, sizeof(L.note), "spacing: use %%4d");
                L.noteAt = Add(TileAt(r, L.n - 1), V3(0.2f, 1.05f, 0.3f));
            }
            BotShakeHead(&L.bot);
            BotMood(&L.bot, MOOD_SAD);
            PlaybackShake(0.12f);
            break;
        case LPF_TOTAL:
            BotMoveTo(&L.bot, DoorFront(), 0.7f);
            Soon(0.6f, DO_DOOR_WRONG, (int)e->value, V3(0, 0, 0), 0);
            ShotDoor(1.0f);
            break;
        case LPF_EXTRA:
            L.doorTo = 0;
            L.doorLight = (Color){ 235, 90, 90, 255 };
            Copy(L.botMsg, sizeof(L.botMsg), L.ev->actual);
            BotMood(&L.bot, MOOD_SAD);
            PlaybackShake(0.15f);
            break;
        default:                        // a line the robot can't read
            Copy(L.botMsg, sizeof(L.botMsg), L.ev->actual[0] ? L.ev->actual : "???");
            BotMood(&L.bot, MOOD_CONFUSED);
            BotShakeHead(&L.bot);
            FxSparks(BotHead(&L.bot), 14, (Color){ 255, 200, 90, 255 });
            PlaybackShake(0.12f);
            break;
    }
    if (e->code != LPF_LEFT && e->code != LPF_GATE && e->code != LPF_SPACING) BotMood(&L.bot, MOOD_SAD);
    return 2.4f;
}

static float Crash(void)
{
    L.ended = true;
    LeftBehind(0, 0);
    L.note[0] = '\0';
    bool timeout = L.ev && L.ev->fail == FAIL_TIMEOUT;
    if (timeout) snprintf(L.botMsg, sizeof(L.botMsg), "stopped after %d s", RUN_TIME_LIMIT);
    else snprintf(L.botMsg, sizeof(L.botMsg), "crashed!");
    BotPowerDown(&L.bot);
    PlaybackShake(0.22f);
    return 2.4f;
}

static float Win(void)
{
    Soon(0.2f, DO_CHEER, 0, V3(0, 0, 0), 0);
    return 2.4f;
}

static float LoopsEvent(const GameEvent *e)
{
    float k = L.n > 6 ? 0.6f : 1.0f;      // big tables play quicker
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == LP_TRY && L.blockCount < TRY_MAX) {
                Block *b = &L.blocks[L.blockCount++];
                b->value = (int)e->value;
                b->state = BLOCK_WAIT;
                b->home = b->pos = BlockHome(L.blockCount - 1);
            } else if (e->code == LP_GRID) {
                L.n = e->a < 0 ? 0 : e->a > GMAX ? GMAX : e->a;
                L.pop0 = L.time;
                Place();
            }
            return 0;
        case EV_PRINT: return 0.04f;
        case EV_ACT:   return Act(e, k);
        case EV_FAIL:  return Fail(e);
        case EV_CRASH: return Crash();
        case EV_WIN:   return Win();
    }
    return 0;
}

// ---- animation
static void DoLater(const Later *w)
{
    Block *b = BlockAt(w->a);
    switch (w->what) {
        case DO_MOVE:
            BotMoveTo(&L.bot, w->v, w->f);
            break;
        case DO_REJECT:
            L.lamp = (Color){ 235, 90, 90, 255 };
            L.lampGlow = 1;
            snprintf(L.gateMsg, sizeof(L.gateMsg), "Please enter 1 to 12");
            L.gateMsgColor = (Color){ 255, 140, 140, 255 };
            if (b) { b->state = BLOCK_OUT; b->t = 0; }
            FxSparks(Scanner(), 10, (Color){ 255, 90, 90, 255 });
            BotShakeHead(&L.bot);
            break;
        case DO_ACCEPT:
            L.lamp = (Color){ 80, 220, 130, 255 };
            L.lampGlow = 1;
            L.gateTo = 1;
            snprintf(L.gateMsg, sizeof(L.gateMsg), "n = %d", L.n);
            L.gateMsgColor = (Color){ 140, 255, 180, 255 };
            if (b) { b->state = BLOCK_TAKEN; b->t = 0; }
            FxSparkle(Scanner(), 14, (Color){ 120, 255, 170, 255 });
            break;
        case DO_GATE_TEXT:
            L.lamp = (Color){ 255, 170, 60, 255 };
            L.lampGlow = 1;
            Copy(L.gateMsg, sizeof(L.gateMsg), L.ev->actual[0] ? L.ev->actual : "(nothing)");
            L.gateMsgColor = (Color){ 255, 140, 140, 255 };
            FxSparks(Scanner(), 16, (Color){ 255, 170, 60, 255 });
            PlaybackShake(0.14f);
            break;
        case DO_LET_IN:
            L.lamp = (Color){ 235, 90, 90, 255 };
            L.lampGlow = 1;
            L.gateTo = 0.55f;
            snprintf(L.gateMsg, sizeof(L.gateMsg), "%d got in!", b ? b->value : 0);
            L.gateMsgColor = (Color){ 255, 140, 140, 255 };
            if (b) { b->state = BLOCK_TAKEN; b->t = 0; }
            FxSparks(Add(GatePos(), V3(0, 1.0f, 0)), 30, (Color){ 255, 120, 80, 255 });
            BotShakeHead(&L.bot);
            PlaybackShake(0.2f);
            break;
        case DO_DOOR_OPEN:
            BotFace(&L.bot, 180);
            snprintf(L.doorMsg, sizeof(L.doorMsg), "Total: %d", w->a);
            L.doorMsgColor = (Color){ 140, 255, 180, 255 };
            L.doorLight = (Color){ 80, 220, 130, 255 };
            L.doorGlow = 1;
            L.doorTo = 1;
            FxSparkle(CounterPos(), 20, (Color){ 255, 220, 120, 255 });
            BotMood(&L.bot, MOOD_HAPPY);
            break;
        case DO_DOOR_WRONG:
            BotFace(&L.bot, 180);
            snprintf(L.doorMsg, sizeof(L.doorMsg), "Total: %d", w->a);
            L.doorMsgColor = (Color){ 255, 130, 130, 255 };
            L.doorLight = (Color){ 235, 90, 90, 255 };
            L.doorGlow = 1;
            {
                const char *want = L.ev ? L.ev->expected : "";
                if (strncmp(want, "Total: ", 7) == 0) want += 7;
                snprintf(L.note, sizeof(L.note), "should be %.40s", want);
            }
            L.noteAt = Add(CounterPos(), V3(0, -0.55f, 0));
            BotShakeHead(&L.bot);
            FxSparks(Add(DoorPos(), V3(0, 1.0f, 0)), 20, (Color){ 255, 90, 90, 255 });
            PlaybackShake(0.2f);
            break;
        case DO_BUMP: {
            BotBump(&L.bot);
            PlaybackShake(0.25f);
            char t[16];
            snprintf(t, sizeof(t), "%d ?", w->a);
            PopUp(t, Add(w->v, V3(0, 1.6f, 0)), (Color){ 255, 130, 130, 255 });
            break;
        }
        case DO_CHEER:
            BotFace(&L.bot, 0);
            BotCheer(&L.bot);
            FxConfetti(Add(DoorPos(), V3(0, 1.0f, 0.3f)), PlaybackFinale() ? 140 : 60, 0.5f);
            if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.4f);
            break;
    }
}

static void LoopsUpdate(float dt)
{
    L.time += dt;
    BotUpdate(&L.bot, dt);
    for (int i = 0; i < LATER_MAX; i++)
        if (L.later[i].live && L.time >= L.later[i].at) { L.later[i].live = false; DoLater(&L.later[i]); }

    L.gate = Smooth(L.gate, L.gateTo, 6, dt);
    L.door = Smooth(L.door, L.doorTo, 4, dt);
    L.lampGlow = Smooth(L.lampGlow, 0.45f, 1.5f, dt);
    L.shown = Smooth(L.shown, (float)L.total, 9, dt);

    for (int i = 0; i < L.blockCount; i++) {
        Block *b = &L.blocks[i];
        b->t += dt;
        b->spin += dt * 40;
        if (b->state == BLOCK_IN) {                  // hops into the scanner
            float k = EaseInOutQuad(b->t / 0.45f);
            b->pos = LerpV3(b->home, Scanner(), k);
            b->pos.y += sinf(k * PI) * 0.8f;
        } else if (b->state == BLOCK_OUT) {          // thrown off the stage
            float t = b->t;
            b->pos = Add(Scanner(), V3(-1.4f * t, 2.2f * t - 6.0f * t * t, 2.2f * t));
            b->spin += dt * 700;
        } else if (b->state == BLOCK_TAKEN) {
            b->pos = Scanner();
        }
    }

    for (int r = 0; r < L.n; r++)
        for (int c = 0; c < L.n; c++) {
            if (L.flipAt[r][c] >= 0 && L.time >= L.flipAt[r][c] && L.flip[r][c] < 0) {
                L.flip[r][c] = 0;
                Vector3 at = Add(CoinSpot(r, c), V3(0, 0.4f, 0));
                if (L.state[r][c] == COIN_BAD) {
                    FxSparks(at, 30, (Color){ 255, 90, 90, 255 });
                    BotShakeHead(&L.bot);
                    PlaybackShake(0.22f);
                } else {
                    FxSparkle(at, 10, (Color){ 255, 220, 120, 255 });
                }
            }
            if (L.flip[r][c] >= 0) L.flip[r][c] += dt / 0.2f;
            if (L.state[r][c] == COIN_GOOD && !L.flown[r][c] && L.flip[r][c] > 2.2f) {    // off to the counter
                L.flown[r][c] = true;
                for (int i = 0; i < FLY_MAX; i++)
                    if (!L.fly[i].live) {
                        L.fly[i] = (Fly){ Add(CoinSpot(r, c), V3(0, 0.32f, 0)), 0, L.value[r][c], true };
                        break;
                    }
            }
        }

    for (int i = 0; i < FLY_MAX; i++) {
        Fly *f = &L.fly[i];
        if (!f->live) continue;
        f->t += dt / 0.6f;
        if (f->t >= 1) {
            f->live = false;
            L.total += f->value;
            FxSparkle(CounterPos(), 6, (Color){ 255, 220, 120, 255 });
        }
    }
    for (int i = 0; i < POP_MAX; i++) L.pops[i].t += dt;
}

// ---- drawing
static void LoopsDraw(void)
{
    float half = Half();
    Vector3 start = StartPos(), door = DoorPos();
    float minZ = Row0Z() - TILE * 0.5f - 1.2f;
    float maxZ = fmaxf(LastZ() + TILE * 0.5f + 0.9f, BlockHome(L.blockCount > 0 ? L.blockCount - 1 : 0).z + 0.6f);
    Vector3 center = { (start.x - 0.8f + door.x + 2.2f) * 0.5f, 0, (minZ + maxZ) * 0.5f };
    KitStage(center, (door.x + 2.2f - (start.x - 0.8f)) * 0.5f, (maxZ - minZ) * 0.5f);

    if (L.n > 0) {
        // the walls round the grid, open at the gate (first row) and the door (last row)
        float top = Row0Z() - TILE * 0.5f - 0.1f, bottom = LastZ() + TILE * 0.5f + 0.1f;
        float x0 = -half - 0.1f, x1 = half + 0.1f;
        KitWall(V3(x0, 0, top), V3(x1, 0, top));
        KitWall(V3(x0, 0, bottom), V3(x1, 0, bottom));
        if (bottom - (Row0Z() + TILE * 0.5f) > 0.2f) KitWall(V3(x0, 0, Row0Z() + TILE * 0.5f), V3(x0, 0, bottom));
        if ((LastZ() - TILE * 0.5f) - top > 0.2f) KitWall(V3(x1, 0, top), V3(x1, 0, LastZ() - TILE * 0.5f));

        for (int r = 0; r < L.n; r++)
            for (int c = 0; c < L.n; c++) {
                float pop = (L.time - L.pop0) * 2.4f - (r + c) * 0.07f;
                Color top = { 66, 74, 100, 255 };
                float glow = 0;
                int s = L.state[r][c];
                bool turned = L.flip[r][c] >= 1;
                if (s == COIN_GOOD && turned) { top = (Color){ 60, 104, 176, 255 }; glow = 0.18f; }
                if (s == COIN_BAD && turned) { top = (Color){ 170, 60, 64, 255 }; glow = 0.5f; }
                if (s == COIN_LEFT) { top = Orange(); glow = 0.25f + 0.35f * Pulse(L.time, 6); }
                if (r == L.curRow && c == L.curCol && s == COIN_GOOD) glow += 0.2f;
                KitTile(TileAt(r, c), TILE * 0.94f, Clamp01(pop), top, glow);

                if (pop < 0.6f) continue;
                Vector3 cs = CoinSpot(r, c);
                if (s == COIN_HIDDEN || s == COIN_LEFT || L.flip[r][c] < 0)
                    KitCoin(cs, 0, 0, s == COIN_LEFT ? 0.3f * Pulse(L.time, 6) : 0, false);
                else if (!L.flown[r][c])
                    KitCoin(cs, Clamp01(L.flip[r][c]), 0, 0.45f, s == COIN_BAD);
            }
    }

    KitGate(GatePos(), 0, L.gate, L.lamp, L.lampGlow);
    if (L.n > 0) KitDoor(door, 0, L.door, L.doorLight, L.doorGlow);

    for (int i = 0; i < L.blockCount; i++) {
        Block *b = &L.blocks[i];
        if (b->state == BLOCK_OUT && b->pos.y < -3) continue;
        float size = 0.36f;
        if (b->state == BLOCK_TAKEN) size *= 1 - Clamp01((b->t - 0.1f) / 0.35f);
        if (size < 0.02f) continue;
        KitBlock(b->pos, size, (Color){ 70, 110, 200, 255 }, 0.35f, b->state == BLOCK_WAIT ? 0 : b->spin);
    }

    BotDraw(&L.bot);

    for (int i = 0; i < FLY_MAX; i++) {
        const Fly *f = &L.fly[i];
        if (!f->live) continue;
        float k = EaseInOutQuad(f->t);
        Vector3 p = LerpV3(f->from, CounterPos(), k);
        p.y += sinf(k * PI) * 1.4f;
        KitCoin(V3(p.x, p.y - 0.36f, p.z), 1, f->t * 720, 0.9f, false);
    }

    // ---- labels: the loop counters, the numbers on the coins, the gate, the counter
    Color dim = { 12, 14, 22, 190 }, hi = { 86, 156, 255, 235 };
    for (int r = 0; r < L.n; r++) {
        bool on = (r == L.curRow);
        Label3D(Add(TileAt(r, 0), V3(-1.0f, 0.2f, 0)), TextFormat("row=%d", r + 1), 14, on ? WHITE : (Color){ 170, 180, 205, 255 }, on ? hi : dim, LABEL_CODE);
    }
    for (int c = 0; c < L.n; c++) {
        bool on = (c == L.curCol);
        Label3D(Add(TileAt(0, c), V3(0, 0.2f, -0.82f)), TextFormat("col=%d", c + 1), 14, on ? WHITE : (Color){ 170, 180, 205, 255 }, on ? hi : dim, LABEL_CODE);
    }
    for (int r = 0; r < L.n; r++)
        for (int c = 0; c < L.n; c++) {
            int s = L.state[r][c];
            float f = L.flip[r][c];
            Vector3 at = Add(CoinSpot(r, c), V3(0, 0.36f, 0.05f));
            if (s == COIN_GOOD && f >= 0.6f) {
                Vector3 p = at;
                if (r == L.spacingRow) p.x += sinf(L.time * 40 + c) * 0.04f;
                Color fg = r == L.spacingRow ? Orange() : (Color){ 255, 222, 130, 255 };
                Label3D(p, TextFormat("%d", L.value[r][c]), 17, fg, (Color){ 20, 18, 10, 200 }, LABEL_CODE);
            } else if (s == COIN_BAD && f >= 0.6f) {
                Label3D(at, TextFormat("%d", L.value[r][c]), 19, WHITE, (Color){ 200, 50, 55, 240 }, LABEL_CODE);
                Label3D(Add(at, V3(0, -0.42f, 0.35f)), TextFormat("should be %s", L.want[r][c]), 13, (Color){ 255, 200, 200, 255 }, dim, LABEL_CODE);
            } else if (s == COIN_LEFT) {
                Label3D(at, "?", 17, Orange(), dim, LABEL_CODE);
            }
        }

    for (int i = 0; i < L.blockCount; i++) {
        const Block *b = &L.blocks[i];
        if ((b->state == BLOCK_OUT && b->pos.y < -1) || (b->state == BLOCK_TAKEN && b->t > 0.3f)) continue;
        Label3D(Add(b->pos, V3(0, 0.36f, 0)), TextFormat("%d", b->value), 16, WHITE, (Color){ 40, 70, 150, 230 }, LABEL_CODE);
    }
    if (L.gateMsg[0]) Label3D(Add(GatePos(), V3(0, 1.75f, 0.8f)), L.gateMsg, 15, L.gateMsgColor, dim, LABEL_CODE);
    if (L.n > 0) {
        if (L.doorMsg[0]) Label3D(CounterPos(), L.doorMsg, 17, L.doorMsgColor, dim, LABEL_CODE);
        else Label3D(CounterPos(), TextFormat("total = %d", (int)lroundf(L.shown)), 16, (Color){ 255, 222, 130, 255 }, dim, LABEL_CODE);
    }
    if (L.note[0]) Label3D(L.noteAt, L.note, 14, Orange(), dim, LABEL_CODE);
    if (L.botMsg[0]) Label3D(Add(BotHead(&L.bot), V3(0, 0.25f, 0)), L.botMsg, 14, (Color){ 255, 150, 150, 255 }, dim, LABEL_CODE);
    for (int i = 0; i < POP_MAX; i++) {
        const PopText *p = &L.pops[i];
        if (!p->text[0] || p->t > 1.2f) continue;
        Color c = p->color;
        c.a = (unsigned char)(255 * Clamp01(1.2f - p->t));
        Label3D(Add(p->at, V3(0, p->t * 0.6f, 0)), p->text, 16, c, (Color){ 12, 14, 22, (unsigned char)(c.a * 0.7f) }, LABEL_CODE);
    }
}

const LevelDef LEVEL_LOOPS = {
    "c-loops-2", "Coin Grid", "Every number your loops print turns a coin over on the grid.",
    { 255, 205, 70, 255 },
    ParseLoops, LoopsReset, LoopsEvent, LoopsUpdate, LoopsDraw, NULL,
};
