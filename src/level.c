// A level is one tutorial: the Briefing (the lesson pages), the Output Rush (its quiz)
// and its missions - the main game first, then the extras.
#include "app.h"
#include "game/levels.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int   page;
static float lessonScroll;

static const char *ModeBlurb(GameMode mode)
{
    switch (mode) {
        case MODE_ROBOT: return "Write real code that drives the robot through the maze.";
        case MODE_HEIST: return "Follow pointers along a street of memory without tripping the alarm.";
        case MODE_BUGS:  return "Squash the bugs in a broken program before the clock runs out.";
        default:         return "Your program is the machine: every test is a crate on the conveyor belt.";
    }
}

static bool RushCleared(int tut) { return prog.rushStars[tut] > 0 || GetTutorial(tut)->quizCount == 0; }

void OpenLevel(int tut)
{
    app.tut = tut;
    app.screen = SCREEN_LEVEL;
}

// One row on the level screen. Returns true when its button is pressed.
static bool StageCard(Rectangle r, int index, void (*icon)(Vector2, float, Color), GameMode mode, bool useMode, Color color,
                      const char *title, const char *sub, int stars, unsigned char starBits, bool showStars,
                      const char *button, bool locked, const char *lockText, bool done)
{
    bool hover = false;
    Rectangle hit = r;
    bool clicked = !locked && UiClickArea(hit, &hover);
    DrawRectangleRounded(r, 0.18f, 8, hover ? COL_PANEL_HI : (Color){ 38, 41, 53, 255 });
    if (done) DrawRectangleRounded((Rectangle){ r.x, r.y, 6, r.height }, 0.5f, 4, COL_GOOD);

    Vector2 ic = { r.x + 52, r.y + r.height / 2 };
    DrawCircleV(ic, 30, ColorAlpha(color, locked ? 0.08f : 0.18f));
    if (useMode) DrawModeIcon(mode, ic, 38);
    else icon(ic, 38, color);
    UiText(TextFormat("%d", index), r.x + 14, r.y + 8, 14, COL_MUTED);

    UiText(title, r.x + 100, r.y + 16, 23, locked ? COL_MUTED : COL_TEXT);
    UiText(locked ? lockText : sub, r.x + 100, r.y + 52, 17, locked ? (Color){ 220, 160, 120, 255 } : COL_MUTED);

    float right = r.x + r.width - 20;
    if (locked) {
        IconLock((Vector2){ right - 30, r.y + r.height / 2 }, 34, COL_MUTED);
        return false;
    }
    Rectangle btn = { right - 130, r.y + (r.height - 44) / 2, 130, 44 };
    bool pressed = UiButton(btn, button, done ? COL_PANEL_HI : COL_ACCENT) || clicked;
    if (showStars) DrawStarRow(btn.x - 70, r.y + r.height / 2, 12, starBits, stars);
    return pressed;
}

void ScreenLevel(void)
{
    int tut = app.tut;
    const Tutorial *t = GetTutorial(tut);
    if (IsKeyPressed(KEY_ESCAPE)) { OpenMap(); return; }

    DrawTopBar(TextFormat("%d. %s", tut + 1, t->title));
    DrawPlayerChip(SCREEN_W - 24, 14);
    if (BackButton("< Map")) { OpenMap(); return; }

    Rectangle panel = ContentPanel();
    panel.height = SCREEN_H - 108.0f;
    DrawRectangleRounded(panel, 0.03f, 8, COL_PANEL);
    float x = panel.x + 32, w = panel.width - 64, y = panel.y + 22;

    int region = RegionOf(tut);
    const Tutorial *first = GetTutorial(RegionFirstTut(region));
    const char *regionName = first->section ? first->section : "Intro to C";
    UiText(TextFormat("%s   -   Level %d of %d", regionName, tut - RegionFirstTut(region) + 1, RegionTutCount(region)), x, y, 17, COL_ACCENT);
    if (t->lecture) {
        const char *lec = TextFormat("Lecture: %s", t->lecture);
        UiText(lec, x + w - UiTextWidth(lec, 17), y, 17, COL_MUTED);
    }
    y += 28;
    UiText(t->title, x, y, 34, COL_TEXT);
    y += 46;
    y += UiWrapped(t->summary, x, y, w, 19, COL_MUTED) + 12;

    int missions[8];
    int mcount = LevelMissions(tut, missions);
    int rows = 2 + mcount;
    float gap = 12;
    float cardH = fminf(96, (panel.y + panel.height - 20 - y - gap * (rows - 1)) / rows);
    if (cardH < 70) cardH = 70;
    int index = 1;

    // 1. Briefing
    Rectangle r = { x, y, w, cardH };
    bool read = prog.lessonRead[tut];
    if (StageCard(r, index++, IconBook, 0, false, (Color){ 160, 200, 120, 255 }, "Briefing",
                  TextFormat("Read the lesson first: %d page%s from the lecture notes.", t->pageCount, t->pageCount == 1 ? "" : "s"),
                  0, 0, false, read ? "Read again" : "Read >", false, NULL, read)) {
        OpenBriefing(tut, 0);
        return;
    }
    y += cardH + gap;

    // 2. Output Rush
    if (t->quizCount > 0) {
        r = (Rectangle){ x, y, w, cardH };
        int rs = prog.rushStars[tut];
        if (StageCard(r, index++, IconRush, 0, false, COL_RUSH, "Output Rush",
                      TextFormat("%d quiz questions fall from the sky - answer each one before it lands.", t->quizCount),
                      rs, 0, true, rs ? "Play again" : "Play >", false, NULL, rs > 0)) {
            OpenRush(tut);
            return;
        }
        y += cardH + gap;
    }

    // 3+. Missions
    for (int i = 0; i < mcount; i++) {
        const MissionInfo *m = GetMission(missions[i]);
        r = (Rectangle){ x, y, w, cardH };
        bool locked = !RushCleared(tut);
        // the level's own project plays as its 3D level when it has one
        const LevelDef *lv = (m->role == ROLE_MAIN) ? LevelFor(t->id) : NULL;
        const char *kind = m->role == ROLE_SIDE ? "Side quest - " : m->role == ROLE_BONUS ? "Bonus - " : "";
        char title[160], blurb[220];       // (TextFormat's few buffers could be reused inside StageCard)
        if (lv) snprintf(title, sizeof(title), "%s: %s", lv->name, m->title);
        else snprintf(title, sizeof(title), "%s%s: %s", kind, ModeName(m->mode), m->title);
        if (m->role == ROLE_SIDE) snprintf(blurb, sizeof(blurb), "Optional. %s Its stars count towards your side quest total.", ModeBlurb(m->mode));
        else snprintf(blurb, sizeof(blurb), "%s", lv ? lv->tagline : ModeBlurb(m->mode));
        Color color = lv ? lv->color : ModeColor(m->mode);
        bool done = MissionCleared(missions[i]);
        if (StageCard(r, index++, NULL, m->mode, true, color, title, blurb, -1, prog.stars[missions[i]], true,
                      done ? "Play again" : "Play >", locked, "Clear the Output Rush to unlock this mission.", done)) {
            OpenMission(missions[i]);
            return;
        }
        y += cardH + gap;
    }
}

// ---------------------------------------------------------------- the briefing (lesson pages)

static void SetPage(int p)
{
    page = p;
    lessonScroll = 0;
    if (page == GetTutorial(app.tut)->pageCount - 1) AwardLesson(app.tut);
}

void OpenBriefing(int tut, int p)
{
    app.tut = tut;
    SetPage(p);
    app.screen = SCREEN_BRIEFING;
}

void ScreenBriefing(void)
{
    const Tutorial *t = GetTutorial(app.tut);
    int last = t->pageCount - 1;

    if (IsKeyPressed(KEY_ESCAPE)) { OpenLevel(app.tut); return; }
    if (IsKeyPressed(KEY_RIGHT) && page < last) SetPage(page + 1);
    if (IsKeyPressed(KEY_LEFT) && page > 0) SetPage(page - 1);

    const LessonPage *p = &t->pages[page];

    DrawTopBar(TextFormat("Briefing: %s", t->title));
    if (t->lecture) TopBarRight(TextFormat("Lecture: %s   -   Page %d of %d", t->lecture, page + 1, t->pageCount), COL_MUTED);
    else TopBarRight(TextFormat("Page %d of %d", page + 1, t->pageCount), COL_MUTED);
    if (BackButton("< Level")) { OpenLevel(app.tut); return; }

    Rectangle panel = ContentPanel();
    float left = panel.x, right = panel.x + panel.width, navY = NavY();
    DrawRectangleRounded(panel, 0.04f, 8, COL_PANEL);
    float x = panel.x + 40, w = panel.width - 80;

    // Pages marked "Extra:" go beyond the lecture notes and get a badge instead of the prefix
    const char *heading = p->heading;
    bool extra = strncmp(heading, "Extra: ", 7) == 0;
    if (extra) heading += 7;

    // Long pages scroll (mouse wheel or the up/down keys)
    float codeH = 0;
    if (p->code) {
        int lines = 1;
        for (const char *c = p->code; *c; c++) if (*c == '\n') lines++;
        codeH = 14 + lines * 18 * 1.3f + 32;
    }
    float contentH = 60 + UiWrappedHeight(p->body, w, 21) + codeH;
    Rectangle view = { panel.x, panel.y + 16, panel.width, panel.height - 32 };
    float maxScroll = contentH + 16 - view.height;
    if (maxScroll < 0) maxScroll = 0;
    if (CheckCollisionPointRec(GetMousePosition(), panel)) lessonScroll -= GetMouseWheelMove() * 50;
    if (IsKeyDown(KEY_DOWN)) lessonScroll += 10;
    if (IsKeyDown(KEY_UP)) lessonScroll -= 10;
    if (lessonScroll > maxScroll) lessonScroll = maxScroll;
    if (lessonScroll < 0) lessonScroll = 0;

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
    float y = panel.y + 32 - lessonScroll;
    float hx = x;
    if (extra) {
        const char *tag = "EXTRA";
        float tw = UiTextWidth(tag, 17) + 24;
        DrawRectangleRounded((Rectangle){ x, y + 5, tw, 30 }, 0.5f, 6, (Color){ 120, 95, 40, 255 });
        UiText(tag, x + 12, y + 11, 17, (Color){ 255, 225, 150, 255 });
        hx += tw + 14;
    }
    UiText(heading, hx, y, 36, COL_TEXT);
    if (extra) UiText("Beyond the lecture notes", hx + UiTextWidth(heading, 36) + 18, y + 13, 17, (Color){ 240, 200, 110, 255 });
    y += 60;
    y += UiWrapped(p->body, x, y, w, 21, COL_TEXT);
    if (p->code) UiCodeBlock(p->code, x, y + 14, w, 18);
    EndScissorMode();

    if (maxScroll > 0) {
        const char *more = lessonScroll < maxScroll ? "scroll for more  v" : "^  top of page: scroll up";
        UiText(more, right - 24 - UiTextWidth(more, 16), panel.y + panel.height - 26, 16, COL_ACCENT);
    }

    // Page dots
    float dotsW = t->pageCount * 22.0f;
    for (int i = 0; i < t->pageCount; i++)
        DrawCircle((int)(SCREEN_W / 2 - dotsW / 2 + 11 + i * 22), (int)navY + 25, 6, i == page ? COL_ACCENT : COL_PANEL_HI);

    if (page > 0 && UiButton((Rectangle){ left, navY, 180, 50 }, "< Previous", COL_PANEL_HI)) SetPage(page - 1);
    if (page < last) {
        if (UiButton((Rectangle){ right - 180, navY, 180, 50 }, "Next >", COL_ACCENT)) SetPage(page + 1);
    } else if (t->quizCount > 0) {
        if (UiButton((Rectangle){ right - 260, navY, 260, 50 }, prog.rushStars[app.tut] ? "Output Rush again >" : "Start the Output Rush >", COL_RUSH))
            OpenRush(app.tut);
    } else {
        if (UiButton((Rectangle){ right - 220, navY, 220, 50 }, "Back to the level", COL_ACCENT)) OpenLevel(app.tut);
    }
}
