#include "heistview.h"
#include "ui.h"
#include "art.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HMAX     24
#define HMAXEV   1200
#define HMAXHEAP 32

enum { KIND_STREET, KIND_DOORS, KIND_ESTATE };

typedef struct {
    int  kind;
    int  n;                       // houses (street or estate) or doors
    int  loot[HMAX];
    int  number[HMAX], alarm[HMAX];   // estate houses
    char doors[HMAX + 1];
    int  tip;                     // -2 none, -1 NULL, else a house
    int  blockFirst, blockCount;  // blockCount 0 = no block
    int  trail[HMAX], trailN;
    int  chain[HMAX], chainN;
} HWorld;

typedef struct {
    char code;                    // G T R U O A F N K V C P L E(nd)
    int  a, b, c;
    char text[96];                // values for V / C / P, gadget name for U
} HEvent;

typedef struct {
    bool used, freed, leaked, isNew, isArray;
    int  size, line;
    char contents[96];
} HBlock;

static HWorld worlds[MAX_TESTS];
static int    worldCount;

static struct {
    int    test;
    HWorld w;                     // working copy: loot disappears as it's taken
    HEvent ev[HMAXEV];
    int    count, idx;
    float  t;
    int    from, to;              // thief slots for the current walk
    int    pos;                   // thief slot when standing still
    bool   won, crashed, celebrated, alarm;
    float  speed;
    HBlock heap[HMAXHEAP];
    bool   doorOpen[HMAX];
    char   gadget[16];
    int    gadgetDoor, noise;
    int    bag;
    bool   hasHeap;
} play = { .test = -1, .speed = 3 };

// ---------------------------------------------------------------- loading

static int ReadInts(const char **p, int *out, int max)
{
    int n = 0, v, used;
    while (n < max && sscanf(*p, "%d%n", &v, &used) == 1) {
        out[n++] = v;
        *p += used;
    }
    return n;
}

static void ParseWorld(const char *text, HWorld *w)
{
    memset(w, 0, sizeof(*w));
    w->tip = -2;
    const char *p = text;
    char key[16];
    int used;
    while (p && sscanf(p, "%15s%n", key, &used) == 1) {
        p += used;
        if (strcmp(key, "END") == 0) break;
        if (strcmp(key, "STREET") == 0) {
            int n = 0;
            sscanf(p, "%d%n", &n, &used);
            p += used;
            w->n = n > HMAX ? HMAX : n;
            ReadInts(&p, w->loot, w->n);
            w->kind = KIND_STREET;
        } else if (strcmp(key, "TIP") == 0) {
            sscanf(p, "%d%n", &w->tip, &used);
            p += used;
            if (w->tip < 0) w->tip = -1;
        } else if (strcmp(key, "BLOCK") == 0) {
            sscanf(p, "%d %d%n", &w->blockFirst, &w->blockCount, &used);
            p += used;
        } else if (strcmp(key, "DOORS") == 0) {
            sscanf(p, "%24s%n", w->doors, &used);
            p += used;
            w->n = (int)strlen(w->doors);
            w->kind = KIND_DOORS;
        } else if (strcmp(key, "ESTATE") == 0) {
            int n = 0;
            sscanf(p, "%d%n", &n, &used);
            p += used;
            w->n = n > HMAX ? HMAX : n;
            for (int i = 0; i < w->n; i++) {
                int v[3] = { 0 };
                ReadInts(&p, v, 3);
                w->number[i] = v[0];
                w->loot[i] = v[1];
                w->alarm[i] = v[2];
            }
            w->kind = KIND_ESTATE;
        } else if (strcmp(key, "TRAIL") == 0 || strcmp(key, "CHAIN") == 0) {
            bool trail = key[0] == 'T';
            int n = 0;
            sscanf(p, "%d%n", &n, &used);
            p += used;
            if (n > HMAX) n = HMAX;
            ReadInts(&p, trail ? w->trail : w->chain, n);
            if (trail) w->trailN = n; else w->chainN = n;
        }
    }
}

void HeistLoad(const TestCase *tests, int count)
{
    worldCount = count > MAX_TESTS ? MAX_TESTS : count;
    for (int i = 0; i < worldCount; i++) ParseWorld(tests[i].input, &worlds[i]);
    play.test = -1;
}

// slot 0 is the hideout, houses are 1..n, n + 1 is the NULL house, n + 2 is "off the street"
static int SlotOf(const char *arg, int n)
{
    if (strncmp(arg, "NULL", 4) == 0) return n + 1;
    if (strncmp(arg, "LOST", 4) == 0) return n + 2;
    int i = atoi(arg);
    return i < 0 ? 0 : i + 1;
}

static void ResetState(void)
{
    play.w = worlds[play.test];
    play.idx = 0;
    play.t = 0;
    play.pos = play.from = play.to = 0;
    play.celebrated = false;
    play.alarm = false;
    memset(play.heap, 0, sizeof(play.heap));
    memset(play.doorOpen, 0, sizeof(play.doorOpen));
    play.gadget[0] = '\0';
    play.gadgetDoor = -1;
    play.noise = 0;
    play.bag = 0;
}

void HeistPlay(int test, const char *events, bool won, bool crashed)
{
    if (test < 0 || test >= worldCount) return;
    play.test = test;
    play.won = won;
    play.crashed = crashed;
    play.count = 0;
    play.hasHeap = worlds[test].chainN > 0;
    int n = worlds[test].n;
    const char *p = events;
    while (p && *p && play.count < HMAXEV) {
        HEvent e = { 0 };
        char arg[16] = "";
        e.code = p[0];
        const char *rest = p + 1;
        const char *eol = strchr(p, '\n');
        int len = eol ? (int)(eol - rest) : (int)strlen(rest);
        switch (e.code) {
            case 'G': sscanf(rest, "%15s", arg); e.a = SlotOf(arg, n); break;
            case 'T': sscanf(rest, "%d %d", &e.a, &e.b); break;
            case 'R': if (sscanf(rest, "%15s %d", arg, &e.b) >= 1) e.a = strncmp(arg, "NULL", 4) == 0 ? -1 : atoi(arg); break;
            case 'U': sscanf(rest, "%d %15s", &e.a, e.text); break;
            case 'O': case 'K': case 'L': sscanf(rest, "%d", &e.a); break;
            case 'A': case 'F': case 'N': sscanf(rest, "%d %d %d", &e.a, &e.b, &e.c); play.hasHeap = true; break;
            case 'V': case 'C': case 'P': {
                int used = 0;
                sscanf(rest, "%d%n", &e.a, &used);
                int tl = len - used;
                while (tl > 0 && (rest[used + tl - 1] == '\r' || rest[used + tl - 1] == ' ')) tl--;   // Windows line endings
                if (tl > (int)sizeof(e.text) - 1) tl = (int)sizeof(e.text) - 1;
                if (tl > 0) { memcpy(e.text, rest + used, (size_t)tl); e.text[tl] = '\0'; }
                break;
            }
            case 'E': break;   // END
            default: e.code = 0; break;
        }
        if (e.code && e.code != 'E') play.ev[play.count++] = e;
        p = eol ? eol + 1 : NULL;
    }
    ResetState();
}

void HeistReplay(void) { if (play.test >= 0) ResetState(); }
void HeistSetSpeed(float eventsPerSecond) { play.speed = eventsPerSecond * 0.5f; }
bool HeistPlaying(void) { return play.test >= 0 && play.idx < play.count; }

// Both buffers are 96 bytes (HEvent.text and HBlock.contents)
static void CopyText(char *dst, const char *src)
{
    size_t n = strlen(src);
    if (n > 95) n = 95;
    memmove(dst, src, n);
    dst[n] = '\0';
}

static void Apply(const HEvent *e)
{
    HWorld *w = &play.w;
    switch (e->code) {
        case 'G':
            play.pos = e->a;
            if (e->a > w->n) play.alarm = true;
            break;
        case 'T':
            if (e->a >= 0 && e->a < w->n) { play.bag += w->loot[e->a]; w->loot[e->a] = 0; }
            break;
        case 'R':
            if (e->a < 0) { play.pos = w->n + 1; play.alarm = true; break; }
            if (e->a < w->n) {
                play.pos = e->a + 1;
                if (w->alarm[e->a]) play.alarm = true;
                else { play.bag += w->loot[e->a]; w->loot[e->a] = 0; }
            }
            break;
        case 'U':
            snprintf(play.gadget, sizeof(play.gadget), "%.*s", (int)sizeof(play.gadget) - 1, e->text);
            play.gadgetDoor = e->a;
            play.pos = e->a + 1;
            play.noise += strcmp(e->text, "pick") == 0 ? 1 : strcmp(e->text, "drill") == 0 ? 3 : strcmp(e->text, "blast") == 0 ? 10 : 0;
            break;
        case 'O':
            if (e->a >= 0 && e->a < HMAX) play.doorOpen[e->a] = true;
            break;
        case 'A': case 'N':
            if (e->a >= 0 && e->a < HMAXHEAP) {
                HBlock *b = &play.heap[e->a];
                *b = (HBlock){ true, false, false, e->code == 'N', e->code == 'N' && e->c, e->b, e->code == 'A' ? e->c : 0, "" };
            }
            break;
        case 'F': case 'K':
            if (e->a >= 0 && e->a < HMAXHEAP) play.heap[e->a].freed = true;
            break;
        case 'L':
            if (e->a >= 0 && e->a < HMAXHEAP) play.heap[e->a].leaked = true;
            break;
        case 'V': case 'C':
            if (e->a >= 0 && e->a < HMAXHEAP) CopyText(play.heap[e->a].contents, e->text);
            break;
        case 'P':
            for (int i = HMAXHEAP - 1; i >= 0; i--) {
                if (play.heap[i].used && !play.heap[i].freed) {
                    CopyText(play.heap[i].contents, e->text);
                    break;
                }
            }
            break;
    }
}

static void Advance(float dt)
{
    if (play.test < 0 || play.idx >= play.count) return;
    if (play.t == 0) {
        const HEvent *e = &play.ev[play.idx];
        play.from = play.pos;
        if (e->code == 'G') play.to = e->a;
        else if (e->code == 'R') play.to = e->a < 0 ? play.w.n + 1 : e->a + 1;
        else if (e->code == 'U') play.to = e->a + 1;
        else play.to = play.pos;
    }
    play.t += dt * play.speed * (play.ev[play.idx].code == 'G' || play.ev[play.idx].code == 'R' ? 1.0f : 2.0f);
    if (play.t >= 1) {
        Apply(&play.ev[play.idx]);
        play.idx++;
        play.t = 0;
        if (play.idx >= play.count && (play.crashed || (!play.won && play.alarm))) play.alarm = true;
    }
}

// ---------------------------------------------------------------- drawing

static void DrawThief(Vector2 feet, float s, bool alarm)
{
    float time = (float)GetTime();
    float bob = HeistPlaying() ? fabsf(sinf(time * 10)) * s * 0.05f : 0;
    Vector2 c = { feet.x, feet.y - s * 0.45f - bob };
    DrawEllipse((int)feet.x, (int)feet.y, s * 0.22f, s * 0.06f, ColorAlpha(BLACK, 0.3f));
    // loot bag
    DrawCircleV((Vector2){ c.x + s * 0.2f, c.y + s * 0.12f }, s * 0.13f, (Color){ 170, 140, 90, 255 });
    UiTextCentered("$", c.x + s * 0.2f, c.y + s * 0.03f, s * 0.16f, (Color){ 90, 70, 40, 255 });
    DrawRectangleRounded((Rectangle){ c.x - s * 0.16f, c.y - s * 0.05f, s * 0.32f, s * 0.42f }, 0.5f, 6, (Color){ 40, 42, 55, 255 });
    DrawCircleV((Vector2){ c.x, c.y - s * 0.18f }, s * 0.16f, (Color){ 235, 200, 170, 255 });
    DrawRectangleRounded((Rectangle){ c.x - s * 0.17f, c.y - s * 0.24f, s * 0.34f, s * 0.1f }, 0.6f, 4, (Color){ 30, 30, 40, 255 });
    DrawCircleV((Vector2){ c.x - s * 0.06f, c.y - s * 0.19f }, s * 0.025f, WHITE);
    DrawCircleV((Vector2){ c.x + s * 0.06f, c.y - s * 0.19f }, s * 0.025f, WHITE);
    DrawRectangleRounded((Rectangle){ c.x - s * 0.17f, c.y - s * 0.36f, s * 0.34f, s * 0.12f }, 0.6f, 4, (Color){ 40, 42, 55, 255 });
    if (alarm) UiTextCentered("!", c.x, c.y - s * 0.75f, s * 0.4f, (Color){ 255, 90, 90, 255 });
}

static void DrawHouse(Rectangle body, Color wall, Color roof, const char *top, const char *mid, Color midColor, bool dim)
{
    Vector2 a = { body.x - body.width * 0.08f, body.y }, b = { body.x + body.width * 1.08f, body.y };
    Vector2 apex = { body.x + body.width / 2, body.y - body.height * 0.42f };
    DrawTriangle(apex, a, b, roof);
    DrawTriangle(apex, b, a, roof);
    DrawRectangleRec(body, wall);
    Rectangle door = { body.x + body.width * 0.38f, body.y + body.height * 0.55f, body.width * 0.24f, body.height * 0.45f };
    DrawRectangleRec(door, ColorBrightness(wall, -0.45f));
    if (top) UiTextCentered(top, apex.x, body.y - body.height * 0.25f, fmaxf(11, body.width * 0.17f), (Color){ 255, 255, 255, 220 });
    if (mid) UiTextCentered(mid, body.x + body.width / 2, body.y + body.height * 0.12f, fmaxf(12, body.width * 0.22f), midColor);
    if (dim) DrawRectangleRec((Rectangle){ apex.x - body.width * 0.6f, apex.y, body.width * 1.2f, body.height * 1.45f }, ColorAlpha(BLACK, 0.35f));
}

static void Arrow(Vector2 from, Vector2 to, Color color, float thick)
{
    DrawLineEx(from, to, thick, color);
    float a = atan2f(to.y - from.y, to.x - from.x);
    Vector2 l = { to.x - cosf(a - 0.45f) * 10, to.y - sinf(a - 0.45f) * 10 };
    Vector2 r = { to.x - cosf(a + 0.45f) * 10, to.y - sinf(a + 0.45f) * 10 };
    DrawTriangle(to, l, r, color);
    DrawTriangle(to, r, l, color);
}

// A curved arrow over the houses, for next pointers
static void ArcArrow(Vector2 a, Vector2 b, float lift, Color color)
{
    Vector2 prev = a;
    for (int i = 1; i <= 16; i++) {
        float t = i / 16.0f;
        Vector2 p = { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t - sinf(t * PI) * lift };
        if (i == 16) Arrow(prev, p, color, 2);
        else DrawLineEx(prev, p, 2, color);
        prev = p;
    }
}

void HeistDraw(Rectangle area, int test)
{
    if (test < 0 || test >= worldCount) return;
    bool live = (play.test == test);
    if (live) Advance(GetFrameTime());
    const HWorld *w = live ? &play.w : &worlds[test];
    float time = (float)GetTime();
    bool alarm = live && play.alarm;
    bool heapShown = live ? play.hasHeap : (w->chainN > 0);

    // night sky and street
    DrawRectangleGradientV((int)area.x, (int)area.y, (int)area.width, (int)area.height, (Color){ 22, 26, 52, 255 }, (Color){ 44, 40, 70, 255 });
    for (int i = 0; i < 18; i++) {
        float sx = area.x + fmodf(i * 97.0f, area.width), sy = area.y + fmodf(i * 53.0f, area.height * 0.3f);
        DrawCircleV((Vector2){ sx, sy }, 1.2f + (i % 3) * 0.5f, ColorAlpha(WHITE, 0.4f + 0.3f * sinf(time * 2 + i)));
    }

    Rectangle heapArea = { area.x + 10, area.y + 8, area.width - 20, heapShown ? area.height * 0.3f : 0 };
    float groundY = area.y + area.height - 34;
    DrawRectangle((int)area.x, (int)groundY, (int)area.width, 34, (Color){ 52, 54, 66, 255 });
    DrawRectangle((int)area.x, (int)groundY, (int)area.width, 4, (Color){ 90, 92, 110, 255 });

    int slots = w->n + 2;
    float slotW = area.width / slots;
    float hw = fminf(slotW * 0.84f, 110);
    float room = groundY - (heapArea.y + heapArea.height) - 80;   // space for the roofs, the tip arrow and labels
    float hh = fminf(fmaxf(hw * 1.15f, 64), room * 0.72f);
    if (hh < 24) hh = 24;
    float houseY = groundY - hh;

    // a moon and a far-off skyline behind the street
    DrawCircleV((Vector2){ area.x + area.width - 50, area.y + 46 + heapArea.height }, 26, (Color){ 240, 236, 200, 40 });
    DrawCircleV((Vector2){ area.x + area.width - 50, area.y + 46 + heapArea.height }, 18, (Color){ 240, 236, 200, 200 });
    for (int i = 0; i < 16; i++) {
        float bw = area.width / 16;
        float bh = hh * 0.6f + fmodf(i * 37.0f, hh * 0.9f + 1);
        DrawRectangle((int)(area.x + i * bw), (int)(groundY - bh), (int)(bw - 3), (int)bh, (Color){ 36, 38, 66, 255 });
        if (i % 3 == 0) DrawRectangle((int)(area.x + i * bw + bw * 0.4f), (int)(groundY - bh + 8), 4, 4, (Color){ 255, 220, 120, 120 });
    }
    #define SLOT_X(s) (area.x + slotW * (s) + slotW / 2)

    // hideout
    DrawHouse((Rectangle){ SLOT_X(0) - hw / 2, houseY, hw, hh }, (Color){ 70, 80, 70, 255 }, (Color){ 40, 50, 40, 255 },
              "hideout", "HQ", (Color){ 200, 255, 200, 255 }, false);
    // NULL house
    {
        Rectangle b = { SLOT_X(w->n + 1) - hw / 2, houseY, hw, hh };
        bool flash = alarm && fmodf(time, 0.5f) < 0.25f;
        DrawHouse(b, flash ? (Color){ 200, 50, 50, 255 } : (Color){ 80, 40, 40, 255 }, (Color){ 120, 30, 30, 255 },
                  "NULL", "0x0", (Color){ 255, 160, 160, 255 }, false);
        DrawCircleV((Vector2){ b.x + b.width / 2, houseY - hh * 0.5f }, hw * 0.08f, flash ? (Color){ 255, 80, 80, 255 } : (Color){ 120, 50, 50, 255 });
    }

    for (int i = 0; i < w->n; i++) {
        Rectangle b = { SLOT_X(i + 1) - hw / 2, houseY, hw, hh };
        char top[24], mid[24];
        Color wall = (Color){ 150, 120, 95, 255 }, roof = (Color){ 120, 60, 55, 255 };
        bool dim = false;
        Color midColor = COL_GOLD;
        if (w->kind == KIND_DOORS) {
            bool open = live && play.doorOpen[i];
            Rectangle d = { b.x + hw * 0.1f, groundY - hh * 1.2f, hw * 0.8f, hh * 1.2f };
            DrawRectangleRec(d, open ? (Color){ 20, 20, 30, 255 } : (Color){ 110, 115, 130, 255 });
            DrawRectangleLinesEx(d, 3, (Color){ 70, 72, 90, 255 });
            if (!open) {
                char lock = w->doors[i];
                const char *name = lock == 'K' ? "KEYHOLE" : lock == 'S' ? "STEEL" : "WALL";
                UiTextCentered(TextFormat("%c", lock), d.x + d.width / 2, d.y + d.height * 0.25f, hw * 0.35f, (Color){ 255, 230, 160, 255 });
                UiTextCentered(name, d.x + d.width / 2, d.y + d.height * 0.65f, fmaxf(10, hw * 0.13f), (Color){ 230, 230, 240, 255 });
            } else {
                UiTextCentered("OPEN", d.x + d.width / 2, d.y + d.height * 0.4f, fmaxf(11, hw * 0.16f), (Color){ 120, 255, 160, 255 });
            }
            UiTextCentered(TextFormat("door %d", i + 1), d.x + d.width / 2, groundY + 8, 13, (Color){ 200, 200, 215, 255 });
            continue;
        }
        if (w->kind == KIND_ESTATE) {
            snprintf(top, sizeof(top), "no. %d", w->number[i]);
            snprintf(mid, sizeof(mid), "$%d", w->loot[i]);
            if (w->alarm[i]) roof = (Color){ 170, 40, 40, 255 };
        } else {
            snprintf(top, sizeof(top), "%d", 1000 + 4 * i);
            snprintf(mid, sizeof(mid), "$%d", w->loot[i]);
            if (w->blockCount > 0 && (i < w->blockFirst || i >= w->blockFirst + w->blockCount)) dim = true;
        }
        if (w->loot[i] == 0) midColor = (Color){ 200, 200, 210, 160 };
        DrawHouse(b, wall, roof, top, mid, midColor, dim);
        if (w->kind == KIND_ESTATE && w->alarm[i]) {
            float pulse = 0.6f + 0.4f * sinf(time * 6);
            DrawCircleV((Vector2){ b.x + b.width * 0.85f, b.y + hh * 0.2f }, hw * 0.08f, ColorAlpha((Color){ 255, 70, 70, 255 }, pulse));
        }
        if (dim) UiTextCentered("guard", b.x + b.width / 2, groundY + 8, 12, (Color){ 255, 150, 150, 200 });
        else if (w->kind == KIND_STREET) UiTextCentered(TextFormat("[%d]", i), b.x + b.width / 2, groundY + 8, 13, (Color){ 200, 200, 215, 255 });
    }

    // the tip pointer
    if (w->tip != -2) {
        int slot = w->tip == -1 ? w->n + 1 : w->tip + 1;
        Vector2 to = { SLOT_X(slot), houseY - hh * 0.5f };
        Vector2 from = { to.x, to.y - 46 };
        Arrow(from, to, (Color){ 255, 220, 90, 255 }, 3);
        UiTextCentered("tip", from.x, from.y - 20, 18, (Color){ 255, 220, 90, 255 });
    }
    // the unguarded block
    if (w->blockCount > 0) {
        float x1 = SLOT_X(w->blockFirst + 1) - hw / 2, x2 = SLOT_X(w->blockFirst + w->blockCount) + hw / 2;
        float y = houseY - hh * 0.5f - 18;
        DrawLineEx((Vector2){ x1, y }, (Vector2){ x2, y }, 2, (Color){ 120, 220, 255, 255 });
        DrawLineEx((Vector2){ x1, y }, (Vector2){ x1, y + 8 }, 2, (Color){ 120, 220, 255, 255 });
        DrawLineEx((Vector2){ x2, y }, (Vector2){ x2, y + 8 }, 2, (Color){ 120, 220, 255, 255 });
        UiText("first", x1, y - 20, 16, (Color){ 120, 220, 255, 255 });
        const char *endLabel = "first + count";
        UiText(endLabel, x2 - UiTextWidth(endLabel, 16), y - 20, 16, (Color){ 120, 220, 255, 255 });
    }
    // the trail of next pointers
    if (w->kind == KIND_ESTATE && w->trailN > 0) {
        float y = houseY - hh * 0.45f;
        for (int k = 0; k + 1 < w->trailN; k++)
            ArcArrow((Vector2){ SLOT_X(w->trail[k] + 1), y }, (Vector2){ SLOT_X(w->trail[k + 1] + 1), y }, 28 + 6 * (k % 3),
                     (Color){ 255, 220, 90, 230 });
        int last = w->trail[w->trailN - 1];
        UiTextCentered("next = NULL", SLOT_X(last + 1), y - 40, 14, (Color){ 255, 160, 160, 255 });
        UiTextCentered("first_clue", SLOT_X(w->trail[0] + 1), houseY + hh + 10, 13, (Color){ 255, 220, 90, 255 });
    }

    // the heap: memory from malloc and new
    if (heapShown) {
        DrawRectangleRounded(heapArea, 0.08f, 6, (Color){ 30, 34, 60, 230 });
        UiText("THE HEAP  (malloc / new)", heapArea.x + 10, heapArea.y + 6, 15, (Color){ 170, 180, 230, 255 });
        float bx = heapArea.x + 10, by = heapArea.y + 28, bh = heapArea.height - 36;
        int shown = 0;
        for (int i = 0; i < HMAXHEAP; i++) {
            if (!live) break;
            HBlock *blk = &play.heap[i];
            if (!blk->used) continue;
            float bw = fminf(170, (heapArea.width - 20) / 6.0f);
            Rectangle r = { bx + shown * (bw + 8), by, bw, bh };
            if (r.x + r.width > heapArea.x + heapArea.width) break;
            Color fill = blk->leaked ? (Color){ 150, 50, 55, 255 } : blk->freed ? (Color){ 60, 62, 78, 255 } : (Color){ 60, 110, 170, 255 };
            DrawRectangleRounded(r, 0.12f, 6, fill);
            const char *what = blk->isNew ? (blk->isArray ? "new[]" : "new") : "malloc";
            UiText(TextFormat("%s  %d B", what, blk->size), r.x + 8, r.y + 6, 14, WHITE);
            if (blk->line > 0) UiText(TextFormat("line %d", blk->line), r.x + 8, r.y + 24, 13, (Color){ 210, 215, 240, 255 });
            if (blk->contents[0]) UiCode(blk->contents, r.x + 6, r.y + 42, 13, (Color){ 255, 230, 160, 255 }, r.width - 12);
            const char *state = blk->leaked ? "LEAKED!" : blk->freed ? "freed" : "in use";
            UiText(state, r.x + 8, r.y + r.height - 20, 14, blk->leaked ? (Color){ 255, 200, 200, 255 } : (Color){ 200, 255, 210, 255 });
            shown++;
        }
        if (!live && w->chainN > 0)
            UiText("Safehouse nodes appear here when your program runs.", bx, by + 4, 15, (Color){ 180, 185, 210, 255 });
        if (live && w->chainN > 0) {
            // chain nodes are the first allocations; point them at their houses
            for (int k = 0; k < w->chainN && k < shown; k++) {
                float bw = fminf(170, (heapArea.width - 20) / 6.0f);
                Vector2 from = { bx + k * (bw + 8) + bw / 2, by + bh };
                Vector2 to = { SLOT_X(w->chain[k] + 1), houseY - hh * 0.45f };
                if (!play.heap[k].freed) Arrow(from, to, ColorAlpha((Color){ 255, 220, 90, 255 }, 0.7f), 2);
            }
        }
    }

    // noise meter for the gadget doors
    if (w->kind == KIND_DOORS && live) {
        UiText(TextFormat("noise: %d", play.noise), area.x + area.width - 120, area.y + 12, 18, (Color){ 255, 200, 120, 255 });
        if (play.gadget[0] && play.idx > 0 && play.idx <= play.count) {
            float x = SLOT_X(play.gadgetDoor + 1);
            UiTextCentered(play.gadget, x, groundY - hh * 1.2f - 30, 18, (Color){ 140, 255, 200, 255 });
        }
    }

    // the thief
    float tx;
    if (live) {
        float t = play.t;
        float ease = t * t * (3 - 2 * t);
        int to = play.to > w->n + 1 ? w->n + 1 : play.to;
        int from = play.from > w->n + 1 ? w->n + 1 : play.from;
        tx = (play.idx < play.count) ? SLOT_X(from) + (SLOT_X(to) - SLOT_X(from)) * ease : SLOT_X(play.pos > w->n + 1 ? w->n + 1 : play.pos);
    } else {
        tx = SLOT_X(0);
    }
    DrawThief((Vector2){ tx, groundY + 20 }, fmaxf(40, fminf(hh, 90)), alarm);

    if (live) UiText(TextFormat("bag: $%d", play.bag), area.x + 12, groundY + 8, 16, COL_GOLD);

    if (alarm && fmodf(time, 0.6f) < 0.3f) {
        DrawRectangleLinesEx(area, 6, (Color){ 255, 60, 60, 255 });
        UiTextCentered("ALARM!", area.x + area.width / 2, area.y + area.height * 0.42f, 40, (Color){ 255, 80, 80, 255 });
    }
    if (live && play.won && play.idx >= play.count && !play.celebrated) {
        play.celebrated = true;
        ParticlesBurst(SLOT_X(0), houseY, 50, 260);
    }
    #undef SLOT_X
}
