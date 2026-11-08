// C and C++ for Dummies - a raylib game that teaches C and C++, following the
// Advanced Programming lecture notes.
//
// The world map holds every level. A level is a Briefing (the lesson), an Output Rush
// (its quiz) and missions in four games: Robot Rescue, Memory Heist, Code Factory and
// Bug Hunter. Stars, XP, badges and a daily streak keep score.
#include "app.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

App app;
Progress prog;

// ---------------------------------------------------------------- shared helpers

void DrawTopBar(const char *title)
{
    DrawRectangle(0, 0, SCREEN_W, 64, COL_PANEL);
    DrawRectangle(0, 64, SCREEN_W, 1, COL_BORDER);
    UiText(title, 200, 17, 28, COL_TEXT);
}

void TopBarRight(const char *text, Color color)
{
    UiText(text, SCREEN_W - 30 - UiTextWidth(text, 20), 22, 20, color);
}

bool BackButton(const char *label)
{
    return UiButton((Rectangle){ 20, 10, 160, 44 }, label, COL_PANEL_HI);
}

// The big panel used by briefings and overviews: it fills the screen but stops growing
// on very wide monitors, so lines of text stay easy to read.
Rectangle ContentPanel(void)
{
    float w = SCREEN_W - 160.0f;
    if (w > 1400) w = 1400;
    return (Rectangle){ (SCREEN_W - w) / 2, 88, w, SCREEN_H - 200.0f };
}

float NavY(void) { return SCREEN_H - 93.0f; }

bool CtrlDown(void)
{
    return IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
}

// The game headers live in game/ next to the app
const char *GameDir(void)
{
    static char dir[1024];
    if (dir[0]) return dir;
    snprintf(dir, sizeof(dir), "%sgame", GetApplicationDirectory());
    if (!DirectoryExists(dir)) snprintf(dir, sizeof(dir), "game");
    for (char *c = dir; *c; c++) if (*c == '\\') *c = '/';
    return dir;
}

// Level, XP towards the next level, streak and stars, drawn from the right edge leftwards
void DrawPlayerChip(float right, float y)
{
    int level = PlayerLevel(prog.xp);
    int from = LevelXp(level), to = LevelXp(level + 1);
    float k = (to > from) ? (prog.xp - from) / (float)(to - from) : 1;

    const char *stars = TextFormat("%d", TotalStars());
    float x = right - UiTextWidth(stars, 20);
    UiText(stars, x, y + 8, 20, COL_GOLD);
    DrawStar((Vector2){ x - 14, y + 18 }, 10, COL_GOLD);
    x -= 44;

    if (prog.streak > 0) {
        const char *s = TextFormat("%d", prog.streak);
        x -= UiTextWidth(s, 20);
        UiText(s, x, y + 8, 20, (Color){ 255, 160, 80, 255 });
        IconFlame((Vector2){ x - 14, y + 18 }, 22, (Color){ 255, 120, 60, 255 });
        x -= 44;
    }

    float barW = 120;
    x -= barW;
    DrawRectangleRounded((Rectangle){ x, y + 22, barW, 10 }, 1, 6, COL_PANEL_HI);
    DrawRectangleRounded((Rectangle){ x, y + 22, barW * k, 10 }, 1, 6, COL_ACCENT);
    UiText(TextFormat("%d XP", prog.xp), x, y + 1, 15, COL_MUTED);
    x -= 46;
    DrawCircleV((Vector2){ x + 18, y + 18 }, 18, COL_ACCENT);
    UiTextCentered(TextFormat("%d", level), x + 18, y + 7, 20, WHITE);
}

// ---------------------------------------------------------------- toasts

#define MAX_TOASTS 6

typedef struct {
    char   title[64];
    char   text[128];
    Color  color;
    int    icon;
    double at;
} Toast;

static Toast toasts[MAX_TOASTS];
static int   toastCount;

void ToastPush(const char *title, const char *text, Color color, int icon)
{
    if (toastCount == MAX_TOASTS) {
        memmove(toasts, toasts + 1, sizeof(Toast) * (MAX_TOASTS - 1));
        toastCount--;
    }
    Toast *t = &toasts[toastCount++];
    snprintf(t->title, sizeof(t->title), "%s", title);
    snprintf(t->text, sizeof(t->text), "%s", text);
    t->color = color;
    t->icon = icon;
    t->at = GetTime() + toastCount * 0.25;   // stagger toasts that arrive together
}

static void DrawToasts(void)
{
    double now = GetTime();
    float y = 76;
    for (int i = 0; i < toastCount; i++) {
        Toast *t = &toasts[i];
        double age = now - t->at;
        if (age < 0) continue;
        if (age > 4.2) {
            memmove(toasts + i, toasts + i + 1, sizeof(Toast) * (toastCount - i - 1));
            toastCount--;
            i--;
            continue;
        }
        float slide = age < 0.3 ? (float)(1 - age / 0.3) : 0;
        float alpha = age > 3.8 ? (float)((4.2 - age) / 0.4) : 1;
        float w = 380, h = 66;
        Rectangle r = { SCREEN_W - w - 20 + slide * (w + 30), y, w, h };
        DrawRectangleRounded(r, 0.2f, 8, ColorAlpha((Color){ 22, 24, 31, 255 }, 0.95f * alpha));
        DrawRectangleRounded((Rectangle){ r.x, r.y, 6, h }, 0.5f, 4, ColorAlpha(t->color, alpha));
        Vector2 c = { r.x + 36, r.y + h / 2 };
        if (t->icon == TOAST_BADGE) IconMedal(c, 40, t->color, i);
        else if (t->icon == TOAST_STREAK) IconFlame(c, 36, t->color);
        else if (t->icon == TOAST_LEVEL) IconCrown(c, 36, t->color);
        else DrawStar(c, 16, t->color);
        UiText(t->title, r.x + 66, r.y + 10, 20, ColorAlpha(COL_TEXT, alpha));
        BeginScissorMode((int)r.x, (int)r.y, (int)r.width - 8, (int)r.height);
        UiText(t->text, r.x + 66, r.y + 38, 15, ColorAlpha(COL_MUTED, alpha));
        EndScissorMode();
        y += h + 10;
    }
}

// ---------------------------------------------------------------- main menu

void OpenMenu(void) { app.screen = SCREEN_MENU; }

static int ClearedMissions(void)
{
    int n = 0;
    for (int i = 0; i < MissionCount(); i++) if (MissionCleared(i)) n++;
    return n;
}

static void PlayerCard(Rectangle r)
{
    DrawRectangleRounded(r, 0.08f, 8, COL_PANEL);
    int level = PlayerLevel(prog.xp);
    int from = LevelXp(level), to = LevelXp(level + 1);
    float k = (to > from) ? (prog.xp - from) / (float)(to - from) : 1;
    float time = (float)GetTime();
    float x = r.x + 28, y = r.y + 24;

    IconRobot((Vector2){ x + 44, y + 48 + sinf(time * 2) * 4 }, 84, COL_ROBOT);
    UiText(TextFormat("Level %d", level), x + 108, y + 8, 34, COL_TEXT);
    UiText(PlayerTitle(level), x + 110, y + 50, 20, COL_ACCENT);
    y += 110;

    float barW = r.width - 56;
    DrawRectangleRounded((Rectangle){ x, y, barW, 14 }, 1, 6, COL_PANEL_HI);
    DrawRectangleRounded((Rectangle){ x, y, barW * k, 14 }, 1, 6, COL_ACCENT);
    UiText(TextFormat("%d / %d XP to level %d", prog.xp, to, level + 1), x, y + 20, 16, COL_MUTED);
    y += 58;

    char values[3][40];
    snprintf(values[0], 40, "%d / %d", TotalStars(), MaxStars());
    snprintf(values[1], 40, "%d / %d", ClearedMissions(), MissionCount());
    if (prog.streak) snprintf(values[2], 40, "%d day%s", prog.streak, prog.streak == 1 ? "" : "s");
    else snprintf(values[2], 40, "start one today!");
    static const char *labels[3] = { "Stars", "Missions", "Streak" };
    Color colors[3] = { COL_GOLD, COL_TEXT, { 255, 150, 80, 255 } };
    for (int i = 0; i < 3; i++) {
        UiText(labels[i], x, y, 18, COL_MUTED);
        UiText(values[i], x + 110, y, 18, colors[i]);
        y += 30;
    }
    int badges = 0;
    for (int i = 0; i < BADGE_COUNT; i++) if (HasBadge(i)) badges++;
    UiText("Badges", x, y, 18, COL_MUTED);
    for (int i = 0; i < BADGE_COUNT && i < 10; i++) {
        Vector2 c = { x + 120 + i * 26.0f, y + 10 };
        if (HasBadge(i)) DrawCircleV(c, 9, GetBadge(i)->color);
        else DrawCircleLinesV(c, 9, COL_PANEL_HI);
    }
    UiText(TextFormat("%d / %d", badges, BADGE_COUNT), x + 120 + 10 * 26 + 6, y, 18, COL_TEXT);
}

void ScreenMenu(void)
{
    float time = (float)GetTime();

    // faint code in the background
    UiCode("#include <stdio.h>\n\nint main(void)\n{\n    move();\n    turn_left();\n    return 0;\n}", 60, SCREEN_H - 260.0f, 20,
           (Color){ 255, 255, 255, 16 }, 0);
    UiCode("int *p = &vault;\ngo(p);\ngrab(p);", SCREEN_W - 330.0f, 60, 20, (Color){ 255, 255, 255, 16 }, 0);

    float cardW = 440;
    bool wide = SCREEN_W >= 1180;
    float colX = wide ? SCREEN_W / 2.0f - (420 + 40 + cardW) / 2 : SCREEN_W / 2.0f - 210;
    float top = fmaxf(40, (SCREEN_H - 640) / 2.0f);

    // title with the four game icons
    DrawModeIcon(MODE_ROBOT, (Vector2){ colX + 26, top + 30 }, 48);
    DrawModeIcon(MODE_HEIST, (Vector2){ colX + 86, top + 30 }, 48);
    DrawModeIcon(MODE_FACTORY, (Vector2){ colX + 146, top + 30 }, 48);
    DrawModeIcon(MODE_BUGS, (Vector2){ colX + 206, top + 30 }, 48);
    UiText("C and C++ for Dummies", colX, top + 68, 48, COL_TEXT);
    UiText("Learn C and C++ by playing through the lecture notes.", colX, top + 126, 20, COL_MUTED);

    float y = top + 176, bw = 420;
    int next = NextLevel();
    const char *cont = next >= 0 ? TextFormat("%s: Level %d", ClearedMissions() ? "Continue" : "Start", next + 1) : "World Map";
    if (UiButton((Rectangle){ colX, y, bw, 62 }, cont, COL_ACCENT)) {
        if (next >= 0) OpenLevel(next); else OpenMap();
    }
    if (next >= 0) UiText(GetTutorial(next)->title, colX + 4, y + 68, 17, COL_MUTED);
    y += 98;
    struct { const char *label; int id; } buttons[] = {
        { "World Map", 1 }, { "Warm-up: Output Rush", 2 }, { "Factory Contracts", 3 }, { "Profile & Badges", 4 }, { "Exit", 5 },
    };
    for (int i = 0; i < 5; i++) {
        Rectangle b = { colX, y, bw, 50 };
        Color c = buttons[i].id == 2 ? (Color){ 150, 60, 85, 255 } : COL_PANEL_HI;
        if (UiButton(b, buttons[i].label, c)) {
            switch (buttons[i].id) {
                case 1: OpenMap(); break;
                case 2: OpenWarmup(); break;
                case 3: OpenContracts(-1); break;
                case 4: OpenProfile(); break;
                case 5: app.quit = true; break;
            }
        }
        y += 60;
    }

    if (wide) PlayerCard((Rectangle){ colX + bw + 40, top + 176, cardW, 400 });

    UiTextCentered("F11 switches between full screen and a window", SCREEN_W / 2.0f, SCREEN_H - 36.0f, 16, COL_MUTED);
    (void)time;
}

// ---------------------------------------------------------------- profile

void OpenProfile(void) { app.screen = SCREEN_PROFILE; }

void ScreenProfile(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { OpenMenu(); return; }
    DrawTopBar("Profile & Badges");
    if (BackButton("< Menu")) { OpenMenu(); return; }

    Rectangle panel = ContentPanel();
    panel.height = SCREEN_H - 108.0f;
    DrawRectangleRounded(panel, 0.03f, 8, COL_PANEL);
    float x = panel.x + 32, w = panel.width - 64, y = panel.y + 24;

    int level = PlayerLevel(prog.xp);
    int from = LevelXp(level), to = LevelXp(level + 1);
    float k = (to > from) ? (prog.xp - from) / (float)(to - from) : 1;
    IconRobot((Vector2){ x + 40, y + 40 }, 76, COL_ROBOT);
    UiText(TextFormat("Level %d  %s", level, PlayerTitle(level)), x + 96, y + 4, 32, COL_TEXT);
    DrawRectangleRounded((Rectangle){ x + 96, y + 48, 420, 14 }, 1, 6, COL_PANEL_HI);
    DrawRectangleRounded((Rectangle){ x + 96, y + 48, 420 * k, 14 }, 1, 6, COL_ACCENT);
    UiText(TextFormat("%d XP  -  %d more to level %d", prog.xp, to - prog.xp, level + 1), x + 530, y + 44, 17, COL_MUTED);
    y += 100;

    // numbers
    int levels = 0, rushes = 0, contracts = 0;
    for (int t = 0; t < TutorialCount(); t++) { if (LevelCleared(t)) levels++; if (prog.rushStars[t]) rushes++; }
    for (int p = 0; p < ProjectCount(); p++) if (prog.projSteps[p] >= GetProject(p)->stepCount) contracts++;
    // (TextFormat only has a few buffers, so each value gets its own)
    static const char *labels[7] = { "Stars", "Levels cleared", "Missions cleared", "Output Rushes",
                                     "Contracts done", "Streak", "Warm-up best" };
    char values[7][32];
    snprintf(values[0], 32, "%d / %d", TotalStars(), MaxStars());
    snprintf(values[1], 32, "%d / %d", levels, TutorialCount());
    snprintf(values[2], 32, "%d / %d", ClearedMissions(), MissionCount());
    snprintf(values[3], 32, "%d / %d", rushes, TutorialCount());
    snprintf(values[4], 32, "%d / %d", contracts, ProjectCount());
    snprintf(values[5], 32, "%d (best %d)", prog.streak, prog.bestStreak);
    snprintf(values[6], 32, "%d", prog.warmupBest);
    int cols = w > 1000 ? 4 : 3;
    float cw = (w - (cols - 1) * 12) / cols;
    for (int i = 0; i < 7; i++) {
        Rectangle r = { x + (i % cols) * (cw + 12), y + (i / cols) * 70, cw, 60 };
        DrawRectangleRounded(r, 0.2f, 6, (Color){ 38, 41, 53, 255 });
        UiText(labels[i], r.x + 14, r.y + 8, 16, COL_MUTED);
        UiText(values[i], r.x + 14, r.y + 28, 22, i == 0 ? COL_GOLD : COL_TEXT);
    }
    y += ((7 + cols - 1) / cols) * 70 + 16;

    // badges
    UiText("Badges", x, y, 24, COL_TEXT);
    y += 36;
    int bcols = w > 1000 ? 4 : 3;
    float bw = (w - (bcols - 1) * 12) / bcols;
    float bh = fminf(84, (panel.y + panel.height - 16 - y - 12 * 3) / ((BADGE_COUNT + bcols - 1) / bcols));
    for (int i = 0; i < BADGE_COUNT; i++) {
        Rectangle r = { x + (i % bcols) * (bw + 12), y + (i / bcols) * (bh + 12), bw, bh };
        bool has = HasBadge(i);
        DrawRectangleRounded(r, 0.2f, 6, has ? (Color){ 44, 48, 62, 255 } : (Color){ 32, 34, 44, 255 });
        Vector2 c = { r.x + 34, r.y + bh / 2 };
        if (has) IconMedal(c, fminf(52, bh - 16), GetBadge(i)->color, i);
        else IconLock(c, 30, COL_PANEL_HI);
        UiText(GetBadge(i)->name, r.x + 68, r.y + bh / 2 - 22, 19, has ? COL_TEXT : COL_MUTED);
        BeginScissorMode((int)r.x, (int)r.y, (int)r.width - 6, (int)r.height);
        UiText(GetBadge(i)->desc, r.x + 68, r.y + bh / 2 + 2, 15, COL_MUTED);
        EndScissorMode();
    }
}

// ---------------------------------------------------------------- main

#ifndef CFD_NO_MAIN
int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(BASE_W, BASE_H, "C and C++ for Dummies");
    SetWindowMinSize(1100, 640);
    ToggleBorderlessWindowed();   // start full screen, at the monitor's own resolution
    SetExitKey(KEY_NULL);         // Esc is used for "back", not quit
    SetTargetFPS(60);

    UiInit();
    MissionsInit();               // before loading: progress is matched to missions by id
    ProgressLoad(&prog, PROGRESS_FILE);
    MissionInit();
    app.screen = SCREEN_MENU;

    while (!WindowShouldClose() && !app.quit) {
        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        UiBeginFrame();
        BeginDrawing();
        ClearBackground(COL_BG);

        switch (app.screen) {
            case SCREEN_MENU:      ScreenMenu();      break;
            case SCREEN_MAP:       ScreenMap();       break;
            case SCREEN_LEVEL:     ScreenLevel();     break;
            case SCREEN_BRIEFING:  ScreenBriefing();  break;
            case SCREEN_RUSH:      ScreenRush();      break;
            case SCREEN_MISSION:   ScreenMission();   break;
            case SCREEN_CONTRACTS: ScreenContracts(); break;
            case SCREEN_CONTRACT:  ScreenContract();  break;
            case SCREEN_PROFILE:   ScreenProfile();   break;
        }

        ParticlesDraw();
        DrawToasts();
        EndDrawing();
        UiEndFrame();
    }

    MissionLeave();
    SaveProgress();

    MissionShutdown();
    UiShutdown();
    CloseWindow();
    return 0;
}
#endif
