// Memory Heist: the 3D scene for the heist side quests and the heist boss.
//
// Memory is a street of houses - street[0] at address 1000, street[1] at 1004 - and the
// thief can only get around by following pointers. The game library (game/cfd_heist.h)
// writes everything the program does to a trace: "G 4" go to house 4 ("G -1" the hideout,
// "G NULL" a NULL pointer), "T 4 120" grab its loot, "A 0 20 12" / "F 0 30" malloc and
// free, "N" / "K" new and delete, "U 0 pick" / "O 0" a gadget on a vault door, "R 2 150"
// rob an estate house, "V" / "C" / "P" the van, the crates and the report, "L 0" a leak -
// and ends it with END WIN or END FAIL and the reason (the alarm).
//
// The world comes from the test input (STREET, TIP, BLOCK, DOORS, ESTATE, TRAIL, CHAIN);
// the trace is replayed by the thief; then the program's output is checked like the
// robot's radio messages. The verdict is read exactly the way the runner reads it.
#include "levels.h"
#include "stage.h"
#include "../ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum {
    HS_STREET = 1, HS_HOUSE, HS_TIP, HS_BLOCK, HS_DOORS, HS_ESTATE, HS_EHOUSE, HS_TRAIL, HS_CHAIN,
    HA_GO, HA_GRAB, HA_MALLOC, HA_FREE, HA_NEW, HA_DELETE, HA_VAN, HA_CRATES, HA_USE, HA_OPEN, HA_ROB,
    HA_REPORT, HA_LEAK, HA_SKIP, HA_HOME, HA_SAY,
};
enum { HSF_GAME = 1, HSF_LINE, HSF_EXTRA };

#define HN      24          // the library's limit on houses
#define SPOT_HIDEOUT  -1
#define SPOT_NULL     -2
#define SPOT_LOST     -3
#define ESTATE_SPOT  100    // + estate house
#define DOOR_SPOT    200    // + vault door

// ---------------------------------------------------------------- the parser

static bool Space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }

// One word, like scanf("%31s")
static bool Word(const char **p, char *out, int cap)
{
    while (Space(**p)) (*p)++;
    if (!**p) return false;
    int n = 0;
    while (**p && !Space(**p) && n < cap - 1) out[n++] = *(*p)++;
    out[n] = '\0';
    return true;
}

// One whole number, like scanf("%d"): when there isn't one, nothing is read and it's 0
static int Int(const char **p)
{
    const char *q = *p;
    while (Space(*q)) q++;
    bool neg = (*q == '-');
    if (*q == '-' || *q == '+') q++;
    if (*q < '0' || *q > '9') return 0;
    long v = 0;
    for (int d = 0; *q >= '0' && *q <= '9'; q++, d++) if (d < 9) v = v * 10 + (*q - '0');
    *p = q;
    return (int)(neg ? -v : v);
}

// The whole numbers after a trace line's letter: "T 4 120" -> 4, 120 (stops at anything else)
static int Ints(const char *s, int len, int *out, int max)
{
    int n = 0, i = 1;
    while (n < max) {
        while (i < len && s[i] == ' ') i++;
        if (i >= len) break;
        bool neg = (s[i] == '-');
        if (neg) i++;
        if (i >= len || s[i] < '0' || s[i] > '9') break;
        long v = 0;
        for (int d = 0; i < len && s[i] >= '0' && s[i] <= '9'; i++, d++) if (d < 9) v = v * 10 + (s[i] - '0');
        out[n++] = (int)(neg ? -v : v);
    }
    return n;
}

// The text after a trace line's letter and first number: "V 0 40 10 25" -> "40 10 25"
static void Rest(const char *s, int len, char *out, int cap)
{
    int i = 1, n = 0;
    while (i < len && s[i] == ' ') i++;
    while (i < len && s[i] != ' ') i++;
    while (i < len && s[i] == ' ') i++;
    while (i < len && n < cap - 1) { char c = s[i++]; out[n++] = (c < 32 || c > 126) ? '?' : c; }
    out[n] = '\0';
}

static void ParseHeist(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);

    // the world, read the way the library reads it
    const char *in = run->input ? run->input : "";
    char key[32];
    int n = 0;
    for (int guard = 0; Word(&in, key, sizeof(key)) && guard < 400; guard++) {
        if (strcmp(key, "END") == 0) break;
        if (strcmp(key, "STREET") == 0) {
            int count = Int(&in);
            n = count < 0 ? 0 : count > HN ? HN : count;
            EvSpawn(&p, HS_STREET, n, 0, 0);
            for (int i = 0; i < n; i++) EvSpawn(&p, HS_HOUSE, i, 0, Int(&in));
        } else if (strcmp(key, "TIP") == 0) {
            int k = Int(&in);
            EvSpawn(&p, HS_TIP, k >= 0 && k < n ? k : -1, 0, 0);
        } else if (strcmp(key, "BLOCK") == 0) {
            int f = Int(&in), c = Int(&in);
            EvSpawn(&p, HS_BLOCK, f, c, 0);
        } else if (strcmp(key, "DOORS") == 0) {
            char doors[HN + 1];
            if (!Word(&in, doors, sizeof(doors))) break;
            EvText(EvSpawn(&p, HS_DOORS, (int)strlen(doors), 0, 0), doors);
        } else if (strcmp(key, "ESTATE") == 0) {
            int count = Int(&in);
            EvSpawn(&p, HS_ESTATE, count < 0 ? 0 : count > HN ? HN : count, 0, 0);
            for (int i = 0; i < count && i < 4000; i++) {
                int number = Int(&in), loot = Int(&in), alarm = Int(&in);
                if (i < HN) EvSpawn(&p, HS_EHOUSE, i + (alarm ? 1000 : 0), number, loot);
            }
        } else if (strcmp(key, "TRAIL") == 0 || strcmp(key, "CHAIN") == 0) {
            bool trail = key[0] == 'T';
            int count = Int(&in);
            for (int i = 0; i < count && i < 4000; i++) {
                int k = Int(&in);
                if (i < HN) EvSpawn(&p, trail ? HS_TRAIL : HS_CHAIN, i, k, 0);
            }
        }
    }
    if (ParsePreview(&p)) return;

    // the actions, up to the first END line (a long run skips ahead to where the thief ended up)
    const char *t = run->trace ? run->trace : "";
    int limit = EV_MAX - 2 * PARSE_LINES - 40, skipped = 0, pos = SPOT_HIDEOUT;
    bool skipping = false;
    const char *q = t;
    for (int lines = 0; *q && lines < 40000; lines++) {
        const char *e = strchr(q, '\n');
        int len = e ? (int)(e - q) : (int)strlen(q);
        while (len > 0 && (q[len - 1] == '\r' || q[len - 1] == ' ')) len--;
        if (len >= 4 && memcmp(q, "END ", 4) == 0) break;
        int v[3], k = len >= 2 && q[1] == ' ' ? Ints(q, len, v, 3) : -1;
        char text[EV_TEXT];
        GameEvent *g = NULL;
        bool act = true;
        if (skipping) act = false;
        if (len >= 2 && q[1] == ' ') {
            switch (q[0]) {
                case 'G': {
                    int where = len == 6 && memcmp(q, "G NULL", 6) == 0 ? SPOT_NULL : len == 6 && memcmp(q, "G LOST", 6) == 0 ? SPOT_LOST
                              : k == 1 ? v[0] : -99;
                    if (where == -99) break;
                    pos = where;
                    if (act) g = EvAct(&p, HA_GO, where, 0, 0);
                    else skipped++;
                    break;
                }
                case 'T': if (k == 2) { if (act) EvAct(&p, HA_GRAB, v[0], 0, v[1]); else skipped++; } break;
                case 'A': if (k == 3) { if (act) EvAct(&p, HA_MALLOC, v[0], v[1], v[2]); else skipped++; } break;
                case 'F': if (k == 2) { if (act) EvAct(&p, HA_FREE, v[0], v[1], 0); else skipped++; } break;
                case 'N': if (k == 3) { if (act) EvAct(&p, HA_NEW, v[0], v[1], v[2]); else skipped++; } break;
                case 'K': if (k == 1) { if (act) EvAct(&p, HA_DELETE, v[0], 0, 0); else skipped++; } break;
                case 'L': if (k == 1) { if (act) EvAct(&p, HA_LEAK, v[0], 0, 0); else skipped++; } break;
                case 'O': if (k == 1) { if (act) EvAct(&p, HA_OPEN, v[0], 0, 0); else skipped++; } break;
                case 'V': case 'C': case 'P':
                    if (k >= 1) {
                        Rest(q, len, text, sizeof(text));
                        if (act) g = EvAct(&p, q[0] == 'V' ? HA_VAN : q[0] == 'C' ? HA_CRATES : HA_REPORT, v[0], 0, 0);
                        else skipped++;
                        if (g) EvText(g, text);
                    }
                    break;
                case 'U':
                    if (k == 1) {
                        Rest(q, len, text, sizeof(text));
                        if (act) EvText(EvAct(&p, HA_USE, v[0], 0, 0), text);
                        else skipped++;
                    }
                    break;
                case 'R':
                    if (len == 6 && memcmp(q, "R NULL", 6) == 0) { if (act) EvAct(&p, HA_ROB, SPOT_NULL, 0, 0); else skipped++; }
                    else if (k == 2) { if (act) EvAct(&p, HA_ROB, v[0], 0, v[1]); else skipped++; }
                    break;
            }
        }
        if (!skipping && ev->count >= limit) skipping = true;
        if (!e) break;
        q = e + 1;
    }
    if (skipping) {
        EvAct(&p, HA_SKIP, skipped, 0, 0);
        EvAct(&p, HA_GO, pos, 0, 0);
    }
    // the game's verdict; after a clean getaway, what the program printed
    ParseGameEnd(&p, HA_HOME, HA_SAY, HSF_GAME, HSF_LINE, HSF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_CHEER, DO_LAND };
enum { SAY_GOOD = 1, SAY_BAD, SAY_FREE };

#define ROAD_Z   1.35f
#define LOT_Z    3.1f
#define HEAP_MAX 8
#define THIEF_BODY   (Color){ 112, 102, 146, 255 }
#define THIEF_ACCENT (Color){ 215, 150, 255, 255 }

typedef struct { char text[EV_TEXT]; int state; } Bubble;
typedef struct { int id, size; bool isNew, arr, live, leak; float t; char vals[EV_TEXT]; } Block;

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time, step;
    // the street
    int   n, loot[HN];
    float grabbed[HN];            // when its loot was taken (0: still there)
    int   tip;                    // -2 none, -1 NULL, else a house
    bool  block;
    int   bfirst, bcount;
    int   chain[HN], nchain;
    // the estate
    int   m, number[HN], eloot[HN];
    bool  alarmOn[HN], robbed[HN];
    int   trail[HN], ntrail;
    // the vault doors
    char  doors[HN + 1];
    int   nd;
    float open[HN];
    // the heap
    Block heap[HEAP_MAX];
    int   nheap, heapTotal;
    // the thief
    Bot   thief;
    int   at, bag, noise;
    bool  home, failed, won, alarm;
    Bubble say[3];
    int   says;
    char  want[FAIL_TEXT];
} H;

static float RowX(int i, int count) { return (i - (count - 1) * 0.5f) * 1.38f + 0.9f; }
static float HouseX(int i) { return RowX(i, H.n); }
static float EstateX(int i) { return RowX(i, H.m); }
static float EstateZ(void) { return H.n > 0 ? -2.8f : 0; }
static float DoorX(int k) { return (k - (H.nd - 1) * 0.5f) * 1.4f - 0.5f; }
static bool  HasHeap(void) { return H.heapTotal > 0; }

static float LeftX(void)
{
    float x = 0;
    if (H.n > 0) x = fminf(x, HouseX(0));
    if (H.m > 0) x = fminf(x, EstateX(0));
    if (H.nd > 0) x = fminf(x, DoorX(0));
    return x;
}
static float RightX(void)
{
    float x = 0;
    if (H.n > 0) x = fmaxf(x, HouseX(H.n - 1));
    if (H.m > 0) x = fmaxf(x, EstateX(H.m - 1));
    if (H.nd > 0) x = fmaxf(x, DoorX(H.nd - 1) + 1.7f);    // the vault
    return x;
}
static float HideoutX(void) { return LeftX() - 2.3f; }
static float PitX(void) { return RightX() + 2.0f; }
static bool  HasPit(void) { return H.nd == 0; }

static Vector3 Spot(int where)        // where the thief stands for each place
{
    if (where >= DOOR_SPOT && where - DOOR_SPOT < H.nd) return V3(DoorX(where - DOOR_SPOT), 0, ROAD_Z - 0.35f);
    if (where >= ESTATE_SPOT && where - ESTATE_SPOT < H.m) return V3(EstateX(where - ESTATE_SPOT), 0, EstateZ() + ROAD_Z);
    if (where >= 0 && where < H.n) return V3(HouseX(where), 0, ROAD_Z);
    if (where == SPOT_NULL) return V3(PitX(), 0, ROAD_Z);
    if (where == SPOT_LOST) return V3(PitX() + 0.6f, 0, -1.2f);
    return V3(HideoutX(), 0, ROAD_Z);
}

static void Settle(void);

static void HeistReset(const EventList *ev)
{
    memset(&H, 0, sizeof(H));
    H.ev = ev;
    H.tip = -2;
    H.at = SPOT_HIDEOUT;
    int actions = 0;
    for (int i = 0; ev && i < ev->count; i++) {
        if (ev->ev[i].type == EV_ACT) actions++;
        if (ev->ev[i].type == EV_ACT && (ev->ev[i].code == HA_MALLOC || ev->ev[i].code == HA_NEW)) H.heapTotal++;
    }
    H.step = actions <= 30 ? 1.0f : fmaxf(0.08f, 30.0f / actions);
    Settle();
}

// Once the world is known: the thief at the hideout, the camera taking it all in
static void Settle(void)
{
    BotInit(&H.thief, Spot(SPOT_HIDEOUT), 90);
    H.thief.body = THIEF_BODY;
    H.thief.accent = THIEF_ACCENT;
    H.thief.scale = 0.95f;
    float left = HideoutX() - 0.9f, right = (HasPit() ? PitX() : RightX()) + 0.9f;
    float back = (H.m > 0 && H.n > 0) ? -3.6f : -0.8f, front = HasHeap() ? LOT_Z + 0.9f : ROAD_Z + 0.7f;
    float w = right - left, d = front - back;
    float dist = fmaxf((w * 0.5f + 0.2f) / 0.6f, (d * 0.7f + 1.8f) / 0.34f);
    ShotOrbit(V3((left + right) / 2, 0.7f, (back + front) / 2 + 0.1f), dist < 8 ? 8 : dist, 40, 0, 38, 0);
}

static float Walk(int where)
{
    Vector3 to = Spot(where);
    float dist = Vector3Distance(H.thief.pos, to);
    float dur = Clamp01(dist * 0.16f + 0.25f) * H.step;
    BotMoveTo(&H.thief, to, dur);
    H.at = where;
    return dur;
}

static Block *FindBlock(int id, bool isNew)
{
    for (int i = H.nheap - 1; i >= 0; i--) if (H.heap[i].id == id && H.heap[i].isNew == isNew) return &H.heap[i];
    return NULL;
}

static void AddBlock(int id, int size, bool isNew, bool arr)
{
    if (H.nheap == HEAP_MAX) { memmove(&H.heap[0], &H.heap[1], sizeof(Block) * (HEAP_MAX - 1)); H.nheap--; }
    H.heap[H.nheap++] = (Block){ id, size, isNew, arr, true, false, 0, "" };
}

static Vector3 BlockPos(int j)
{
    float cx = (HideoutX() + (HasPit() ? PitX() : RightX())) / 2;
    return V3(cx + (j - (HEAP_MAX - 1) * 0.5f) * 1.25f, 0, LOT_Z);
}

static void AddBubble(const char *text, int state)
{
    if (H.says == 3) { H.say[0] = H.say[1]; H.say[1] = H.say[2]; H.says = 2; }
    StrCopy(H.say[H.says].text, EV_TEXT, text);
    H.say[H.says].state = state;
    H.says++;
}

static Vector3 Above(Vector3 p, float y) { return V3(p.x, p.y + y, p.z); }

static float Act(const GameEvent *e)
{
    int i = e->a;
    switch (e->code) {
        case HA_GO: {
            float dur = Walk(i);
            if (i == SPOT_NULL) PopAdd(&H.pops, "NULL!", Above(Spot(SPOT_NULL), 1.4f), LBL_BAD);
            if (i == SPOT_LOST) PopAdd(&H.pops, "no such address!", Above(Spot(SPOT_LOST), 1.4f), LBL_BAD);
            return dur + 0.15f * H.step;
        }
        case HA_GRAB:
            if (i >= 0 && i < H.n) {
                H.grabbed[i] = H.time + 0.01f;
                H.bag += (int)e->value;
                if (e->value != 0) FxSparkle(V3(HouseX(i), 1.1f, 0.6f), 16, (Color){ 255, 214, 90, 255 });
                PopAdd(&H.pops, TextFormat("+%d", (int)e->value), V3(HouseX(i), 2.0f, 0.4f), LBL_GOLD);
            }
            return 0.55f * H.step;
        case HA_ROB: {
            if (i == SPOT_NULL) return Walk(SPOT_NULL);
            if (i < 0 || i >= H.m) return 0;
            float dur = Walk(ESTATE_SPOT + i);
            H.robbed[i] = true;
            H.bag += (int)e->value;
            PopAdd(&H.pops, TextFormat("+%d", (int)e->value), V3(EstateX(i), 2.0f, EstateZ() + 0.4f), LBL_GOLD);
            return dur + 0.45f * H.step;
        }
        case HA_USE: {
            if (i < 0 || i >= H.nd) return 0;
            float dur = Walk(DOOR_SPOT + i);
            int cost = strcmp(e->text, "pick") == 0 ? 1 : strcmp(e->text, "drill") == 0 ? 3 : strcmp(e->text, "blast") == 0 ? 10 : 0;
            H.noise += cost;
            Vector3 d = V3(DoorX(i), 0.9f, 0.4f);
            PopAdd(&H.pops, TextFormat("%s  +%d noise", e->text, cost), Above(d, 1.1f), cost >= 10 ? LBL_BAD : LBL_GOLD);
            if (cost >= 10) { FxSparks(d, 50, (Color){ 255, 170, 70, 255 }); PlaybackShake(0.3f); }
            else FxSparks(d, 12, (Color){ 200, 220, 255, 255 });
            return dur + 0.6f * H.step;
        }
        case HA_OPEN:
            if (i >= 0 && i < H.nd) H.open[i] = H.time;
            return 0.5f * H.step;
        case HA_MALLOC: case HA_NEW: {
            bool isNew = e->code == HA_NEW;
            AddBlock(i, e->b, isNew, isNew && e->value != 0);
            const Block *b = &H.heap[H.nheap - 1];
            PopAdd(&H.pops, TextFormat("%s(%d bytes)", isNew ? (b->arr ? "new[]" : "new") : "malloc", e->b), Above(BlockPos(H.nheap - 1), 1.3f), LBL_HI);
            return 0.6f * H.step;
        }
        case HA_FREE: case HA_DELETE: {
            Block *b = FindBlock(i, e->code == HA_DELETE);
            if (b) { b->live = false; b->t = 0; }
            if (e->code == HA_DELETE && i >= 0 && i < H.nchain) H.chain[i] = -1;   // a safehouse node is gone (they were new'd first)
            return 0.5f * H.step;
        }
        case HA_LEAK: {
            Block *b = FindBlock(i, false);
            if (!b) b = FindBlock(i, true);
            if (b) b->leak = true;
            return 0.4f;
        }
        case HA_VAN: case HA_CRATES: {
            Block *b = FindBlock(i, e->code == HA_CRATES);
            if (b) StrCopy(b->vals, EV_TEXT, e->text);
            PopAdd(&H.pops, TextFormat("%s: %s", e->code == HA_VAN ? "van" : "crates", e->text), Above(H.thief.pos, 1.6f), LBL_GOLD);
            return 0.9f * H.step;
        }
        case HA_REPORT:
            PopAdd(&H.pops, TextFormat("report: %s", e->text), Above(H.thief.pos, 1.6f), LBL_GOLD);
            return 0.9f * H.step;
        case HA_SKIP:
            PopAdd(&H.pops, TextFormat("fast-forward: %d more", e->a), Above(H.thief.pos, 1.8f), LBL_GOLD);
            return 0.6f;
        case HA_HOME:
            H.home = true;
            BotFace(&H.thief, 0);
            return 0.7f;
        case HA_SAY:
            AddBubble(e->text, e->b ? SAY_FREE : SAY_GOOD);
            return 0.9f;
    }
    return 0;
}

static float HeistEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN: {
            int i = e->a;
            switch (e->code) {
                case HS_STREET: H.n = i >= 0 && i <= HN ? i : 0; break;
                case HS_HOUSE: if (i >= 0 && i < H.n) H.loot[i] = (int)e->value; break;
                case HS_TIP: H.tip = i; break;
                case HS_BLOCK: H.block = true; H.bfirst = i; H.bcount = e->b; break;
                case HS_DOORS: StrCopy(H.doors, sizeof(H.doors), e->text); H.nd = (int)strlen(H.doors); break;
                case HS_ESTATE: H.m = i >= 0 && i <= HN ? i : 0; break;
                case HS_EHOUSE: {
                    int k = i % 1000;
                    if (k >= 0 && k < H.m) { H.number[k] = e->b; H.eloot[k] = (int)e->value; H.alarmOn[k] = i >= 1000; }
                    break;
                }
                case HS_TRAIL: if (i >= 0 && i < HN && i == H.ntrail) H.trail[H.ntrail++] = e->b; break;
                case HS_CHAIN: if (i >= 0 && i < HN && i == H.nchain) H.chain[H.nchain++] = e->b; break;
            }
            Settle();
            return 0;
        }
        case EV_PRINT:
            return 0;
        case EV_ACT:
            return Act(e);
        case EV_FAIL:
            H.failed = true;
            BotMood(&H.thief, MOOD_SAD);
            BotShakeHead(&H.thief);
            if (e->code == HSF_GAME) {
                H.alarm = true;
                PopAdd(&H.pops, "ALARM!", Above(H.thief.pos, 1.7f), LBL_BAD);
                PlaybackShake(0.3f);
            } else if (H.ev) {
                AddBubble(H.ev->actual[0] ? H.ev->actual : "(silence)", SAY_BAD);
                StrCopy(H.want, sizeof(H.want), H.ev->fail == FAIL_EXTRA ? "(nothing more)" : H.ev->expected);
                PlaybackShake(0.2f);
            }
            return 2.8f;
        case EV_CRASH: {
            H.failed = true;
            bool timeout = H.ev && H.ev->fail == FAIL_TIMEOUT;
            if (timeout) { BotLookAround(&H.thief); PopAdd(&H.pops, "stuck in a loop?", Above(H.thief.pos, 1.6f), LBL_GOLD); }
            else { BotPowerDown(&H.thief); PopAdd(&H.pops, "program crashed!", Above(H.thief.pos, 1.6f), LBL_BAD); PlaybackShake(0.3f); }
            return 2.6f;
        }
        case EV_WIN:
            H.won = true;
            BotCheer(&H.thief);
            CueAt(&H.cues, H.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void HeistUpdate(float dt)
{
    H.time += dt;
    BotUpdate(&H.thief, dt);
    for (int i = 0; i < H.nheap; i++) H.heap[i].t += dt;
    PopsUpdate(&H.pops, dt);
    Cue c;
    while (CueDue(&H.cues, H.time, &c)) {
        if (c.what == DO_CHEER) {
            bool finale = PlaybackFinale();
            FxConfetti(Above(H.thief.pos, 1.2f), finale ? 160 : 70, 2.6f);
            if (finale) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static Vector3 Roof(float x, float z) { return V3(x, 1.75f, z); }

static void DrawPost(Vector3 at, const char *name, Color col)
{
    Part(MESH_CYLINDER, V3(at.x, 0.45f, at.z), V3(0.06f, 0.9f, 0.06f), (Color){ 78, 86, 112, 255 }, 0);
    Part(MESH_SPHERE, V3(at.x, 0.95f, at.z), V3(0.16f, 0.16f, 0.16f), col, 0.8f);
    Label3D(V3(at.x, 1.2f, at.z), name, 12, col, LBL_DIM, LABEL_CODE);
}

static void HeistDraw(void)
{
    float alarm = H.alarm ? Pulse(H.time, 6) : 0;
    float left = HideoutX() - 0.9f, right = (HasPit() ? PitX() : RightX()) + 0.9f;
    float back = (H.m > 0 && H.n > 0) ? -3.8f : -1.0f, front = HasHeap() ? LOT_Z + 0.9f : ROAD_Z + 0.75f;
    KitStage(V3((left + right) / 2, 0, (back + front) / 2), (right - left) / 2, (front - back) / 2);
    KitRoad(V3(left + 0.2f, 0, ROAD_Z), right - left - 0.4f);
    if (H.m > 0 && H.n > 0) KitRoad(V3(left + 0.2f, 0, EstateZ() + ROAD_Z), right - left - 0.4f);

    // the hideout
    float hx = HideoutX();
    KitHouse(V3(hx, 0, 0), (Color){ 52, 50, 70, 255 }, (Color){ 120, 80, 180, 255 }, H.home ? 0.8f : 0.2f);
    Label3D(V3(hx, 0.08f, 0.85f), "&hideout", 12, (Color){ 210, 180, 255, 255 }, LBL_DIM, LABEL_CODE);

    // the street: an address under each house, its loot over the roof
    for (int i = 0; i < H.n; i++) {
        float x = HouseX(i);
        bool guarded = H.block && (i < H.bfirst || i >= H.bfirst + H.bcount);
        Color roof = guarded ? (Color){ 150, 60, 66, 255 } : (Color){ 70, 96, 150, 255 };
        KitHouse(V3(x, 0, 0), (Color){ 150, 140, 120, 255 }, roof, H.alarm ? alarm * 0.8f : 0);
        if (guarded) Part(MESH_SPHERE, V3(x - 0.3f, 1.55f, 0.1f), V3(0.16f, 0.16f, 0.16f), COL3_BAD, 0.5f + 0.4f * Pulse(H.time + i, 3));
        bool full = H.loot[i] != 0 && (H.grabbed[i] == 0 || H.time < H.grabbed[i]);
        if (full) {
            KitGold(V3(x + 0.62f, 0, 0.35f), H.loot[i] >= 200 ? 6 : H.loot[i] >= 100 ? 4 : H.loot[i] >= 30 ? 3 : 1, 0.35f);
            Label3D(V3(x + 0.62f, 0.95f, 0.4f), TextFormat("%d", H.loot[i]), 12, LBL_GOLD, LBL_DIM, LABEL_CODE);
        }
        Label3D(V3(x, 0.06f, 0.8f), TextFormat("%d", 1000 + 4 * i), 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    }
    if (H.block && H.bcount > 0 && H.bfirst >= 0 && H.bfirst < H.n) {
        int last = H.bfirst + H.bcount - 1 < H.n ? H.bfirst + H.bcount - 1 : H.n - 1;
        float x0 = HouseX(H.bfirst) - 0.75f, x1 = HouseX(last) + 0.75f;
        KitLot(V3((x0 + x1) / 2, 0, 0.1f), x1 - x0, 1.6f, (Color){ 120, 220, 150, 255 }, 0.6f);
        Vector3 post = V3(HideoutX() + 1.0f, 0, 0.6f);
        DrawPost(post, "first", (Color){ 140, 230, 170, 255 });
        Label3D(V3((x0 + x1) / 2, 0.06f, 1.02f), TextFormat("count = %d", H.bcount), 11, (Color){ 140, 230, 170, 255 }, LBL_DIM, LABEL_CODE);
        KitArc(V3(post.x, 1.0f, post.z), Roof(HouseX(H.bfirst), 0.1f), 0.7f, 1, (Color){ 120, 220, 150, 255 }, 0.7f);
    }
    if (H.tip != -2) {                                       // the fence's tip: a pointer to a house, or NULL
        Vector3 post = V3(hx + 1.0f, 0, 0.6f);
        DrawPost(post, "tip", LBL_HI);
        Vector3 to = H.tip >= 0 ? Roof(HouseX(H.tip), 0.1f) : V3(PitX(), 0.4f, ROAD_Z);
        KitArc(V3(post.x, 1.0f, post.z), to, 1.2f, 1, (Color){ 86, 156, 255, 255 }, 0.7f);
    }
    if (H.nchain > 0) {                                      // the safehouse list: head -> node -> node
        Vector3 post = V3(hx + 1.0f, 0, 0.6f);
        DrawPost(post, "head", LBL_HI);
        Vector3 from = V3(post.x, 1.0f, post.z);
        for (int k = 0; k < H.nchain; k++) {
            int house = H.chain[k];
            if (house < 0 || house >= H.n) continue;
            Vector3 to = Roof(HouseX(house), 0.1f);
            KitArc(from, to, 0.8f, 1, (Color){ 86, 156, 255, 255 }, 0.6f);
            from = to;
        }
    }

    // the estate: house numbers, loot and alarms; the trail of next pointers
    float ez = EstateZ();
    for (int i = 0; i < H.m; i++) {
        float x = EstateX(i);
        KitHouse(V3(x, 0, ez), H.robbed[i] ? (Color){ 110, 104, 92, 255 } : (Color){ 160, 150, 130, 255 }, (Color){ 96, 80, 140, 255 },
                 H.alarm ? alarm * 0.8f : 0);
        if (H.alarmOn[i]) Part(MESH_SPHERE, V3(x - 0.3f, 1.55f, ez + 0.1f), V3(0.18f, 0.18f, 0.18f), COL3_BAD, 0.5f + 0.4f * Pulse(H.time + i, 3));
        if (!H.robbed[i] && H.eloot[i] > 0) {
            KitGold(V3(x + 0.62f, 0, ez + 0.35f), H.eloot[i] >= 200 ? 6 : H.eloot[i] >= 100 ? 4 : 2, 0.35f);
            Label3D(V3(x + 0.62f, 0.95f, ez + 0.4f), TextFormat("%d", H.eloot[i]), 11, LBL_GOLD, LBL_DIM, LABEL_CODE);
        }
        Label3D(V3(x, 0.06f, ez + 0.8f), TextFormat("no.%d", H.number[i]), 12, H.alarmOn[i] ? LBL_BAD : LBL_SOFT, LBL_DIM, LABEL_CODE);
    }
    if (H.ntrail > 0) {
        Vector3 post = V3(hx + 1.0f, 0, ez + 0.6f);
        DrawPost(post, "first_clue", LBL_HI);
        Vector3 from = V3(post.x, 1.0f, post.z);
        for (int k = 0; k < H.ntrail; k++) {
            int e = H.trail[k];
            if (e < 0 || e >= H.m) continue;
            Vector3 to = Roof(EstateX(e), ez + 0.1f);
            KitArc(from, to, 0.9f, 1, (Color){ 86, 156, 255, 255 }, 0.6f);
            from = to;
        }
    }

    // the vault doors, each with its lock, and the vault behind the last one
    for (int k = 0; k < H.nd; k++) {
        float x = DoorX(k), o = H.open[k] > 0 ? Clamp01((H.time - H.open[k]) * 2.5f) : 0;
        char c = H.doors[k];
        Color light = c == 'K' ? (Color){ 120, 220, 140, 255 } : c == 'S' ? (Color){ 255, 190, 80, 255 } : (Color){ 255, 100, 90, 255 };
        KitDoor(V3(x, 0, 0), 0, o, o >= 1 ? COL3_GOOD : light, 0.6f);
        const char *name = c == 'K' ? "K keyhole" : c == 'S' ? "S steel" : c == 'W' ? "W wall" : "?";
        Label3D(V3(x, 2.1f, 0.2f), name, 11, light, LBL_DIM, LABEL_CODE);
    }
    if (H.nd > 0) {
        bool all = H.nd > 0 && H.open[H.nd - 1] > 0;
        KitVault(V3(DoorX(H.nd - 1) + 1.7f, 0, -0.1f), all ? Clamp01((H.time - H.open[H.nd - 1]) * 1.5f) : 0,
                 H.alarm ? COL3_BAD : (Color){ 255, 214, 90, 255 }, 0.6f + alarm * 0.4f);
    }

    // NULL: nowhere at all
    if (HasPit()) {
        float px = PitX();
        Part(MESH_CYLINDER, V3(px, 0.02f, ROAD_Z), V3(1.0f, 0.04f, 1.0f), (Color){ 8, 8, 12, 255 }, 0);
        Part(MESH_CYLINDER, V3(px, 0.03f, ROAD_Z), V3(1.12f, 0.03f, 1.12f), (Color){ 255, 90, 90, 255 }, 0.3f);
        Label3D(V3(px, 0.2f, ROAD_Z + 0.75f), "NULL", 12, LBL_BAD, LBL_DIM, LABEL_CODE);
    }

    // the heap: what malloc and new handed out
    if (HasHeap()) {
        Vector3 a = BlockPos(0), b = BlockPos(HEAP_MAX - 1);
        KitLot(V3((a.x + b.x) / 2, 0, LOT_Z), b.x - a.x + 1.1f, 1.3f, (Color){ 86, 156, 255, 255 }, 0.45f);
        Label3D(V3(a.x - 0.9f, 0.1f, LOT_Z + 0.85f), "the heap", 12, LBL_HI, LBL_DIM, LABEL_CODE);
        for (int j = 0; j < H.nheap; j++) {
            const Block *k = &H.heap[j];
            Vector3 p = BlockPos(j);
            float drop = k->live ? (1 - EaseOutBounce(Clamp01(k->t * 2.5f))) * 1.5f : 0;
            Color col = k->leak ? COL3_BAD : !k->live ? (Color){ 70, 74, 92, 255 } : k->isNew ? (Color){ 120, 200, 255, 255 } : (Color){ 180, 150, 255, 255 };
            float glow = k->leak ? 0.5f + 0.5f * Pulse(H.time, 5) : k->live ? 0.25f : 0;
            float s = 0.42f + fminf(0.3f, k->size / 120.0f);
            KitBlock(V3(p.x, s / 2 + drop, p.z), s, col, glow, 0);
            const char *what = k->leak ? "leak!" : !k->live ? (k->isNew ? "deleted" : "freed") : k->vals[0] ? k->vals : TextFormat("%d B", k->size);
            Label3D(V3(p.x, s + 0.35f + drop, p.z), what, 11, k->leak ? LBL_BAD : k->live ? WHITE : LBL_SOFT, LBL_DIM, LABEL_CODE);
        }
    }

    // the thief, with the loot and noise so far
    BotDraw(&H.thief);
    if (H.bag > 0) {
        float yaw = H.thief.yaw * DEG2RAD, size = 0.3f + fminf(0.3f, H.bag / 1500.0f);
        KitBag(V3(H.thief.pos.x - cosf(yaw) * 0.42f, 0, H.thief.pos.z + sinf(yaw) * 0.42f), size, (Color){ 150, 120, 80, 255 });
    }
    Vector3 head = BotHead(&H.thief);
    if (H.bag || H.noise) {
        const char *bag = H.nd > 0 ? TextFormat("noise %d", H.noise) : TextFormat("loot %d", H.bag);
        Label3D(V3(head.x, head.y + 0.3f, head.z), bag, 12, LBL_GOLD, LBL_DIM, LABEL_CODE);
    }
    int from = H.want[0] && H.says > 0 ? H.says - 1 : 0;
    float base = head.y + ((H.bag || H.noise) ? 0.72f : 0.4f);
    for (int k = from; k < H.says; k++) {
        const Bubble *s = &H.say[k];
        Color fg = s->state == SAY_GOOD ? LBL_GOOD : s->state == SAY_BAD ? LBL_BAD : WHITE;
        Label3D(V3(head.x, base + (H.says - 1 - k) * 0.42f, head.z), s->text, 13, fg, LBL_DIM, LABEL_CODE);
    }
    if (H.want[0]) Label3D(V3(head.x, base + 0.42f, head.z), TextFormat("expected: %.40s", H.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    PopsDraw(&H.pops);
}

const LevelDef LEVEL_HEIST = {
    "", "Memory Heist", "Your pointers move the thief: every go(), grab(), malloc() and free() is replayed on the street.",
    { 190, 130, 255, 255 },
    ParseHeist, HeistReset, HeistEvent, HeistUpdate, HeistDraw, NULL,
};
