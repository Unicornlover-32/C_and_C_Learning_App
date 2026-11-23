// Robot Rescue: the 3D scene for the robot side quests and the two robot bosses.
//
// These programs drive a robot through a maze with move(), turn_left() and the rest. The
// game library (game/cfd_robot.h) writes every action to a trace - "0 M" a move, "0 L" /
// "0 R" a turn, "0 X" a bump into a wall, "COPY 2 0" a hologram copy of a Robot object -
// and ends it with END WIN or END FAIL and the reason. The maze comes from the test's
// input; the trace is replayed action by action, and then the robot radios in: the lines
// the program printed, checked against the expected output (when the test has one).
//
// The verdict is read exactly the way the runner reads it (the last END line), so the
// scene can never disagree with Run & Check. Runs of the same action are merged ("move 5
// tiles", "spin 300 times"), and a run that still has too many actions for the event list
// skips ahead, then puts every robot and coin where the trace left them.
#include "levels.h"
#include "stage.h"
#include "../ui.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { RB_SIZE = 1, RB_ROW, RB_BOT, RB_MOVE, RB_TURN, RB_BUMP, RB_COPY, RB_SKIP, RB_PLACE, RB_TAKE, RB_HOME, RB_SAY };
enum { RBF_GAME = 1, RBF_LINE, RBF_EXTRA };

#define RW    40           // the library's limits
#define RH    30
#define RBOTS 8

static const int DX[4] = { 0, 1, 0, -1 };   // facing north, east, south, west
static const int DY[4] = { -1, 0, 1, 0 };

// ---------------------------------------------------------------- the parser

typedef struct { int x, y, dir; bool ghost, alive; } SimBot;
typedef struct {
    int    w, h;
    char   g[RH][RW + 1];
    SimBot bot[RBOTS];
    int    bots;
} Sim;

static bool Space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }

static int DirOf(char c) { return c == '^' ? 0 : c == '>' ? 1 : c == 'v' ? 2 : c == '<' ? 3 : -1; }

// The maze, read the way the library reads it: "<width> <height>", then one word per row
static bool LoadMaze(const char *in, Sim *s)
{
    memset(s, 0, sizeof(*s));
    if (!in) return false;
    const char *p = in;
    int v[2];
    for (int k = 0; k < 2; k++) {
        while (Space(*p)) p++;
        bool neg = (*p == '-');
        if (*p == '-' || *p == '+') p++;
        if (*p < '0' || *p > '9') return false;
        long n = 0;
        for (int d = 0; *p >= '0' && *p <= '9'; p++, d++) if (d < 6) n = n * 10 + (*p - '0');
        v[k] = (int)(neg ? -n : n);
    }
    if (v[0] < 1 || v[1] < 1 || v[0] > RW || v[1] > RH) return false;
    s->w = v[0];
    s->h = v[1];
    for (int y = 0; y < s->h; y++) {
        while (Space(*p)) p++;
        if (!*p) { memset(s, 0, sizeof(*s)); return false; }
        const char *row = p;
        int len = 0;
        while (*p && !Space(*p)) { p++; len++; }
        for (int x = 0; x < s->w; x++) {
            char c = x < len ? row[x] : '#';
            int dir = DirOf(c);
            if (dir >= 0) {
                if (s->bots < RBOTS) s->bot[s->bots++] = (SimBot){ x, y, dir, false, true };
                c = '.';
            }
            s->g[y][x] = c;
        }
        s->g[y][s->w] = '\0';
    }
    return s->bots > 0;
}

// One line of the trace: "3 M", "0 L", "COPY 2 0"... false for anything else
static bool TraceLine(const char *s, int len, char *op, int *a, int *b)
{
    while (len > 0 && (s[len - 1] == '\r' || s[len - 1] == ' ')) len--;
    if (len >= 8 && memcmp(s, "COPY ", 5) == 0) {
        int i = 5, v[2] = { 0, 0 };
        for (int k = 0; k < 2; k++) {
            int digits = 0;
            while (i < len && s[i] >= '0' && s[i] <= '9') { if (digits < 4) v[k] = v[k] * 10 + (s[i] - '0'); digits++; i++; }
            if (digits == 0 || digits > 4) return false;
            if (k == 0) { if (i >= len || s[i] != ' ') return false; i++; }
        }
        if (i != len) return false;
        *op = 'C';
        *a = v[0];
        *b = v[1];
        return true;
    }
    if (len == 3 && s[0] >= '0' && s[0] <= '9' && s[1] == ' ' && (s[2] == 'M' || s[2] == 'L' || s[2] == 'R' || s[2] == 'X')) {
        *op = s[2];
        *a = s[0] - '0';
        *b = 0;
        return true;
    }
    return false;
}

typedef struct { char op; int bot, count; } Pending;

static void Flush(Parse *p, Pending *pd)
{
    if (!pd->op) return;
    if (pd->op == 'M') EvAct(p, RB_MOVE, pd->bot, pd->count, 0);
    else EvAct(p, RB_TURN, pd->bot, pd->count, pd->op == 'R' ? 1 : -1);
    pd->op = 0;
}

static void ParseRobot(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    static Sim s;
    LoadMaze(run->input, &s);
    int coins = 0;
    EvSpawn(&p, RB_SIZE, s.w, s.h, 0);
    for (int y = 0; y < s.h; y++) {
        EvText(EvSpawn(&p, RB_ROW, y, 0, 0), s.g[y]);
        for (int x = 0; x < s.w; x++) if (s.g[y][x] == 'C') coins++;
    }
    for (int i = 0; i < s.bots; i++) EvSpawn(&p, RB_BOT, i, s.bot[i].x + s.bot[i].y * 64, s.bot[i].dir);
    if (ParsePreview(&p)) return;

    const char *t = run->trace ? run->trace : "";

    // the actions, up to the first END line. Past `limit` events the rest only move the
    // simulation on, and the robots and coins are put in place at the end.
    int limit = EV_MAX - 2 * PARSE_LINES - RBOTS - (coins < 200 ? coins : 200) - 24;
    static unsigned char late[RH][RW];
    memset(late, 0, sizeof(late));
    Pending pend = { 0 };
    bool skipping = false;
    int skipped = 0;
    const char *q = t;
    for (int lines = 0; *q && lines < 40000; lines++) {
        const char *e = strchr(q, '\n');
        int len = e ? (int)(e - q) : (int)strlen(q);
        if (len >= 4 && memcmp(q, "END ", 4) == 0) break;
        char op;
        int a, b;
        if (TraceLine(q, len, &op, &a, &b)) {
            if (op == 'C') {
                if (a < RBOTS && b < s.bots && s.bot[b].alive) {
                    for (int k = s.bots; k < a; k++) s.bot[k].alive = false;
                    s.bot[a] = s.bot[b];
                    s.bot[a].ghost = true;
                    s.bot[a].alive = true;
                    if (a >= s.bots) s.bots = a + 1;
                    if (!skipping) { Flush(&p, &pend); EvAct(&p, RB_COPY, a, b, 0); }
                }
            } else if (a < s.bots && s.bot[a].alive) {
                SimBot *r = &s.bot[a];
                if (op == 'M') {
                    int nx = r->x + DX[r->dir], ny = r->y + DY[r->dir];
                    if (nx >= 0 && ny >= 0 && nx < s.w && ny < s.h) { r->x = nx; r->y = ny; }
                    if (!r->ghost && s.g[r->y][r->x] == 'C') {
                        s.g[r->y][r->x] = '.';
                        if (skipping) late[r->y][r->x] = 1;
                    }
                } else if (op == 'L' || op == 'R') {
                    r->dir = (r->dir + (op == 'R' ? 1 : 3)) % 4;
                } else if (r->ghost) {
                    r->alive = false;             // a hologram that drives into a wall fizzles out
                }
                if (skipping) skipped++;
                else if (op != 'X' && pend.op == op && pend.bot == a && pend.count < 100000) pend.count++;
                else {
                    Flush(&p, &pend);
                    if (op == 'X') EvAct(&p, RB_BUMP, a, 0, 0);
                    else pend = (Pending){ op, a, 1 };
                }
            }
            if (!skipping && ev->count >= limit) { Flush(&p, &pend); skipping = true; }
        }
        if (!e) break;
        q = e + 1;
    }
    Flush(&p, &pend);
    if (skipping) {
        EvAct(&p, RB_SKIP, skipped, 0, 0);
        for (int i = 0; i < s.bots; i++)
            EvAct(&p, RB_PLACE, i, s.bot[i].x + s.bot[i].y * 64, s.bot[i].dir + (s.bot[i].ghost ? 4 : 0) + (s.bot[i].alive ? 8 : 0));
        for (int y = 0; y < s.h; y++)
            for (int x = 0; x < s.w; x++) if (late[y][x]) EvAct(&p, RB_TAKE, x, y, 0);
    }

    // the game's verdict; if the robots made it, they radio in what the program printed
    ParseGameEnd(&p, RB_HOME, RB_SAY, RBF_GAME, RBF_LINE, RBF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_BACK, DO_CHEER };
enum { SAY_GOOD = 1, SAY_BAD, SAY_FREE };

#define WALL_COL  (Color){ 50, 54, 78, 255 }
#define WALL_TOP  (Color){ 66, 62, 104, 255 }
#define FLOOR_TOP (Color){ 128, 142, 192, 255 }
#define BOT_SCALE 0.8f
#define HOLO_COL  (Color){ 120, 235, 255, 255 }

static const Color FLEET[4] = { { 255, 95, 95, 255 }, { 90, 160, 255, 255 }, { 255, 200, 80, 255 }, { 120, 220, 120, 255 } };
static const char *FLEET_NAMES[4] = { "red", "blue", "gold", "green" };

typedef struct { char text[EV_TEXT]; int state; } Bubble;

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time, step;
    int   w, h;
    char  g[RH][RW + 1];       // 'c': a coin that is being picked up
    float gone[RH][RW];        // when it disappears
    Bot   bot[RBOTS];
    int   bx[RBOTS], by[RBOTS], bdir[RBOTS];
    bool  used[RBOTS], ghost[RBOTS], alive[RBOTS];
    float fade[RBOTS], spin[RBOTS];
    int   spinDir[RBOTS];
    int   bots, real, coins, taken, actions;
    bool  home, failed, won;
    Bubble say[3];
    int   says;
    char  want[FAIL_TEXT];
} RS;

static Vector3 Tile(int x, int y) { return V3((x - (RS.w - 1) * 0.5f), 0, (y - (RS.h - 1) * 0.5f)); }
static float   YawOf(int dir) { return 180.0f - 90.0f * dir; }
static bool    Inside(int x, int y) { return x >= 0 && y >= 0 && x < RS.w && y < RS.h; }
static Bot    *MainBot(void) { return RS.used[0] ? &RS.bot[0] : NULL; }

static void RobotReset(const EventList *ev)
{
    memset(&RS, 0, sizeof(RS));
    RS.ev = ev;
    // the pace: a short run plays slowly, a long one (up to the 2000-action battery) speeds up
    int actions = 0;
    for (int i = 0; ev && i < ev->count; i++) {
        const GameEvent *e = &ev->ev[i];
        if (e->type != EV_ACT) continue;
        if (e->code == RB_MOVE) actions += e->b;
        else if (e->code == RB_TURN) actions += e->b < 4 ? e->b : 4;
        else if (e->code == RB_BUMP || e->code == RB_COPY) actions += 2;
    }
    RS.step = actions <= 28 ? 0.34f : fmaxf(0.012f, 9.5f / actions);
    ShotOrbit(V3(0, 0.2f, 0.35f), 9, 54, 0, 40, 0);
}

static bool Walkable(int x, int y) { return Inside(x, y) && RS.g[y][x] != '#'; }

// The camera takes in the part of the maze the robots can reach, and the walls around it
static void FrameMaze(void)
{
    int x0 = RS.w, x1 = -1, y0 = RS.h, y1 = -1;
    for (int y = 0; y < RS.h; y++)
        for (int x = 0; x < RS.w; x++)
            if (Walkable(x, y)) { x0 = x < x0 ? x : x0; x1 = x > x1 ? x : x1; y0 = y < y0 ? y : y0; y1 = y > y1 ? y : y1; }
    if (x1 < 0) { x0 = 0; x1 = RS.w > 0 ? RS.w - 1 : 6; y0 = 0; y1 = RS.h > 0 ? RS.h - 1 : 4; }
    Vector3 a = Tile(x0, y0), b = Tile(x1, y1);
    float bw = (float)(x1 - x0 + 3), bh = (float)(y1 - y0 + 3);
    float d = fmaxf((bw * 0.5f + 0.5f) / 0.62f, (bh * 0.4f + 0.8f) / 0.36f);
    ShotOrbit(V3((a.x + b.x) / 2, 0.3f, (a.z + b.z) / 2 + 0.3f), d < 6.5f ? 6.5f : d, 52, 0, 40, 0);
}

static void Recolour(void)
{
    for (int i = 0; i < RS.bots; i++) {
        Bot *b = &RS.bot[i];
        if (RS.ghost[i]) { b->body = HOLO_COL; b->accent = (Color){ 60, 200, 255, 255 }; }
        else if (RS.real > 1) { b->body = (Color){ 0 }; b->accent = FLEET[i % 4]; }
    }
}

static void PlaceBot(int i, int x, int y, int dir, bool ghost)
{
    if (i < 0 || i >= RBOTS || !Inside(x, y)) return;
    RS.used[i] = true;
    RS.alive[i] = true;
    RS.ghost[i] = ghost;
    RS.fade[i] = 1;
    RS.spin[i] = 0;
    RS.bx[i] = x;
    RS.by[i] = y;
    RS.bdir[i] = dir & 3;
    BotInit(&RS.bot[i], Tile(x, y), YawOf(dir & 3));
    RS.bot[i].scale = BOT_SCALE;
    if (i >= RS.bots) RS.bots = i + 1;
    Recolour();
}

static void AddBubble(const char *text, int state)
{
    if (RS.says == 3) { RS.say[0] = RS.say[1]; RS.say[1] = RS.say[2]; RS.says = 2; }
    StrCopy(RS.say[RS.says].text, EV_TEXT, text);
    RS.say[RS.says].state = state;
    RS.says++;
}

static float Act(const GameEvent *e)
{
    int i = e->a;
    bool ok = i >= 0 && i < RBOTS && RS.used[i];
    switch (e->code) {
        case RB_MOVE: {
            if (!ok) return 0;
            int n = e->b < 1 ? 1 : e->b, x = RS.bx[i], y = RS.by[i], d = RS.bdir[i], steps = 0;
            for (int k = 0; k < n && k < 64; k++) {
                int nx = x + DX[d], ny = y + DY[d];
                if (!Inside(nx, ny)) break;
                x = nx;
                y = ny;
                steps++;
                if (!RS.ghost[i] && RS.g[y][x] == 'C') { RS.g[y][x] = 'c'; RS.gone[y][x] = RS.time + RS.step * (k + 0.75f); }
            }
            float dur = RS.step * (steps > 0 ? steps : 1);
            BotMoveTo(&RS.bot[i], Tile(x, y), dur);
            RS.bx[i] = x;
            RS.by[i] = y;
            RS.actions += n;
            return dur;
        }
        case RB_TURN: {
            if (!ok) return 0;
            int n = e->b < 1 ? 1 : e->b, sgn = e->value > 0 ? 1 : -1;
            RS.bdir[i] = ((RS.bdir[i] + sgn * (n % 4)) % 4 + 4) % 4;
            RS.actions += n;
            float dur = RS.step * 0.75f * (n < 4 ? n : 4);
            if (n >= 4) { RS.spin[i] = dur; RS.spinDir[i] = sgn; }
            else BotFace(&RS.bot[i], YawOf(RS.bdir[i]));
            if (n >= 8) PopAdd(&RS.pops, TextFormat("spun %d times", n / 4), BotHead(&RS.bot[i]), LBL_GOLD);
            return dur;
        }
        case RB_BUMP: {
            if (!ok) return 0;
            int d = RS.bdir[i];
            Vector3 at = Tile(RS.bx[i], RS.by[i]);
            BotMoveTo(&RS.bot[i], V3(at.x + DX[d] * 0.3f, 0, at.z + DY[d] * 0.3f), 0.1f);
            CueAt(&RS.cues, RS.time + 0.11f, DO_BACK, i, 0, at, 0);
            RS.actions++;
            if (RS.ghost[i]) {
                RS.alive[i] = false;
                PopAdd(&RS.pops, "fizz!", BotHead(&RS.bot[i]), (Color){ 140, 240, 255, 255 });
            } else {
                PopAdd(&RS.pops, "CRASH!", BotHead(&RS.bot[i]), LBL_BAD);
                PlaybackShake(0.35f);
            }
            return fmaxf(0.55f, RS.step * 2);
        }
        case RB_COPY: {
            int src = e->b;
            if (i < 0 || i >= RBOTS || src < 0 || src >= RBOTS || !RS.used[src]) return 0;
            PlaceBot(i, RS.bx[src], RS.by[src], RS.bdir[src], true);
            RS.bot[i].pos = RS.bot[src].pos;
            FxSparkle(BotHead(&RS.bot[i]), 22, HOLO_COL);
            PopAdd(&RS.pops, "a copy!", BotHead(&RS.bot[i]), (Color){ 140, 240, 255, 255 });
            return fmaxf(0.5f, RS.step * 2);
        }
        case RB_SKIP:
            RS.actions += e->a;
            PopAdd(&RS.pops, TextFormat("fast-forward: %d more", e->a), V3(0, 1.6f, 0), LBL_GOLD);
            return 0.7f;
        case RB_PLACE: {
            int x = e->b % 64, y = e->b / 64, v = (int)e->value;
            if (i < 0 || i >= RBOTS || !Inside(x, y)) return 0;
            if (!RS.used[i]) PlaceBot(i, x, y, v & 3, (v & 4) != 0);
            BotMoveTo(&RS.bot[i], Tile(x, y), 0.35f);
            RS.bx[i] = x;
            RS.by[i] = y;
            RS.bdir[i] = v & 3;
            RS.alive[i] = (v & 8) != 0;
            return 0;
        }
        case RB_TAKE:
            if (Inside(e->a, e->b) && RS.g[e->b][e->a] == 'C') { RS.g[e->b][e->a] = 'c'; RS.gone[e->b][e->a] = RS.time + 0.3f; }
            return 0;
        case RB_HOME:
            RS.home = true;
            for (int k = 0; k < RS.bots; k++) if (RS.used[k] && !RS.ghost[k]) BotFace(&RS.bot[k], 0);   // turn to the camera
            return 0.8f;
        case RB_SAY:
            AddBubble(e->text, e->b ? SAY_FREE : SAY_GOOD);
            return 0.9f;
    }
    return 0;
}

static float RobotEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == RB_SIZE) {
                RS.w = (e->a >= 1 && e->a <= RW && e->b >= 1 && e->b <= RH) ? e->a : 0;
                RS.h = RS.w ? e->b : 0;
                FrameMaze();
            } else if (e->code == RB_ROW && e->a >= 0 && e->a < RS.h) {
                StrCopy(RS.g[e->a], RW + 1, e->text);
                for (int x = 0; x < RS.w; x++) {
                    if (RS.g[e->a][x] == '\0') { for (int k = x; k < RS.w; k++) RS.g[e->a][k] = '#'; RS.g[e->a][RS.w] = '\0'; break; }
                    if (RS.g[e->a][x] == 'C') RS.coins++;
                }
                FrameMaze();
            } else if (e->code == RB_BOT) {
                RS.real++;
                PlaceBot(e->a, e->b % 64, e->b / 64, (int)e->value, false);
            }
            return 0;
        case EV_PRINT:
            return 0;
        case EV_ACT:
            return Act(e);
        case EV_FAIL: {
            RS.failed = true;
            Bot *b = MainBot();
            if (b) { BotMood(b, MOOD_SAD); BotShakeHead(b); }
            if (e->code != RBF_GAME && RS.ev) {
                AddBubble(RS.ev->actual[0] ? RS.ev->actual : "(silence)", SAY_BAD);
                StrCopy(RS.want, sizeof(RS.want), RS.ev->fail == FAIL_EXTRA ? "(nothing more)" : RS.ev->expected);
            }
            PlaybackShake(0.2f);
            return 2.8f;
        }
        case EV_CRASH: {
            RS.failed = true;
            Bot *b = MainBot();
            bool timeout = RS.ev && RS.ev->fail == FAIL_TIMEOUT;
            if (b && timeout) { BotLookAround(b); PopAdd(&RS.pops, "stuck in a loop?", BotHead(b), LBL_GOLD); }
            else if (b) { BotPowerDown(b); PopAdd(&RS.pops, "program crashed!", BotHead(b), LBL_BAD); PlaybackShake(0.3f); }
            return 2.6f;
        }
        case EV_WIN:
            RS.won = true;
            for (int k = 0; k < RS.bots; k++) if (RS.used[k] && !RS.ghost[k]) BotCheer(&RS.bot[k]);
            CueAt(&RS.cues, RS.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void RobotUpdate(float dt)
{
    RS.time += dt;
    for (int i = 0; i < RS.bots; i++) {
        if (!RS.used[i]) continue;
        Bot *b = &RS.bot[i];
        if (RS.spin[i] > 0) {
            RS.spin[i] -= dt;
            b->yawTo = b->yaw + RS.spinDir[i] * 60.0f;
            if (RS.spin[i] <= 0) BotFace(b, YawOf(RS.bdir[i]));
        }
        if (!RS.alive[i]) RS.fade[i] = fmaxf(0, RS.fade[i] - dt * 1.6f);
        b->scale = BOT_SCALE * (0.3f + 0.7f * RS.fade[i]);
        BotUpdate(b, dt);
    }
    for (int y = 0; y < RS.h; y++)
        for (int x = 0; x < RS.w; x++)
            if (RS.g[y][x] == 'c' && RS.time >= RS.gone[y][x]) {
                RS.g[y][x] = '.';
                RS.taken++;
                Vector3 p = Tile(x, y);
                FxSparkle(V3(p.x, 0.5f, p.z), 10, (Color){ 255, 214, 90, 255 });
                if (RS.step > 0.05f) PopAdd(&RS.pops, "+1", V3(p.x, 0.9f, p.z), LBL_GOLD);
            }
    PopsUpdate(&RS.pops, dt);
    Cue c;
    while (CueDue(&RS.cues, RS.time, &c)) {
        if (c.what == DO_BACK && c.a >= 0 && c.a < RBOTS) {
            BotBump(&RS.bot[c.a]);
            BotMoveTo(&RS.bot[c.a], c.v, 0.2f);
            BotFace(&RS.bot[c.a], YawOf(RS.bdir[c.a]));
        } else if (c.what == DO_CHEER) {
            bool finale = PlaybackFinale();
            Vector3 at = MainBot() ? MainBot()->pos : V3(0, 0, 0);
            FxConfetti(V3(at.x, 1.4f, at.z), finale ? 160 : 70, 2.6f);
            if (finale) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static void DrawSign(Vector3 p, char c)
{
    Color col = c == 'L' ? (Color){ 90, 160, 255, 255 } : c == 'R' ? (Color){ 255, 160, 70, 255 } : (Color){ 120, 220, 140, 255 };
    Part(MESH_CYLINDER, V3(p.x + 0.3f, 0.36f, p.z - 0.3f), V3(0.05f, 0.62f, 0.05f), (Color){ 78, 86, 112, 255 }, 0);
    KitPanel(V3(p.x + 0.3f, 0.74f, p.z - 0.28f), 0.34f, 0.26f, col, 0.45f);
    char t[2] = { c, '\0' };
    Label3D(V3(p.x + 0.3f, 0.74f, p.z - 0.2f), t, 13, WHITE, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
}

static void RobotDraw(void)
{
    float hw = RS.w * 0.5f + 0.4f, hd = RS.h * 0.5f + 0.4f;
    if (RS.w == 0) { hw = 3.5f; hd = 2.5f; }
    KitStage(V3(0, 0, 0), hw, hd);
    float pulse = Pulse(RS.time, 2.5f);
    for (int y = 0; y < RS.h; y++)
        for (int x = 0; x < RS.w; x++) {
            char c = RS.g[y][x];
            Vector3 p = Tile(x, y);
            if (c == '#') {           // walls stand only along the corridors; solid rock is left flat
                bool edge = false;
                for (int dy = -1; dy <= 1 && !edge; dy++)
                    for (int dx = -1; dx <= 1; dx++) if (Walkable(x + dx, y + dy)) { edge = true; break; }
                if (!edge) continue;
                Part(MESH_CUBE, V3(p.x, 0.26f, p.z), V3(1.0f, 0.52f, 1.0f), WALL_COL, 0);
                Part(MESH_CUBE, V3(p.x, 0.525f, p.z), V3(0.9f, 0.012f, 0.9f), WALL_TOP, 0.12f);
                continue;
            }
            float pop = Clamp01(RS.time * 2.2f - (x + y) * 0.04f);
            if (c == 'E') {
                bool bad = RS.failed && !RS.home;
                Color ec = bad ? COL3_BAD : COL3_GOOD;
                KitTile(p, 0.96f, pop, ec, 0.12f + 0.12f * pulse + (RS.home ? 0.25f : 0));
                Part(MESH_CYLINDER, V3(p.x, 0.22f + 0.08f * pulse, p.z), V3(0.7f, 0.025f, 0.7f), ec, 0.45f);
                Part(MESH_CYLINDER, V3(p.x, 0.22f + 0.08f * pulse, p.z), V3(0.56f, 0.03f, 0.56f), (Color){ 30, 34, 48, 255 }, 0);
                continue;
            }
            KitTile(p, 0.96f, pop, FLOOR_TOP, 0);
            if (c == 'C' || (c == 'c' && RS.time < RS.gone[y][x])) {
                rlPushMatrix();
                rlTranslatef(p.x, 0.1f, p.z);
                rlScalef(1.35f, 1.35f, 1.35f);
                KitCoin(V3(0, 0, 0), 1, RS.time * 140 + x * 37 + y * 11, 0.45f + (RS.failed ? 0.3f * pulse : 0), RS.failed);
                rlPopMatrix();
            }
            else if (c == 'L' || c == 'R' || c == 'S')
                DrawSign(p, c);
        }

    for (int i = 0; i < RS.bots; i++) {
        if (!RS.used[i] || RS.fade[i] <= 0.02f) continue;
        BotDraw(&RS.bot[i]);
        Vector3 h = BotHead(&RS.bot[i]);
        if (RS.ghost[i]) Label3D(V3(h.x, h.y + 0.25f, h.z), "hologram", 11, HOLO_COL, LBL_DIM, LABEL_CODE);
        else if (RS.real > 1) Label3D(V3(h.x, h.y + 0.25f, h.z), FLEET_NAMES[i % 4], 11, FLEET[i % 4], LBL_DIM, LABEL_CODE);
    }

    // what the robot radioed in, newest nearest its head
    Bot *b = MainBot();
    Vector3 head = b ? BotHead(b) : V3(0, 1, 0);
    // (after a wrong line: just that line, and the one mission control wanted above it)
    int from = RS.want[0] && RS.says > 0 ? RS.says - 1 : 0;
    for (int k = from; k < RS.says; k++) {
        const Bubble *s = &RS.say[k];
        Color fg = s->state == SAY_GOOD ? LBL_GOOD : s->state == SAY_BAD ? LBL_BAD : WHITE;
        Label3D(V3(head.x, head.y + 0.45f + (RS.says - 1 - k) * 0.42f, head.z), s->text, 13, fg, LBL_DIM, LABEL_CODE);
    }
    if (RS.want[0])
        Label3D(V3(head.x, head.y + 0.87f, head.z), TextFormat("expected: %.40s", RS.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    PopsDraw(&RS.pops);
}

// The counters, in the bottom-left corner of the view
static void RobotOverlay(Rectangle view)
{
    if (!RS.ev || RS.ev->preview || RS.w == 0) return;
    const char *text = RS.coins ? TextFormat("coins %d / %d     moves and turns %d", RS.taken, RS.coins, RS.actions)
                                : TextFormat("moves and turns %d", RS.actions);
    float w = UiTextWidth(text, 15) + 24;
    Rectangle box = { view.x + 12, view.y + view.height - 40, w, 28 };
    DrawRectangleRounded(box, 0.5f, 6, (Color){ 12, 14, 22, 200 });
    UiText(text, box.x + 12, box.y + 5, 15, RS.coins && RS.taken == RS.coins ? (Color){ 140, 255, 180, 255 } : (Color){ 255, 222, 130, 255 });
}

const LevelDef LEVEL_ROBOT = {
    "", "Robot Rescue", "Your code drives the robot: every move, turn and bump is replayed in the maze.",
    { 120, 190, 255, 255 },
    ParseRobot, RobotReset, RobotEvent, RobotUpdate, RobotDraw, RobotOverlay,
};
