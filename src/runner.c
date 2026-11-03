// Compiles and runs the student's code. Kept separate from raylib on purpose:
// <windows.h> clashes with raylib names, so it is only included here.
#if !defined(_WIN32)
    #define _DEFAULT_SOURCE        // fork, kill, nanosleep, setrlimit with -std=c99
#endif

#include "runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <signal.h>
    #include <sys/resource.h>
    #include <sys/wait.h>
    #include <time.h>
    #include <unistd.h>
#endif

// Override with e.g. -DCFD_CC=\"clang\" in the Makefile if needed.
#ifndef CFD_CC
#define CFD_CC "gcc"
#endif
#ifndef CFD_CXX
#define CFD_CXX "g++"
#endif

#define SRC_C    "cfd_practice.c"
#define SRC_CPP  "cfd_practice.cpp"
#define IN_FILE  "cfd_input.txt"
#define OUT_FILE "cfd_output.txt"
#define LOG_FILE "cfd_compile.txt"

#if defined(_WIN32)
    #define EXE_FILE "cfd_practice.exe"
#else
    #define EXE_FILE "cfd_practice"
#endif

#define OUTPUT_FILE_LIMIT (1L << 20)   // 1 MB of output is plenty for any exercise

const char *CompilerName(Language lang) { return lang == LANG_CPP ? CFD_CXX : CFD_CC; }

// Makes sure gcc/g++ can be found. If they aren't on PATH (for example when the app
// is started by double-clicking it), look in the usual raylib install folders and
// add the first one that has gcc to this program's PATH.
// Returns false if no compiler could be found anywhere.
#if defined(_WIN32)
static bool DirHasGcc(const char *dir)
{
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s\\gcc.exe", dir);
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

static bool EnsureCompilerOnPath(void)
{
    static int state = 0;   // 0 = not checked yet, 1 = found, -1 = missing
    if (state != 0) return state > 0;

    char found[MAX_PATH];
    if (SearchPathA(NULL, CFD_CC ".exe", NULL, MAX_PATH, found, NULL)) { state = 1; return true; }

    // Folder of this app, so a w64devkit next to it (or a few levels up) is found too
    char appDir[MAX_PATH] = "";
    DWORD n = GetModuleFileNameA(NULL, appDir, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        char *slash = strrchr(appDir, '\\');
        if (slash) *slash = '\0';
    }

    char candidates[6][MAX_PATH];
    int count = 0;
    const char *rel[] = { "\\w64devkit\\bin", "\\..\\w64devkit\\bin", "\\..\\..\\w64devkit\\bin",
                          "\\..\\..\\..\\w64devkit\\bin" };
    for (int i = 0; i < 4 && appDir[0]; i++)
        snprintf(candidates[count++], MAX_PATH, "%s%s", appDir, rel[i]);
    snprintf(candidates[count++], MAX_PATH, "C:\\raylib\\w64devkit\\bin");
    snprintf(candidates[count++], MAX_PATH, "C:\\w64devkit\\bin");

    for (int i = 0; i < count; i++) {
        if (!DirHasGcc(candidates[i])) continue;
        // Turn "...\App\..\..\w64devkit\bin" into a clean "C:\raylib\w64devkit\bin"
        char full[MAX_PATH];
        if (!GetFullPathNameA(candidates[i], MAX_PATH, full, NULL)) snprintf(full, sizeof(full), "%s", candidates[i]);
        // _putenv updates both the C runtime's copy (used by system()) and Windows' copy
        static char env[32768];
        const char *old = getenv("PATH");
        snprintf(env, sizeof(env), "PATH=%s;%s", full, old ? old : "");
        _putenv(env);
        state = 1;
        return true;
    }
    state = -1;
    return false;
}
#else
static bool EnsureCompilerOnPath(void) { return true; }
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

// ---------------------------------------------------------------------------
// Running the program with a time limit
// ---------------------------------------------------------------------------
#if defined(_WIN32)

static RunStatus RunWithLimit(void)
{
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };   // handles the child can inherit
    HANDLE hin  = CreateFileA(IN_FILE, GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hout = CreateFileA(OUT_FILE, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);
    if (hin == INVALID_HANDLE_VALUE || hout == INVALID_HANDLE_VALUE) {
        if (hin != INVALID_HANDLE_VALUE) CloseHandle(hin);
        if (hout != INVALID_HANDLE_VALUE) CloseHandle(hout);
        return RUN_NOT_STARTED;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hin;
    si.hStdOutput = hout;
    si.hStdError = hout;

    // ".\" makes Windows run the freshly built file in the current folder, not an
    // older copy that might sit next to the app itself.
    char cmd[] = ".\\" EXE_FILE;   // CreateProcess may modify this buffer
    BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(hin);
    CloseHandle(hout);
    if (!ok) return RUN_NOT_STARTED;

    RunStatus status = RUN_OK;
    DWORD waited = 0;
    for (;;) {
        if (WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0) break;
        waited += 50;

        LARGE_INTEGER size;
        WIN32_FILE_ATTRIBUTE_DATA attr;
        if (GetFileAttributesExA(OUT_FILE, GetFileExInfoStandard, &attr)) {
            size.HighPart = (LONG)attr.nFileSizeHigh;
            size.LowPart = attr.nFileSizeLow;
            if (size.QuadPart > OUTPUT_FILE_LIMIT) { status = RUN_TOO_MUCH_OUTPUT; break; }
        }
        if (waited >= RUN_TIME_LIMIT * 1000) { status = RUN_TIMEOUT; break; }
    }

    if (status != RUN_OK) {
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 2000);
    } else {
        DWORD code = 0;
        GetExitCodeProcess(pi.hProcess, &code);
        if (code >= 0xC0000000u) status = RUN_CRASHED;   // e.g. access violation
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return status;
}

#else   // Linux, macOS

static RunStatus RunWithLimit(void)
{
    fflush(NULL);
    pid_t pid = fork();
    if (pid < 0) return RUN_NOT_STARTED;

    if (pid == 0) {
        // Child: plug the files into stdin/stdout/stderr and start the program
        int in = open(IN_FILE, O_RDONLY);
        int out = open(OUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (in < 0 || out < 0) _exit(127);
        dup2(in, 0);
        dup2(out, 1);
        dup2(out, 2);
        close(in);
        close(out);

        struct rlimit fileLimit = { OUTPUT_FILE_LIMIT, OUTPUT_FILE_LIMIT };
        setrlimit(RLIMIT_FSIZE, &fileLimit);

        execl("./" EXE_FILE, EXE_FILE, (char *)NULL);
        _exit(127);
    }

    // Parent: wait, but no longer than the time limit
    int status = 0;
    struct timespec nap = { 0, 20 * 1000000L };   // 20 ms
    for (int waited = 0;; waited += 20) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) break;
        if (r < 0) return RUN_NOT_STARTED;
        if (waited >= RUN_TIME_LIMIT * 1000) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            return RUN_TIMEOUT;
        }
        nanosleep(&nap, NULL);
    }

    if (WIFSIGNALED(status)) return WTERMSIG(status) == SIGXFSZ ? RUN_TOO_MUCH_OUTPUT : RUN_CRASHED;
    if (WIFEXITED(status) && WEXITSTATUS(status) == 127) return RUN_NOT_STARTED;
    return RUN_OK;
}

#endif

// ---------------------------------------------------------------------------

void RunPractice(const char *code, Language lang, const TestCase *tests, int testCount, RunResult *out)
{
    memset(out, 0, sizeof(*out));
    if (testCount > MAX_TESTS) testCount = MAX_TESTS;
    out->testCount = testCount;

    const char *src = (lang == LANG_CPP) ? SRC_CPP : SRC_C;
    if (!WriteText(src, code)) {
        snprintf(out->compileLog, LOG_CAP, "Could not save your code to %s.", src);
        return;
    }

    if (!EnsureCompilerOnPath()) {
        snprintf(out->compileLog, LOG_CAP,
                 "This isn't a problem with your code - the app couldn't find the C compiler (%s).\n\n"
                 "It looked on your PATH and in C:\\raylib\\w64devkit\\bin.\n"
                 "Install raylib's w64devkit there, or add the folder containing %s.exe to your PATH,\n"
                 "then restart the app.",
                 CompilerName(lang), CompilerName(lang));
        return;
    }

    // Compile
    char cmd[512];
    if (lang == LANG_CPP)
        snprintf(cmd, sizeof(cmd), "%s -Wall -std=c++17 -o %s %s 2> %s", CFD_CXX, EXE_FILE, src, LOG_FILE);
    else
        snprintf(cmd, sizeof(cmd), "%s -Wall -o %s %s -lm 2> %s", CFD_CC, EXE_FILE, src, LOG_FILE);

    remove(EXE_FILE);
    int rc = system(cmd);
    ReadText(LOG_FILE, out->compileLog, LOG_CAP);
    out->compiled = (rc == 0 && FileThere(EXE_FILE));

    if (!out->compiled) {
        if (out->compileLog[0] == '\0')
            snprintf(out->compileLog, LOG_CAP,
                     "The compiler could not be run. Make sure '%s' is installed and on your PATH.",
                     CompilerName(lang));
        remove(LOG_FILE);
        return;
    }

    // Run every test case
    out->allPassed = true;
    for (int i = 0; i < testCount; i++) {
        char raw[OUTPUT_CAP], got[OUTPUT_CAP], want[OUTPUT_CAP];

        WriteText(IN_FILE, tests[i].input ? tests[i].input : "");
        remove(OUT_FILE);
        RunStatus status = RunWithLimit();

        ReadText(OUT_FILE, raw, sizeof(raw));
        Normalize(raw, got, sizeof(got));
        Normalize(tests[i].expected, want, sizeof(want));

        out->status[i] = status;
        out->passed[i] = (status == RUN_OK && strcmp(got, want) == 0);
        if (!out->passed[i]) out->allPassed = false;

        const char *note = NULL;
        switch (status) {
            case RUN_CRASHED:         note = "[your program crashed]"; break;
            case RUN_TIMEOUT:         note = "[stopped: ran longer than 5 seconds - is there a loop that never ends?]"; break;
            case RUN_TOO_MUCH_OUTPUT: note = "[stopped: printed far too much - is there a loop that never ends?]"; break;
            case RUN_NOT_STARTED:     note = "[your program could not be started]"; break;
            default: break;
        }
        if (note && got[0]) snprintf(out->actual[i], OUTPUT_CAP, "%s\n%.700s", note, got);
        else if (note)      snprintf(out->actual[i], OUTPUT_CAP, "%s", note);
        else                snprintf(out->actual[i], OUTPUT_CAP, "%s", got);
    }

    remove(IN_FILE);
    remove(OUT_FILE);
    remove(LOG_FILE);
}
