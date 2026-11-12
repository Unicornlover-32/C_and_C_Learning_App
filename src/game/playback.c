// Plays a level's scene once for each test of a run, in order: "Test 2 of 4". The player
// can replay a test, skip to its end, speed up to 2x or 4x, or pick a test by its tab.
// Next to the scene, the program's output appears line by line as the scene acts it out.
#include "levels.h"
#include "gfx.h"
#include "../ui.h"
#include "../art.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static const float SPEEDS[] = { 1, 2, 4 };

static struct {
    const LevelDef *level;
    const TestCase *tests;
    int             testCount;
    EventList       lists[MAX_TESTS];
    bool            passed[MAX_TESTS];
    bool            hasRun, compileFailed, allPassed;

    int    cur;              // the test being shown
    int    next;             // its next event
    float  wait;             // until that event may start
    bool   done;             // every event of this test has started and finished
    bool   autoPlay;         // carry on to the next test when this one ends
    float  hold;             // pause between tests
    float  endT;             // time since this test ended (the PASSED / FAILED stamp)
    int    printed;          // output lines shown so far
    int    speed;
    float  slowScale, slowLeft;
    bool   fast;             // fast-forwarding to the end of a test
    float  fixedDt;
    bool   showFps;
} pb;

static char outputs[MAX_TESTS][OUTPUT_CAP];   // what the program printed in each test of the last run

static const EventList *Cur(void) { return &pb.lists[pb.cur]; }

static void ParseAll(const RunResult *res)
{
    for (int i = 0; i < pb.testCount; i++) {
        LevelRun run = { pb.tests[i].input ? pb.tests[i].input : "", pb.tests[i].expected, NULL, RUN_NOT_STARTED, false, "" };
        if (res) {
            memcpy(outputs[i], res->output[i], OUTPUT_CAP);
            run.output = outputs[i];
            run.status = res->status[i];
            run.passed = res->passed[i];
            run.trace = res->events[i];
            pb.passed[i] = res->passed[i];
        } else {
            pb.passed[i] = false;
            outputs[i][0] = '\0';
        }
        pb.level->parse(&run, &pb.lists[i]);
    }
}

static void Advance(float dt);

static void StartTest(int test)
{
    pb.cur = test < 0 ? 0 : test >= pb.testCount ? pb.testCount - 1 : test;
    pb.next = 0;
    pb.wait = 0;
    pb.done = false;
    pb.hold = 0;
    pb.endT = 0;
    pb.printed = 0;
    pb.slowLeft = 0;
    FxClear();
    pb.level->reset(Cur());
    // the world appears at once
    const EventList *ev = Cur();
    while (pb.next < ev->count && ev->ev[pb.next].type == EV_SPAWN) pb.level->event(&ev->ev[pb.next++]);
    if (!pb.hasRun) pb.done = true;
}

static void FastForward(void)
{
    pb.fast = true;
    FxEnable(false);
    for (int i = 0; i < 6000 && !pb.done; i++) Advance(0.05f);
    for (int i = 0; i < 50; i++) Advance(0.05f);          // let the last animations settle
    FxEnable(true);
    pb.fast = false;
    pb.slowLeft = 0;
}

void PlaybackOpen(const LevelDef *level, const TestCase *tests, int testCount)
{
    float fixedDt = pb.fixedDt;
    bool showFps = pb.showFps;
    memset(&pb, 0, sizeof(pb));
    pb.fixedDt = fixedDt;
    pb.showFps = showFps;
    pb.level = level;
    pb.tests = tests;
    pb.testCount = testCount > MAX_TESTS ? MAX_TESTS : testCount;
    if (!level || pb.testCount <= 0) { pb.level = NULL; return; }
    ParseAll(NULL);
    StartTest(0);
}

void PlaybackRun(const RunResult *result)
{
    if (!pb.level) return;
    pb.hasRun = true;
    pb.compileFailed = false;
    pb.allPassed = result->allPassed;
    ParseAll(result);
    pb.autoPlay = true;
    StartTest(0);
}

void PlaybackCompileFailed(void)
{
    if (!pb.level) return;
    pb.hasRun = false;
    pb.compileFailed = true;
    ParseAll(NULL);
    StartTest(pb.cur);
}

void PlaybackClear(void)
{
    if (!pb.level || (!pb.hasRun && !pb.compileFailed)) return;
    pb.hasRun = false;
    pb.compileFailed = false;
    pb.autoPlay = false;
    ParseAll(NULL);
    StartTest(pb.cur);
}

bool PlaybackBusy(void) { return pb.level && pb.hasRun && (pb.autoPlay || !pb.done); }
bool PlaybackHasRun(void) { return pb.level && pb.hasRun; }
int  PlaybackTest(void) { return pb.cur; }
bool PlaybackTestDone(void) { return pb.level && pb.done; }
void PlaybackSetFixedStep(float dt) { pb.fixedDt = dt; }

void PlaybackShake(float amount) { if (!pb.fast) CamShake(amount); }
void PlaybackSlowMo(float scale, float seconds) { if (!pb.fast) { pb.slowScale = scale; pb.slowLeft = seconds; } }
bool PlaybackFinale(void) { return pb.hasRun && pb.allPassed && pb.cur == pb.testCount - 1; }

// ---------------------------------------------------------------- time

static void Advance(float dt)
{
    CamUpdate(dt);
    FxUpdate(dt);
    pb.level->update(dt);
    if (pb.done) return;
    const EventList *ev = Cur();
    pb.wait -= dt;
    while (pb.wait <= 0 && pb.next < ev->count) {
        const GameEvent *e = &ev->ev[pb.next++];
        if (e->line >= 0 && e->line + 1 > pb.printed) pb.printed = e->line + 1;
        pb.wait += pb.level->event(e);
    }
    if (pb.next >= ev->count && pb.wait <= 0) {
        pb.done = true;
        pb.hold = pb.passed[pb.cur] ? 1.1f : 2.6f;
        pb.printed = ev->printedLines;
    }
}

static int FirstFailure(void)
{
    for (int i = 0; i < pb.testCount; i++) if (!pb.passed[i]) return i;
    return -1;
}

void PlaybackUpdate(float frameTime)
{
    if (!pb.level) return;
    if (IsKeyPressed(KEY_F3)) pb.showFps = !pb.showFps;
    float dt = pb.fixedDt > 0 ? pb.fixedDt : frameTime;
    if (dt > 0.05f) dt = 0.05f;
    float scale = 1;
    if (pb.slowLeft > 0) { pb.slowLeft -= dt; scale = pb.slowScale; }
    Advance(dt * SPEEDS[pb.speed] * scale);

    if (pb.done && pb.hasRun) {
        pb.endT += dt;
        if (pb.autoPlay) {
            pb.hold -= dt * SPEEDS[pb.speed];
            if (pb.hold <= 0) {
                if (pb.cur + 1 < pb.testCount) {
                    StartTest(pb.cur + 1);
                } else {
                    pb.autoPlay = false;
                    int f = FirstFailure();          // end on the first failure, so it can be studied
                    if (f >= 0 && f != pb.cur) { StartTest(f); FastForward(); }
                }
            }
        }
    }
}

// ---------------------------------------------------------------- drawing

static void Chip(float x, float y, const char *text, Color fg, Color bg, float size)
{
    float w = UiTextWidth(text, size) + size * 1.2f;
    DrawRectangleRounded((Rectangle){ x, y, w, size * 1.7f }, 0.5f, 6, bg);
    UiText(text, x + size * 0.6f, y + size * 0.33f, size, fg);
}

// "0\n15\n2\n" -> "0 15 2", cut short if long
static void OneLine(const char *src, char *dst, int cap)
{
    int n = 0;
    bool space = false;
    for (const char *s = src ? src : ""; *s && n < cap - 4; s++) {
        bool ws = (*s == '\n' || *s == '\r' || *s == ' ' || *s == '\t');
        if (ws) { space = n > 0; continue; }
        if (space) { dst[n++] = ' '; space = false; }
        dst[n++] = (*s >= 32 && *s < 127) ? *s : '?';
    }
    if (src && n >= cap - 4) { dst[n++] = '.'; dst[n++] = '.'; dst[n++] = '.'; }
    dst[n] = '\0';
}

static const char *Verdict(const EventList *ev, Color *color)
{
    *color = COL_BAD;
    switch (ev->result) {
        case EV_WIN:   *color = COL_GOOD; return "PASSED";
        case EV_CRASH: return ev->fail == FAIL_TIMEOUT ? "STOPPED" : "CRASHED";
        default:       return "FAILED";
    }
}

void PlaybackDraw(Rectangle view)
{
    if (!pb.level) return;
    SceneBegin(view);
    pb.level->draw();
    FxDraw();
    SceneEnd(view);
    if (pb.level->overlay) pb.level->overlay(view);

    // heads-up: which test, and its input
    float s = view.height > 500 ? 19 : 16;
    char input[48];
    OneLine(pb.tests[pb.cur].input, input, sizeof(input));
    float x = view.x + 12, y = view.y + 10;
    const char *which = TextFormat("TEST %d OF %d", pb.cur + 1, pb.testCount);
    Chip(x, y, which, WHITE, ColorAlpha(pb.level->color, 0.85f), s);
    x += UiTextWidth(which, s) + s * 1.2f + 8;
    if (input[0]) Chip(x, y, TextFormat("input: %s", input), COL_TEXT, (Color){ 12, 14, 20, 200 }, s);

    if (pb.compileFailed) {
        DrawRectangleRec(view, (Color){ 10, 10, 16, 150 });
        UiTextCentered("Your code didn't compile, so nothing ran.", view.x + view.width / 2, view.y + view.height / 2 - 22, 24, COL_BAD);
        UiTextCentered("The compiler's messages are below - start with the first one.", view.x + view.width / 2, view.y + view.height / 2 + 12, 18, COL_TEXT);
    } else if (!pb.hasRun) {
        const char *tip = "This is the world for this test. Press Run & Check and your program's output will play it out.";
        float tw = UiTextWidth(tip, 16) + 28;
        if (tw > view.width - 20) tw = view.width - 20;
        Rectangle box = { view.x + (view.width - tw) / 2, view.y + view.height - 44, tw, 32 };
        DrawRectangleRounded(box, 0.5f, 6, (Color){ 12, 14, 20, 210 });
        BeginScissorMode((int)box.x, (int)box.y, (int)box.width, (int)box.height);
        UiTextCentered(tip, box.x + box.width / 2, box.y + 7, 16, COL_TEXT);
        EndScissorMode();
    } else if (pb.done) {
        // the stamp: pops in, then settles
        Color c;
        const char *v = Verdict(Cur(), &c);
        float k = EaseOutBack(Clamp01(pb.endT * 3));
        float size = (view.height > 500 ? 40 : 32) * (0.6f + 0.4f * k);
        float w = UiTextWidth(v, size) + size;
        Rectangle box = { view.x + view.width - w - 14, view.y + 10, w, size * 1.35f };
        DrawRectangleRounded(box, 0.3f, 6, ColorAlpha(c, 0.9f * Clamp01(pb.endT * 4)));
        UiTextCentered(v, box.x + box.width / 2, box.y + size * 0.17f, size, WHITE);
    }
    if (pb.showFps) UiText(TextFormat("%d fps", GetFPS()), view.x + view.width - 80, view.y + view.height - 26, 16, COL_GOOD);
}

void PlaybackBar(Rectangle bar)
{
    if (!pb.level) return;
    float bw = 76, gap = 6;
    Rectangle speedBtn = { bar.x + bar.width - 52, bar.y, 52, bar.height };
    Rectangle skipBtn = { speedBtn.x - gap - bw, bar.y, bw, bar.height };
    Rectangle replayBtn = { skipBtn.x - gap - bw, bar.y, bw, bar.height };
    if (UiButton(speedBtn, TextFormat("%dx", (int)SPEEDS[pb.speed]), COL_PANEL_HI)) pb.speed = (pb.speed + 1) % 3;
    if (pb.hasRun) {
        if (UiButton(replayBtn, "Replay", COL_PANEL_HI)) { pb.autoPlay = false; StartTest(pb.cur); }
        if (UiButton(skipBtn, pb.done ? "Next" : "Skip", COL_PANEL_HI)) {
            if (!pb.done) FastForward();
            else if (pb.cur + 1 < pb.testCount) StartTest(pb.cur + 1);
        }
    }

    float tabsW = replayBtn.x - 10 - bar.x;
    float tabW = fminf(118, (tabsW - gap * (pb.testCount - 1)) / (float)pb.testCount);
    for (int i = 0; i < pb.testCount; i++) {
        Rectangle tab = { bar.x + i * (tabW + gap), bar.y, tabW, bar.height };
        bool hover = false;
        if (UiClickArea(tab, &hover)) {
            pb.autoPlay = false;
            StartTest(i);
            if (!pb.hasRun) pb.done = true;
        }
        Color fill = (i == pb.cur) ? COL_PANEL_HI : (Color){ 38, 41, 53, 255 };
        if (hover) fill = ColorBrightness(fill, 0.15f);
        DrawRectangleRounded(tab, 0.3f, 6, fill);
        Color dot = COL_MUTED;
        if (pb.hasRun) dot = pb.passed[i] ? COL_GOOD : COL_BAD;
        if (pb.hasRun && i == pb.cur && !pb.done) dot = COL_ACCENT;
        DrawCircleV((Vector2){ tab.x + 14, tab.y + bar.height / 2 }, 6, dot);
        UiText(tabW > 84 ? TextFormat("Test %d", i + 1) : TextFormat("%d", i + 1), tab.x + 26, tab.y + bar.height / 2 - 9, 18,
               i == pb.cur ? COL_TEXT : COL_MUTED);
    }
}

// ---------------------------------------------------------------- the output panel

static float CodeCharW(float size)
{
    return MeasureTextEx(gCodeFont, "0000000000", size, UiCodeSpacing(size)).x / 10.0f;
}

// One line of output; dots mark its spaces when spacing might be the problem
static void CodeLine(const char *s, int len, float x, float y, float size, Color color, bool dots)
{
    char buf[200];
    int n = len < (int)sizeof(buf) - 1 ? len : (int)sizeof(buf) - 1;
    memcpy(buf, s, (size_t)n);
    buf[n] = '\0';
    UiCode(buf, x, y, size, color, 0);
    if (dots) {
        float cw = CodeCharW(size);
        for (int i = 0; i < n; i++)
            if (buf[i] == ' ') DrawCircleV((Vector2){ x + cw * (i + 0.5f), y + size * 0.56f }, 1.5f, ColorAlpha(color, 0.6f));
    }
}

// True when two lines differ only in their spaces ("   1   2" and "1 2")
static bool SameWithoutSpaces(const char *a, const char *b)
{
    while (*a || *b) {
        while (*a == ' ' || *a == '\t') a++;
        while (*b == ' ' || *b == '\t') b++;
        if (*a != *b) return false;
        if (*a) { a++; b++; }
    }
    return true;
}

// Draws text line by line. Lines past `upto` are hidden (not printed yet); `mark` is highlighted.
static void OutputBox(Rectangle r, const char *title, Color titleColor, const char *text, int upto, int mark, Color markColor,
                      bool dotsOnMark, const char *empty)
{
    UiText(title, r.x, r.y, 15, titleColor);
    Rectangle box = { r.x, r.y + 21, r.width, r.height - 21 };
    DrawRectangleRounded(box, 0.06f, 6, COL_CODE_BG);
    float size = 15, lh = size * 1.3f;
    int lines = 0;
    for (const char *c = text; c && *c; c++) if (*c == '\n') lines++;
    if (text && text[0]) lines++;
    if (upto >= 0 && upto < lines) lines = upto;

    // keep the newest line in sight
    float inner = box.height - 14;
    int focus = mark >= 0 ? mark : (upto >= 0 ? lines - 1 : -1);
    float scroll = (focus + 1) * lh - inner;
    if (scroll < 0) scroll = 0;

    BeginScissorMode((int)box.x, (int)box.y + 4, (int)box.width, (int)box.height - 8);
    if (lines == 0) {
        UiText(empty, box.x + 10, box.y + 8, 15, COL_MUTED);
    } else {
        const char *s = text;
        for (int i = 0; i < lines && s; i++) {
            const char *e = strchr(s, '\n');
            int len = e ? (int)(e - s) : (int)strlen(s);
            float y = box.y + 7 + i * lh - scroll;
            if (i == mark) DrawRectangle((int)box.x + 3, (int)(y - 2), (int)box.width - 6, (int)lh, ColorAlpha(markColor, 0.28f));
            CodeLine(s, len, box.x + 10, y, size, i == mark ? WHITE : COL_CODE_TEXT, i == mark && dotsOnMark);
            s = e ? e + 1 : NULL;
        }
    }
    EndScissorMode();
}

static int LineOf(const char *text, const char *line)
{
    // the index of `line` in text, or -1
    int i = 0, n = (int)strlen(line);
    for (const char *s = text; s && *s; i++) {
        const char *e = strchr(s, '\n');
        int len = e ? (int)(e - s) : (int)strlen(s);
        if (len == n && strncmp(s, line, (size_t)n) == 0) return i;
        s = e ? e + 1 : NULL;
    }
    return -1;
}

void PlaybackResults(Rectangle r)
{
    if (!pb.level) return;
    const TestCase *tc = &pb.tests[pb.cur];
    const EventList *ev = Cur();

    // what happened, in a line or two (each quoted line has at most FAIL_TEXT characters)
    char msg[3 * FAIL_TEXT + 160];
    Color col = COL_MUTED;
    if (!pb.hasRun) {
        snprintf(msg, sizeof(msg), "Test %d of %d: this is what your program will be given and what it must print.", pb.cur + 1, pb.testCount);
    } else if (!pb.done) {
        snprintf(msg, sizeof(msg), "Test %d of %d: playing back what your program printed...", pb.cur + 1, pb.testCount);
        col = COL_ACCENT;
    } else if (ev->result == EV_WIN) {
        snprintf(msg, sizeof(msg), "Test %d passed%s", pb.cur + 1, tc->expected && tc->expected[0] ? ": every line matched." : "!");
        col = COL_GOOD;
    } else {
        col = COL_BAD;
        switch (ev->fail) {
            case FAIL_CRASH:
                snprintf(msg, sizeof(msg), "Test %d: your program crashed after printing %d line%s.", pb.cur + 1, ev->printedLines,
                         ev->printedLines == 1 ? "" : "s");
                break;
            case FAIL_TIMEOUT:
                snprintf(msg, sizeof(msg), "Test %d: stopped after %d seconds (or far too much output) - is a loop going round forever?",
                         pb.cur + 1, RUN_TIME_LIMIT);
                break;
            case FAIL_MISSING:
                snprintf(msg, sizeof(msg), "Test %d: your program stopped early. Next it should print \"%s\".", pb.cur + 1, ev->expected);
                break;
            case FAIL_GAME:
                snprintf(msg, sizeof(msg), "Test %d: %s", pb.cur + 1, ev->note);
                break;
            case FAIL_EXTRA:
                snprintf(msg, sizeof(msg), "Test %d: your program printed an extra line: \"%s\".", pb.cur + 1, ev->actual);
                break;
            default:
                snprintf(msg, sizeof(msg), "Test %d, line %d: expected \"%s\" but your program printed \"%s\".%s%s", pb.cur + 1,
                         ev->failLine + 1, ev->expected, ev->actual, ev->note[0] ? "  " : "", ev->note);
                break;
        }
    }
    // up to two lines of explanation
    float msgH = UiWrappedHeight(msg, r.width, 15);
    if (msgH > 40) msgH = 40;
    BeginScissorMode((int)r.x, (int)r.y, (int)r.width, (int)msgH + 2);
    UiWrapped(msg, r.x, r.y + 1, r.width, 15, col);
    EndScissorMode();

    float top = r.y + msgH + 6, h = r.height - msgH - 6;
    float gap = 10, inW = r.width * 0.22f, colW = (r.width - inW - gap * 2) / 2;
    Rectangle a = { r.x, top, inW, h }, b = { r.x + inW + gap, top, colW, h }, c = { b.x + colW + gap, top, colW, h };

    OutputBox(a, "Input", COL_MUTED, tc->input, -1, -1, COL_ACCENT, false, "(no input)");

    bool failed = pb.hasRun && pb.done && ev->result != EV_WIN;
    int failLine = failed ? ev->failLine : -1;
    bool spacing = failed && ev->fail == FAIL_WRONG && SameWithoutSpaces(ev->expected, ev->actual);
    // the expected line that goes with the current (or failing) output line
    int expMark = -1;
    if (pb.hasRun) {
        if (failed && ev->expected[0]) expMark = LineOf(tc->expected, ev->expected);
        else if (!pb.done && pb.printed > 0) expMark = pb.printed - 1;
    }
    OutputBox(b, "Expected output", COL_MUTED, tc->expected, -1, expMark, failed ? COL_GOOD : COL_ACCENT, spacing, "(nothing)");

    if (!pb.hasRun) {
        OutputBox(c, "Your output", COL_MUTED, "", 0, -1, COL_ACCENT, false, pb.compileFailed ? "(it didn't compile)" : "(press Run & Check)");
        return;
    }
    const char *text = outputs[pb.cur];      // lines past `printed` stay hidden until the scene gets there
    int mark = failed ? failLine : (!pb.done && pb.printed > 0 ? pb.printed - 1 : -1);
    Color titleCol = !pb.done ? COL_ACCENT : ev->result == EV_WIN ? COL_GOOD : COL_BAD;
    const char *title = ev->result == EV_CRASH && pb.done ? (ev->fail == FAIL_TIMEOUT ? "Your output (stopped)" : "Your output (crashed)") : "Your output";
    OutputBox(c, title, titleCol, text, pb.done ? -1 : pb.printed, mark, failed ? COL_BAD : COL_ACCENT, spacing && mark == failLine, "(nothing printed)");
}
