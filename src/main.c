// C and C++ for Dummies - a raylib app that teaches C and C++ step by step.
#include "raylib.h"
#include "ui.h"
#include "editor.h"
#include "tutorials.h"
#include "runner.h"
#include "progress.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    SCREEN_MAIN,
    SCREEN_TUTORIALS,
    SCREEN_LESSON,
    SCREEN_QUIZ,
    SCREEN_PRACTICE,
} Screen;

typedef struct {
    Screen screen;
    bool   quit;

    int    tut;                 // current tutorial
    int    page;                // current lesson page
    float  listScroll;

    // Quiz
    int    quizIndex;
    bool   quizWrong[MAX_OPTIONS];
    bool   quizCorrect;
    bool   quizLastWrong;

    // Project
    Editor    editor;
    RunResult result;
    bool      hasResult;
    bool      runPending;
    float     resultScroll;
    float     briefScroll;
    bool      showHint;
    double    solutionConfirmAt;   // time of the first "Solution" click, 0 = not asked
    char      notice[160];         // a short message shown in the results panel
} App;

static App app;
static Progress prog;

static const Tutorial *Cur(void) { return GetTutorial(app.tut); }

// ---------------------------------------------------------------- helpers

static int CountCompleted(void)
{
    int n = 0;
    for (int i = 0; i < TutorialCount(); i++) if (prog.completed[i]) n++;
    return n;
}

static bool AnyProgress(void)
{
    for (int i = 0; i < TutorialCount(); i++)
        if (prog.completed[i] || prog.quizPassed[i] || prog.hasDraft[i]) return true;
    return false;
}

static int NextTutorial(void)
{
    for (int i = 0; i < TutorialCount(); i++) if (!prog.completed[i]) return i;
    return -1;
}

static void Save(void) { ProgressSave(&prog, PROGRESS_FILE); }

static void DrawTopBar(const char *title)
{
    DrawRectangle(0, 0, SCREEN_W, 64, COL_PANEL);
    DrawRectangle(0, 64, SCREEN_W, 1, COL_BORDER);
    UiText(title, 200, 17, 28, COL_TEXT);
}

static void TopBarRight(const char *text, Color color)
{
    UiText(text, SCREEN_W - 30 - UiTextWidth(text, 20), 22, 20, color);
}

static bool BackButton(const char *label)
{
    return UiButton((Rectangle){ 20, 10, 160, 44 }, label, COL_PANEL_HI);
}

static void OpenLesson(int tut, int page)
{
    app.tut = tut;
    app.page = page;
    app.screen = SCREEN_LESSON;
}

static void ResetQuestion(void)
{
    memset(app.quizWrong, 0, sizeof(app.quizWrong));
    app.quizCorrect = false;
    app.quizLastWrong = false;
}

static void OpenQuiz(int tut)
{
    app.tut = tut;
    app.quizIndex = 0;
    ResetQuestion();
    app.screen = SCREEN_QUIZ;
}

static void SaveDraft(void)
{
    memcpy(prog.drafts[app.tut], app.editor.text, (size_t)app.editor.len + 1);
    prog.hasDraft[app.tut] = true;
    Save();
}

static void OpenPractice(int tut)
{
    app.tut = tut;
    EditorSetText(&app.editor, prog.hasDraft[tut] ? prog.drafts[tut] : GetTutorial(tut)->starterCode);
    app.editor.focused = true;
    app.hasResult = false;
    app.runPending = false;
    app.resultScroll = 0;
    app.briefScroll = 0;
    app.showHint = false;
    app.solutionConfirmAt = 0;
    app.notice[0] = '\0';
    app.screen = SCREEN_PRACTICE;
}

// Where "Continue" should take you for a tutorial: the quiz/lesson, or straight to the project.
static void StartTutorial(int tut)
{
    if (prog.quizPassed[tut] || GetTutorial(tut)->quizCount == 0) OpenPractice(tut);
    else OpenLesson(tut, 0);
}

static void LeavePractice(void)
{
    SaveDraft();
    OpenLesson(app.tut, Cur()->pageCount - 1);
}

// ---------------------------------------------------------------- main menu

static void ScreenMainMenu(void)
{
    float cx = SCREEN_W / 2.0f;
    int next = NextTutorial();

    // Faint code in the background for decoration
    const char *deco = "#include <stdio.h>\n\nint main(void)\n{\n    printf(\"Hello!\\n\");\n    return 0;\n}";
    UiCode(deco, 70, 470, 20, (Color){ 255, 255, 255, 18 }, 0);
    UiCode("for (int i = 0; i < 10; i++)\n{\n    learn(i);\n}", 900, 90, 20, (Color){ 255, 255, 255, 18 }, 0);

    UiTextCentered("{ }", cx, 105, 64, COL_ACCENT);
    UiTextCentered("C and C++ for Dummies", cx, 190, 64, COL_TEXT);
    UiTextCentered("Learn to code in C and C++, one small step at a time.", cx, 268, 24, COL_MUTED);

    // Progress bar
    int done = CountCompleted();
    Rectangle bar = { cx - 200, 326, 400, 10 };
    DrawRectangleRounded(bar, 1.0f, 8, COL_PANEL_HI);
    if (done > 0) {
        Rectangle fill = bar;
        fill.width = bar.width * done / (float)TutorialCount();
        DrawRectangleRounded(fill, 1.0f, 8, COL_GOOD);
    }
    UiTextCentered(TextFormat("%d of %d tutorials completed", done, TutorialCount()), cx, 346, 18, COL_MUTED);
    if (next >= 0 && AnyProgress())
        UiTextCentered(TextFormat("Next up: %d. %s", next + 1, GetTutorial(next)->title), cx, 372, 18, COL_TEXT);
    else if (next < 0)
        UiTextCentered("Every tutorial complete - brilliant work!", cx, 372, 18, COL_GOOD);

    float y = 410;
    if (next >= 0) {
        const char *label = AnyProgress() ? "Continue Learning" : "Start Learning";
        if (UiButton((Rectangle){ cx - 160, y, 320, 60 }, label, COL_ACCENT)) StartTutorial(next);
        y += 75;
        if (UiButton((Rectangle){ cx - 160, y, 320, 54 }, "View Tutorials", COL_PANEL_HI)) app.screen = SCREEN_TUTORIALS;
    } else {
        if (UiButton((Rectangle){ cx - 160, y, 320, 60 }, "View Tutorials", COL_ACCENT)) app.screen = SCREEN_TUTORIALS;
    }
    y += 69;
    if (UiButton((Rectangle){ cx - 160, y, 320, 54 }, "Exit", COL_PANEL_HI)) app.quit = true;
}

// ---------------------------------------------------------------- tutorial list

static void ScreenTutorials(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_MAIN; return; }

    const float top = 66, cardH = 96, gap = 14, headH = 64;
    int count = TutorialCount();

    float contentH = 24;
    for (int i = 0; i < count; i++) {
        if (i == 0 || GetTutorial(i)->lang != GetTutorial(i - 1)->lang) contentH += headH;
        contentH += cardH + gap;
    }
    float viewH = SCREEN_H - top;
    float maxScroll = contentH - viewH;
    if (maxScroll < 0) maxScroll = 0;

    app.listScroll -= GetMouseWheelMove() * 50;
    if (app.listScroll < 0) app.listScroll = 0;
    if (app.listScroll > maxScroll) app.listScroll = maxScroll;

    BeginScissorMode(0, (int)top, SCREEN_W, (int)viewH);
    float y = top + 16 - app.listScroll;
    for (int i = 0; i < count; i++) {
        const Tutorial *t = GetTutorial(i);

        // Section heading when the language changes
        if (i == 0 || t->lang != GetTutorial(i - 1)->lang) {
            bool isC = (t->lang == LANG_C);
            const char *name = isC ? "C" : "C++";
            UiText(name, 84, y + 14, 34, COL_ACCENT);
            UiText(isC ? "The foundations - start here." : "Builds on everything in the C section.",
                   84 + UiTextWidth(name, 34) + 18, y + 26, 19, COL_MUTED);
            y += headH;
        }

        Rectangle card = { 80, y, SCREEN_W - 160, cardH };
        bool hover = false;
        bool clicked = GetMouseY() > top && UiClickArea(card, &hover);

        DrawRectangleRounded(card, 0.18f, 8, hover ? COL_PANEL_HI : COL_PANEL);

        float circleX = card.x + 52, circleY = card.y + cardH / 2;
        DrawCircle((int)circleX, (int)circleY, 26, prog.completed[i] ? COL_GOOD_DARK : COL_ACCENT);
        UiTextCentered(TextFormat("%d", i + 1), circleX, circleY - 13, 26, WHITE);

        UiText(t->title, card.x + 100, card.y + 18, 26, COL_TEXT);
        UiText(t->summary, card.x + 100, card.y + 56, 18, COL_MUTED);

        const char *status;
        Color statusCol;
        if (prog.completed[i])        { status = "Completed";      statusCol = COL_GOOD; }
        else if (prog.quizPassed[i])  { status = "Project next  >"; statusCol = COL_ACCENT; }
        else                          { status = "Start  >";        statusCol = hover ? COL_ACCENT : COL_MUTED; }
        UiText(status, card.x + card.width - 30 - UiTextWidth(status, 20), circleY - 10, 20, statusCol);

        if (clicked) OpenLesson(i, 0);
        y += cardH + gap;
    }
    EndScissorMode();

    DrawTopBar("Tutorials");
    if (BackButton("< Home")) app.screen = SCREEN_MAIN;
}

// ---------------------------------------------------------------- lesson pages

static void ScreenLesson(void)
{
    const Tutorial *t = Cur();
    int last = t->pageCount - 1;
    bool needsQuiz = t->quizCount > 0 && !prog.quizPassed[app.tut];

    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_TUTORIALS; return; }
    if (IsKeyPressed(KEY_RIGHT) && app.page < last) app.page++;
    if (IsKeyPressed(KEY_LEFT) && app.page > 0) app.page--;

    const LessonPage *p = &t->pages[app.page];

    DrawTopBar(TextFormat("%d. %s", app.tut + 1, t->title));
    TopBarRight(TextFormat("Page %d of %d", app.page + 1, t->pageCount), COL_MUTED);
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
    } else if (needsQuiz) {
        if (UiButton((Rectangle){ SCREEN_W - 320, 627, 240, 50 }, "Take the Quiz >", COL_ACCENT))
            OpenQuiz(app.tut);
    } else {
        if (UiButton((Rectangle){ SCREEN_W - 320, 627, 240, 50 }, "Start Project >", COL_GOOD_DARK))
            OpenPractice(app.tut);
        if (t->quizCount > 0 && UiButton((Rectangle){ SCREEN_W - 516, 627, 180, 50 }, "Retake Quiz", COL_PANEL_HI))
            OpenQuiz(app.tut);
    }
}

// ---------------------------------------------------------------- quiz

static void ScreenQuiz(void)
{
    const Tutorial *t = Cur();
    const QuizQuestion *q = &t->quiz[app.quizIndex];
    bool lastQuestion = (app.quizIndex == t->quizCount - 1);

    if (IsKeyPressed(KEY_ESCAPE)) { OpenLesson(app.tut, t->pageCount - 1); return; }

    DrawTopBar(TextFormat("Quiz: %s", t->title));
    TopBarRight(TextFormat("Question %d of %d", app.quizIndex + 1, t->quizCount), COL_MUTED);
    if (BackButton("< Lesson")) { OpenLesson(app.tut, t->pageCount - 1); return; }

    Rectangle panel = { 80, 88, SCREEN_W - 160, 520 };
    DrawRectangleRounded(panel, 0.04f, 8, COL_PANEL);
    float x = panel.x + 40, w = panel.width - 80;
    float y = panel.y + 30;

    y += UiWrapped(q->question, x, y, w, 27, COL_TEXT) + 6;
    if (q->code) y += UiCodeBlock(q->code, x, y, w, 17) + 16;
    else y += 10;

    // Answer cards (click, or press 1-4)
    int picked = -1;
    for (int i = 0; i < q->optionCount; i++) {
        Rectangle r = { x, y, w, 48 };
        bool open = !app.quizCorrect && !app.quizWrong[i];
        bool hover = false;
        bool clicked = open && UiClickArea(r, &hover);
        if (open && IsKeyPressed(KEY_ONE + i)) clicked = true;
        if (clicked) picked = i;

        Color fill = COL_PANEL_HI;
        if (app.quizCorrect && i == q->correct) fill = COL_GOOD_DARK;
        else if (app.quizWrong[i])              fill = COL_BAD_DARK;
        else if (hover)                         fill = ColorBrightness(COL_PANEL_HI, 0.15f);
        DrawRectangleRounded(r, 0.25f, 8, fill);

        UiText(TextFormat("%d", i + 1), r.x + 18, r.y + 13, 21, app.quizWrong[i] || (app.quizCorrect && i == q->correct) ? WHITE : COL_ACCENT);
        UiText(q->options[i], r.x + 52, r.y + 13, 21, (app.quizCorrect && i != q->correct) ? COL_MUTED : COL_TEXT);
        y += 58;
    }

    if (picked >= 0) {
        if (picked == q->correct) {
            app.quizCorrect = true;
            app.quizLastWrong = false;
        } else {
            app.quizWrong[picked] = true;
            app.quizLastWrong = true;
        }
    }

    // Feedback and next step
    float fbW = SCREEN_W - 160 - 280;
    if (app.quizCorrect) {
        UiText("Correct!", 80, 622, 21, COL_GOOD);
        UiWrapped(q->explanation, 80, 650, fbW, 17, COL_TEXT);

        bool go = IsKeyPressed(KEY_ENTER) ||
                  UiButton((Rectangle){ SCREEN_W - 320, 627, 240, 50 },
                           lastQuestion ? "Start Project >" : "Next Question >",
                           lastQuestion ? COL_GOOD_DARK : COL_ACCENT);
        if (go) {
            if (lastQuestion) {
                prog.quizPassed[app.tut] = true;
                Save();
                OpenPractice(app.tut);
            } else {
                app.quizIndex++;
                ResetQuestion();
            }
        }
    } else if (app.quizLastWrong) {
        UiText("Not quite - have another go.", 80, 640, 21, COL_BAD);
    } else {
        UiText("Pick an answer (or press 1-4).", 80, 640, 19, COL_MUTED);
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
        UiText(TextFormat("Compiling with %s and running your code...", CompilerName(t->lang)), x, y, 22, COL_ACCENT);
        return;
    }
    if (!app.hasResult) {
        UiText("Output", x, y, 22, COL_TEXT);
        float h = 36;
        if (app.notice[0]) {
            UiWrapped(app.notice, x, y + h, w, 18, COL_ACCENT);
            h += 32;
        }
        UiWrapped("Press Run & Check (or Ctrl+Enter) to compile your code and test it against the expected output.\n"
                  "Editing: Ctrl+Z undo, Ctrl+Y redo, Ctrl+A select all, Ctrl+C / Ctrl+X / Ctrl+V copy, cut and paste, "
                  "Tab / Shift+Tab indent selected lines.",
                  x, y + h, w, 17, COL_MUTED);
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
    else if (res->status[firstFail] == RUN_TIMEOUT || res->status[firstFail] == RUN_TOO_MUCH_OUTPUT)
        UiText(TextFormat("Test #%d was stopped. Check for a loop that never ends.", firstFail + 1), x, y, 22, COL_BAD);
    else if (res->status[firstFail] == RUN_CRASHED)
        UiText(TextFormat("Your program crashed on test #%d.", firstFail + 1), x, y, 22, COL_BAD);
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
        if (app.tut + 1 < TutorialCount()) {
            if (UiButton((Rectangle){ x + boxW + 24, boxY + boxH - 52, 236, 52 }, "Next Tutorial >", COL_ACCENT)) {
                SaveDraft();
                OpenLesson(app.tut + 1, 0);
            }
        } else {
            UiWrapped("You've finished every tutorial so far. More coming soon!",
                      x + boxW + 24, boxY + 24, 230, 18, COL_MUTED);
        }
    }
}

static void ScreenPractice(void)
{
    const Tutorial *t = Cur();

    // A run was requested last frame (after the "Compiling..." message was drawn)
    if (app.runPending) {
        SaveDraft();
        RunPractice(app.editor.text, t->lang, t->tests, t->testCount, &app.result);
        app.hasResult = true;
        app.runPending = false;
        app.resultScroll = 0;
        app.editor.focused = true;
        if (app.result.allPassed && !prog.completed[app.tut]) {
            prog.completed[app.tut] = true;
            Save();
        }
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
    if (ctrl && IsKeyPressed(KEY_ENTER)) app.runPending = true;
    if (IsKeyPressed(KEY_ESCAPE)) { LeavePractice(); return; }
    if (app.solutionConfirmAt > 0 && GetTime() - app.solutionConfirmAt > 3.0) app.solutionConfirmAt = 0;

    DrawTopBar(TextFormat("Project: %s", t->projectTitle));
    const char *lang = (t->lang == LANG_CPP) ? "C++" : "C";
    if (prog.completed[app.tut]) TopBarRight(TextFormat("%s  -  Completed", lang), COL_GOOD);
    else TopBarRight(lang, COL_MUTED);
    if (BackButton("< Lesson")) { LeavePractice(); return; }

    // ---- Left: the task, the hint and the buttons
    Rectangle left = { 20, 84, 380, SCREEN_H - 104 };
    DrawRectangleRounded(left, 0.03f, 8, COL_PANEL);
    UiText("Your task", left.x + 20, left.y + 18, 26, COL_TEXT);

    float bw = left.width - 40;
    Rectangle runBtn  = { left.x + 20, left.y + left.height - 116, bw, 52 };
    float smallW = (bw - 16) / 3;
    Rectangle hintBtn = { left.x + 20,                    left.y + left.height - 56, smallW, 42 };
    Rectangle solBtn  = { left.x + 20 + smallW + 8,       left.y + left.height - 56, smallW, 42 };
    Rectangle rstBtn  = { left.x + 20 + (smallW + 8) * 2, left.y + left.height - 56, smallW, 42 };

    float briefBottom = runBtn.y - 14;
    if (app.showHint && t->hint) {
        float hh = UiWrappedHeight(t->hint, bw - 28, 17) + 44;
        Rectangle hb = { left.x + 20, runBtn.y - 14 - hh, bw, hh };
        DrawRectangleRounded(hb, 0.08f, 6, (Color){ 60, 52, 30, 255 });
        UiText("Hint", hb.x + 14, hb.y + 10, 18, (Color){ 240, 200, 110, 255 });
        UiWrapped(t->hint, hb.x + 14, hb.y + 34, bw - 28, 17, COL_TEXT);
        briefBottom = hb.y - 10;
    }

    // Task text (scrolls with the mouse wheel if it is long)
    Rectangle briefArea = { left.x, left.y + 58, left.width, briefBottom - (left.y + 58) };
    float briefH = UiWrappedHeight(t->projectBrief, left.width - 40, 18);
    if (CheckCollisionPointRec(GetMousePosition(), briefArea)) app.briefScroll -= GetMouseWheelMove() * 40;
    float maxBrief = briefH - briefArea.height + 8;
    if (app.briefScroll > maxBrief) app.briefScroll = maxBrief;
    if (app.briefScroll < 0) app.briefScroll = 0;
    BeginScissorMode((int)briefArea.x, (int)briefArea.y, (int)briefArea.width, (int)briefArea.height);
    UiWrapped(t->projectBrief, left.x + 20, briefArea.y + 4 - app.briefScroll, left.width - 40, 18, COL_TEXT);
    EndScissorMode();
    if (maxBrief > 0)
        UiText("scroll for more", left.x + left.width - 20 - UiTextWidth("scroll for more", 14),
               briefArea.y + briefArea.height - 16, 14, COL_MUTED);

    if (UiButton(runBtn, app.runPending ? "Running..." : "Run & Check", COL_GOOD_DARK)) app.runPending = true;

    if (UiButton(hintBtn, app.showHint ? "Hide Hint" : "Hint", (Color){ 120, 95, 40, 255 })) {
        app.showHint = !app.showHint;
        app.solutionConfirmAt = 0;
    }

    bool confirming = app.solutionConfirmAt > 0;
    if (UiButton(solBtn, confirming ? "Sure?" : "Solution", confirming ? COL_BAD_DARK : COL_PANEL_HI)) {
        if (!confirming) {
            app.solutionConfirmAt = GetTime();
        } else {
            EditorReplaceAll(&app.editor, t->solution);
            app.editor.focused = true;
            app.hasResult = false;
            app.solutionConfirmAt = 0;
            snprintf(app.notice, sizeof(app.notice),
                     "The solution is in the editor. Read it line by line, then try it yourself - Ctrl+Z brings your own code back.");
        }
    }

    if (UiButton(rstBtn, "Reset", COL_PANEL_HI)) {
        EditorReplaceAll(&app.editor, t->starterCode);
        app.editor.focused = true;
        app.hasResult = false;
        app.solutionConfirmAt = 0;
        snprintf(app.notice, sizeof(app.notice), "Code reset to the starter code. Press Ctrl+Z to undo.");
    }

    // ---- Right: the editor and the results
    Rectangle edRect = { 420, 84, SCREEN_W - 440, 370 };
    EditorUpdate(&app.editor, edRect);
    EditorDraw(&app.editor, edRect);

    Rectangle resRect = { 420, 468, SCREEN_W - 440, SCREEN_H - 488 };
    DrawResults(resRect, t);
}

// ---------------------------------------------------------------- main

#ifndef CFD_NO_MAIN
int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_W, SCREEN_H, "C and C++ for Dummies");
    SetExitKey(KEY_NULL);   // Esc is used for "back", not quit
    SetTargetFPS(60);

    UiInit();
    EditorInit(&app.editor, 20);
    ProgressLoad(&prog, PROGRESS_FILE);
    app.screen = SCREEN_MAIN;

    while (!WindowShouldClose() && !app.quit) {
        UiBeginFrame();
        BeginDrawing();
        ClearBackground(COL_BG);

        switch (app.screen) {
            case SCREEN_MAIN:      ScreenMainMenu();  break;
            case SCREEN_TUTORIALS: ScreenTutorials(); break;
            case SCREEN_LESSON:    ScreenLesson();    break;
            case SCREEN_QUIZ:      ScreenQuiz();      break;
            case SCREEN_PRACTICE:  ScreenPractice();  break;
        }

        EndDrawing();
        UiEndFrame();
    }

    if (app.screen == SCREEN_PRACTICE) SaveDraft();
    Save();

    EditorFree(&app.editor);
    UiShutdown();
    CloseWindow();
    return 0;
}
#endif
