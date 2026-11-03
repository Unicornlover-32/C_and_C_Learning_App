#ifndef RUNNER_H
#define RUNNER_H

#include <stdbool.h>

#define MAX_TESTS  6
#define OUTPUT_CAP 1024
#define LOG_CAP    4096

// One test: text fed to the program's input (stdin) and the output we expect.
typedef struct {
    const char *input;
    const char *expected;
} TestCase;

typedef struct {
    bool compiled;
    bool allPassed;
    int  testCount;
    bool passed[MAX_TESTS];
    char actual[MAX_TESTS][OUTPUT_CAP];
    char compileLog[LOG_CAP];
} RunResult;

// Saves the code to a file, compiles it with gcc, runs it once per test case
// and compares the output (ignoring trailing spaces and blank lines at the end).
void RunPractice(const char *code, const TestCase *tests, int testCount, RunResult *out);

#endif
