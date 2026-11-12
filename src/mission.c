// The mission screen: the task on the left, the code editor, and the game on the right.
//   3D levels    - a level's own project: the brief and the editor on the left, and the
//                  level's 3D scene on the right, played from the program's real output
//                  for each test, with the expected and actual output underneath
//   Code Factory - each test is a crate on a conveyor belt; wrong output falls off the belt
//   Robot Rescue - the maze, and a replay of what the robot did
//   Memory Heist - the street of memory, and a replay of what the thief did
//   Bug Hunter   - Code Factory, plus bugs sitting on the broken lines and a clock
// Factory Contracts (the bigger, multi-step projects) use this screen too.
#include "app.h"
#include "editor.h"
#include "robotview.h"
#include "heistview.h"
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

    int    sel;                  // the test shown in the game view
    int    speed;                // replay speed: 0, 1 or 2
    double beltStart;            // when the conveyor started moving
    int    crate;                // the crate whose details are shown, -1 = first failure

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

static bool IsGame(GameMode mode) { return mode == MODE_ROBOT || mode == MODE_HEIST; }

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
    ms.sel = 0;
    ms.crate = -1;
    ms.celebrate = false;
    ms.burst = false;
    ms.timeUp = false;
    ms.squashed = false;
    ms.details = false;
    ms.bigView = false;
    ms.startTime = GetTime();
    for (int i = 0; i < MAX_BUGS; i++) ms.bugAt[i] = -1;
    Task t = CurTask();
    if (t.mode == MODE_ROBOT) RobotLoad(t.tests, t.testCount);
    if (t.mode == MODE_HEIST) HeistLoad(t.tests, t.testCount);
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

static int FirstFailure(void)
{
    for (int i = 0; i < result.testCount; i++) if (!result.passed[i]) return i;
    return -1;
}

static void ShowTest(int test)
{
    Task t = CurTask();
    ms.sel = test;
    if (!ms.hasResult || !result.compiled || CurLevel()) return;    // a 3D level plays every test itself
    if (t.mode == MODE_ROBOT) RobotPlay(test, result.events[test], result.won[test]);
    if (t.mode == MODE_HEIST) HeistPlay(test, result.events[test], result.won[test], result.status[test] == RUN_CRASHED);
}

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
    ms.crate = -1;
    ms.beltStart = GetTime();
    editor.focused = true;

    int fail = FirstFailure();
    ShowTest(fail >= 0 ? fail : 0);
    if (CurLevel()) {
        if (result.compiled) PlaybackRun(&result);
        else PlaybackCompileFailed();
    }
    if (result.allPassed) OnSuccess(&t);
}

// ---------------------------------------------------------------- Code Factory: the conveyor belt

static void DrawCrate(Vector2 c, float s, int number, int stamp, float angle)
{
    Rectangle r = { c.x, c.y, s, s };
    Color wood = { 176, 128, 76, 255 };
    DrawRectanglePro(r, (Vector2){ s / 2, s / 2 }, angle, wood);
    if (fabsf(angle) < 0.5f) {
        Rectangle b = { c.x - s / 2, c.y - s / 2, s, s };
        DrawRectangleLinesEx(b, 2, (Color){ 120, 82, 45, 255 });
        DrawLineEx((Vector2){ b.x + 4, b.y + 4 }, (Vector2){ b.x + s - 4, b.y + s - 4 }, 2, (Color){ 140, 98, 55, 255 });
        UiTextCentered(TextFormat("#%d", number), c.x, c.y - 9, 17, (Color){ 60, 38, 15, 255 });
        if (stamp == 1) {
            DrawCircleV((Vector2){ c.x + s * 0.32f, c.y - s * 0.32f }, s * 0.2f, (Color){ 40, 160, 90, 255 });
            UiTextCentered("OK", c.x + s * 0.32f, c.y - s * 0.32f - 8, 14, WHITE);
        } else if (stamp == 2) {
            DrawCircleV((Vector2){ c.x + s * 0.32f, c.y - s * 0.32f }, s * 0.2f, (Color){ 190, 50, 55, 255 });
            UiTextCentered("X", c.x + s * 0.32f, c.y - s * 0.32f - 8, 15, WHITE);
        }
    }
}

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

static void DrawFactory(Rectangle r, const Task *t)
{
    DrawRectangleRounded(r, 0.04f, 8, COL_PANEL);
    float time = (float)GetTime();
    int n = t->testCount;
    float x0 = r.x + 18, w = r.width - 36;

    // --- the factory floor (squeezed a little on short screens, so the output boxes stay readable)
    bool compact = r.height < 380;
    float beltY = r.y + (compact ? 60 : 78), beltH = 22;
    float crateS = compact ? 38 : 44;
    Rectangle machine = { r.x + r.width * 0.5f - 80, r.y + 12, 160, compact ? 86.0f : 104.0f };
    bool busy = ms.runPending;
    float spin = busy ? time * 6 : (ms.hasResult ? 0 : time * 0.6f);

    DrawRectangle((int)x0, (int)(beltY + beltH), (int)w, 6, (Color){ 30, 32, 40, 255 });
    DrawRectangleRounded((Rectangle){ x0, beltY, w, beltH }, 0.5f, 6, (Color){ 58, 60, 72, 255 });
    float offset = fmodf(time * (busy ? 120 : 30), 24);
    BeginScissorMode((int)x0, (int)beltY, (int)w, (int)beltH);
    for (float sx = x0 - 24 + offset; sx < x0 + w; sx += 24)
        DrawLineEx((Vector2){ sx, beltY + 3 }, (Vector2){ sx + 10, beltY + beltH - 3 }, 3, (Color){ 80, 84, 98, 255 });
    EndScissorMode();

    // reject bin and shipping bay
    Rectangle bin = { r.x + r.width * 0.5f + 96, beltY + beltH + 10, 230, compact ? 36.0f : 44.0f };
    DrawRectangleRounded(bin, 0.2f, 6, (Color){ 90, 40, 44, 255 });
    UiText("REJECTS", bin.x + bin.width + 8, bin.y + bin.height / 2 - 8, 14, (Color){ 255, 170, 170, 255 });
    Rectangle bay = { r.x + r.width - 18 - 150, r.y + 14, 150, beltY - r.y - 18 };
    DrawRectangleRounded(bay, 0.15f, 6, (Color){ 36, 70, 52, 255 });
    UiTextCentered("SHIPPED", bay.x + bay.width / 2, bay.y + 4, 14, (Color){ 170, 255, 200, 255 });

    // the machine is the student's program
    DrawRectangleRounded(machine, 0.15f, 8, (Color){ 70, 76, 100, 255 });
    DrawRectangleRounded((Rectangle){ machine.x + 8, machine.y + 8, machine.width - 16, 28 }, 0.3f, 6, (Color){ 40, 44, 60, 255 });
    UiTextCentered("YOUR PROGRAM", machine.x + machine.width / 2, machine.y + 13, 15, COL_FACTORY);
    IconGear((Vector2){ machine.x + machine.width / 2 - 26, machine.y + 68 }, 38, spin, COL_FACTORY);
    IconGear((Vector2){ machine.x + machine.width / 2 + 22, machine.y + 74 }, 26, -spin * 1.4f, (Color){ 200, 140, 60, 255 });
    if (ms.hasResult && !result.compiled) {
        UiTextCentered("JAMMED", machine.x + machine.width / 2, machine.y + machine.height + 4, 18, COL_BAD);
        for (int i = 0; i < 3; i++)
            DrawCircleV((Vector2){ machine.x + 30 + i * 50, machine.y - 6 - fmodf(time * 20 + i * 9, 20) }, 8, (Color){ 140, 140, 150, 120 });
    }

    // crates: waiting on the left, then through the machine
    float inX = machine.x - 30, outX = machine.x + machine.width + 30;
    for (int i = 0; i < n; i++) {
        float queueX = x0 + 30 + i * (crateS + 8);
        Vector2 pos = { queueX, beltY - crateS / 2 + 2 };
        int stamp = 0;
        float angle = 0;
        bool shown = true;
        if (ms.hasResult && result.compiled) {
            float t = (float)(GetTime() - ms.beltStart) - i * 0.45f;
            bool pass = result.passed[i];
            if (t < 0) {
                pos.x = queueX;
            } else if (t < 0.5f) {
                pos.x = queueX + (inX - queueX) * (t / 0.5f);
            } else if (t < 0.8f) {
                shown = false;                                   // inside the machine
            } else if (t < 1.3f) {
                stamp = pass ? 1 : 2;
                pos.x = outX + (t - 0.8f) / 0.5f * 60;
            } else {
                stamp = pass ? 1 : 2;
                float k = fminf(1, (t - 1.3f) / 0.5f);
                if (pass) {
                    int slot = 0;
                    for (int j = 0; j < i; j++) if (result.passed[j]) slot++;
                    Vector2 dest = { bay.x + 28 + (slot % 3) * 48, bay.y + 42 + (slot / 3) * 46 };
                    pos.x = outX + 60 + (dest.x - outX - 60) * k;
                    pos.y = pos.y + (dest.y - pos.y) * k;
                } else {
                    int slot = 0;
                    for (int j = 0; j < i; j++) if (!result.passed[j]) slot++;
                    Vector2 dest = { bin.x + 26 + slot * 44, bin.y + bin.height / 2 };
                    pos.x = outX + 60 + (dest.x - outX - 60) * k;
                    pos.y = pos.y + (dest.y - pos.y) * k * k;
                    angle = sinf(k * PI) * (slot % 2 ? -25.0f : 25.0f);   // tumbles on the way, lands upright
                }
            }
        }
        if (!shown) continue;
        Rectangle hit = { pos.x - crateS / 2, pos.y - crateS / 2, crateS, crateS };
        bool hover = false;
        if (ms.hasResult && result.compiled && UiClickArea(hit, &hover)) ms.crate = i;
        DrawCrate(pos, crateS, i + 1, stamp, angle);
        if (hover || ms.crate == i) DrawRectangleLinesEx((Rectangle){ hit.x - 3, hit.y - 3, hit.width + 6, hit.height + 6 }, 2, COL_ACCENT);
        if (!ms.hasResult) UiTextCentered("?", pos.x, pos.y + crateS / 2 + 2, 16, COL_MUTED);
    }

    // --- under the floor: what happened
    float below = beltY + beltH + (compact ? 52 : 64);
    Rectangle info = { x0, below, w, r.y + r.height - below - 12 };
    if (ms.runPending) {
        UiText(TextFormat("Compiling with %s and sending the crates through...", CompilerName(t->lang)), info.x, info.y, 20, COL_ACCENT);
        return;
    }
    if (!ms.hasResult) {
        float h = 0;
        if (ms.notice[0]) h += UiWrapped(ms.notice, info.x, info.y, info.width, 18, COL_ACCENT) + 8;
        UiWrapped("Each crate is one test: its input goes into your program, and the output must match exactly. "
                  "Press Run & Check (or Ctrl+Enter) to start the belt.\n"
                  "Editing: Ctrl+Z undo, Ctrl+Y redo, Ctrl+A select all, Ctrl+C / X / V, Tab / Shift+Tab indent.",
                  info.x, info.y + h, info.width, 17, COL_MUTED);
        return;
    }
    if (!result.compiled) {
        DrawCompileLog(info, "The machine jammed: your code didn't compile. Read the first error, fix it and try again.");
        return;
    }

    int passCount = 0;
    for (int i = 0; i < result.testCount; i++) if (result.passed[i]) passCount++;
    int show = ms.crate >= 0 ? ms.crate : FirstFailure();
    if (result.allPassed) {
        UiText(TextFormat("All %d crates shipped!", result.testCount), info.x, info.y, 21, COL_GOOD);
    } else {
        int f = FirstFailure();
        const char *msg;
        if (result.status[f] == RUN_TIMEOUT || result.status[f] == RUN_TOO_MUCH_OUTPUT)
            msg = TextFormat("Crate #%d jammed the machine - is there a loop that never ends?", f + 1);
        else if (result.status[f] == RUN_CRASHED)
            msg = TextFormat("Your program crashed on crate #%d.", f + 1);
        else
            msg = TextFormat("%d of %d crates shipped. The rest fell off the belt - click one to see why.", passCount, result.testCount);
        UiText(msg, info.x, info.y, 20, COL_BAD);
    }
    if (show < 0) show = 0;
    const TestCase *tc = &t->tests[show];
    float boxY = info.y + 30, boxH = info.height - 30;
    float colW = (info.width - 24) / 3;
    DrawOutputBox(TextFormat("Crate #%d input", show + 1), tc->input ? tc->input : "", (Rectangle){ info.x, boxY, colW, boxH }, COL_MUTED);
    DrawOutputBox("Expected output", tc->expected, (Rectangle){ info.x + colW + 12, boxY, colW, boxH }, COL_MUTED);
    DrawOutputBox("Your output", result.actual[show], (Rectangle){ info.x + (colW + 12) * 2, boxY, colW, boxH },
                  result.passed[show] ? COL_GOOD : COL_BAD);
}

// ---------------------------------------------------------------- Robot Rescue and Memory Heist

static void DrawGameView(Rectangle r, const Task *t)
{
    DrawRectangleRounded(r, 0.03f, 8, COL_PANEL);
    bool robot = (t->mode == MODE_ROBOT);
    const char *what = robot ? "Maze" : "Street";
    float x = r.x + 12, w = r.width - 24;

    // controls on the right, tabs (one per test) in the space left of them
    Rectangle replay = { r.x + r.width - 12 - 86, r.y + 10, 86, 34 };
    Rectangle speed = { replay.x - 6 - 52, r.y + 10, 52, 34 };
    if (ms.hasResult && result.compiled && UiButton(replay, "Replay", COL_PANEL_HI)) {
        if (robot) RobotReplay(); else HeistReplay();
    }
    static const float speeds[] = { 5, 14, 40 };
    if (UiButton(speed, ms.speed == 0 ? "1x" : ms.speed == 1 ? "3x" : "8x", COL_PANEL_HI)) ms.speed = (ms.speed + 1) % 3;
    RobotSetSpeed(speeds[ms.speed]);
    HeistSetSpeed(speeds[ms.speed]);

    float tabsW = speed.x - 10 - x;
    float tabW = fminf(120, (tabsW - 6 * (t->testCount - 1)) / (float)t->testCount);
    bool shortLabels = tabW < 92;
    for (int i = 0; i < t->testCount; i++) {
        Rectangle tab = { x + i * (tabW + 6), r.y + 10, tabW, 34 };
        bool hover = false;
        if (UiClickArea(tab, &hover)) ShowTest(i);
        Color fill = (i == ms.sel) ? COL_PANEL_HI : (Color){ 38, 41, 53, 255 };
        if (hover) fill = ColorBrightness(fill, 0.15f);
        DrawRectangleRounded(tab, 0.3f, 6, fill);
        Color dot = COL_MUTED;
        if (ms.hasResult && result.compiled) dot = result.passed[i] ? COL_GOOD : COL_BAD;
        DrawCircleV((Vector2){ tab.x + 14, tab.y + 17 }, 6, dot);
        const char *label = shortLabels ? TextFormat("%d", i + 1) : TextFormat("%s %d", what, i + 1);
        UiText(label, tab.x + 26, tab.y + 7, 18, i == ms.sel ? COL_TEXT : COL_MUTED);
    }

    float consoleH = fmaxf(90, r.height * 0.22f);
    float statusH = 48;
    Rectangle canvas = { x, r.y + 54, w, r.height - 54 - consoleH - statusH - 20 };
    DrawRectangleRounded(canvas, 0.03f, 6, (Color){ 24, 26, 34, 255 });
    Rectangle inner = { canvas.x + 8, canvas.y + 8, canvas.width - 16, canvas.height - 16 };
    if (robot) RobotDraw(inner, ms.sel); else HeistDraw(inner, ms.sel);

    // status line
    float sy = canvas.y + canvas.height + 10;
    const char *status = NULL;
    Color statusColor = COL_MUTED;
    bool playing = robot ? RobotPlaying() : HeistPlaying();
    if (ms.runPending) {
        status = TextFormat("Compiling with %s and sending in the %s...", CompilerName(t->lang), robot ? "robot" : "thief");
        statusColor = COL_ACCENT;
    } else if (!ms.hasResult && ms.notice[0]) {
        status = ms.notice;
        statusColor = COL_ACCENT;
    } else if (!ms.hasResult) {
        status = robot ? TextFormat("%s %d of %d. Plan the route, then press Run & Check to send the robot in.", what, ms.sel + 1, t->testCount)
                       : TextFormat("%s %d of %d. Plan the job, then press Run & Check to send the thief in.", what, ms.sel + 1, t->testCount);
    } else if (!result.compiled) {
        status = robot ? "The robot won't boot: your code didn't compile." : "The job's off: your code didn't compile.";
        statusColor = COL_BAD;
    } else if (playing) {
        status = robot ? "The robot is following your program..." : "The thief is following your pointers...";
        statusColor = COL_ACCENT;
    } else {
        int i = ms.sel;
        if (result.passed[i]) {
            status = robot ? "Rescued! The robot made it out." : "Clean getaway! Nobody saw a thing.";
            statusColor = COL_GOOD;
        } else if (result.status[i] == RUN_CRASHED) {
            status = robot ? "Your program crashed." : "ALARM! Your program crashed - often a NULL or deleted pointer being used.";
            statusColor = COL_BAD;
        } else if (result.status[i] == RUN_TIMEOUT || result.status[i] == RUN_TOO_MUCH_OUTPUT) {
            status = "Stopped after 5 seconds - is a loop going round forever?";
            statusColor = COL_BAD;
        } else if (!result.won[i]) {
            status = result.verdict[i][0] ? result.verdict[i] : "That didn't work.";
            statusColor = COL_BAD;
        } else {
            status = robot ? "The robot made it, but your program didn't print the right message - see below."
                           : "The job worked, but your program didn't print the right thing - see below.";
            statusColor = COL_BAD;
        }
    }
    BeginScissorMode((int)x, (int)sy, (int)w, (int)statusH);
    UiWrapped(status, x, sy, w, 16, statusColor);
    EndScissorMode();

    // console: what the program printed, or the compiler's errors
    Rectangle console = { x, r.y + r.height - consoleH - 10, w, consoleH };
    if (ms.hasResult && !result.compiled) {
        DrawCompileLog(console, "Compiler errors - read the first one first:");
        return;
    }
    const TestCase *tc = &t->tests[ms.sel];
    bool showExpected = ms.hasResult && tc->expected && !result.outputOk[ms.sel];
    float half = showExpected ? (w - 12) / 2 : w;
    if (ms.hasResult) DrawOutputBox("Your program printed", result.actual[ms.sel], (Rectangle){ x, console.y, half, consoleH }, COL_MUTED);
    else if (tc->expected) DrawOutputBox("It should print", tc->expected, (Rectangle){ x, console.y, half, consoleH }, COL_MUTED);
    else DrawOutputBox("Your program printed", "", (Rectangle){ x, console.y, half, consoleH }, COL_MUTED);
    if (showExpected) DrawOutputBox("It should print", tc->expected, (Rectangle){ x + half + 12, console.y, half, consoleH }, COL_BAD);
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

static float DrawObjectives(float x, float y, float w, const Task *t)
{
    if (ms.inContract) return 0;
    unsigned char have = prog.stars[ms.mission];
    int lines = CountCodeLines(editor.text);
    const char *third;
    bool thirdLost = false;
    if (IsGame(t->mode)) {
        third = TextFormat("Use %d lines of code or fewer (now %d)", t->parLines, lines);
        thirdLost = lines > t->parLines;
    } else {
        third = "Pass on your first Run & Check";
        thirdLost = !ms.fresh || (ms.runs > 0 && !(ms.hasResult && result.allPassed && ms.runs == 1));
    }
    const char *texts[3] = { t->mode == MODE_BUGS ? "Squash every bug before the clock runs out" : "Clear the mission",
                             "Don't use the hint or the solution", third };
    bool lost[3] = { t->mode == MODE_BUGS && ms.timeUp, ms.usedHelp || !ms.fresh, thirdLost };
    for (int i = 0; i < 3; i++) {
        bool got = (have >> i) & 1;
        Vector2 c = { x + 10, y + 11 };
        if (got) DrawStar(c, 9, COL_GOLD);
        else DrawStarOutline(c, 9, 1.5f, lost[i] ? COL_BAD : COL_MUTED);
        UiText(texts[i], x + 28, y + 2, 16, got ? COL_TEXT : lost[i] ? (Color){ 220, 140, 140, 255 } : COL_MUTED);
        y += 24;
    }
    (void)w;
    return 76;
}

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

static void Buttons(Rectangle runBtn, Rectangle hintBtn, Rectangle solBtn, Rectangle rstBtn, const Task *t);

static void DrawLeftPanel(Rectangle left, const Task *t)
{
    DrawRectangleRounded(left, 0.03f, 8, COL_PANEL);
    float x = left.x + 20, w = left.width - 40, y = left.y + 16;

    // mode chip and title
    Color mc = ModeColor(t->mode);
    const char *modeName = ms.inContract ? "FACTORY CONTRACT" : TextFormat("%s%s", ModeName(t->mode), Cur()->role == ROLE_BOSS ? "  -  BOSS" : "");
    float cw = UiTextWidth(modeName, 15) + 44;
    DrawRectangleRounded((Rectangle){ x, y, cw, 28 }, 0.5f, 6, ColorAlpha(mc, 0.22f));
    DrawModeIcon(t->mode, (Vector2){ x + 15, y + 14 }, 22);
    UiText(modeName, x + 32, y + 6, 15, mc);
    if (ms.inContract) DrawStepPicker(left.x + left.width - 20, y + 14);
    y += 38;
    if (ms.inContract) {
        const Project *p = GetProject(ms.proj);
        UiText(TextFormat("Step %d of %d: %s", ms.step + 1, p->stepCount, p->steps[ms.step].title), x, y, 20, COL_TEXT);
        y += 32;
    } else {
        UiText(t->title, x, y, 22, COL_TEXT);
        y += 34;
        y += DrawObjectives(x, y, w, t) + 6;
    }

    float bw = w;
    Rectangle runBtn = { x, left.y + left.height - 116, bw, 52 };
    float smallW = (bw - 16) / 3;
    Rectangle hintBtn = { x, left.y + left.height - 56, smallW, 42 };
    Rectangle solBtn = { x + smallW + 8, left.y + left.height - 56, smallW, 42 };
    Rectangle rstBtn = { x + (smallW + 8) * 2, left.y + left.height - 56, smallW, 42 };

    float bottom = runBtn.y - 12;
    if (ms.showHint && t->hint) {
        float hh = UiWrappedHeight(t->hint, bw - 28, 16) + 42;
        Rectangle hb = { x, bottom - hh, bw, hh };
        DrawRectangleRounded(hb, 0.08f, 6, (Color){ 60, 52, 30, 255 });
        UiText("Hint", hb.x + 14, hb.y + 9, 17, (Color){ 240, 200, 110, 255 });
        UiWrapped(t->hint, hb.x + 14, hb.y + 32, bw - 28, 16, COL_TEXT);
        bottom = hb.y - 10;
    }
    if (t->api) {
        int lines = 1;
        for (const char *c = t->api; *c; c++) if (*c == '\n') lines++;
        float ah = lines * 16 * 1.3f + 30;
        if (ah > (bottom - y) * 0.5f) ah = (bottom - y) * 0.5f;
        Rectangle ab = { x, bottom - ah, bw, ah };
        DrawRectangleRounded(ab, 0.06f, 6, COL_CODE_BG);
        UiText(t->mode == MODE_ROBOT ? "The robot understands" : "The game gives you", ab.x + 10, ab.y + 6, 15, mc);
        BeginScissorMode((int)ab.x, (int)ab.y, (int)ab.width, (int)ab.height);
        UiCode(t->api, ab.x + 10, ab.y + 26, 15, COL_CODE_TEXT, 0);
        EndScissorMode();
        bottom = ab.y - 10;
    }

    // the brief scrolls if it's long
    Rectangle briefArea = { left.x, y, left.width, bottom - y };
    float briefH = UiWrappedHeight(t->brief, w, 17);
    if (CheckCollisionPointRec(GetMousePosition(), briefArea)) ms.briefScroll -= GetMouseWheelMove() * 40;
    float maxBrief = briefH - briefArea.height + 8;
    if (ms.briefScroll > maxBrief) ms.briefScroll = maxBrief;
    if (ms.briefScroll < 0) ms.briefScroll = 0;
    BeginScissorMode((int)briefArea.x, (int)briefArea.y, (int)briefArea.width, (int)briefArea.height);
    UiWrapped(t->brief, x, briefArea.y + 2 - ms.briefScroll, w, 17, COL_TEXT);
    EndScissorMode();
    if (maxBrief > 0) {
        const char *more = ms.briefScroll < maxBrief - 1 ? "scroll for more  v" : "^  scroll up";
        float mw = UiTextWidth(more, 14) + 16;
        Rectangle tag = { left.x + left.width - 20 - mw, briefArea.y + briefArea.height - 20, mw, 20 };
        DrawRectangleRounded(tag, 0.5f, 6, COL_PANEL_HI);
        UiText(more, tag.x + 8, tag.y + 3, 14, COL_ACCENT);
    }

    Buttons(runBtn, hintBtn, solBtn, rstBtn, t);
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
    const char *chip = ms.inContract ? "CONTRACT" : bugs ? "BUG HUNTER" : boss ? "BOSS" : lv->name;
    Color cc = ms.inContract ? ModeColor(MODE_FACTORY) : bugs ? COL_BUGS : boss ? COL_BAD : lv->color;
    float cw = UiTextWidth(chip, 15) + 24;
    DrawRectangleRounded((Rectangle){ x, y, cw, 26 }, 0.5f, 6, ColorAlpha(cc, 0.2f));
    UiText(chip, x + 12, y + 5, 15, cc);
    const char *title = t->title;
    if (ms.inContract) {
        const Project *p = GetProject(ms.proj);
        title = TextFormat("Step %d of %d: %s", ms.step + 1, p->stepCount, p->steps[ms.step].title);
    }
    BeginScissorMode((int)(x + cw + 10), (int)y, (int)(head.width - cw - 42), 30);
    UiText(title, x + cw + 12, y + 1, 21, COL_TEXT);
    EndScissorMode();
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
    if ((t->mode == MODE_ROBOT && RobotPlaying()) || (t->mode == MODE_HEIST && HeistPlaying()) || (CurLevel() && PlaybackBusy()))
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
        if (UiButton(b, Cur()->role == ROLE_BOSS ? "World Map >" : "Back to the level >", COL_ACCENT)) { ms.celebrate = false; GoBack(); }
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

    bool replaying = (t.mode == MODE_ROBOT && RobotPlaying()) || (t.mode == MODE_HEIST && HeistPlaying()) || (lv && PlaybackBusy());
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
        DrawTopBar(m->tut >= 0 ? TextFormat("%d. %s", m->tut + 1, GetTutorial(m->tut)->title) : m->title);
        unsigned char have = prog.stars[ms.mission];
        DrawStarRow(SCREEN_W - 150, 32, 11, have, -1);
        UiText(lang, SCREEN_W - 70, 22, 20, COL_MUTED);
    }
    // While a card is showing on top, everything underneath still draws but ignores the mouse
    UiSetBlocked(overlay);
    if (BackButton("< Back")) { UiSetBlocked(false); GoBack(); return; }

    if (lv) {
        // a 3D level: brief and editor on the left, the scene and the output on the right
        float colW = SCREEN_W * 0.41f;
        if (colW < 470) colW = 470;
        if (colW > 760) colW = 760;
        Rectangle col = { 20, 84, colW, SCREEN_H - 104.0f };
        Rectangle right = { col.x + colW + 16, 84, SCREEN_W - (col.x + colW + 16) - 20, SCREEN_H - 104.0f };
        if (ms.bigView) right = (Rectangle){ 20, 84, SCREEN_W - 40.0f, SCREEN_H - 104.0f };
        else DrawLevelLeft(col, &t, lv, overlay);
        DrawLevelRight(right, &t);
        UiSetBlocked(false);
        if (t.mode == MODE_BUGS && !ms.inContract && ms.timeUp && !ms.squashed) DrawTimeUp();
        DrawCelebration(&t);
        return;
    }

    float leftW = SCREEN_W >= 1600 ? 460 : 380;
    Rectangle left = { 20, 84, leftW, SCREEN_H - 104.0f };
    DrawLeftPanel(left, &t);

    float rightX = left.x + left.width + 20;
    float rightW = SCREEN_W - rightX - 20;
    if (IsGame(t.mode)) {
        float edW = rightW * (SCREEN_W >= 1600 ? 0.46f : 0.42f);
        editorRect = (Rectangle){ rightX, 84, edW, SCREEN_H - 104.0f };
        if (!overlay) EditorUpdate(&editor, editorRect);
        EditorDraw(&editor, editorRect);
        Rectangle view = { rightX + edW + 16, 84, rightW - edW - 16, SCREEN_H - 104.0f };
        DrawGameView(view, &t);
    } else {
        float top = 84;
        if (t.mode == MODE_BUGS && !ms.inContract) {
            DrawBugClock((Rectangle){ rightX, 84, rightW, 26 }, &t);
            top += 34;
        }
        float edH = (SCREEN_H - top - 34) * 0.56f;
        editorRect = (Rectangle){ rightX, top, rightW, edH };
        if (!overlay) EditorUpdate(&editor, editorRect);
        EditorDraw(&editor, editorRect);
        DrawBugs(&t);
        float resY = editorRect.y + edH + 14;
        DrawFactory((Rectangle){ rightX, resY, rightW, SCREEN_H - 20 - resY }, &t);
    }

    UiSetBlocked(false);
    if (t.mode == MODE_BUGS && !ms.inContract && ms.timeUp && !ms.squashed) DrawTimeUp();
    DrawCelebration(&t);
}
