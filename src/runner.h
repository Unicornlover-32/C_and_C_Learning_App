#ifndef RUNNER_H
#define RUNNER_H

#include <stdbool.h>

#define MAX_TESTS      6
#define OUTPUT_CAP     4096
#define LOG_CAP        4096
#define EVENTS_CAP     12000
#define VERDICT_CAP    320
#define RUN_TIME_LIMIT 5      // seconds before a student's program is stopped

typedef enum { LANG_C, LANG_CPP } Language;

// One test: text fed to the program's input (stdin) and the output we expect.
// In the games the input also describes the world (a maze or a street), and
// expected may be NULL when it doesn't matter what the program prints.
typedef struct {
    const char *input;
    const char *expected;
} TestCase;

typedef enum {
    RUN_OK,             // the program finished on its own
    RUN_CRASHED,        // it crashed (for example a bad pointer)
    RUN_TIMEOUT,        // it ran longer than RUN_TIME_LIMIT and was stopped
    RUN_TOO_MUCH_OUTPUT,// it printed far too much and was stopped
    RUN_NOT_STARTED,    // the program could not be started at all
} RunStatus;

typedef struct {
    bool      compiled;
    bool      allPassed;
    int       testCount;
    bool      passed[MAX_TESTS];
    bool      outputOk[MAX_TESTS];          // what it printed matched (always true when nothing is expected)
    bool      won[MAX_TESTS];               // games: the game said the run was a success
    RunStatus status[MAX_TESTS];
    char      actual[MAX_TESTS][OUTPUT_CAP];    // what to show: the output, after a note if it crashed or was stopped
    char      output[MAX_TESTS][OUTPUT_CAP];    // exactly what it printed (normalised), for the game levels
    char      events[MAX_TESTS][EVENTS_CAP];   // games: what happened, one event per line
    char      verdict[MAX_TESTS][VERDICT_CAP]; // games: why the run failed (empty when it won)
    char      compileLog[LOG_CAP];
} RunResult;

typedef struct {
    const char     *code;
    Language        lang;
    const char     *prelude;      // put in front of the code, e.g. "#include \"cfd_robot.h\"" (NULL for none)
    const char     *includeDir;   // folder holding the game headers (NULL for none)
    bool            game;         // a test also needs the game's "END WIN" in cfd_events.txt
    const TestCase *tests;
    int             testCount;
} RunRequest;

// Saves the code to a file, compiles it (gcc for C, g++ for C++), runs it once per
// test case with a time limit and compares the output (ignoring trailing spaces
// and blank lines at the end). Games also read the event log the game wrote.
void RunProgram(const RunRequest *req, RunResult *out);

// The plain version: no game, just the code and its tests.
void RunPractice(const char *code, Language lang, const TestCase *tests, int testCount, RunResult *out);

const char *CompilerName(Language lang);

// The comparison rules: drops CR characters, spaces and tabs at the end of each line, and blank
// lines at the end. Two outputs match when their normalised texts are equal.
void NormalizeOutput(const char *src, char *dst, unsigned long cap);

#endif
