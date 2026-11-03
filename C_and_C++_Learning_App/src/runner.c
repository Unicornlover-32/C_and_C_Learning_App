#include "runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Override with e.g. -DCFD_COMPILER=\"clang\" in the Makefile if needed.
#ifndef CFD_COMPILER
#define CFD_COMPILER "gcc"
#endif

#define SRC_FILE "cfd_practice.c"
#define IN_FILE  "cfd_input.txt"
#define OUT_FILE "cfd_output.txt"
#define LOG_FILE "cfd_compile.txt"

#if defined(_WIN32)
    #define EXE_FILE "cfd_practice.exe"
    #define RUN_CMD  "cfd_practice.exe"
#elif defined(__linux__)
    #define EXE_FILE "cfd_practice"
    #define RUN_CMD  "timeout 5 ./cfd_practice"   // stops infinite loops after 5 seconds
#else
    #define EXE_FILE "cfd_practice"
    #define RUN_CMD  "./cfd_practice"
#endif

static bool WriteText(const char *path, const char *text)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    fputs(text, f);
    fclose(f);
    return true;
}

static void ReadText(const char *path, char *dst, size_t cap)
{
    dst[0] = '\0';
    FILE *f = fopen(path, "rb");
    if (!f) return;
    size_t n = fread(dst, 1, cap - 1, f);
    dst[n] = '\0';
    fclose(f);
}

static bool FileThere(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

// Removes '\r', trailing spaces on every line and blank lines at the end,
// so "Hello\r\n" and "Hello  \n\n" both match "Hello".
static void Normalize(const char *src, char *dst, size_t cap)
{
    size_t n = 0, lineStart = 0;
    for (const char *p = src; *p && n + 1 < cap; p++) {
        if (*p == '\r') continue;
        if (*p == '\n') {
            while (n > lineStart && (dst[n - 1] == ' ' || dst[n - 1] == '\t')) n--;
            dst[n++] = '\n';
            lineStart = n;
            continue;
        }
        dst[n++] = *p;
    }
    while (n > lineStart && (dst[n - 1] == ' ' || dst[n - 1] == '\t')) n--;
    while (n > 0 && dst[n - 1] == '\n') n--;
    dst[n] = '\0';
}

void RunPractice(const char *code, const TestCase *tests, int testCount, RunResult *out)
{
    memset(out, 0, sizeof(*out));
    if (testCount > MAX_TESTS) testCount = MAX_TESTS;
    out->testCount = testCount;

    if (!WriteText(SRC_FILE, code)) {
        snprintf(out->compileLog, LOG_CAP, "Could not save your code to %s.", SRC_FILE);
        return;
    }

    // Compile
    remove(EXE_FILE);
    int rc = system(CFD_COMPILER " -Wall -o " EXE_FILE " " SRC_FILE " 2> " LOG_FILE);
    ReadText(LOG_FILE, out->compileLog, LOG_CAP);
    out->compiled = (rc == 0 && FileThere(EXE_FILE));

    if (!out->compiled) {
        if (out->compileLog[0] == '\0')
            snprintf(out->compileLog, LOG_CAP,
                     "The compiler could not be run. Make sure '%s' is installed and on your PATH.",
                     CFD_COMPILER);
        remove(LOG_FILE);
        return;
    }

    // Run every test case
    out->allPassed = true;
    for (int i = 0; i < testCount; i++) {
        char raw[OUTPUT_CAP], got[OUTPUT_CAP], want[OUTPUT_CAP];

        WriteText(IN_FILE, tests[i].input ? tests[i].input : "");
        remove(OUT_FILE);
        int status = system(RUN_CMD " < " IN_FILE " > " OUT_FILE " 2>&1");

        ReadText(OUT_FILE, raw, sizeof(raw));
        Normalize(raw, got, sizeof(got));
        Normalize(tests[i].expected, want, sizeof(want));

        out->passed[i] = (strcmp(got, want) == 0);
        if (!out->passed[i]) out->allPassed = false;

        if (got[0] == '\0' && status != 0)
            snprintf(out->actual[i], OUTPUT_CAP, "(no output - the program crashed or ran too long)");
        else
            snprintf(out->actual[i], OUTPUT_CAP, "%s", got);
    }

    remove(IN_FILE);
    remove(OUT_FILE);
    remove(LOG_FILE);
}
