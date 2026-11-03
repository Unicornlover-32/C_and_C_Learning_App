#ifndef RUNNER_H
#define RUNNER_H

#include <stdbool.h>

#define MAX_TESTS      6
#define OUTPUT_CAP     1024
#define LOG_CAP        4096
#define RUN_TIME_LIMIT 5      // seconds before a student's program is stopped

typedef enum { LANG_C, LANG_CPP } Language;

// One test: text fed to the program's input (stdin) and the output we expect.
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
    RunStatus status[MAX_TESTS];
    char      actual[MAX_TESTS][OUTPUT_CAP];
    char      compileLog[LOG_CAP];
} RunResult;

// Saves the code to a file, compiles it (gcc for C, g++ for C++), runs it once per
// test case with a time limit and compares the output (ignoring trailing spaces
// and blank lines at the end).
void RunPractice(const char *code, Language lang, const TestCase *tests, int testCount, RunResult *out);

const char *CompilerName(Language lang);

#endif
