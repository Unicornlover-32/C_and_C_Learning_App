#include "robotview.h"
#include "ui.h"
#include "art.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RMAXW    40
#define RMAXH    30
#define RMAXBOTS 8
#define RMAXEV   2400

typedef struct {
    int  w, h;
    char g[RMAXH][RMAXW + 1];
    int  bots;
    int  bx[RMAXBOTS], by[RMAXBOTS], bdir[RMAXBOTS];
} RWorld;

typedef struct { int bot; char code; int arg; } REvent;

typedef struct {
    int   x, y, dir;
    bool  alive, ghost, born;
    int   coins;
} RBot;

static RWorld worlds[MAX_TESTS];
static int    worldCount;

static struct {
    int    test;            // which maze is replaying, -1 for none
    RWorld w;               // working copy: coins disappear as they're collected
    REvent ev[RMAXEV];
    int    count, idx;
    float  t;               // progress through the current event, 0 to 1
    RBot   bot[RMAXBOTS];
    int    bots;
    bool   won, celebrated;
    float  speed;
    int    crashBot;        // the robot that crashed, or -1
    float  crashTime;
} play = { .test = -1, .speed = 6, .crashBot = -1 };

static const int DX[4] = { 0, 1, 0, -1 };
static const int DY[4] = { -1, 0, 1, 0 };
static const Color BOT_COLORS[] = { { 255, 95, 95, 255 }, { 90, 160, 255, 255 }, { 255, 200, 80, 255 }, { 120, 220, 120, 255 } };

static bool ParseWorld(const char *text, RWorld *w)
{
    memset(w, 0, sizeof(*w));
    if (!text || sscanf(text, "%d %d", &w->w, &w->h) != 2) return false;
    if (w->w < 1 || w->h < 1 || w->w > RMAXW || w->h > RMAXH) return false;
    const char *p = strchr(text, '\n');
    for (int y = 0; y < w->h; y++) {
        if (!p) return false;
        p++;
        for (int x = 0; x < w->w; x++) {
            char c = (p[x] && p[x] != '\n') ? p[x] : '#';
            int dir = c == '^' ? 0 : c == '>' ? 1 : c == 'v' ? 2 : c == '<' ? 3 : -1;
            if (dir >= 0) {
                if (w->bots < RMAXBOTS) {
                    w->bx[w->bots] = x;
                    w->by[w->bots] = y;
                    w->bdir[w->bots] = dir;
                    w->bots++;
                }
                c = '.';
            }
            w->g[y][x] = c;
        }
        p = strchr(p, '\n');
    }
    return true;
}

void RobotLoad(const TestCase *tests, int count)
{
    worldCount = count > MAX_TESTS ? MAX_TESTS : count;
    for (int i = 0; i < worldCount; i++) ParseWorld(tests[i].input, &worlds[i]);
    play.test = -1;
}

static void ResetBots(void)
{
    play.w = worlds[play.test];
    play.bots = play.w.bots;
    for (int i = 0; i < RMAXBOTS; i++) {
        play.bot[i] = (RBot){ 0 };
        if (i < play.bots) {
            play.bot[i] = (RBot){ play.w.bx[i], play.w.by[i], play.w.bdir[i], true, false, true, 0 };
        }
    }
    play.idx = 0;
    play.t = 0;
    play.celebrated = false;
    play.crashBot = -1;
}

void RobotPlay(int test, const char *events, bool won)
{
    if (test < 0 || test >= worldCount) return;
    play.test = test;
    play.won = won;
    play.count = 0;
    const char *p = events;
    while (p && *p && play.count < RMAXEV) {
        int bot, a, b;
        char code;
        if (sscanf(p, "COPY %d %d", &a, &b) == 2) {
            play.ev[play.count++] = (REvent){ a, 'C', b };
        } else if (sscanf(p, "%d %c", &bot, &code) == 2 && bot >= 0 && bot < RMAXBOTS) {
            play.ev[play.count++] = (REvent){ bot, code, 0 };
        }
        p = strchr(p, '\n');
        if (p) p++;
    }
    ResetBots();
}

void RobotReplay(void) { if (play.test >= 0) ResetBots(); }
void RobotSetSpeed(float eventsPerSecond) { play.speed = eventsPerSecond; }
bool RobotPlaying(void) { return play.test >= 0 && play.idx < play.count; }

// Finishes the current event
static void Apply(const REvent *e)
{
    RBot *b = &play.bot[e->bot];
    switch (e->code) {
        case 'M': {
            b->x += DX[b->dir];
            b->y += DY[b->dir];
            char *tile = &play.w.g[b->y][b->x];
            if (!b->ghost && *tile == 'C') { *tile = '.'; b->coins++; }
            break;
        }
        case 'L': b->dir = (b->dir + 3) % 4; break;
        case 'R': b->dir = (b->dir + 1) % 4; break;
        case 'X':
            if (b->ghost) b->alive = false;
            else { play.crashBot = e->bot; play.crashTime = (float)GetTime(); }
            break;
        case 'C': {
            RBot *src = &play.bot[e->arg];
            *b = *src;
            b->ghost = true;
            b->alive = true;
            b->born = true;
            b->coins = 0;
            if (e->bot >= play.bots) play.bots = e->bot + 1;
            break;
        }
    }
}

static void Advance(float dt)
{
    if (play.test < 0 || play.idx >= play.count) return;
    play.t += dt * play.speed;
    while (play.t >= 1 && play.idx < play.count) {
        Apply(&play.ev[play.idx]);
        play.idx++;
        play.t -= 1;
    }
    if (play.idx >= play.count) play.t = 0;
}

// ---------------------------------------------------------------- drawing

static void DrawRobotBody(Vector2 c, float s, float angleDeg, Color color, float alpha, bool ghost)
{
    float a = angleDeg * DEG2RAD;
    Vector2 f = { sinf(a), -cosf(a) };          // facing direction (0 degrees = north)
    Vector2 side = { -f.y, f.x };
    Color body = ColorAlpha(color, alpha);
    Color dark = ColorAlpha((Color){ 20, 26, 40, 255 }, alpha);

    if (!ghost) DrawEllipse((int)c.x, (int)(c.y + s * 0.36f), s * 0.34f, s * 0.1f, ColorAlpha(BLACK, 0.25f * alpha));
    // wheels
    DrawCircleV((Vector2){ c.x + side.x * s * 0.3f, c.y + side.y * s * 0.3f }, s * 0.12f, dark);
    DrawCircleV((Vector2){ c.x - side.x * s * 0.3f, c.y - side.y * s * 0.3f }, s * 0.12f, dark);
    DrawCircleV(c, s * 0.32f, body);
    DrawCircleV(c, s * 0.22f, ColorAlpha(ColorBrightness(color, 0.35f), alpha));
    // eyes look where the robot is facing
    Vector2 eye = { c.x + f.x * s * 0.14f, c.y + f.y * s * 0.14f };
    DrawCircleV((Vector2){ eye.x + side.x * s * 0.09f, eye.y + side.y * s * 0.09f }, s * 0.06f, dark);
    DrawCircleV((Vector2){ eye.x - side.x * s * 0.09f, eye.y - side.y * s * 0.09f }, s * 0.06f, dark);
    // nose arrow
    Vector2 tip = { c.x + f.x * s * 0.44f, c.y + f.y * s * 0.44f };
    Vector2 l = { c.x + f.x * s * 0.3f + side.x * s * 0.1f, c.y + f.y * s * 0.3f + side.y * s * 0.1f };
    Vector2 r = { c.x + f.x * s * 0.3f - side.x * s * 0.1f, c.y + f.y * s * 0.3f - side.y * s * 0.1f };
    DrawTriangle(tip, l, r, ColorAlpha(WHITE, 0.9f * alpha));
    DrawTriangle(tip, r, l, ColorAlpha(WHITE, 0.9f * alpha));
    if (ghost) DrawCircleLinesV(c, s * 0.36f, ColorAlpha((Color){ 140, 255, 255, 255 }, alpha));
}

void RobotDraw(Rectangle area, int test)
{
    if (test < 0 || test >= worldCount) return;
    bool live = (play.test == test);
    if (live) Advance(GetFrameTime());
    const RWorld *w = live ? &play.w : &worlds[test];
    if (w->w == 0) return;

    float ts = fminf(area.width / w->w, area.height / w->h);
    if (ts > 64) ts = 64;
    float ox = area.x + (area.width - ts * w->w) / 2;
    float oy = area.y + (area.height - ts * w->h) / 2;
    float time = (float)GetTime();

    for (int y = 0; y < w->h; y++) {
        for (int x = 0; x < w->w; x++) {
            Rectangle r = { ox + x * ts, oy + y * ts, ts, ts };
            char c = w->g[y][x];
            if (c == '#') {
                DrawRectangleRec(r, (Color){ 46, 52, 72, 255 });
                DrawRectangle((int)r.x, (int)r.y, (int)ts, (int)(ts * 0.18f), (Color){ 64, 72, 98, 255 });
                DrawRectangleLinesEx(r, 1, (Color){ 30, 34, 48, 255 });
                continue;
            }
            bool light = ((x + y) % 2) == 0;
            DrawRectangleRec(r, light ? (Color){ 206, 214, 226, 255 } : (Color){ 192, 201, 215, 255 });
            Vector2 mid = { r.x + ts / 2, r.y + ts / 2 };
            if (c == 'E') {
                DrawRectangleRec(r, (Color){ 70, 190, 120, 255 });
                DrawRectangleLinesEx((Rectangle){ r.x + ts * 0.12f, r.y + ts * 0.12f, ts * 0.76f, ts * 0.76f }, 2, (Color){ 210, 255, 220, 255 });
                DrawLineEx((Vector2){ mid.x - ts * 0.1f, mid.y + ts * 0.28f }, (Vector2){ mid.x - ts * 0.1f, mid.y - ts * 0.3f }, 2, WHITE);
                DrawTriangle((Vector2){ mid.x - ts * 0.1f, mid.y - ts * 0.3f }, (Vector2){ mid.x - ts * 0.1f, mid.y - ts * 0.08f },
                             (Vector2){ mid.x + ts * 0.22f, mid.y - ts * 0.19f }, (Color){ 255, 220, 90, 255 });
            } else if (c == 'C') {
                float bob = sinf(time * 3 + x + y) * ts * 0.04f;
                DrawCircleV((Vector2){ mid.x, mid.y + bob }, ts * 0.2f, COL_GOLD_DARK);
                DrawCircleV((Vector2){ mid.x, mid.y + bob - 1 }, ts * 0.17f, COL_GOLD);
                DrawCircleV((Vector2){ mid.x - ts * 0.05f, mid.y + bob - ts * 0.06f }, ts * 0.05f, (Color){ 255, 245, 200, 255 });
            } else if (c == 'L' || c == 'R' || c == 'S') {
                DrawRectangle((int)(mid.x - ts * 0.03f), (int)(mid.y - ts * 0.05f), (int)(ts * 0.06f + 1), (int)(ts * 0.38f), (Color){ 120, 85, 50, 255 });
                Rectangle board = { mid.x - ts * 0.3f, mid.y - ts * 0.38f, ts * 0.6f, ts * 0.36f };
                DrawRectangleRounded(board, 0.2f, 4, (Color){ 190, 140, 80, 255 });
                const char *txt = c == 'L' ? "L" : c == 'R' ? "R" : "S";
                UiTextCentered(txt, mid.x, board.y + board.height * 0.1f, ts * 0.3f, (Color){ 60, 40, 20, 255 });
            }
        }
    }

    // robots
    int bots = live ? play.bots : w->bots;
    for (int i = 0; i < bots; i++) {
        float fx, fy, ang;
        bool ghost = false;
        float alpha = 1;
        int colorIndex = i;
        if (live) {
            RBot *b = &play.bot[i];
            if (!b->born || !b->alive) continue;
            fx = (float)b->x;
            fy = (float)b->y;
            ang = b->dir * 90.0f;
            ghost = b->ghost;
            if (play.idx < play.count && play.ev[play.idx].bot == i) {
                const REvent *e = &play.ev[play.idx];
                float t = play.t;
                float ease = t * t * (3 - 2 * t);
                if (e->code == 'M') {
                    fx += DX[b->dir] * ease;
                    fy += DY[b->dir] * ease;
                } else if (e->code == 'L') {
                    ang -= 90 * ease;
                } else if (e->code == 'R') {
                    ang += 90 * ease;
                } else if (e->code == 'X') {
                    float bump = sinf(t * PI) * 0.22f;
                    fx += DX[b->dir] * bump;
                    fy += DY[b->dir] * bump;
                }
            }
            if (ghost) {
                alpha = 0.45f + 0.15f * sinf(time * 8);
                colorIndex = 0;
            }
        } else {
            fx = (float)w->bx[i];
            fy = (float)w->by[i];
            ang = w->bdir[i] * 90.0f;
        }
        Vector2 c = { ox + (fx + 0.5f) * ts, oy + (fy + 0.5f) * ts };
        Color color = ghost ? (Color){ 120, 240, 255, 255 } : (worlds[test].bots > 1 ? BOT_COLORS[colorIndex % 4] : COL_ROBOT);
        DrawRobotBody(c, ts, ang, color, alpha, ghost);
        if (ghost) UiTextCentered("HOLOGRAM", c.x, c.y - ts * 0.75f, fmaxf(12, ts * 0.26f), (Color){ 140, 255, 255, 255 });
        if (live && play.crashBot == i && play.idx >= play.count) {
            float k = fminf(1, (float)GetTime() - play.crashTime);
            DrawCircleV(c, ts * 0.45f * k, ColorAlpha((Color){ 235, 70, 70, 255 }, 0.35f));
            UiTextCentered("CRASH!", c.x, c.y - ts * 0.85f, fmaxf(14, ts * 0.34f), (Color){ 255, 90, 90, 255 });
        }
    }

    // celebrate once the replay of a winning run ends
    if (live && play.won && play.idx >= play.count && !play.celebrated) {
        play.celebrated = true;
        for (int i = 0; i < play.bots; i++) {
            if (play.bot[i].ghost) continue;
            ParticlesBurst(ox + (play.bot[i].x + 0.5f) * ts, oy + (play.bot[i].y + 0.5f) * ts, 40, 260);
        }
    }
}
