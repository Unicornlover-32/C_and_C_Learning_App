// C and C++ for Dummies - a raylib app that teaches C and C++ step by step.
#include "raylib.h"
#include "ui.h"
#include "editor.h"
#include "tutorials.h"
#include "runner.h"
#include "progress.h"
#include "projects.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    SCREEN_MAIN,
    SCREEN_TUTORIALS,
    SCREEN_LESSON,
    SCREEN_QUIZ,
    SCREEN_PRACTICE,
    SCREEN_PROJECTS,
    SCREEN_PROJECT,
} Screen;

typedef struct {
    Screen screen;
    bool   quit;

    int    tut;                 // current tutorial
    int    page;                // current lesson page
    float  lessonScroll;        // for lesson pages taller than the panel
    float  listScroll;
    float  projListScroll;

    // Quiz
    int    quizIndex;
    bool   quizWrong[MAX_OPTIONS];
    bool   quizCorrect;
    bool   quizLastWrong;

    // Bigger projects
    int    proj;                // current project
    int    step;                // step being worked on
    bool   inProject;           // the practice screen is on a project step, not a tutorial
    double restartConfirmAt;    // time of the first "Start Over" click, 0 = not asked

    // Practice screen
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
static const Project  *CurProject(void) { return GetProject(app.proj); }

// What the practice screen is working on: a tutorial's project or one step of a bigger project.
typedef struct {
    Language        lang;
    const char     *brief;
    const char     *hint;
    const char     *solution;
    const char     *resetCode;   // what the Reset button puts back
    const TestCase *tests;
    int             testCount;
} Task;

static Task CurTask(void)
{
    if (app.inProject) {
        const Project *p = CurProject();
        const ProjectStep *s = &p->steps[app.step];
        // Reset goes back to the start of this step, which is where the last step's solution left off
        const char *reset = app.step > 0 ? p->steps[app.step - 1].solution : p->starterCode;
        return (Task){ p->lang, s->brief, s->hint, s->solution, reset, s->tests, s->testCount };
    }
    const Tutorial *t = Cur();
    return (Task){ t->lang, t->projectBrief, t->hint, t->solution, t->starterCode, t->tests, t->testCount };
}

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

static bool ProjectDone(int i) { return prog.projSteps[i] >= GetProject(i)->stepCount; }

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

// The big panel used by lessons, quizzes and project overviews. It fills the screen
// but stops growing on very wide monitors, so lines of text stay easy to read.
static Rectangle ContentPanel(void)
{
    float w = SCREEN_W - 160.0f;
    if (w > 1400) w = 1400;
    return (Rectangle){ (SCREEN_W - w) / 2, 88, w, SCREEN_H - 200.0f };
}

static float NavY(void) { return SCREEN_H - 93.0f; }   // the row of buttons under ContentPanel()

static void SetPage(int page)
{
    app.page = page;
    app.lessonScroll = 0;
}

static void OpenLesson(int tut, int page)
{
    app.tut = tut;
    SetPage(page);
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
    if (app.inProject) {
        memcpy(prog.projDrafts[app.proj], app.editor.text, (size_t)app.editor.len + 1);
        prog.projHasDraft[app.proj] = true;
    } else {
        memcpy(prog.drafts[app.tut], app.editor.text, (size_t)app.editor.len + 1);
        prog.hasDraft[app.tut] = true;
    }
    Save();
}

// Clears the results, hint and notices, but leaves the code in the editor alone.
static void ResetPracticePanel(void)
{
    app.editor.focused = true;
    app.hasResult = false;
    app.runPending = false;
    app.resultScroll = 0;
    app.briefScroll = 0;
    app.showHint = false;
    app.solutionConfirmAt = 0;
    app.notice[0] = '\0';
}

static void OpenPractice(int tut)
{
    app.tut = tut;
    app.inProject = false;
    EditorSetText(&app.editor, prog.hasDraft[tut] ? prog.drafts[tut] : GetTutorial(tut)->starterCode);
    ResetPracticePanel();
    app.screen = SCREEN_PRACTICE;
}

static void OpenProjectInfo(int proj)
{
    app.proj = proj;
    app.restartConfirmAt = 0;
    app.screen = SCREEN_PROJECT;
}

// Opens the editor on one step of a project. The code is shared by every step.
static void OpenProjectStep(int proj, int step)
{
    const Project *p = GetProject(proj);
    if (step >= p->stepCount) step = p->stepCount - 1;
    app.proj = proj;
    app.step = step;
    app.inProject = true;
    EditorSetText(&app.editor, prog.projHasDraft[proj] ? prog.projDrafts[proj] : p->starterCode);
    ResetPracticePanel();
    app.screen = SCREEN_PRACTICE;
}

// Moves to another step of the open project without touching the code.
static void GoToStep(int step)
{
    SaveDraft();
    app.step = step;
    ResetPracticePanel();
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
    if (app.inProject) OpenProjectInfo(app.proj);
    else OpenLesson(app.tut, Cur()->pageCount - 1);
}

// ---------------------------------------------------------------- main menu

static void ScreenMainMenu(void)
{
    float cx = SCREEN_W / 2.0f;
    float top = (SCREEN_H - BASE_H) / 2.0f;   // keeps the menu centred on big screens
    int next = NextTutorial();

    // Faint code in the background for decoration
    const char *deco = "#include <stdio.h>\n\nint main(void)\n{\n    printf(\"Hello!\\n\");\n    return 0;\n}";
    UiCode(deco, 70, SCREEN_H - 250, 20, (Color){ 255, 255, 255, 18 }, 0);
    UiCode("for (int i = 0; i < 10; i++)\n{\n    learn(i);\n}", SCREEN_W - 380, 90, 20, (Color){ 255, 255, 255, 18 }, 0);

    UiTextCentered("{ }", cx, top + 85, 64, COL_ACCENT);
    UiTextCentered("C and C++ for Dummies", cx, top + 170, 64, COL_TEXT);
    UiTextCentered("Learn C and C++ step by step, following the Advanced Programming lecture notes.", cx, top + 248, 24, COL_MUTED);

    // Progress bar
    int done = CountCompleted();
    Rectangle bar = { cx - 200, top + 306, 400, 10 };
    DrawRectangleRounded(bar, 1.0f, 8, COL_PANEL_HI);
    if (done > 0) {
        Rectangle fill = bar;
        fill.width = bar.width * done / (float)TutorialCount();
        DrawRectangleRounded(fill, 1.0f, 8, COL_GOOD);
    }
    UiTextCentered(TextFormat("%d of %d tutorials completed", done, TutorialCount()), cx, top + 326, 18, COL_MUTED);
    if (next >= 0 && AnyProgress())
        UiTextCentered(TextFormat("Next up: %d. %s", next + 1, GetTutorial(next)->title), cx, top + 352, 18, COL_TEXT);
    else if (next < 0)
        UiTextCentered("Every tutorial complete - brilliant work!", cx, top + 352, 18, COL_GOOD);

    float y = top + 390;
    if (next >= 0) {
        const char *label = AnyProgress() ? "Continue Learning" : "Start Learning";
        if (UiButton((Rectangle){ cx - 160, y, 320, 60 }, label, COL_ACCENT)) StartTutorial(next);
        y += 74;
        if (UiButton((Rectangle){ cx - 160, y, 320, 50 }, "View Tutorials", COL_PANEL_HI)) app.screen = SCREEN_TUTORIALS;
    } else {
        if (UiButton((Rectangle){ cx - 160, y, 320, 60 }, "View Tutorials", COL_ACCENT)) app.screen = SCREEN_TUTORIALS;
        y += 10;
    }
    y += 64;
    if (UiButton((Rectangle){ cx - 160, y, 320, 50 }, "Projects", COL_PANEL_HI)) app.screen = SCREEN_PROJECTS;
    y += 64;
    if (UiButton((Rectangle){ cx - 160, y, 320, 50 }, "Exit", COL_PANEL_HI)) app.quit = true;

    UiTextCentered("F11 switches between full screen and a window", cx, SCREEN_H - 36, 16, COL_MUTED);
}

// ---------------------------------------------------------------- tutorial list

static void ScreenTutorials(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_MAIN; return; }

    const float top = 66, cardH = 96, gap = 14, headH = 64;
    int count = TutorialCount();

    float contentH = 24;
    for (int i = 0; i < count; i++) {
        if (GetTutorial(i)->section) contentH += headH;
        contentH += cardH + gap;
    }
    float viewH = SCREEN_H - top;
    float maxScroll = contentH - viewH;
    if (maxScroll < 0) maxScroll = 0;

    app.listScroll -= GetMouseWheelMove() * 50;
    if (app.listScroll < 0) app.listScroll = 0;
    if (app.listScroll > maxScroll) app.listScroll = maxScroll;

    float cardW = SCREEN_W - 160.0f;
    if (cardW > 1400) cardW = 1400;
    float cardX = (SCREEN_W - cardW) / 2;

    BeginScissorMode(0, (int)top, SCREEN_W, (int)viewH);
    float y = top + 16 - app.listScroll;
    for (int i = 0; i < count; i++) {
        const Tutorial *t = GetTutorial(i);

        // Section heading at the start of each section
        if (t->section) {
            UiText(t->section, cardX + 4, y + 14, 34, COL_ACCENT);
            if (t->sectionBlurb)
                UiText(t->sectionBlurb, cardX + 4 + UiTextWidth(t->section, 34) + 18, y + 26, 19, COL_MUTED);
            y += headH;
        }

        Rectangle card = { cardX, y, cardW, cardH };
        bool hover = false;
        bool clicked = GetMouseY() > top && UiClickArea(card, &hover);

        DrawRectangleRounded(card, 0.18f, 8, hover ? COL_PANEL_HI : COL_PANEL);

        float circleX = card.x + 52, circleY = card.y + cardH / 2;
        DrawCircle((int)circleX, (int)circleY, 26, prog.completed[i] ? COL_GOOD_DARK : COL_ACCENT);
        UiTextCentered(TextFormat("%d", i + 1), circleX, circleY - 13, 26, WHITE);

        UiText(t->title, card.x + 100, card.y + 18, 26, COL_TEXT);
        if (t->lecture) {
            const char *lecture = TextFormat("Lecture: %s", t->lecture);
            UiText(lecture, card.x + card.width - 30 - UiTextWidth(lecture, 15), card.y + 12, 15, COL_ACCENT);
        }
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
    if (IsKeyPressed(KEY_RIGHT) && app.page < last) SetPage(app.page + 1);
    if (IsKeyPressed(KEY_LEFT) && app.page > 0) SetPage(app.page - 1);

    const LessonPage *p = &t->pages[app.page];

    DrawTopBar(TextFormat("%d. %s", app.tut + 1, t->title));
    if (t->lecture) TopBarRight(TextFormat("Lecture: %s   -   Page %d of %d", t->lecture, app.page + 1, t->pageCount), COL_MUTED);
    else TopBarRight(TextFormat("Page %d of %d", app.page + 1, t->pageCount), COL_MUTED);
    if (BackButton("< Tutorials")) { app.screen = SCREEN_TUTORIALS; return; }

    // Content
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
    if (CheckCollisionPointRec(GetMousePosition(), panel)) app.lessonScroll -= GetMouseWheelMove() * 50;
    if (IsKeyDown(KEY_DOWN)) app.lessonScroll += 10;
    if (IsKeyDown(KEY_UP)) app.lessonScroll -= 10;
    if (app.lessonScroll > maxScroll) app.lessonScroll = maxScroll;
    if (app.lessonScroll < 0) app.lessonScroll = 0;

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
    float y = panel.y + 32 - app.lessonScroll;
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
        const char *more = app.lessonScroll < maxScroll ? "scroll for more  v" : "^  top of page: scroll up";
        UiText(more, right - 24 - UiTextWidth(more, 16), panel.y + panel.height - 26, 16, COL_ACCENT);
    }

    // Page dots
    float dotsW = t->pageCount * 22.0f;
    for (int i = 0; i < t->pageCount; i++)
        DrawCircle((int)(SCREEN_W / 2 - dotsW / 2 + 11 + i * 22), (int)navY + 25, 6, i == app.page ? COL_ACCENT : COL_PANEL_HI);

    // Navigation
    if (app.page > 0 && UiButton((Rectangle){ left, navY, 180, 50 }, "< Previous", COL_PANEL_HI))
        SetPage(app.page - 1);

    if (app.page < last) {
        if (UiButton((Rectangle){ right - 180, navY, 180, 50 }, "Next >", COL_ACCENT)) SetPage(app.page + 1);
    } else if (needsQuiz) {
        if (UiButton((Rectangle){ right - 240, navY, 240, 50 }, "Take the Quiz >", COL_ACCENT))
            OpenQuiz(app.tut);
    } else {
        if (UiButton((Rectangle){ right - 240, navY, 240, 50 }, "Start Project >", COL_GOOD_DARK))
            OpenPractice(app.tut);
        if (t->quizCount > 0 && UiButton((Rectangle){ right - 436, navY, 180, 50 }, "Retake Quiz", COL_PANEL_HI))
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

    Rectangle panel = ContentPanel();
    float left = panel.x, right = panel.x + panel.width, navY = NavY();
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
    float fbW = panel.width - 280;
    if (app.quizCorrect) {
        UiText("Correct!", left, navY - 5, 21, COL_GOOD);
        UiWrapped(q->explanation, left, navY + 23, fbW, 17, COL_TEXT);

        bool go = IsKeyPressed(KEY_ENTER) ||
                  UiButton((Rectangle){ right - 240, navY, 240, 50 },
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
        UiText("Not quite - have another go.", left, navY + 13, 21, COL_BAD);
    } else {
        UiText("Pick an answer (or press 1-4).", left, navY + 13, 19, COL_MUTED);
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

static void DrawResults(Rectangle r, const Task *task)
{
    DrawRectangleRounded(r, 0.05f, 8, COL_PANEL);
    float x = r.x + 18, y = r.y + 14, w = r.width - 36;

    if (app.runPending) {
        UiText(TextFormat("Compiling with %s and running your code...", CompilerName(task->lang)), x, y, 22, COL_ACCENT);
        return;
    }
    if (!app.hasResult) {
        UiText("Output", x, y, 22, COL_TEXT);
        float h = 36;
        if (app.notice[0]) {
            h += UiWrapped(app.notice, x, y + h, w, 18, COL_ACCENT) + 8;
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

    bool lastStep = app.inProject && app.step == CurProject()->stepCount - 1;
    if (res->allPassed) {
        const char *what = !app.inProject ? "tutorial" : lastStep ? "project" : "step";
        UiText(TextFormat("All %d tests passed - %s complete!", res->testCount, what), x, y, 22, COL_GOOD);
    }
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
        const TestCase *tc = &task->tests[firstFail];
        float colW = (w - 24) / 3;
        DrawOutputBox(TextFormat("Test #%d input", firstFail + 1), tc->input ? tc->input : "",
                      (Rectangle){ x, boxY, colW, boxH });
        DrawOutputBox("Expected output", tc->expected, (Rectangle){ x + colW + 12, boxY, colW, boxH });
        DrawOutputBox("Your output", res->actual[firstFail], (Rectangle){ x + (colW + 12) * 2, boxY, colW, boxH });
        return;
    }

    float boxW = w - 260;
    Rectangle nextBtn = { x + boxW + 24, boxY + boxH - 52, 236, 52 };
    DrawOutputBox("Your output (test #1)", res->actual[0], (Rectangle){ x, boxY, boxW, boxH });

    if (app.inProject) {
        if (!lastStep) {
            UiWrapped("Your code carries on into the next step.", nextBtn.x, boxY + 24, nextBtn.width, 17, COL_MUTED);
            if (UiButton(nextBtn, "Next Step >", COL_ACCENT)) {
                GoToStep(app.step + 1);
                snprintf(app.notice, sizeof(app.notice),
                         "Step %d: keep building on your code from the last step. The new task is on the left.", app.step + 1);
            }
        } else {
            UiWrapped("You built the whole project - well done! Try adding a feature of your own.",
                      nextBtn.x, boxY + 24, nextBtn.width, 17, COL_MUTED);
            if (UiButton(nextBtn, "All Projects", COL_ACCENT)) {
                SaveDraft();
                app.screen = SCREEN_PROJECTS;
            }
        }
    } else if (app.tut + 1 < TutorialCount()) {
        if (UiButton(nextBtn, "Next Tutorial >", COL_ACCENT)) {
            SaveDraft();
            OpenLesson(app.tut + 1, 0);
        }
    } else {
        UiWrapped("You've finished every tutorial. Ready for something bigger? Try the Projects on the main menu.",
                  x + boxW + 24, boxY + 24, 230, 18, COL_MUTED);
    }
}

// The step circles shown at the top of the task panel in a project. Finished steps
// and the next one can be clicked to switch between them.
static void DrawStepPicker(float right, float y)
{
    const Project *p = CurProject();
    float r = 13, gap = 32;
    for (int i = p->stepCount - 1; i >= 0; i--) {
        float cx = right - r - (p->stepCount - 1 - i) * gap;
        bool unlocked = i <= prog.projSteps[app.proj];
        bool hover = false;
        Rectangle hit = { cx - r, y - r, r * 2, r * 2 };
        bool clicked = unlocked && i != app.step && UiClickArea(hit, &hover);

        Color fill = COL_PANEL_HI;
        if (i < prog.projSteps[app.proj]) fill = COL_GOOD_DARK;
        if (i == app.step)                fill = COL_ACCENT;
        if (hover)                        fill = ColorBrightness(fill, 0.2f);
        DrawCircle((int)cx, (int)y, r, fill);
        UiTextCentered(TextFormat("%d", i + 1), cx, y - 9, 18, unlocked ? WHITE : COL_MUTED);

        if (clicked) GoToStep(i);
    }
}

static void ScreenPractice(void)
{
    Task task = CurTask();

    // A run was requested last frame (after the "Compiling..." message was drawn)
    if (app.runPending) {
        SaveDraft();
        RunPractice(app.editor.text, task.lang, task.tests, task.testCount, &app.result);
        app.hasResult = true;
        app.runPending = false;
        app.resultScroll = 0;
        app.editor.focused = true;
        if (app.result.allPassed) {
            if (app.inProject) {
                if (prog.projSteps[app.proj] < app.step + 1) {
                    prog.projSteps[app.proj] = app.step + 1;
                    Save();
                }
            } else if (!prog.completed[app.tut]) {
                prog.completed[app.tut] = true;
                Save();
            }
        }
    }

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);
    if (ctrl && IsKeyPressed(KEY_ENTER)) app.runPending = true;
    if (IsKeyPressed(KEY_ESCAPE)) { LeavePractice(); return; }
    if (app.solutionConfirmAt > 0 && GetTime() - app.solutionConfirmAt > 3.0) app.solutionConfirmAt = 0;

    const char *lang = (task.lang == LANG_CPP) ? "C++" : "C";
    bool done;
    if (app.inProject) {
        DrawTopBar(TextFormat("Project: %s", CurProject()->title));
        done = ProjectDone(app.proj);
    } else {
        DrawTopBar(TextFormat("Project: %s", Cur()->projectTitle));
        done = prog.completed[app.tut];
    }
    if (done) TopBarRight(TextFormat("%s  -  Completed", lang), COL_GOOD);
    else TopBarRight(lang, COL_MUTED);
    if (BackButton(app.inProject ? "< Overview" : "< Lesson")) { LeavePractice(); return; }

    // ---- Left: the task, the hint and the buttons
    float leftW = SCREEN_W >= 1600 ? 460 : 380;
    Rectangle left = { 20, 84, leftW, SCREEN_H - 104.0f };
    DrawRectangleRounded(left, 0.03f, 8, COL_PANEL);

    float briefTop = left.y + 58;
    if (app.inProject) {
        const Project *p = CurProject();
        UiText(TextFormat("Step %d of %d", app.step + 1, p->stepCount), left.x + 20, left.y + 18, 26, COL_TEXT);
        DrawStepPicker(left.x + left.width - 20, left.y + 34);
        UiText(p->steps[app.step].title, left.x + 20, left.y + 56, 21, COL_ACCENT);
        briefTop = left.y + 92;
    } else {
        UiText("Your task", left.x + 20, left.y + 18, 26, COL_TEXT);
    }

    float bw = left.width - 40;
    Rectangle runBtn  = { left.x + 20, left.y + left.height - 116, bw, 52 };
    float smallW = (bw - 16) / 3;
    Rectangle hintBtn = { left.x + 20,                    left.y + left.height - 56, smallW, 42 };
    Rectangle solBtn  = { left.x + 20 + smallW + 8,       left.y + left.height - 56, smallW, 42 };
    Rectangle rstBtn  = { left.x + 20 + (smallW + 8) * 2, left.y + left.height - 56, smallW, 42 };

    float briefBottom = runBtn.y - 14;
    if (app.showHint && task.hint) {
        float hh = UiWrappedHeight(task.hint, bw - 28, 17) + 44;
        Rectangle hb = { left.x + 20, runBtn.y - 14 - hh, bw, hh };
        DrawRectangleRounded(hb, 0.08f, 6, (Color){ 60, 52, 30, 255 });
        UiText("Hint", hb.x + 14, hb.y + 10, 18, (Color){ 240, 200, 110, 255 });
        UiWrapped(task.hint, hb.x + 14, hb.y + 34, bw - 28, 17, COL_TEXT);
        briefBottom = hb.y - 10;
    }

    // Task text (scrolls with the mouse wheel if it is long)
    Rectangle briefArea = { left.x, briefTop, left.width, briefBottom - briefTop };
    float briefH = UiWrappedHeight(task.brief, left.width - 40, 18);
    if (CheckCollisionPointRec(GetMousePosition(), briefArea)) app.briefScroll -= GetMouseWheelMove() * 40;
    float maxBrief = briefH - briefArea.height + 8;
    if (app.briefScroll > maxBrief) app.briefScroll = maxBrief;
    if (app.briefScroll < 0) app.briefScroll = 0;
    BeginScissorMode((int)briefArea.x, (int)briefArea.y, (int)briefArea.width, (int)briefArea.height);
    UiWrapped(task.brief, left.x + 20, briefArea.y + 4 - app.briefScroll, left.width - 40, 18, COL_TEXT);
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
            EditorReplaceAll(&app.editor, task.solution);
            app.editor.focused = true;
            app.hasResult = false;
            app.solutionConfirmAt = 0;
            snprintf(app.notice, sizeof(app.notice),
                     "The solution is in the editor. Read it line by line, then try it yourself - Ctrl+Z brings your own code back.");
        }
    }

    if (UiButton(rstBtn, "Reset", COL_PANEL_HI)) {
        EditorReplaceAll(&app.editor, task.resetCode);
        app.editor.focused = true;
        app.hasResult = false;
        app.solutionConfirmAt = 0;
        if (app.inProject && app.step > 0)
            snprintf(app.notice, sizeof(app.notice), "Code reset to the start of this step (the finished code from step %d). Press Ctrl+Z to undo.", app.step);
        else
            snprintf(app.notice, sizeof(app.notice), "Code reset to the starter code. Press Ctrl+Z to undo.");
    }

    // ---- Right: the editor and the results
    float rightX = left.x + left.width + 20;
    float edH = (SCREEN_H - 118.0f) * 0.62f;
    Rectangle edRect = { rightX, 84, SCREEN_W - rightX - 20, edH };
    EditorUpdate(&app.editor, edRect);
    EditorDraw(&app.editor, edRect);

    float resY = edRect.y + edH + 14;
    Rectangle resRect = { rightX, resY, edRect.width, SCREEN_H - 20 - resY };
    DrawResults(resRect, &task);
}

// ---------------------------------------------------------------- project list

static void ScreenProjects(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_MAIN; return; }

    const float top = 66, cardH = 96, gap = 14, headH = 64, introH = 70;
    int count = ProjectCount();

    float contentH = 24 + introH;
    for (int i = 0; i < count; i++) {
        if (i == 0 || GetProject(i)->lang != GetProject(i - 1)->lang) contentH += headH;
        contentH += cardH + gap;
    }
    float viewH = SCREEN_H - top;
    float maxScroll = contentH - viewH;
    if (maxScroll < 0) maxScroll = 0;

    app.projListScroll -= GetMouseWheelMove() * 50;
    if (app.projListScroll < 0) app.projListScroll = 0;
    if (app.projListScroll > maxScroll) app.projListScroll = maxScroll;

    float cardW = SCREEN_W - 160.0f;
    if (cardW > 1400) cardW = 1400;
    float cardX = (SCREEN_W - cardW) / 2;

    BeginScissorMode(0, (int)top, SCREEN_W, (int)viewH);
    float y = top + 22 - app.projListScroll;
    UiWrapped("Bigger programs, built one step at a time - the kind every coding course has you make. "
              "Each step adds a feature, and your code carries on from one step to the next.",
              cardX + 4, y, cardW - 8, 19, COL_MUTED);
    y += introH;

    for (int i = 0; i < count; i++) {
        const Project *p = GetProject(i);

        if (i == 0 || p->lang != GetProject(i - 1)->lang) {
            bool isC = (p->lang == LANG_C);
            const char *name = isC ? "C Projects" : "C++ Projects";
            UiText(name, cardX + 4, y + 14, 34, COL_ACCENT);
            UiText(isC ? "Use what you learned in the C tutorials." : "For after the C++ tutorials.",
                   cardX + 4 + UiTextWidth(name, 34) + 18, y + 26, 19, COL_MUTED);
            y += headH;
        }

        Rectangle card = { cardX, y, cardW, cardH };
        bool hover = false;
        bool clicked = GetMouseY() > top && UiClickArea(card, &hover);

        DrawRectangleRounded(card, 0.18f, 8, hover ? COL_PANEL_HI : COL_PANEL);

        float circleX = card.x + 52, circleY = card.y + cardH / 2;
        DrawCircle((int)circleX, (int)circleY, 26, ProjectDone(i) ? COL_GOOD_DARK : COL_ACCENT);
        UiTextCentered(TextFormat("%d", i + 1), circleX, circleY - 13, 26, WHITE);

        UiText(p->title, card.x + 100, card.y + 18, 26, COL_TEXT);
        UiText(p->summary, card.x + 100, card.y + 56, 18, COL_MUTED);

        const char *status;
        Color statusCol;
        int stepsDone = prog.projSteps[i];
        if (ProjectDone(i))    { status = "Completed"; statusCol = COL_GOOD; }
        else if (stepsDone > 0 || prog.projHasDraft[i]) {
            status = TextFormat("Step %d of %d  >", stepsDone + 1, p->stepCount);
            statusCol = COL_ACCENT;
        }
        else { status = TextFormat("%d steps  >", p->stepCount); statusCol = hover ? COL_ACCENT : COL_MUTED; }
        UiText(status, card.x + card.width - 30 - UiTextWidth(status, 20), card.y + 20, 20, statusCol);

        if (clicked) OpenProjectInfo(i);
        y += cardH + gap;
    }
    EndScissorMode();

    DrawTopBar("Projects");
    if (BackButton("< Home")) app.screen = SCREEN_MAIN;
}

// ---------------------------------------------------------------- project overview

static void ScreenProjectInfo(void)
{
    const Project *p = CurProject();
    int stepsDone = prog.projSteps[app.proj];
    bool started = stepsDone > 0 || prog.projHasDraft[app.proj];

    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = SCREEN_PROJECTS; return; }
    if (app.restartConfirmAt > 0 && GetTime() - app.restartConfirmAt > 3.0) app.restartConfirmAt = 0;

    DrawTopBar(TextFormat("Project: %s", p->title));
    TopBarRight(p->lang == LANG_CPP ? "C++" : "C", COL_MUTED);
    if (BackButton("< Projects")) { app.screen = SCREEN_PROJECTS; return; }

    Rectangle panel = ContentPanel();
    float left = panel.x, right = panel.x + panel.width, navY = NavY();
    DrawRectangleRounded(panel, 0.04f, 8, COL_PANEL);
    float x = panel.x + 40, w = panel.width - 80;
    float y = panel.y + 32;

    UiText(p->title, x, y, 36, COL_TEXT);
    y += 60;
    y += UiWrapped(p->intro, x, y, w, 21, COL_TEXT) + 6;
    if (p->lecture)
        y += UiWrapped(TextFormat("Based on the lecture: %s", p->lecture), x, y, w, 18, COL_ACCENT) + 2;
    y += UiWrapped(TextFormat("Best after these tutorials: %s", p->needs), x, y, w, 18, COL_MUTED) + 18;

    // The steps: finished ones and the next one can be opened directly
    for (int i = 0; i < p->stepCount; i++) {
        Rectangle row = { x, y, w, 44 };
        bool unlocked = i <= stepsDone && i < p->stepCount;
        bool hover = false;
        bool clicked = unlocked && UiClickArea(row, &hover);
        if (hover) DrawRectangleRounded(row, 0.3f, 8, COL_PANEL_HI);

        Color circle = COL_PANEL_HI;
        if (i < stepsDone)       circle = COL_GOOD_DARK;
        else if (i == stepsDone) circle = COL_ACCENT;
        DrawCircle((int)(row.x + 24), (int)(row.y + 22), 15, circle);
        UiTextCentered(TextFormat("%d", i + 1), row.x + 24, row.y + 12, 19, unlocked ? WHITE : COL_MUTED);
        UiText(p->steps[i].title, row.x + 54, row.y + 11, 21, unlocked ? COL_TEXT : COL_MUTED);

        const char *status = i < stepsDone ? "Done" : i == stepsDone ? "Up next" : "";
        UiText(status, row.x + row.width - 16 - UiTextWidth(status, 18), row.y + 13, 18,
               i < stepsDone ? COL_GOOD : COL_ACCENT);

        if (clicked) { OpenProjectStep(app.proj, i); return; }
        y += 50;
    }

    // Buttons
    const char *label;
    if (ProjectDone(app.proj)) label = "Open Project >";
    else if (started)          label = TextFormat("Continue: Step %d >", stepsDone + 1);
    else                       label = "Start Project >";
    if (UiButton((Rectangle){ right - 260, navY, 260, 50 }, label, ProjectDone(app.proj) ? COL_ACCENT : COL_GOOD_DARK)) {
        OpenProjectStep(app.proj, stepsDone);
        return;
    }

    if (started) {
        bool confirming = app.restartConfirmAt > 0;
        if (UiButton((Rectangle){ left, navY, 200, 50 }, confirming ? "Sure? Click again" : "Start Over",
                     confirming ? COL_BAD_DARK : COL_PANEL_HI)) {
            if (!confirming) {
                app.restartConfirmAt = GetTime();
            } else {
                prog.projSteps[app.proj] = 0;
                prog.projHasDraft[app.proj] = false;
                prog.projDrafts[app.proj][0] = '\0';
                app.restartConfirmAt = 0;
                Save();
            }
        }
        if (confirming)
            UiText("This deletes your code for this project.", left + 216, navY + 15, 18, COL_BAD);
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
    EditorInit(&app.editor, 20);
    ProgressLoad(&prog, PROGRESS_FILE);
    app.screen = SCREEN_MAIN;

    while (!WindowShouldClose() && !app.quit) {
        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        UiBeginFrame();
        BeginDrawing();
        ClearBackground(COL_BG);

        switch (app.screen) {
            case SCREEN_MAIN:      ScreenMainMenu();    break;
            case SCREEN_TUTORIALS: ScreenTutorials();   break;
            case SCREEN_LESSON:    ScreenLesson();      break;
            case SCREEN_QUIZ:      ScreenQuiz();        break;
            case SCREEN_PRACTICE:  ScreenPractice();    break;
            case SCREEN_PROJECTS:  ScreenProjects();    break;
            case SCREEN_PROJECT:   ScreenProjectInfo(); break;
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
