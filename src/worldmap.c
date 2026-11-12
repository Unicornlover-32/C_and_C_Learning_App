// The world map: two islands, C and C++, each with two regions of levels. Every level
// is a node on a winding path with its stars, every region ends in a boss, and each
// region has a factory where its Factory Contracts are.
#include "app.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAP_W 1600.0f
#define MAP_H 900.0f
#define MAX_NODES 48

typedef struct {
    Vector2 p;          // map coordinates
    int     tut;        // level, or -1 for the boss
    int     mission;    // the boss's mission
    int     region;
} Node;

typedef struct {
    const char *name;
    const char *theme;
    Rectangle   area;       // map coordinates
    int         rows;       // 1: one wavy row left to right; 2: there along the bottom and back along the top
    bool        topFirst;   // two rows: start along the top instead
    Color       tint;
} Region;

static const Region REGIONS[MAX_REGIONS] = {
    { "Robot Rescue Bay", "Intro to C",            {  70, 470, 660, 360 }, 2, false, {  90, 170, 110, 255 } },
    { "Memory City",      "Pointers and Memory",   {  70,  90, 660, 330 }, 1, false, { 120, 110, 170, 255 } },
    { "Class Town",       "C++ and Classes",       { 870,  90, 660, 360 }, 2, true,  { 170, 110, 150, 255 } },
    { "STL Harbour",      "Templates and the STL", { 870, 510, 660, 330 }, 1, false, {  70, 150, 170, 255 } },
};

static Node nodes[MAX_NODES];
static int  nodeCount;
static int  regionNodeStart[MAX_REGIONS + 1];
static float scale, ox, oy;
static int  hover = -1;

static Vector2 M(Vector2 p) { return (Vector2){ ox + p.x * scale, oy + p.y * scale }; }
static float   S(float v)   { return v * scale; }

static void Layout(void)
{
    nodeCount = 0;
    for (int r = 0; r < RegionCount() && r < MAX_REGIONS; r++) {
        regionNodeStart[r] = nodeCount;
        const Region *rg = &REGIONS[r];
        int levels = RegionTutCount(r);
        int boss = BossOf(r);
        int n = levels + (boss >= 0 ? 1 : 0);
        Rectangle a = rg->area;
        float margin = 80;
        for (int i = 0; i < n && nodeCount < MAX_NODES; i++) {
            Vector2 p;
            if (rg->rows == 1) {
                float t = n > 1 ? i / (float)(n - 1) : 0.5f;
                p.x = a.x + margin + t * (a.width - 2 * margin);
                p.y = a.y + a.height * 0.55f + sinf(t * PI * 2.2f) * a.height * 0.16f;
            } else {
                int first = (n + 1) / 2;           // nodes on the first row
                bool firstRow = i < first;
                int k = firstRow ? i : i - first;
                int rowN = firstRow ? first : n - first;
                float t = rowN > 1 ? k / (float)(rowN - 1) : 0.5f;
                if (!firstRow) t = 1 - t;           // the second row runs back the other way
                if (!firstRow && rowN < first) t = t * (rowN - 1) / (float)(first - 1);   // keep the spacing even
                float yTop = a.y + a.height * 0.36f, yBottom = a.y + a.height * 0.76f;
                bool onTop = rg->topFirst ? firstRow : !firstRow;
                p.x = a.x + margin + t * (a.width - 2 * margin);
                p.y = (onTop ? yTop : yBottom) + sinf(t * PI * 3) * 10;
            }
            bool isBoss = (boss >= 0 && i == n - 1);
            nodes[nodeCount++] = (Node){ p, isBoss ? -1 : RegionFirstTut(r) + i, isBoss ? boss : -1, r };
        }
    }
    regionNodeStart[RegionCount()] = nodeCount;
}

int NextLevel(void)
{
    for (int t = 0; t < TutorialCount(); t++) if (!LevelCleared(t)) return t;
    return -1;
}

void OpenMap(void)
{
    app.screen = SCREEN_MAP;
}

static bool NodeCleared(const Node *n)
{
    return n->tut >= 0 ? LevelCleared(n->tut) : MissionCleared(n->mission);
}

static unsigned char NodeStars(const Node *n)
{
    int m = n->tut >= 0 ? MainMissionOf(n->tut) : n->mission;
    return m >= 0 ? prog.stars[m] : 0;
}

// ---------------------------------------------------------------- drawing

static void Palm(Vector2 p, float s)
{
    Vector2 b = M(p);
    float k = S(s);
    DrawLineEx(b, (Vector2){ b.x + k * 0.15f, b.y - k }, k * 0.12f, (Color){ 120, 85, 50, 255 });
    Vector2 top = { b.x + k * 0.15f, b.y - k };
    for (int i = 0; i < 5; i++) {
        float a = -PI / 2 + (i - 2) * 0.6f;
        DrawLineEx(top, (Vector2){ top.x + cosf(a) * k * 0.55f, top.y + sinf(a) * k * 0.3f + k * 0.15f }, k * 0.14f, (Color){ 60, 150, 80, 255 });
    }
}

static void TinyHouse(Vector2 p, float s, Color wall)
{
    Vector2 b = M(p);
    float k = S(s);
    DrawRectangle((int)(b.x - k / 2), (int)(b.y - k * 0.6f), (int)k, (int)(k * 0.6f), wall);
    Vector2 l = { b.x - k * 0.6f, b.y - k * 0.6f }, r = { b.x + k * 0.6f, b.y - k * 0.6f }, t = { b.x, b.y - k * 1.05f };
    DrawTriangle(t, l, r, ColorBrightness(wall, -0.35f));
    DrawTriangle(t, r, l, ColorBrightness(wall, -0.35f));
}

static void DrawIsland(Rectangle island)
{
    Vector2 a = M((Vector2){ island.x, island.y });
    Rectangle r = { a.x, a.y, S(island.width), S(island.height) };
    DrawRectangleRounded((Rectangle){ r.x + S(10), r.y + S(16), r.width, r.height }, 0.3f, 16, ColorAlpha(BLACK, 0.25f));
    // a coastline with a few bumps
    for (int i = 0; i < 7; i++) {
        float t = i / 6.0f;
        DrawCircleV((Vector2){ r.x + r.width * t, r.y + S(10) + (i % 2) * S(14) }, S(46), (Color){ 230, 210, 160, 255 });
        DrawCircleV((Vector2){ r.x + r.width * t, r.y + r.height - S(10) - (i % 2) * S(14) }, S(46), (Color){ 230, 210, 160, 255 });
    }
    DrawRectangleRounded(r, 0.3f, 16, (Color){ 230, 210, 160, 255 });
    DrawRectangleRounded((Rectangle){ r.x + S(18), r.y + S(18), r.width - S(36), r.height - S(36) }, 0.28f, 16, (Color){ 112, 170, 96, 255 });
}

static void DrawRegion(int r)
{
    const Region *rg = &REGIONS[r];
    Vector2 a = M((Vector2){ rg->area.x, rg->area.y });
    Rectangle box = { a.x, a.y, S(rg->area.width), S(rg->area.height) };
    DrawRectangleRounded(box, 0.12f, 12, ColorAlpha(rg->tint, 0.55f));
    DrawRectangleRoundedLinesEx(box, 0.12f, 12, 2, ColorAlpha(WHITE, 0.25f));

    // decorations to match each region's game
    if (r == 1) {
        for (int i = 0; i < 9; i++)
            TinyHouse((Vector2){ rg->area.x + 40 + i * 72, rg->area.y + rg->area.height - 18 }, 26,
                      (i % 3 == 0) ? (Color){ 180, 150, 120, 255 } : (i % 3 == 1) ? (Color){ 150, 130, 170, 255 } : (Color){ 170, 120, 110, 255 });
    } else if (r == 3) {
        for (int i = 0; i < 6; i++) {
            Vector2 c = M((Vector2){ rg->area.x + 60 + i * 110, rg->area.y + rg->area.height - 26 });
            DrawRectangle((int)(c.x - S(12)), (int)(c.y - S(12)), (int)S(24), (int)S(24), (Color){ 176, 128, 76, 220 });
        }
    } else if (r == 0) {
        Palm((Vector2){ rg->area.x + 20, rg->area.y + rg->area.height + 10 }, 50);
        Palm((Vector2){ rg->area.x + rg->area.width - 10, rg->area.y + rg->area.height + 14 }, 44);
    } else if (r == 2) {
        for (int i = 0; i < 4; i++) {
            Vector2 c = M((Vector2){ rg->area.x + 90 + i * 160, rg->area.y + 30 });
            DrawRectangle((int)(c.x - S(8)), (int)(c.y - S(10)), (int)S(16), (int)S(26), (Color){ 140, 100, 140, 200 });
            DrawTriangle((Vector2){ c.x, c.y - S(24) }, (Vector2){ c.x - S(12), c.y - S(10) }, (Vector2){ c.x + S(12), c.y - S(10) },
                         (Color){ 110, 70, 120, 220 });
        }
    }
}

// The region's name, then its topic in a lighter tag beside it (drawn after the paths, so it sits on top)
static void DrawRegionBanner(int r)
{
    const Region *rg = &REGIONS[r];
    Vector2 a = M((Vector2){ rg->area.x, rg->area.y });
    Rectangle box = { a.x, a.y, S(rg->area.width), S(rg->area.height) };
    float fs = fmaxf(15, S(24));
    const char *label = rg->name;
    float tw = UiTextWidth(label, fs);
    float ts = fmaxf(12, S(16));
    float themeW = UiTextWidth(rg->theme, ts) + S(20);
    Rectangle banner = { box.x + S(14), box.y - S(16), tw + S(28), fs + S(12) };
    Rectangle tag = { banner.x + banner.width - S(6), banner.y + S(4), themeW + S(6), banner.height - S(8) };
    DrawRectangleRounded(tag, 0.4f, 6, (Color){ 235, 228, 205, 240 });
    UiText(rg->theme, tag.x + S(14), tag.y + (tag.height - ts) / 2, ts, (Color){ 50, 54, 70, 255 });
    DrawRectangleRounded(banner, 0.4f, 6, (Color){ 40, 44, 60, 245 });
    UiText(label, banner.x + S(14), banner.y + S(6), fs, WHITE);
}

static void DrawPath(int from, int to)
{
    for (int i = from; i + 1 < to; i++) {
        Vector2 a = M(nodes[i].p), b = M(nodes[i + 1].p);
        bool done = NodeCleared(&nodes[i]) && NodeCleared(&nodes[i + 1]);
        float len = hypotf(b.x - a.x, b.y - a.y);
        int dots = (int)(len / S(16));
        for (int d = 1; d < dots; d++) {
            float t = d / (float)dots;
            DrawCircleV((Vector2){ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t }, S(4),
                        done ? COL_GOLD : (Color){ 245, 235, 205, 230 });
        }
    }
}

// A dashed route between two map points, bending in the middle (the ferry between islands)
static void DrawRoute(Vector2 a, Vector2 b, float bend, Color color)
{
    Vector2 A = M(a), B = M(b);
    Vector2 mid = { (A.x + B.x) / 2, (A.y + B.y) / 2 - S(bend) };
    for (int i = 0; i < 24; i += 2) {
        float t0 = i / 24.0f, t1 = (i + 1) / 24.0f;
        Vector2 p0 = { (1 - t0) * (1 - t0) * A.x + 2 * (1 - t0) * t0 * mid.x + t0 * t0 * B.x,
                       (1 - t0) * (1 - t0) * A.y + 2 * (1 - t0) * t0 * mid.y + t0 * t0 * B.y };
        Vector2 p1 = { (1 - t1) * (1 - t1) * A.x + 2 * (1 - t1) * t1 * mid.x + t1 * t1 * B.x,
                       (1 - t1) * (1 - t1) * A.y + 2 * (1 - t1) * t1 * mid.y + t1 * t1 * B.y };
        DrawLineEx(p0, p1, S(4), color);
    }
}

static void DrawNode(int i, bool isNext)
{
    const Node *n = &nodes[i];
    Vector2 c = M(n->p);
    bool boss = n->tut < 0;
    bool cleared = NodeCleared(n);
    float r = S(boss ? 38 : 28) * (hover == i ? 1.12f : 1.0f);
    Color tint = REGIONS[n->region].tint;
    float time = (float)GetTime();

    if (isNext) {
        float pulse = fmodf(time, 1.4f) / 1.4f;
        DrawCircleV(c, r + S(10) + pulse * S(16), ColorAlpha(WHITE, 0.35f * (1 - pulse)));
    }
    DrawCircleV((Vector2){ c.x, c.y + S(4) }, r, ColorAlpha(BLACK, 0.3f));
    Color fill;
    if (boss) fill = cleared ? COL_GOLD : (Color){ 170, 50, 60, 255 };
    else if (cleared) fill = COL_GOLD;
    else if (prog.rushStars[n->tut] > 0) fill = ColorBrightness(tint, 0.25f);
    else fill = (Color){ 240, 240, 245, 255 };
    DrawCircleV(c, r, fill);
    DrawRing(c, r - S(3), r, 0, 360, 32, boss ? (Color){ 90, 20, 30, 255 } : ColorBrightness(tint, -0.3f));

    if (boss) {
        IconCrown((Vector2){ c.x, c.y - S(2) }, r * 1.1f, cleared ? (Color){ 120, 80, 20, 255 } : COL_GOLD);
        UiTextCentered("BOSS", c.x, c.y + r + S(4), fmaxf(12, S(16)), (Color){ 255, 230, 230, 255 });
    } else {
        Color num = cleared ? (Color){ 90, 60, 10, 255 } : (Color){ 40, 44, 60, 255 };
        UiTextCentered(TextFormat("%d", n->tut + 1), c.x, c.y - S(13), fmaxf(14, S(26)), num);
        // every level is its own 3D game; the badge shows an optional side quest, if it has one
        int list[8], count = LevelMissions(n->tut, list);
        for (int i = 0; i < count; i++) {
            const MissionInfo *side = GetMission(list[i]);
            if (side->role != ROLE_SIDE) continue;
            Vector2 badge = { c.x + r * 0.8f, c.y - r * 0.8f };
            DrawCircleV(badge, S(13), (Color){ 30, 32, 42, 255 });
            DrawModeIcon(side->mode, badge, S(20));
            break;
        }
    }
    DrawStarRow(c.x, c.y + r + S(boss ? 26 : 12), fmaxf(4, S(7)), NodeStars(n), -1);
}

static void DrawTooltip(int i)
{
    const Node *n = &nodes[i];
    Vector2 c = M(n->p);
    const char *title;
    const char *line;
    if (n->tut >= 0) {
        title = TextFormat("%d. %s", n->tut + 1, GetTutorial(n->tut)->title);
        int main = MainMissionOf(n->tut);
        line = main >= 0 ? TextFormat("%s: %s", ModeName(GetMission(main)->mode), GetMission(main)->title) : "";
    } else {
        title = GetMission(n->mission)->title;
        line = TextFormat("%s  -  best after the rest of %s", ModeName(GetMission(n->mission)->mode), REGIONS[n->region].name);
    }
    char first[160];
    snprintf(first, sizeof(first), "%s", title);
    char second[200];
    snprintf(second, sizeof(second), "%s", line);
    float w = fmaxf(UiTextWidth(first, 20), UiTextWidth(second, 16)) + 32;
    float x = c.x - w / 2, y = c.y - S(60) - 70;
    if (x < 10) x = 10;
    if (x + w > SCREEN_W - 10) x = SCREEN_W - 10 - w;
    if (y < 72) y = c.y + S(60);
    DrawRectangleRounded((Rectangle){ x, y, w, 64 }, 0.2f, 8, (Color){ 22, 24, 31, 240 });
    UiText(first, x + 16, y + 10, 20, COL_TEXT);
    UiText(second, x + 16, y + 38, 16, COL_MUTED);
}

void ScreenMap(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { OpenMenu(); return; }
    Layout();

    float top = 66;
    scale = fminf(SCREEN_W / MAP_W, (SCREEN_H - top) / MAP_H);
    ox = (SCREEN_W - MAP_W * scale) / 2;
    oy = top + (SCREEN_H - top - MAP_H * scale) / 2;
    float time = (float)GetTime();

    // the sea
    DrawRectangleGradientV(0, (int)top, SCREEN_W, SCREEN_H - (int)top, (Color){ 40, 110, 170, 255 }, (Color){ 22, 66, 120, 255 });
    for (int i = 0; i < 40; i++) {
        float wx = fmodf(i * 173.0f + time * 12 * (1 + i % 3), SCREEN_W + 80) - 40;
        float wy = top + fmodf(i * 97.0f, SCREEN_H - top);
        DrawLineEx((Vector2){ wx, wy }, (Vector2){ wx + 14, wy - 4 }, 2, (Color){ 255, 255, 255, 40 });
        DrawLineEx((Vector2){ wx + 14, wy - 4 }, (Vector2){ wx + 28, wy }, 2, (Color){ 255, 255, 255, 40 });
    }

    // the islands and their regions
    DrawIsland((Rectangle){ 30, 40, 740, 830 });
    DrawIsland((Rectangle){ 830, 40, 740, 830 });
    UiTextCentered("C ISLAND", M((Vector2){ 400, 0 }).x, M((Vector2){ 0, 445 }).y - S(14), fmaxf(16, S(26)), (Color){ 40, 60, 40, 200 });
    UiTextCentered("C++ ISLAND", M((Vector2){ 1200, 0 }).x, M((Vector2){ 0, 475 }).y - S(14), fmaxf(16, S(26)), (Color){ 40, 60, 40, 200 });
    for (int r = 0; r < RegionCount() && r < MAX_REGIONS; r++) DrawRegion(r);

    // paths: inside each region, between regions, and the ferry between the islands
    for (int r = 0; r < RegionCount(); r++) DrawPath(regionNodeStart[r], regionNodeStart[r + 1]);
    for (int r = 0; r + 1 < RegionCount(); r++) {
        int a = regionNodeStart[r + 1] - 1, b = regionNodeStart[r + 1];
        if (a < 0 || b >= nodeCount) continue;
        bool ferry = (r == 1);
        DrawRoute(nodes[a].p, nodes[b].p, ferry ? 90 : 0, ferry ? (Color){ 255, 255, 255, 160 } : (Color){ 245, 235, 205, 200 });
        if (ferry) {
            Vector2 A = M(nodes[a].p), B = M(nodes[b].p);
            float t = 0.5f + 0.35f * sinf(time * 0.5f);
            Vector2 boat = { A.x + (B.x - A.x) * t, (A.y + B.y) / 2 - S(60) + S(6) * sinf(time * 2) };
            DrawTriangle((Vector2){ boat.x - S(18), boat.y }, (Vector2){ boat.x + S(18), boat.y }, (Vector2){ boat.x, boat.y + S(10) }, (Color){ 120, 80, 50, 255 });
            DrawTriangle((Vector2){ boat.x - S(18), boat.y }, (Vector2){ boat.x, boat.y + S(10) }, (Vector2){ boat.x + S(18), boat.y }, (Color){ 120, 80, 50, 255 });
            DrawTriangle((Vector2){ boat.x, boat.y - S(26) }, (Vector2){ boat.x, boat.y - S(2) }, (Vector2){ boat.x + S(16), boat.y - S(4) }, WHITE);
            DrawTriangle((Vector2){ boat.x, boat.y - S(26) }, (Vector2){ boat.x + S(16), boat.y - S(4) }, (Vector2){ boat.x, boat.y - S(2) }, WHITE);
        }
    }

    for (int r = 0; r < RegionCount() && r < MAX_REGIONS; r++) DrawRegionBanner(r);

    // factories (Factory Contracts) in each region
    for (int r = 0; r < RegionCount() && r < MAX_REGIONS; r++) {
        int contracts = 0, done = 0;
        for (int p = 0; p < ProjectCount(); p++) {
            if (GetProject(p)->region != r) continue;
            contracts++;
            if (prog.projSteps[p] >= GetProject(p)->stepCount) done++;
        }
        if (contracts == 0) continue;
        const Region *rg = &REGIONS[r];
        Vector2 c = M((Vector2){ rg->area.x + rg->area.width - 48, rg->area.y + 40 });
        Rectangle hit = { c.x - S(34), c.y - S(36), S(68), S(76) };
        bool h = false;
        bool clicked = UiClickArea(hit, &h);
        DrawCircleV(c, S(h ? 34 : 30), ColorAlpha((Color){ 30, 32, 42, 255 }, 0.85f));
        IconFactory(c, S(44), COL_FACTORY);
        UiTextCentered(TextFormat("%d/%d", done, contracts), c.x, c.y + S(30), fmaxf(12, S(15)), WHITE);
        if (h) {
            const char *tip = "Factory Contracts: bigger projects, built step by step";
            float tw = UiTextWidth(tip, 16) + 24;
            DrawRectangleRounded((Rectangle){ c.x - tw, c.y + S(46), tw, 30 }, 0.3f, 6, (Color){ 22, 24, 31, 240 });
            UiText(tip, c.x - tw + 12, c.y + S(46) + 6, 16, COL_TEXT);
        }
        if (clicked) { OpenContracts(r); return; }
    }

    // nodes
    int next = NextLevel();
    int nextNode = -1;
    hover = -1;
    for (int i = 0; i < nodeCount; i++) {
        Vector2 c = M(nodes[i].p);
        float r = S(nodes[i].tut < 0 ? 38 : 28) + 6;
        if (CheckCollisionPointCircle(GetMousePosition(), c, r)) hover = i;
        if (nodes[i].tut >= 0 && nodes[i].tut == next) nextNode = i;
    }
    for (int i = 0; i < nodeCount; i++) DrawNode(i, i == nextNode);

    // the player's robot stands on the next level
    if (nextNode >= 0) {
        Vector2 c = M(nodes[nextNode].p);
        float bob = sinf(time * 3) * S(5);
        IconRobot((Vector2){ c.x, c.y - S(56) + bob }, S(42), COL_ROBOT);
    }

    if (hover >= 0) {
        UiSetCursor(MOUSE_CURSOR_POINTING_HAND);
        DrawTooltip(hover);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            const Node *n = &nodes[hover];
            if (n->tut >= 0) OpenLevel(n->tut);
            else OpenMission(n->mission);
            return;
        }
    }

    // legend
    float lx = 16, ly = SCREEN_H - 40.0f;
    DrawRectangleRounded((Rectangle){ lx - 6, ly - 8, 600, 38 }, 0.4f, 6, (Color){ 22, 24, 31, 200 });
    UiText("Every level is a 3D game.  Badges: side quests -", lx + 4, ly + 2, 16, COL_MUTED);
    GameMode modes[2] = { MODE_ROBOT, MODE_HEIST };
    float x = lx + 8 + UiTextWidth("Every level is a 3D game.  Badges: side quests -", 16);
    for (int i = 0; i < 2; i++) {
        DrawModeIcon(modes[i], (Vector2){ x + 10, ly + 11 }, 22);
        UiText(ModeName(modes[i]), x + 26, ly + 2, 16, COL_TEXT);
        x += 36 + UiTextWidth(ModeName(modes[i]), 16);
    }

    DrawTopBar("World Map");
    DrawPlayerChip(SCREEN_W - 24, 14);
    if (BackButton("< Menu")) OpenMenu();
}
