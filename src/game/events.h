#ifndef GAME_EVENTS_H
#define GAME_EVENTS_H

// What a level's parser turns one test run into: a list of game events that the
// level's scene then plays. Parsers are plain C (no raylib), so the test program can
// run every one of them on right and wrong output without opening a window.
//
// Every parser works the same way:
//   1. The world comes from the test input (SPAWN events), so each test looks different.
//   2. It walks the program's output line by line next to the expected output. Each line
//      it can read becomes ACT events made from the values the program really printed.
//   3. The first line that differs ends the story: one FAIL event, at that moment.
//   4. A crash or timeout becomes a CRASH event; otherwise the run ends with WIN.
#include <stdbool.h>
#include "../runner.h"

#define EV_MAX      1600   // events in one test run (more are dropped, see overflow)
#define EV_TEXT     48     // text carried by one event
#define FAIL_TEXT   160    // the expected / actual line shown when a test fails
#define PARSE_LINES 160    // lines of output a parser looks at

typedef enum {
    EV_SPAWN,      // part of the world, built from the test input
    EV_PRINT,      // the program printed output line `line`
    EV_ACT,        // what that line does in the world (the codes belong to each level)
    EV_FAIL,       // the first line that differs from the expected output
    EV_CRASH,      // the program crashed, ran too long or printed far too much
    EV_WIN,        // every line matched and the program ended normally
} EventType;

typedef enum {
    FAIL_NONE,
    FAIL_WRONG,     // a line the level can read, with a wrong value or wrong text
    FAIL_GARBLED,   // a line the level can't make sense of
    FAIL_MISSING,   // the program stopped before printing everything
    FAIL_EXTRA,     // the program printed more than expected
    FAIL_CRASH,     // the program crashed (or couldn't start)
    FAIL_TIMEOUT,   // stopped after the time limit, or printed far too much
    FAIL_GAME,      // game-library levels: the game itself said the run failed (the note says why)
} FailKind;

typedef struct {
    EventType type;
    int       code;        // the level's own action code
    int       line;        // output line that caused it; -1 = it came from the input
    int       a, b;        // indexes: row and column, slot, house...
    double    value;       // the number the program printed (or the input value, for SPAWN)
    char      text[EV_TEXT];
} GameEvent;

typedef struct {
    GameEvent ev[EV_MAX];
    int       count;
    bool      overflow;                 // more events than fit; the rest were dropped
    bool      preview;                  // nothing has run: only the world was built
    EventType result;                   // EV_WIN, EV_FAIL or EV_CRASH (EV_SPAWN for a preview)
    FailKind  fail;
    RunStatus status;
    int       failLine;                 // the output line that went wrong, -1 if none
    int       printedLines;             // how many lines the program printed
    char      expected[FAIL_TEXT];      // the first wrong line: what it should have been...
    char      actual[FAIL_TEXT];        // ...what the program printed instead...
    char      note[FAIL_TEXT];          // ...and a short explanation from the level
} EventList;

// One test run, as a parser sees it
typedef struct {
    const char *input;        // the test's input ("" if none)
    const char *expected;     // the test's expected output (NULL: a game level where anything goes)
    const char *output;       // what the program printed (normalised); NULL = nothing has run yet
    RunStatus   status;
    bool        passed;       // the runner's verdict
    const char *trace;        // game-library levels: the library's trace; otherwise ""
} LevelRun;

typedef void (*LevelParser)(const LevelRun *run, EventList *out);

// ---------------------------------------------------------------- helpers for parsers (parse.c)

// Output split into lines, normalised exactly like the runner does it
typedef struct {
    char text[OUTPUT_CAP];
    int  start[PARSE_LINES], len[PARSE_LINES];
    int  count;
} Lines;

void        LinesSplit(Lines *l, const char *text);
const char *LineAt(const Lines *l, int i, int *len);                 // NULL past the end
bool        LineIs(const char *s, int len, const char *text);        // the whole line is `text`

// Reading values. They never read past `len`, and fail on anything but the exact pattern.
bool ScanIntList(const char *s, int len, int *out, int max, int *count);   // "  1   2 -3" -> 1, 2, -3
bool ScanAfter(const char *s, int len, const char *prefix, int *value);    // "Total: 36" -> 36
int  InputInts(const char *input, int *out, int max);                      // the input's leading whole numbers

typedef struct {
    const LevelRun *run;
    EventList      *ev;
    Lines           exp, act;
    int             next;       // the next line of the program's output to read
    int             shown;      // output lines that already have a PRINT event
    bool            stopped;    // a failure has ended the story
} Parse;

void        ParseBegin(Parse *p, const LevelRun *run, EventList *ev);
bool        ParsePreview(const Parse *p);           // nothing has run: build the world and stop
GameEvent  *EvSpawn(Parse *p, int code, int a, int b, double value);
GameEvent  *EvAct(Parse *p, int code, int a, int b, double value);  // tied to the last printed line
void        EvText(GameEvent *e, const char *text);
// The next output line equals expected line e: emits PRINT for it and moves on
bool        ParseTake(Parse *p, int e);
// The next output line (NULL when the program printed nothing more)
const char *ParsePeek(const Parse *p, int *len);
// Emits PRINT for the next output line without moving on (before showing part of a wrong line)
void        ParseShow(Parse *p);
// Ends the story at expected line e (-1 = past the end). A missing line becomes a crash or
// timeout when the program was stopped. Returns the FAIL / CRASH event (NULL if full).
GameEvent  *ParseFail(Parse *p, FailKind kind, int code, int e, int a, int b, double value, const char *note);
// After the expected lines: extra output, a crash, or the win
void        ParseEnd(Parse *p, int extraCode);
// Ends the story without pointing at an output line: the game said no (FAIL_GAME, with the
// game's reason as the note), or the program died partway through (FAIL_CRASH / FAIL_TIMEOUT)
GameEvent  *ParseStop(Parse *p, FailKind kind, int code, const char *note);
FailKind    ParseStatusKind(RunStatus status);   // how a stopped run failed: crash or timeout

// Game-library levels (robot, heist). The game writes a trace and ends it with "END WIN" or
// "END FAIL <reason>"; like the runner, the last line starting with "END " decides.
// Returns 1 won, 0 failed (reason filled in, maybe ""), -1 no proper ending.
int         TraceEnding(const char *trace, char *reason, int cap);
// The end of a game level's story, after its actions: the game's verdict (a FAIL_GAME with
// the game's reason, or a crash / timeout); when the game was won, `homeCode`, then every
// output line as a `sayCode` ACT (b = 1 when the test takes any output) checked against the
// expected output, then ParseEnd
void        ParseGameEnd(Parse *p, int homeCode, int sayCode, int gameCode, int lineCode, int extraCode);

#endif
