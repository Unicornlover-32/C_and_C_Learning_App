// C and C++ for Dummies - a raylib app that teaches C step by step.
#include "raylib.h"
#include "ui.h"
#include "editor.h"
#include "tutorials.h"
#include "runner.h"
#include <string.h>

#define MAX_TUTORIALS 32

typedef enum {
    SCREEN_MAIN,
    SCREEN_TUTORIALS,
    SCREEN_LESSON,
    SCREEN_PRACTICE,
} Screen;

typedef struct {
    Screen screen;
    bool   quit;

    int    tut;                 // current tutorial
    int    page;                // current lesson page
    float  listScroll;

    bool   completed[MAX_TUTORIALS];
    bool   hasDraft[MAX_TUTORIALS];
    char   drafts[MAX_TUTORIALS][EDITOR_CAP];  // the student's code is kept per tutorial

    Editor    editor;
    RunResult result;
    bool      hasResult;
    bool      runPending;
    float     resultScroll;
} App;

static App app;

// ---------------------------------------------------------------- helpers

static int CountCompleted(void)
{
    int n = 0;
    for (int i = 0; i < TUTORIAL_COUNT; i++) if (app.completed[i]) n++;
    return n;
}

static void DrawTopBar(const char *title)
{
    DrawRectangle(0, 0, SCREEN_W, 64, COL_PANEL);
    DrawRectangle(0, 64, SCREEN_W, 1, COL_BORDER);
    UiText(title, 200, 17, 28, COL_TEXT);
}

static bool BackButton(const char *label)
{
    return UiButton((Rectangle){ 20, 10, 160, 44 }, label, COL_PANEL_HI);
}

static void OpenLesson(int tut)
{
    app.tut = tut;
    app.page = 0;
    app.screen = SCREEN_LESSON;
}

static void SaveDraft(void)
{
    memcpy(app.drafts[app.tut], app.editor.text, (size_t)app.editor.len + 1);
    app.hasDraft[app.tut] = true;
}

static void OpenPractice(int tut)
{
    app.tut = tut;
    EditorSetText(&app.editor, app.hasDraft[tut] ? app.drafts[tut] : TUTORIALS[tut].starterCode);
    app.editor.focused = true;
    app.hasResult = false;
    app.runPending = false;
    app.resultScroll = 0;
    app.screen = SCREEN_PRACTICE;
}

// ---------------------------------------------------------------- main menu

static void ScreenMainMenu(void)
{
    float cx = SCREEN_W / 2.0f;

    // Faint code in the background for decoration
    const char *deco = "#include <stdio.h>\n\nint main(void)\n{\n    printf(\"Hello!\\n\");\n    return 0;\n}";
    UiCode(deco, 70, 470, 20, (Color){ 255, 255, 255, 18 }, 0);
    UiCode("for (int i = 0; i < 10; i++)\n{\n    learn(i);\n}", 900, 90, 20, (Color){ 255, 255, 255, 18 }, 0);

    UiTextCentered("{ }", cx, 105, 64, COL_ACCENT);

    UiTextCentered("C and C++ for Dummies", cx, 190, 64, COL_TEXT);
    UiTextCentered("Learn to code in C and C++, one small step at a time.", cx, 268, 24, COL_MUTED);

    // Progress bar
    int done = CountCompleted();
    Rectangle bar = { cx - 200, 330, 400, 10 };
    DrawRectangleRounded(bar, 1.0f, 8, COL_PANEL_HI);
    if (done > 0) {
        Rectangle fill = bar;
        fill.width = bar.width * done / (float)TUTORIAL_COUNT;
        DrawRectangleRounded(fill, 1.0f, 8, COL_GOOD);
    }
    UiTextCentered(TextFormat("%d of %d tutorials completed", done, TUTORIAL_COUNT), cx, 350, 18, COL_MUTED);

    if (UiButton((Rectangle){ cx - 160, 410, 320, 60 }, "View Tutorials", COL_ACCENT))
        app.screen = SCREEN_TUTORIALS;
    if (UiButton((Rectangle){ cx - 160, 485, 320, 54 }, "Exit", COL_PANEL_HI))
        app.quit = true;
}

// ---------------------------------------------------------------- tutorial list

static void ScreenTutorials(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_MAIN; return; }

    const float top = 66, cardH = 96, gap = 14;
    float viewH = SCREEN_H - top;
    float contentH = 24 + TUTORIAL_COUNT * (cardH + gap);
    float maxScroll = contentH - viewH;
    if (maxScroll < 0) maxScroll = 0;

    app.listScroll -= GetMouseWheelMove() * 50;
    if (app.listScroll < 0) app.listScroll = 0;
    if (app.listScroll > maxScroll) app.listScroll = maxScroll;

    BeginScissorMode(0, (int)top, SCREEN_W, (int)viewH);
    for (int i = 0; i < TUTORIAL_COUNT; i++) {
        const Tutorial *t = &TUTORIALS[i];
        Rectangle card = { 80, top + 24 + i * (cardH + gap) - app.listScroll, SCREEN_W - 160, cardH };

        bool hover = false;
        bool clicked = GetMouseY() > top && UiClickArea(card, &hover);

        DrawRectangleRounded(card, 0.18f, 8, hover ? COL_PANEL_HI : COL_PANEL);

        float circleX = card.x + 52, circleY = card.y + cardH / 2;
        DrawCircle((int)circleX, (int)circleY, 26, app.completed[i] ? COL_GOOD_DARK : COL_ACCENT);
        UiTextCentered(TextFormat("%d", i + 1), circleX, circleY - 13, 26, WHITE);

        UiText(t->title, card.x + 100, card.y + 18, 26, COL_TEXT);
        UiText(t->summary, card.x + 100, card.y + 56, 18, COL_MUTED);

        const char *status = app.completed[i] ? "Completed" : "Start  >";
        Color statusCol = app.completed[i] ? COL_GOOD : (hover ? COL_ACCENT : COL_MUTED);
        UiText(status, card.x + card.width - 30 - UiTextWidth(status, 20), circleY - 10, 20, statusCol);

        if (clicked) OpenLesson(i);
    }
    EndScissorMode();

    DrawTopBar("Tutorials");
    if (BackButton("< Home")) app.screen = SCREEN_MAIN;
}

// ---------------------------------------------------------------- lesson pages

static void ScreenLesson(void)
{
    const Tutorial *t = &TUTORIALS[app.tut];
    int last = t->pageCount - 1;

    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_TUTORIALS; return; }
    if (IsKeyPressed(KEY_RIGHT) && app.page < last) app.page++;
    if (IsKeyPressed(KEY_LEFT) && app.page > 0) app.page--;

    const LessonPage *p = &t->pages[app.page];

    DrawTopBar(TextFormat("%d. %s", app.tut + 1, t->title));
    const char *pageText = TextFormat("Page %d of %d", app.page + 1, t->pageCount);
    UiText(pageText, SCREEN_W - 30 - UiTextWidth(pageText, 20), 22, 20, COL_MUTED);
    if (BackButton("< Tutorials")) { app.screen = SCREEN_TUTORIALS; return; }

    // Content
    Rectangle panel = { 80, 88, SCREEN_W - 160, 520 };
    DrawRectangleRounded(panel, 0.04f, 8, COL_PANEL);
    float x = panel.x + 40, w = panel.width - 80;
    float y = panel.y + 32;

    UiText(p->heading, x, y, 36, COL_TEXT);
    y += 60;
    y += UiWrapped(p->body, x, y, w, 21, COL_TEXT);
    if (p->code) UiCodeBlock(p->code, x, y + 14, w, 18);

    // Page dots
    float dotsW = t->pageCount * 22.0f;
    for (int i = 0; i < t->pageCount; i++)
        DrawCircle((int)(SCREEN_W / 2 - dotsW / 2 + 11 + i * 22), 652, 6, i == app.page ? COL_ACCENT : COL_PANEL_HI);

    // Navigation
    if (app.page > 0 && UiButton((Rectangle){ 80, 627, 180, 50 }, "< Previous", COL_PANEL_HI))
        app.page--;

    if (app.page < last) {
        if (UiButton((Rectangle){ SCREEN_W - 260, 627, 180, 50 }, "Next >", COL_ACCENT)) app.page++;
    } else {
        if (UiButton((Rectangle){ SCREEN_W - 320, 627, 240, 50 }, "Start Project >", COL_GOOD_DARK))
            OpenPractice(app.tut);
    }
}

// ---------------------------------------------------------------- practice project

static void DrawOutputBox(const char *label, const char *text, Rectangle r)
{
    UiText(label, r.x, r.y, 17, COL_MUTED);
    Rectangle box = { r.x, r.y + 24, r.width, r.height - 24 };
    DrawRectangleRounded(box, 0.06f, 6, COL_CODE_BG);
    BeginScissorMode((int)box.x, (int)box.y, (int)box.width, (int)box.height);
    UiCode(text[0] ? text : "(nothing)", box.x + 10, box.y + 8, 15, text[0] ? COL_CODE_TEXT : COL_MUTED, box.width - 20);
    EndScissorMode();
}

static void DrawResults(Rectangle r, const Tutorial *t)
{
    DrawRectangleRounded(r, 0.05f, 8, COL_PANEL);
    float x = r.x + 18, y = r.y + 14, w = r.width - 36;

    if (app.runPending) {
        UiText("Compiling and running your code...", x, y, 22, COL_ACCENT);
        return;
    }
    if (!app.hasResult) {
        UiText("Output", x, y, 22, COL_TEXT);
        UiWrapped("Press Run & Check (or Ctrl+Enter) to compile your code and test it against the expected output.",
                  x, y + 36, w, 18, COL_MUTED);
        return;
    }

    const RunResult *res = &app.result;

    if (!res->compiled) {
        UiText("Your code didn't compile. Read the first error, fix it and try again.", x, y, 20, COL_BAD);
        Rectangle logArea = { x, y + 34, w, r.y + r.height - (y + 34) - 14 };
        DrawRectangleRounded(logArea, 0.04f, 6, COL_CODE_BG);
        if (CheckCollisionPointRec(GetMousePosition(), logArea)) app.resultScroll -= GetMouseWheelMove() * 40;
        if (app.resultScroll < 0) app.resultScroll = 0;
        BeginScissorMode((int)logArea.x, (int)logArea.y, (int)logArea.width, (int)logArea.height);
        float h = UiCode(res->compileLog, logArea.x + 10, logArea.y + 8 - app.resultScroll, 15, COL_TEXT, logArea.width - 20);
        EndScissorMode();
        float maxScroll = h - logArea.height + 16;
        if (app.resultScroll > maxScroll) app.resultScroll = maxScroll > 0 ? maxScroll : 0;
        return;
    }

    int passCount = 0, firstFail = -1;
    for (int i = 0; i < res->testCount; i++) {
        if (res->passed[i]) passCount++;
        else if (firstFail < 0) firstFail = i;
    }

    if (res->allPassed)
        UiText(TextFormat("All %d tests passed - tutorial complete!", res->testCount), x, y, 22, COL_GOOD);
    else
        UiText(TextFormat("Passed %d of %d tests. Keep going!", passCount, res->testCount), x, y, 22, COL_BAD);

    // One chip per test, right-aligned
    float chipW = 58, chipX = r.x + r.width - 18 - res->testCount * (chipW + 6) + 6;
    for (int i = 0; i < res->testCount; i++) {
        Rectangle chip = { chipX + i * (chipW + 6), y - 2, chipW, 28 };
        DrawRectangleRounded(chip, 0.5f, 6, res->passed[i] ? COL_GOOD_DARK : COL_BAD_DARK);
        UiTextCentered(TextFormat("#%d", i + 1), chip.x + chipW / 2, chip.y + 4, 18, WHITE);
    }

    float boxY = y + 40, boxH = r.y + r.height - boxY - 14;

    if (firstFail >= 0) {
        // Show the first failing test side by side
        const TestCase *tc = &t->tests[firstFail];
        float colW = (w - 24) / 3;
        DrawOutputBox(TextFormat("Test #%d input", firstFail + 1), tc->input ? tc->input : "",
                      (Rectangle){ x, boxY, colW, boxH });
        DrawOutputBox("Expected output", tc->expected, (Rectangle){ x + colW + 12, boxY, colW, boxH });
        DrawOutputBox("Your output", res->actual[firstFail], (Rectangle){ x + (colW + 12) * 2, boxY, colW, boxH });
    } else {
        float boxW = w - 260;
        DrawOutputBox("Your output (test #1)", res->actual[0], (Rectangle){ x, boxY, boxW, boxH });
        if (app.tut + 1 < TUTORIAL_COUNT) {
            if (UiButton((Rectangle){ x + boxW + 24, boxY + boxH - 52, 236, 52 }, "Next Tutorial >", COL_ACCENT)) {
                SaveDraft();
                OpenLesson(app.tut + 1);
            }
        } else {
            UiWrapped("You've finished every tutorial so far. More coming soon!",
                      x + boxW + 24, boxY + 24, 230, 18, COL_MUTED);
        }
    }
}

static void ScreenPractice(void)
{
    const Tutorial *t = &TUTORIALS[app.tut];

    // A run was requested last frame (after the "Compiling..." message was drawn)
    if (app.runPending) {
        SaveDraft();
        RunPractice(app.editor.text, t->tests, t->testCount, &app.result);
        app.hasResult = true;
        app.runPending = false;
        app.resultScroll = 0;
        app.editor.focused = true;
        if (app.result.allPassed) app.completed[app.tut] = true;
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
    if (ctrl && IsKeyPressed(KEY_ENTER)) app.runPending = true;
    if (IsKeyPressed(KEY_ESCAPE)) {
        SaveDraft();
        app.screen = SCREEN_LESSON;
        app.page = t->pageCount - 1;
        return;
    }

    DrawTopBar(TextFormat("Project: %s", t->projectTitle));
    if (app.completed[app.tut]) UiText("Completed", SCREEN_W - 30 - UiTextWidth("Completed", 20), 22, 20, COL_GOOD);
    if (BackButton("< Lesson")) {
        SaveDraft();
        app.screen = SCREEN_LESSON;
        app.page = t->pageCount - 1;
        return;
    }

    // Left: the task
    Rectangle left = { 20, 84, 380, SCREEN_H - 104 };
    DrawRectangleRounded(left, 0.03f, 8, COL_PANEL);
    UiText("Your task", left.x + 20, left.y + 18, 26, COL_TEXT);
    BeginScissorMode((int)left.x, (int)(left.y + 58), (int)left.width, (int)(left.height - 58 - 140));
    UiWrapped(t->projectBrief, left.x + 20, left.y + 62, left.width - 40, 18, COL_TEXT);
    EndScissorMode();

    Rectangle runBtn   = { left.x + 20, left.y + left.height - 128, left.width - 40, 54 };
    Rectangle resetBtn = { left.x + 20, left.y + left.height - 64, left.width - 40, 46 };
    if (UiButton(runBtn, app.runPending ? "Running..." : "Run & Check", COL_GOOD_DARK)) app.runPending = true;
    if (UiButton(resetBtn, "Reset Code", COL_PANEL_HI)) {
        EditorSetText(&app.editor, t->starterCode);
        app.hasResult = false;
    }

    // Right: the editor and the results
    Rectangle edRect = { 420, 84, SCREEN_W - 440, 370 };
    EditorUpdate(&app.editor, edRect);
    EditorDraw(&app.editor, edRect);

    Rectangle resRect = { 420, 468, SCREEN_W - 440, SCREEN_H - 488 };
    DrawResults(resRect, t);
}

// ---------------------------------------------------------------- main

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "C and C++ for Dummies");
    SetExitKey(KEY_NULL);   // Esc is used for "back", not quit
    SetTargetFPS(60);

    UiInit();
    EditorInit(&app.editor, 20);
    app.screen = SCREEN_MAIN;

    while (!WindowShouldClose() && !app.quit) {
        UiBeginFrame();
        BeginDrawing();
        ClearBackground(COL_BG);

        switch (app.screen) {
            case SCREEN_MAIN:      ScreenMainMenu();  break;
            case SCREEN_TUTORIALS: ScreenTutorials(); break;
            case SCREEN_LESSON:    ScreenLesson();    break;
            case SCREEN_PRACTICE:  ScreenPractice();  break;
        }

        EndDrawing();
        UiEndFrame();
    }

    UiShutdown();
    CloseWindow();
    return 0;
}
