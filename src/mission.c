// The mission screen: the brief and the code editor on the left, and the mission's 3D
// level on the right, played from the program's real output (and, for the games, the
// game library's trace) for each test, with the expected and actual output underneath.
// Every mission plays as a 3D level: a tutorial's own project and its Bug Hunter round,
// the Factory Contracts (the bigger, multi-step projects), the bosses, the Robot Rescue
// and Memory Heist side quests, and the chess puzzles.
#include "app.h"
#include "editor.h"
#include "game/levels.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    GameMode        mode;
    Language        lang;
    const char     *title;
    const char     *brief;
    const char     *api;
    const char     *prelude;
    const char     *hint;
    const char     *solution;
    const char     *resetCode;
    const TestCase *tests;
    int             testCount;
    int             parLines;
    int             timeLimit;
} Task;

static Editor    editor;
static RunResult result;

static struct {
    int    mission;              // registry index; -1 on a contract step
    int    proj, step;           // the contract step, when inContract
    bool   inContract;

    bool   hasResult, runPending;
    bool   showHint;
    double solutionConfirmAt;
    float  briefScroll, logScroll;
    char   notice[220];

    // this attempt, for the stars
    bool   fresh, usedHelp;
    int    runs;


    // Bug Hunter
    double startTime;
    bool   timeUp, squashed;
    double squashedAt;
    int    bugAt[MAX_BUGS];      // where each bug was last seen

    // the mission-complete card
    bool          celebrate, burst;
    double        celebrateAt;
    unsigned char before, earned;
    int           xp;

    // 3D levels
    bool   details;              // the plain text results instead of the scene
    bool   bigView;              // the scene fills the screen (the code is one click away)
} ms = { .mission = -1 };

static Rectangle editorRect;     // where the editor was drawn this frame
static Rectangle levelView;      // where the 3D level was drawn this frame

// ---------------------------------------------------------------- the task

static const MissionInfo *Cur(void) { return GetMission(ms.mission); }

static Task CurTask(void)
{
    if (ms.inContract) {
        const Project *p = GetProject(ms.proj);
        const ProjectStep *s = &p->steps[ms.step];
        const char *reset = ms.step > 0 ? p->steps[ms.step - 1].solution : p->starterCode;
        return (Task){ MODE_FACTORY, p->lang, p->title, s->brief, NULL, NULL, s->hint, s->solution, reset,
                       s->tests, s->testCount, 0, 0 };
    }
    const MissionInfo *m = Cur();
    return (Task){ m->mode, m->lang, m->title, m->brief, m->api, m->prelude, m->hint, m->solution, m->starter,
                   m->tests, m->testCount, m->parLines, m->timeLimit };
}

static bool IsGame(GameMode mode) { return mode == MODE_ROBOT || mode == MODE_HEIST || mode == MODE_CHESS; }

// The 3D level this mission plays as: a level's own project, and its Bug Hunter round
// (same tests, same output), play that level; every other program (the Factory Contracts,
// the factory boss) plays the Assembly Line, which reads any output line by line
static const LevelDef *CurLevel(void)
{
    if (ms.inContract) return &LEVEL_FACTORY;
    if (ms.mission < 0) return NULL;
    const MissionInfo *m = Cur();
    if (m->tut >= 0 && ((m->role == ROLE_MAIN && m->mode == MODE_FACTORY) || m->mode == MODE_BUGS)) {
        const LevelDef *lv = LevelFor(GetTutorial(m->tut)->id);
        if (lv) return lv;
    }
    if (m->mode == MODE_FACTORY || m->mode == MODE_BUGS) return &LEVEL_FACTORY;
    if (m->mode == MODE_ROBOT) return &LEVEL_ROBOT;
    if (m->mode == MODE_HEIST) return &LEVEL_HEIST;
    if (m->mode == MODE_CHESS) return &LEVEL_CHESS;
    return NULL;
}

void MissionInit(void) { EditorInit(&editor, 20); }
void MissionShutdown(void) { EditorFree(&editor); }

static void SaveDraft(void)
{
    if (ms.inContract) {
        memcpy(prog.projDrafts[ms.proj], editor.text, (size_t)editor.len + 1);
        prog.projHasDraft[ms.proj] = true;
    } else if (ms.mission >= 0 && Cur()->mode != MODE_BUGS) {
        memcpy(prog.drafts[ms.mission], editor.text, (size_t)editor.len + 1);
        prog.hasDraft[ms.mission] = true;
    }
    SaveProgress();
}

static void ResetPanel(void)
{
    editor.focused = true;
    ms.hasResult = false;
    ms.runPending = false;
    ms.showHint = false;
    ms.solutionConfirmAt = 0;
    ms.briefScroll = 0;
    ms.logScroll = 0;
    ms.notice[0] = '\0';
    ms.celebrate = false;
    ms.burst = false;
    ms.timeUp = false;
    ms.squashed = false;
    ms.details = false;
    ms.bigView = false;
    ms.startTime = GetTime();
    for (int i = 0; i < MAX_BUGS; i++) ms.bugAt[i] = -1;
    Task t = CurTask();
    if (t.mode == MODE_CHESS) ChessSetStyle(prog.chessPieces, prog.chessBoard);
    PlaybackOpen(CurLevel(), t.tests, t.testCount);
    if (!ms.inContract && t.mode == MODE_BUGS)
        for (int i = 0; i < Cur()->bugCount; i++) ms.bugAt[i] = Cur()->bugs[i].bugLine;
}

static void StartAttempt(bool fresh)
{
    ms.fresh = fresh;
    ms.usedHelp = false;
    ms.runs = 0;
}

void OpenMission(int mission)
{
    ms.mission = mission;
    ms.inContract = false;
    const MissionInfo *m = Cur();
    bool draft = (m->mode != MODE_BUGS) && prog.hasDraft[mission];
    EditorSetText(&editor, draft ? prog.drafts[mission] : m->starter);
    ResetPanel();
    StartAttempt(!draft);
    if (draft && StarCount(prog.stars[mission]) < 3)
        snprintf(ms.notice, sizeof(ms.notice), "Your saved code is back. To go for every star, press Reset and start fresh.");
    app.screen = SCREEN_MISSION;
}

void OpenContractStep(int proj, int step)
{
    const Project *p = GetProject(proj);
    if (step >= p->stepCount) step = p->stepCount - 1;
    ms.mission = -1;
    ms.inContract = true;
    ms.proj = proj;
    ms.step = step;
    EditorSetText(&editor, prog.projHasDraft[proj] ? prog.projDrafts[proj] : p->starterCode);
    ResetPanel();
    StartAttempt(false);
    app.screen = SCREEN_MISSION;
}

void MissionLeave(void)
{
    if (app.screen == SCREEN_MISSION) SaveDraft();
}

static void GoBack(void)
{
    SaveDraft();
    if (ms.inContract) OpenContract(ms.proj);
    else if (Cur()->role == ROLE_CHESS) OpenChess();
    else if (Cur()->role == ROLE_BOSS) OpenMap();
    else OpenLevel(Cur()->tut);
}

// Moves to another step of the open contract without touching the code
static void GoToStep(int step)
{
    SaveDraft();
    ms.step = step;
    ResetPanel();
}

// ---------------------------------------------------------------- running

static unsigned char StarsForThisRun(const Task *t)
{
    unsigned char bits = STAR_CLEAR;
    if (ms.fresh && !ms.usedHelp) bits |= STAR_NOHELP;
    if (IsGame(t->mode)) {
        if (CountCodeLines(editor.text) <= t->parLines) bits |= STAR_MASTER;
    } else if (ms.fresh && ms.runs == 1) {
        bits |= STAR_MASTER;
    }
    return bits;
}

static void OnSuccess(const Task *t)
{
    if (ms.inContract) {
        const Project *p = GetProject(ms.proj);
        int before = prog.projSteps[ms.proj];
        if (before < ms.step + 1) prog.projSteps[ms.proj] = ms.step + 1;
        ms.xp = AwardContractStep(ms.proj, before, prog.projSteps[ms.proj], p->stepCount);
    } else {
        ms.before = prog.stars[ms.mission];
        ms.earned = StarsForThisRun(t);
        ms.xp = AwardMission(ms.mission, ms.earned);
    }
    if (t->mode == MODE_BUGS) {
        ms.squashed = true;
        ms.squashedAt = GetTime();
    }
    ms.celebrate = true;
    ms.celebrateAt = GetTime() + (IsGame(t->mode) ? 1.6 : 0.9);   // let the replay play first
    ms.burst = false;
    if (!CurLevel()) { ParticlesBurst(SCREEN_W / 2.0f, SCREEN_H * 0.45f, 90, 420); ms.burst = true; }
}

static void DoRun(void)
{
    Task t = CurTask();
    SaveDraft();
    RunRequest req = { editor.text, t.lang, t.prelude, t.prelude ? GameDir() : NULL, t.prelude != NULL, t.tests, t.testCount };
    RunProgram(&req, &result);
    ms.hasResult = true;
    ms.runPending = false;
    ms.runs++;
    ms.logScroll = 0;
    editor.focused = true;

    if (CurLevel()) {
        if (result.compiled) PlaybackRun(&result);
        else PlaybackCompileFailed();
    }
    if (result.allPassed) OnSuccess(&t);
}

// ---------------------------------------------------------------- output boxes

static void DrawOutputBox(const char *label, const char *text, Rectangle r, Color labelColor)
{
    UiText(label, r.x, r.y, 16, labelColor);
    Rectangle box = { r.x, r.y + 22, r.width, r.height - 22 };
    DrawRectangleRounded(box, 0.06f, 6, COL_CODE_BG);
    BeginScissorMode((int)box.x, (int)box.y, (int)box.width, (int)box.height);
    UiCode(text && text[0] ? text : "(nothing)", box.x + 10, box.y + 8, 15, text && text[0] ? COL_CODE_TEXT : COL_MUTED, box.width - 20);
    EndScissorMode();
}

static float DrawCompileLog(Rectangle area, const char *intro)
{
    UiText(intro, area.x, area.y, 19, COL_BAD);
    Rectangle log = { area.x, area.y + 30, area.width, area.height - 30 };
    DrawRectangleRounded(log, 0.04f, 6, COL_CODE_BG);
    if (CheckCollisionPointRec(GetMousePosition(), log)) ms.logScroll -= GetMouseWheelMove() * 40;
    if (ms.logScroll < 0) ms.logScroll = 0;
    BeginScissorMode((int)log.x, (int)log.y, (int)log.width, (int)log.height);
    float h = UiCode(result.compileLog, log.x + 10, log.y + 8 - ms.logScroll, 15, COL_TEXT, log.width - 20);
    EndScissorMode();
    float maxScroll = h - log.height + 16;
    if (ms.logScroll > maxScroll) ms.logScroll = maxScroll > 0 ? maxScroll : 0;
    return h;
}

// ---------------------------------------------------------------- Bug Hunter

static void DrawBugs(const Task *t)
{
    if (ms.inContract || t->mode != MODE_BUGS) return;
    const MissionInfo *m = Cur();
    float lh = EditorLineHeight(&editor);
    float time = (float)GetTime();
    BeginScissorMode((int)editorRect.x, (int)editorRect.y, (int)editorRect.width, (int)editorRect.height);
    for (int i = 0; i < m->bugCount; i++) {
        int line = EditorFindLine(&editor, m->bugs[i].buggyLine, ms.bugAt[i] >= 0 ? ms.bugAt[i] : m->bugs[i].bugLine);
        bool fixedLine = (line < 0);
        if (!fixedLine) ms.bugAt[i] = line;
        float y = EditorLineY(&editor, editorRect, ms.bugAt[i]) + lh / 2 - 2;
        Vector2 c = { editorRect.x + 14, y };
        if (ms.squashed) {
            float k = fminf(1, (float)(GetTime() - ms.squashedAt) * 2);
            DrawEllipse((int)c.x, (int)c.y, 14 * (1 + k * 0.6f), 14 * (1 - k * 0.7f) + 1, ColorAlpha((Color){ 90, 160, 70, 255 }, 1 - k * 0.5f));
            continue;
        }
        if (!fixedLine)   // tint the whole line the bug is sitting on
            DrawRectangle((int)(editorRect.x + 50), (int)(y - lh / 2 + 2), (int)(editorRect.width - 50), (int)lh,
                          ColorAlpha((Color){ 235, 90, 90, 255 }, 0.12f + 0.05f * sinf(time * 4)));
        float wiggle = fixedLine ? time * 5 : sinf(time * 7 + i) * 0.4f;
        IconBug(c, 28, -PI / 2 + wiggle, fixedLine ? (Color){ 170, 200, 120, 255 } : COL_BUGS);
        Rectangle hit = { c.x - 14, c.y - 14, 28, 28 };
        bool hover = false;
        if (UiClickArea(hit, &hover)) EditorGotoLine(&editor, editorRect, ms.bugAt[i]);
        if (hover) {
            const char *tip = fixedLine ? "This line changed - Run & Check to see if the bug is gone" : "A bug is on this line - click to jump to it";
            float tw = UiTextWidth(tip, 15) + 16;
            DrawRectangleRounded((Rectangle){ c.x + 18, c.y - 14, tw, 26 }, 0.4f, 6, (Color){ 20, 22, 30, 240 });
            UiText(tip, c.x + 26, c.y - 10, 15, COL_TEXT);
        }
    }
    EndScissorMode();
}

static void DrawBugClock(Rectangle bar, const Task *t)
{
    float total = (float)t->timeLimit;
    float left = total - (float)(GetTime() - ms.startTime);
    if (ms.squashed || ms.timeUp) left = ms.squashed ? left : 0;
    if (!ms.squashed && left <= 0 && !ms.timeUp) ms.timeUp = true;
    if (left < 0) left = 0;
    DrawRectangleRounded(bar, 0.5f, 6, (Color){ 38, 41, 53, 255 });
    float k = ms.squashed ? 1 : left / total;
    Color fill = k > 0.5f ? COL_GOOD_DARK : k > 0.2f ? (Color){ 190, 150, 50, 255 } : COL_BAD_DARK;
    DrawRectangleRounded((Rectangle){ bar.x, bar.y, bar.width * k, bar.height }, 0.5f, 6, fill);
    int bugsLeft = ms.squashed ? 0 : Cur()->bugCount;
    const char *label = ms.squashed ? "All bugs squashed!"
                      : TextFormat("%d:%02d left   -   %d bug%s on the loose", (int)left / 60, (int)left % 60, bugsLeft, bugsLeft == 1 ? "" : "s");
    UiTextCentered(label, bar.x + bar.width / 2, bar.y + 3, 17, WHITE);
}

// ---------------------------------------------------------------- the left panel

static void DrawStepPicker(float right, float y)
{
    const Project *p = GetProject(ms.proj);
    float r = 13, gap = 32;
    for (int i = p->stepCount - 1; i >= 0; i--) {
        float cx = right - r - (p->stepCount - 1 - i) * gap;
        bool unlocked = i <= prog.projSteps[ms.proj];
        bool hover = false;
        Rectangle hit = { cx - r, y - r, r * 2, r * 2 };
        bool clicked = unlocked && i != ms.step && UiClickArea(hit, &hover);
        Color fill = COL_PANEL_HI;
        if (i < prog.projSteps[ms.proj]) fill = COL_GOOD_DARK;
        if (i == ms.step) fill = COL_ACCENT;
        if (hover) fill = ColorBrightness(fill, 0.2f);
        DrawCircle((int)cx, (int)y, r, fill);
        UiTextCentered(TextFormat("%d", i + 1), cx, y - 9, 18, unlocked ? WHITE : COL_MUTED);
        if (clicked) GoToStep(i);
    }
}

// Run & Check, Hint, Solution and Reset: the same in every layout
static void Buttons(Rectangle runBtn, Rectangle hintBtn, Rectangle solBtn, Rectangle rstBtn, const Task *t)
{
    bool locked = (t->mode == MODE_BUGS && ms.timeUp && !ms.squashed);
    if (UiButton(runBtn, ms.runPending ? "Running..." : locked ? "Time's up" : "Run & Check", locked ? COL_PANEL_HI : COL_GOOD_DARK) && !locked)
        ms.runPending = true;

    if (UiButton(hintBtn, ms.showHint ? "Hide Hint" : "Hint", (Color){ 120, 95, 40, 255 })) {
        ms.showHint = !ms.showHint;
        if (ms.showHint) ms.usedHelp = true;
        ms.solutionConfirmAt = 0;
    }
    bool confirming = ms.solutionConfirmAt > 0;
    if (UiButton(solBtn, confirming ? "Sure?" : "Solution", confirming ? COL_BAD_DARK : COL_PANEL_HI)) {
        if (!confirming) {
            ms.solutionConfirmAt = GetTime();
        } else {
            EditorReplaceAll(&editor, t->solution);
            editor.focused = true;
            ms.hasResult = false;
            PlaybackClear();
            ms.solutionConfirmAt = 0;
            ms.usedHelp = true;
            snprintf(ms.notice, sizeof(ms.notice),
                     "The solution is in the editor. Read it line by line, then try it yourself - Ctrl+Z brings your own code back.");
        }
    }
    if (UiButton(rstBtn, "Reset", COL_PANEL_HI)) {
        EditorReplaceAll(&editor, t->resetCode);
        ResetPanel();
        StartAttempt(true);
        snprintf(ms.notice, sizeof(ms.notice), ms.inContract && ms.step > 0
                 ? "Code reset to the start of this step. Press Ctrl+Z to undo."
                 : "Fresh start - every star is up for grabs. Press Ctrl+Z to undo.");
    }
}

// ---------------------------------------------------------------- the 3D level layout
//
//   left:  the level's name and stars, the brief (or the hint), the editor, the buttons
//   right: the test tabs and replay controls, the 3D scene, and the output panel where the
//          program's output appears line by line as the scene acts it out

// The three stars in one row
static void DrawObjectivesRow(float x, float y, const Task *t)
{
    unsigned char have = prog.stars[ms.mission];
    bool thirdLost = !ms.fresh || (ms.runs > 0 && !(ms.hasResult && result.allPassed && ms.runs == 1));
    const char *third = "Pass on the first run";
    if (IsGame(t->mode)) {            // robot and heist missions: the third star is for short code
        int lines = CountCodeLines(editor.text);
        third = TextFormat("%d lines or fewer (now %d)", t->parLines, lines);
        thirdLost = lines > t->parLines;
    }
    const char *texts[3] = { t->mode == MODE_BUGS ? "Squash every bug in time" : "Pass every test", "No hint or solution", third };
    bool lost[3] = { t->mode == MODE_BUGS && ms.timeUp, ms.usedHelp || !ms.fresh, thirdLost };
    for (int i = 0; i < 3; i++) {
        bool got = (have >> i) & 1;
        Vector2 c = { x + 9, y + 10 };
        if (got) DrawStar(c, 8, COL_GOLD);
        else DrawStarOutline(c, 8, 1.5f, lost[i] ? COL_BAD : COL_MUTED);
        UiText(texts[i], x + 22, y + 2, 15, got ? COL_TEXT : lost[i] ? (Color){ 220, 140, 140, 255 } : COL_MUTED);
        x += 22 + UiTextWidth(texts[i], 15) + 20;
    }
}

// The brief (or the hint while it's open), scrolling when it's long
static void DrawBriefBox(Rectangle r, const Task *t)
{
    DrawRectangleRounded(r, 0.06f, 8, COL_PANEL);
    bool hint = ms.showHint && t->hint;
    float x = r.x + 16, w = r.width - 32;
    UiText(hint ? "Hint" : "The brief", x, r.y + 8, 15, hint ? (Color){ 240, 200, 110, 255 } : COL_MUTED);
    Rectangle area = { r.x, r.y + 30, r.width, r.height - 36 };
    if (CheckCollisionPointRec(GetMousePosition(), area)) ms.briefScroll -= GetMouseWheelMove() * 40;
    if (ms.briefScroll < 0) ms.briefScroll = 0;

    BeginScissorMode((int)area.x, (int)area.y, (int)area.width, (int)area.height);
    float y = area.y - ms.briefScroll;
    if (ms.notice[0] && !hint) y += UiWrapped(ms.notice, x, y, w, 16, COL_ACCENT) + 8;
    y += UiWrapped(hint ? t->hint : t->brief, x, y, w, 16, COL_TEXT) + 6;
    if (t->api && !hint) {
        UiText(t->mode == MODE_ROBOT ? "The robot understands" : "The game gives you", x, y + 4, 15, ModeColor(t->mode));
        y += 26;
        y += UiCode(t->api, x, y, 15, COL_CODE_TEXT, w);
    }
    EndScissorMode();

    float maxScroll = (y + ms.briefScroll - area.y) - area.height + 6;
    if (ms.briefScroll > maxScroll) ms.briefScroll = maxScroll > 0 ? maxScroll : 0;
    if (maxScroll > 0) {
        const char *more = ms.briefScroll < maxScroll - 1 ? "scroll for more  v" : "^  scroll up";
        float mw = UiTextWidth(more, 14) + 16;
        Rectangle tag = { r.x + r.width - 12 - mw, r.y + 6, mw, 20 };
        DrawRectangleRounded(tag, 0.5f, 6, COL_PANEL_HI);
        UiText(more, tag.x + 8, tag.y + 3, 14, COL_ACCENT);
    }
}

static void DrawLevelLeft(Rectangle col, const Task *t, const LevelDef *lv, bool overlay)
{
    // the level's name, the project and its stars
    Rectangle head = { col.x, col.y, col.width, 74 };
    DrawRectangleRounded(head, 0.14f, 8, COL_PANEL);
    float x = head.x + 16, y = head.y + 10;
    bool bugs = (t->mode == MODE_BUGS);
    bool boss = !ms.inContract && Cur()->role == ROLE_BOSS;
    bool chess = !ms.inContract && Cur()->role == ROLE_CHESS;
    const char *chip = ms.inContract ? "CONTRACT" : bugs ? "BUG HUNTER" : boss ? "BOSS" : chess ? ChessDifficultyName(Cur()->region) : lv->name;
    Color cc = ms.inContract ? ModeColor(MODE_FACTORY) : bugs ? COL_BUGS : boss ? COL_BAD : chess ? ChessDifficultyColor(Cur()->region) : lv->color;
    float cw = UiTextWidth(chip, 15) + 24;
    DrawRectangleRounded((Rectangle){ x, y, cw, 26 }, 0.5f, 6, ColorAlpha(cc, 0.2f));
    UiText(chip, x + 12, y + 5, 15, cc);
    const char *title = t->title;
    if (ms.inContract) {
        const Project *p = GetProject(ms.proj);
        title = TextFormat("Step %d of %d: %s", ms.step + 1, p->stepCount, p->steps[ms.step].title);
    }
    float room = head.width - cw - 42 - (chess ? 124 : 0);
    BeginScissorMode((int)(x + cw + 10), (int)y, (int)room, 30);
    UiText(title, x + cw + 12, y + 1, 21, COL_TEXT);
    EndScissorMode();
    if (chess && UiButton((Rectangle){ head.x + head.width - 132, head.y + 8, 120, 30 }, "Commands", COL_PANEL_HI)) {
        OpenChessHelp(SCREEN_MISSION);
        return;
    }
    if (ms.inContract) {
        UiText("Your code carries on from step to step. Steps:", x, y + 36, 15, COL_MUTED);
        DrawStepPicker(head.x + head.width - 16, y + 46);
    } else {
        DrawObjectivesRow(x, y + 34, t);
    }

    // the buttons along the bottom
    float bh = 46, gap = 8;
    float by = col.y + col.height - bh;
    float runW = col.width * 0.38f, small = (col.width - runW - gap * 3) / 3;
    Rectangle runBtn = { col.x, by, runW, bh };
    Rectangle hintBtn = { col.x + runW + gap, by, small, bh };
    Rectangle solBtn = { hintBtn.x + small + gap, by, small, bh };
    Rectangle rstBtn = { solBtn.x + small + gap, by, small, bh };

    // the brief, then the editor
    float top = head.y + head.height + 8;
    float clock = bugs ? 34 : 0;
    float avail = by - 8 - top - clock;
    float briefH = floorf(avail * 0.3f);
    if (briefH < 104) briefH = 104;
    DrawBriefBox((Rectangle){ col.x, top, col.width, briefH }, t);
    float ey = top + briefH + 8;
    if (bugs) {
        DrawBugClock((Rectangle){ col.x, ey, col.width, 26 }, t);
        ey += clock;
    }
    editorRect = (Rectangle){ col.x, ey, col.width, by - 8 - ey };
    if (!overlay) EditorUpdate(&editor, editorRect);
    EditorDraw(&editor, editorRect);
    DrawBugs(t);
    Buttons(runBtn, hintBtn, solBtn, rstBtn, t);
}

// The plain text results for the test picked in the tabs
static void DrawDetails(Rectangle r, const Task *t)
{
    DrawRectangleRounded(r, 0.03f, 8, COL_PANEL);
    Rectangle in = { r.x + 16, r.y + 14, r.width - 32, r.height - 28 };
    if (ms.runPending) {
        UiText(TextFormat("Compiling with %s...", CompilerName(t->lang)), in.x, in.y, 20, COL_ACCENT);
        return;
    }
    if (!ms.hasResult) {
        UiWrapped("The plain text results show up here after Run & Check: each test's input, the output it expects, "
                  "and exactly what your program printed.", in.x, in.y, in.width, 17, COL_MUTED);
        return;
    }
    if (!result.compiled) {
        DrawCompileLog(in, "Your code didn't compile. Read the first error, fix it and try again.");
        return;
    }
    int pass = 0;
    for (int i = 0; i < result.testCount; i++) if (result.passed[i]) pass++;
    UiText(TextFormat("%d of %d tests passed", pass, result.testCount), in.x, in.y, 21, result.allPassed ? COL_GOOD : COL_BAD);
    int show = PlaybackTest();
    if (show < 0 || show >= t->testCount) show = 0;
    const TestCase *tc = &t->tests[show];
    const char *how = result.passed[show] ? "passed" : result.status[show] == RUN_CRASHED ? "crashed"
                    : result.status[show] == RUN_OK ? "failed: the output doesn't match" : "was stopped (too long or too much output)";
    UiText(TextFormat("Test %d %s", show + 1, how), in.x, in.y + 32, 17, result.passed[show] ? COL_GOOD : COL_BAD);
    float boxY = in.y + 62, boxH = in.y + in.height - boxY;
    float colW = (in.width - 24) / 3;
    DrawOutputBox(TextFormat("Test %d input", show + 1), tc->input ? tc->input : "", (Rectangle){ in.x, boxY, colW, boxH }, COL_MUTED);
    DrawOutputBox("Expected output", tc->expected, (Rectangle){ in.x + colW + 12, boxY, colW, boxH }, COL_MUTED);
    DrawOutputBox("Your output", result.actual[show], (Rectangle){ in.x + (colW + 12) * 2, boxY, colW, boxH },
                  result.passed[show] ? COL_GOOD : COL_BAD);
}

static void DrawLevelRight(Rectangle area, const Task *t)
{
    float bh = 34, bw = 112, gap = 6;
    Rectangle bigBtn = { area.x + area.width - bw, area.y, bw, bh };
    Rectangle detBtn = { bigBtn.x - gap - bw, area.y, bw, bh };
    if (UiButton(bigBtn, ms.bigView ? "Back to code" : "Full view", ms.bigView ? COL_ACCENT : COL_PANEL_HI)) ms.bigView = !ms.bigView;
    if (UiButton(detBtn, ms.details ? "Show game" : "Show details", COL_PANEL_HI)) ms.details = !ms.details;
    PlaybackBar((Rectangle){ area.x, area.y, detBtn.x - 14 - area.x, bh });

    float top = area.y + bh + 8;
    if (ms.details) {
        DrawDetails((Rectangle){ area.x, top, area.width, area.y + area.height - top }, t);
        return;
    }
    float resultsH = ms.bigView ? 200 : 178;
    float viewH = area.y + area.height - top - resultsH - 10;
    if (!ms.bigView && viewH > area.width * 9.0f / 16.0f) viewH = floorf(area.width * 9.0f / 16.0f);
    if (viewH < 180) viewH = 180;
    Rectangle view = { area.x, top, area.width, viewH };
    levelView = view;
    Rectangle res = { area.x, view.y + view.height + 10, area.width, area.y + area.height - (view.y + view.height + 10) };

    PlaybackDraw(view);
    if (ms.runPending) {
        DrawRectangleRec(view, (Color){ 10, 12, 18, 170 });
        UiTextCentered(TextFormat("Compiling with %s and running every test...", CompilerName(t->lang)),
                       view.x + view.width / 2, view.y + view.height / 2 - 12, 22, COL_ACCENT);
    }
    if (ms.hasResult && !result.compiled) DrawCompileLog(res, "Compiler errors - read the first one first:");
    else PlaybackResults(res);
}

// ---------------------------------------------------------------- overlays

static void DrawCelebration(const Task *t)
{
    if (!ms.celebrate) return;
    // let the replay (or the 3D level's playback) finish before the card covers it
    if (CurLevel() && PlaybackBusy())
        ms.celebrateAt = GetTime() + 0.8;
    if (GetTime() < ms.celebrateAt) return;
    if (!ms.burst) {
        ms.burst = true;
        ParticlesBurst(SCREEN_W / 2.0f, SCREEN_H * 0.45f, 90, 420);
    }
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, ColorAlpha(BLACK, 0.55f));
    float cw = 560, ch = ms.inContract ? 300 : 400;
    Rectangle card = { (SCREEN_W - cw) / 2, (SCREEN_H - ch) / 2, cw, ch };
    DrawRectangleRounded(card, 0.08f, 8, COL_PANEL);
    DrawRectangleRoundedLinesEx(card, 0.08f, 8, 2, ModeColor(t->mode));
    float cx = card.x + cw / 2, y = card.y + 26;

    const char *title;
    if (ms.inContract) title = "STEP COMPLETE!";
    else if (Cur()->role == ROLE_BOSS) title = "BOSS DEFEATED!";
    else if (t->mode == MODE_CHESS) title = "PUZZLE SOLVED!";
    else if (t->mode == MODE_BUGS) title = "BUGS SQUASHED!";
    else if (t->mode == MODE_ROBOT) title = "ROBOT RESCUED!";
    else if (t->mode == MODE_HEIST) title = "CLEAN GETAWAY!";
    else if (CurLevel()) title = "LEVEL CLEAR!";
    else title = "ORDER SHIPPED!";
    DrawModeIcon(t->mode, (Vector2){ cx, y + 24 }, 52);
    y += 60;
    UiTextCentered(title, cx, y, 34, COL_TEXT);
    y += 50;

    if (!ms.inContract) {
        float pop = (float)(GetTime() - ms.celebrateAt);
        unsigned char all = ms.before | ms.earned;
        for (int i = 0; i < 3; i++) {
            Vector2 c = { cx + (i - 1) * 70.0f, y + 26 };
            bool on = (all >> i) & 1;
            bool isNew = on && !((ms.before >> i) & 1);
            float s = 26;
            if (isNew) { float k = fminf(1, fmaxf(0, pop * 3 - i * 0.6f)); s = 26 * (k < 1 ? k * 1.3f : 1); }
            if (on && s > 0) { DrawStar((Vector2){ c.x, c.y + 3 }, s, COL_GOLD_DARK); DrawStar(c, s, COL_GOLD); }
            else DrawStarOutline(c, 26, 2, COL_MUTED);
        }
        y += 66;
        const char *notes[3] = { "Cleared", "No hint or solution", IsGame(t->mode) ? TextFormat("Within %d lines", t->parLines) : "First-run pass" };
        for (int i = 0; i < 3; i++) {
            bool on = ((ms.before | ms.earned) >> i) & 1;
            UiTextCentered(TextFormat("%s %s", on ? "+" : "-", notes[i]), cx + (i - 1) * 170.0f, y, 16, on ? COL_GOOD : COL_MUTED);
        }
        y += 34;
    } else {
        const Project *p = GetProject(ms.proj);
        bool last = ms.step == p->stepCount - 1;
        UiTextCentered(last ? "Contract complete - the whole program works!" : "Your code carries on into the next step.", cx, y, 18, COL_MUTED);
        y += 40;
    }
    UiTextCentered(ms.xp > 0 ? TextFormat("+%d XP", ms.xp) : "No new XP this time - try for the missing stars!", cx, y, 22,
                   ms.xp > 0 ? COL_GOLD : COL_MUTED);

    float by = card.y + ch - 70;
    Rectangle a = { cx - 250, by, 240, 50 }, b = { cx + 10, by, 240, 50 };
    if (ms.inContract) {
        const Project *p = GetProject(ms.proj);
        bool last = ms.step == p->stepCount - 1;
        if (UiButton(a, "Keep coding", COL_PANEL_HI)) ms.celebrate = false;
        if (UiButton(b, last ? "Contracts" : "Next Step >", COL_ACCENT)) {
            ms.celebrate = false;
            if (last) { SaveDraft(); OpenContracts(GetProject(ms.proj)->region); }
            else GoToStep(ms.step + 1);
        }
    } else {
        if (UiButton(a, "Keep coding", COL_PANEL_HI)) ms.celebrate = false;
        const char *next = Cur()->role == ROLE_BOSS ? "World Map >" : Cur()->role == ROLE_CHESS ? "More puzzles >" : "Back to the level >";
        if (UiButton(b, next, COL_ACCENT)) { ms.celebrate = false; GoBack(); }
    }
}

static void DrawTimeUp(void)
{
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, ColorAlpha(BLACK, 0.55f));
    Rectangle card = { SCREEN_W / 2.0f - 260, SCREEN_H / 2.0f - 150, 520, 300 };
    DrawRectangleRounded(card, 0.08f, 8, COL_PANEL);
    float time = (float)GetTime();
    for (int i = 0; i < 3; i++)
        IconBug((Vector2){ card.x + 160 + i * 100 + sinf(time * 3 + i) * 10, card.y + 60 }, 40, time * 2 + i, COL_BUGS);
    UiTextCentered("The bugs got away!", card.x + 260, card.y + 110, 32, COL_TEXT);
    UiTextCentered("The clock ran out. Have another go - the code starts broken again.", card.x + 260, card.y + 156, 17, COL_MUTED);
    if (UiButton((Rectangle){ card.x + 20, card.y + 220, 230, 50 }, "Back", COL_PANEL_HI)) GoBack();
    if (UiButton((Rectangle){ card.x + 270, card.y + 220, 230, 50 }, "Try Again", COL_ACCENT)) {
        EditorSetText(&editor, Cur()->starter);
        ResetPanel();
        StartAttempt(true);
    }
}

// ---------------------------------------------------------------- the screen

void ScreenMission(void)
{
    Task t = CurTask();
    const LevelDef *lv = CurLevel();

    // A run was asked for last frame (after "Compiling..." was drawn)
    if (ms.runPending) DoRun();

    bool replaying = lv && PlaybackBusy();
    bool overlay = (ms.celebrate && GetTime() >= ms.celebrateAt && !replaying) || (t.mode == MODE_BUGS && ms.timeUp && !ms.squashed);
    if (!overlay) {
        if (CtrlDown() && IsKeyPressed(KEY_ENTER) && !(t.mode == MODE_BUGS && ms.timeUp)) ms.runPending = true;
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (ms.bigView) ms.bigView = false;
            else { GoBack(); return; }
        }
    }
    if (ms.solutionConfirmAt > 0 && GetTime() - ms.solutionConfirmAt > 3.0) ms.solutionConfirmAt = 0;
    if (lv) PlaybackUpdate(GetFrameTime());

    // top bar
    const char *lang = (t.lang == LANG_CPP) ? "C++" : "C";
    if (ms.inContract) {
        DrawTopBar(TextFormat("Contract: %s", t.title));
        bool done = prog.projSteps[ms.proj] >= GetProject(ms.proj)->stepCount;
        TopBarRight(done ? TextFormat("%s  -  Completed", lang) : lang, done ? COL_GOOD : COL_MUTED);
    } else {
        const MissionInfo *m = Cur();
        DrawTopBar(m->tut >= 0 ? TextFormat("%d. %s", m->tut + 1, GetTutorial(m->tut)->title)
                   : m->role == ROLE_CHESS ? TextFormat("Chess Puzzle: %s", m->title) : m->title);
        unsigned char have = prog.stars[ms.mission];
        DrawStarRow(SCREEN_W - 150, 32, 11, have, -1);
        UiText(lang, SCREEN_W - 70, 22, 20, COL_MUTED);
    }
    // While a card is showing on top, everything underneath still draws but ignores the mouse
    UiSetBlocked(overlay);
    if (BackButton("< Back")) { UiSetBlocked(false); GoBack(); return; }

    // the brief and the editor on the left, the scene and the output on the right
    float colW = SCREEN_W * 0.41f;
    if (colW < 470) colW = 470;
    if (colW > 760) colW = 760;
    Rectangle col = { 20, 84, colW, SCREEN_H - 104.0f };
    Rectangle right = { col.x + colW + 16, 84, SCREEN_W - (col.x + colW + 16) - 20, SCREEN_H - 104.0f };
    if (ms.bigView || !lv) right = (Rectangle){ 20, 84, SCREEN_W - 40.0f, SCREEN_H - 104.0f };
    if (!ms.bigView && lv) DrawLevelLeft(col, &t, lv, overlay);
    DrawLevelRight(right, &t);
    UiSetBlocked(false);
    if (t.mode == MODE_BUGS && !ms.inContract && ms.timeUp && !ms.squashed) DrawTimeUp();
    DrawCelebration(&t);
}
