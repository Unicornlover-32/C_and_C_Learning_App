// make test - checks the projects and the 3D levels' parsers.
//
//   1. Every tutorial's solution passes all of its tests, and every starter fails one.
//   2. For every project with a 3D level, its parser reaches the win state on the
//      solution's real output, and agrees with the runner on the starter's output.
//   3. Wrong output never fools a parser or crashes it: hundreds of changed versions of
//      each expected output (a digit, a letter, a line dropped, doubled, swapped or joined,
//      spaces, cut short, junk bytes, huge lines, CR characters...), each also replayed as
//      a crash and as a timeout. The parser's verdict must always equal the runner's.
//
// Run it from the project folder (the runner writes its files there):
//   ./test_levels           everything
//   ./test_levels --quick   only the projects that have a 3D level
#include "runner.h"
#include "tutorials.h"
#include "missions.h"
#include "projects.h"
#include "game/levels.h"
#include "game/chess_duel.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks, failures;
static RunResult res;
static EventList ev;

#define CHECK(cond, ...)                                                        \
    do {                                                                        \
        checks++;                                                               \
        if (!(cond)) {                                                          \
            failures++;                                                         \
            if (failures <= 40) { printf("    FAIL: "); printf(__VA_ARGS__); printf("\n"); } \
        }                                                                       \
    } while (0)

// The runner's own rule: two outputs match when their normalised texts are equal
static bool SameOutput(const char *a, const char *b)
{
    static char na[OUTPUT_CAP], nb[OUTPUT_CAP];
    NormalizeOutput(a, na, sizeof(na));
    NormalizeOutput(b, nb, sizeof(nb));
    return strcmp(na, nb) == 0;
}

static void ParseRunTrace(const LevelDef *lv, const TestCase *tc, const char *output, RunStatus status, bool passed, const char *trace)
{
    LevelRun run = { tc->input ? tc->input : "", tc->expected, output, status, passed, trace ? trace : "" };
    lv->parse(&run, &ev);
}

static void ParseRun(const LevelDef *lv, const TestCase *tc, const char *output, RunStatus status, bool passed)
{
    ParseRunTrace(lv, tc, output, status, passed, "");
}

// The event list is well formed and ends the way the runner's verdict says it should
static void CheckEvents(bool shouldWin, const char *what)
{
    CHECK(ev.count > 0 && ev.count <= EV_MAX, "%s: %d events", what, ev.count);
    if (ev.count <= 0) return;
    const GameEvent *last = &ev.ev[ev.count - 1];
    if (shouldWin) {
        CHECK(ev.result == EV_WIN && last->type == EV_WIN, "%s: should reach the win state (result %d)", what, ev.result);
    } else {
        CHECK(ev.result == EV_FAIL || ev.result == EV_CRASH, "%s: should reach a fail state (result %d)", what, ev.result);
        CHECK(last->type == EV_FAIL || last->type == EV_CRASH, "%s: the story should end on the failure", what);
    }
    int endings = 0;
    for (int i = 0; i < ev.count; i++) {
        const GameEvent *g = &ev.ev[i];
        if (g->type == EV_WIN || g->type == EV_FAIL || g->type == EV_CRASH) endings++;
        if (g->line < -1 || g->line >= (ev.printedLines > 0 ? ev.printedLines : 1)) {
            CHECK(false, "%s: event %d points at output line %d of %d", what, i, g->line, ev.printedLines);
            break;
        }
        if (!memchr(g->text, 0, EV_TEXT)) { CHECK(false, "%s: event %d text isn't terminated", what, i); break; }
    }
    CHECK(endings == 1, "%s: %d endings instead of 1", what, endings);
}

// ---------------------------------------------------------------- wrong output

static unsigned seed = 12345;
static int Rand(int n)
{
    seed = seed * 1103515245u + 12345u;
    return n > 0 ? (int)((seed >> 16) % (unsigned)n) : 0;
}

static int Count(const char *s, int (*is)(int))
{
    int n = 0;
    for (; *s; s++) if (is((unsigned char)*s)) n++;
    return n;
}

static int IsDigit(int c) { return c >= '0' && c <= '9'; }
static int IsLetter(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static int IsSpace(int c) { return c == ' '; }
static int IsNewline(int c) { return c == '\n'; }

// Position of the k-th character for which is() holds
static int Nth(const char *s, int (*is)(int), int k)
{
    for (int i = 0; s[i]; i++) if (is((unsigned char)s[i]) && k-- == 0) return i;
    return -1;
}

// Start and length of line k
static void LineSpan(const char *s, int k, int *start, int *len)
{
    int i = 0;
    while (k > 0 && s[i]) { if (s[i] == '\n') k--; i++; }
    int e = i;
    while (s[e] && s[e] != '\n') e++;
    *start = i;
    *len = e - i;
}

#define BIG 20000

// Makes one changed version of the expected output
static void Mutate(const char *exp, char *out, int kind)
{
    char tmp[BIG];
    int n = (int)strlen(exp);
    int lines = Count(exp, IsNewline) + 1;
    snprintf(out, BIG, "%s", exp);
    switch (kind) {
        case 0: {                                   // a different digit
            int d = Count(exp, IsDigit);
            if (d) { int i = Nth(exp, IsDigit, Rand(d)); out[i] = (char)('0' + (out[i] - '0' + 1 + Rand(9)) % 10); }
            break;
        }
        case 1: {                                   // a different letter
            int d = Count(exp, IsLetter);
            if (d) { int i = Nth(exp, IsLetter, Rand(d)); out[i] = (out[i] == 'z' || out[i] == 'Z') ? 'a' : (char)(out[i] + 1); }
            break;
        }
        case 2: case 3: {                           // a line dropped, or doubled
            int s, len;
            LineSpan(exp, Rand(lines), &s, &len);
            if (kind == 2) snprintf(out, BIG, "%.*s%s", s, exp, exp + s + len + (exp[s + len] ? 1 : 0));
            else snprintf(out, BIG, "%.*s%.*s\n%s", s + len, exp, len, exp + s, exp + s + len);
            break;
        }
        case 4: {                                   // two lines swapped
            if (lines < 2) break;
            int k = Rand(lines - 1), s1, l1, s2, l2;
            LineSpan(exp, k, &s1, &l1);
            LineSpan(exp, k + 1, &s2, &l2);
            snprintf(out, BIG, "%.*s%.*s\n%.*s%s", s1, exp, l2, exp + s2, l1, exp + s1, exp + s2 + l2);
            break;
        }
        case 5: {                                   // two lines joined
            int c = Count(exp, IsNewline);
            if (c) { int i = Nth(exp, IsNewline, Rand(c)); snprintf(out, BIG, "%.*s%s", i, exp, exp + i + 1); }
            break;
        }
        case 6: {                                   // a line split in two
            if (n) { int i = Rand(n); snprintf(out, BIG, "%.*s\n%s", i, exp, exp + i); }
            break;
        }
        case 7: {                                   // an extra space
            int i = Rand(n + 1);
            snprintf(out, BIG, "%.*s %s", i, exp, exp + i);
            break;
        }
        case 8: {                                   // a space missing
            int d = Count(exp, IsSpace);
            if (d) { int i = Nth(exp, IsSpace, Rand(d)); snprintf(out, BIG, "%.*s%s", i, exp, exp + i + 1); }
            break;
        }
        case 9: {                                   // spaces at the end of a line (still right)
            int s, len;
            LineSpan(exp, Rand(lines), &s, &len);
            snprintf(out, BIG, "%.*s   %s", s + len, exp, exp + s + len);
            break;
        }
        case 10: snprintf(out, BIG, "%s\n\n\n", exp); break;          // blank lines at the end (still right)
        case 11: if (n) out[Rand(n)] = '\0'; break;                    // cut short
        case 12: out[0] = '\0'; break;                                 // nothing at all
        case 13: {                                  // junk bytes
            int len = 1 + Rand(3000);
            for (int i = 0; i < len; i++) out[i] = (char)(1 + Rand(255));
            out[len] = '\0';
            break;
        }
        case 14: {                                  // one enormous line
            int len = 5000 + Rand(4000);
            for (int i = 0; i < len; i++) out[i] = (i % 2) ? ' ' : (char)('0' + Rand(10));
            out[len] = '\0';
            break;
        }
        case 15: {                                  // the first line, hundreds of times
            int s, len, o = 0;
            LineSpan(exp, 0, &s, &len);
            for (int i = 0; i < 400 && o + len + 2 < BIG; i++) { memcpy(out + o, exp + s, (size_t)len); o += len; out[o++] = '\n'; }
            out[o] = '\0';
            break;
        }
        case 16: {                                  // CR characters (dropped by the runner: still right)
            int o = 0;
            for (int i = 0; exp[i] && o < BIG - 3; i++) { if (Rand(4) == 0) out[o++] = '\r'; out[o++] = exp[i]; }
            out[o] = '\0';
            break;
        }
        case 17: {                                  // a number far too big
            int d = Count(exp, IsDigit);
            if (d) { int i = Nth(exp, IsDigit, Rand(d)); snprintf(out, BIG, "%.*s99999999999999999999%s", i, exp, exp + i + 1); }
            break;
        }
        case 18: {                                  // a minus sign
            int d = Count(exp, IsDigit);
            if (d) { int i = Nth(exp, IsDigit, Rand(d)); snprintf(out, BIG, "%.*s-%s", i, exp, exp + i); }
            break;
        }
        case 19: {                                  // tabs for spaces
            for (int i = 0; out[i]; i++) if (out[i] == ' ' && Rand(2)) out[i] = '\t';
            break;
        }
        case 20: snprintf(out, BIG, "%s\n%s", exp, exp); break;        // everything twice
        case 21: {                                  // the output with no newlines
            int o = 0;
            for (int i = 0; exp[i]; i++) if (exp[i] != '\n') out[o++] = exp[i];
            out[o] = '\0';
            break;
        }
        default: {                                  // two changes at once
            Mutate(exp, tmp, Rand(22));
            Mutate(tmp, out, Rand(22));
            break;
        }
    }
}

static void FuzzTests(const LevelDef *lv, const char *id, const TestCase *tests, int testCount, int changes)
{
    static char mut[BIG], norm[OUTPUT_CAP];
    for (int t = 0; t < testCount; t++) {
        const TestCase *tc = &tests[t];
        char what[96];

        // the preview (nothing has run) builds a world and nothing else
        ParseRun(lv, tc, NULL, RUN_NOT_STARTED, false);
        CHECK(ev.preview && ev.result == EV_SPAWN, "%s test %d: preview", id, t + 1);

        // the right output, ended normally, by a crash and by a timeout
        NormalizeOutput(tc->expected, norm, sizeof(norm));
        snprintf(what, sizeof(what), "%s test %d: expected output", id, t + 1);
        ParseRun(lv, tc, norm, RUN_OK, true);
        CheckEvents(true, what);
        ParseRun(lv, tc, norm, RUN_CRASHED, false);
        CheckEvents(false, what);
        ParseRun(lv, tc, norm, RUN_TIMEOUT, false);
        CheckEvents(false, what);

        for (int k = 0; k < changes; k++) {
            int kind = k < 22 * 8 ? k % 22 : 22;
            Mutate(tc->expected, mut, kind);
            NormalizeOutput(mut, norm, sizeof(norm));        // exactly what the runner would hand over
            bool right = SameOutput(norm, tc->expected);
            snprintf(what, sizeof(what), "%s test %d, change %d (kind %d)", id, t + 1, k, kind);
            ParseRun(lv, tc, norm, RUN_OK, right);
            CheckEvents(right, what);
            RunStatus stopped = (k & 1) ? RUN_CRASHED : RUN_TOO_MUCH_OUTPUT;
            ParseRun(lv, tc, norm, stopped, false);
            CheckEvents(false, what);
        }
    }
}

static void FuzzLevel(const LevelDef *lv, const Tutorial *tu)
{
    int before = failures;
    FuzzTests(lv, tu->id, tu->tests, tu->testCount, 400);
    printf("  %-24s %-18s parser: %s\n", tu->id, lv->name, failures == before ? "ok" : "FAILED");
}

// Runs a program and checks the level's parser agrees with the runner on every test
static void CheckProgram(const LevelDef *lv, const char *id, const char *code, Language lang, const TestCase *tests, int count, bool shouldPass)
{
    RunPractice(code, lang, tests, count, &res);
    CHECK(res.compiled, "%s: doesn't compile:\n%s", id, res.compileLog);
    if (shouldPass) CHECK(res.allPassed, "%s: should pass every test", id);
    if (!res.compiled) return;
    for (int t = 0; t < res.testCount; t++) {
        char what[96];
        snprintf(what, sizeof(what), "%s test %d", id, t + 1);
        ParseRun(lv, &tests[t], res.output[t], res.status[t], res.passed[t]);
        CheckEvents(res.passed[t], what);
    }
}

// ---------------------------------------------------------------- game-library levels

// The runner's rule for games: the last line starting with "END " says whether the game was won
static bool TraceWon(const char *trace)
{
    const char *last = NULL;
    for (const char *p = trace; (p = strstr(p, "END ")) != NULL; p++)
        if (p == trace || p[-1] == '\n') last = p;
    return last && strncmp(last, "END WIN", 7) == 0;
}

static char *Append(char *at, char *end, const char *text)
{
    while (*text && at < end - 1) *at++ = *text++;
    *at = '\0';
    return at;
}

// The k-th line of s (without its newline), or NULL
static const char *NthLine(const char *s, int k, int *len)
{
    for (int i = 0; i < k && s; i++) { s = strchr(s, '\n'); if (s) s++; }
    if (!s || !*s) return NULL;
    const char *e = strchr(s, '\n');
    *len = e ? (int)(e - s) : (int)strlen(s);
    return s;
}

static int LineCount(const char *s)
{
    int n = 0;
    for (; *s; s++) if (*s == '\n') n++;
    return n;
}

// A changed copy of a real trace: the endings, the actions and the format all go wrong
static void MutateTrace(const char *trace, char *out, int cap, int kind)
{
    static const char *JUNK[] = { "9 M", "8 X", "7 L", "COPY 99 0", "COPY 7 0", "COPY 3 9", "COPY 1 5", "-1 L", "0 Q", "0 MM",
                                  "", "COPY", "END", "END WINNER", "0 M\r", "  0 M", "0  M", "COPY 0 0", "COPY 12345 1",
                                  "W z9 a1 - x", "W a1 a1 Q #", "B h8", "W e2 e4 K +#", "I", "I e9 e4", "W a1 h8 - xxxxxxxxxxxx",
                                  "B e7 e5 q -", "W a7 a8 Q #extra", "G 3", "T 1 2" };
    char *at = out, *end = out + cap;
    out[0] = '\0';
    int lines = LineCount(trace);
    switch (kind) {
        case 0: Append(at, end, trace); break;
        case 1: {                                                       // the game says no
            const char *w = strstr(trace, "END WIN");
            if (!w) { Append(at, end, trace); break; }
            snprintf(out, (size_t)cap, "%.*sEND FAIL The robot got lost on the way.\n", (int)(w - trace), trace);
            break;
        }
        case 2: {                                                       // no ending at all
            const char *w = strstr(trace, "END ");
            snprintf(out, (size_t)cap, "%.*s", w ? (int)(w - trace) : (int)strlen(trace), trace);
            break;
        }
        case 3: snprintf(out, (size_t)cap, "%.*s", Rand((int)strlen(trace) + 1), trace); break;   // cut anywhere
        case 4: {                                                       // a junk line somewhere
            int k = Rand(lines + 1);
            for (int i = 0; i <= lines; i++) {
                if (i == k) { at = Append(at, end, JUNK[Rand((int)(sizeof(JUNK) / sizeof(JUNK[0])))]); at = Append(at, end, "\n"); }
                int len;
                const char *l = NthLine(trace, i, &len);
                if (!l) continue;
                char one[400];
                snprintf(one, sizeof(one), "%.*s\n", len < 380 ? len : 380, l);
                at = Append(at, end, one);
            }
            break;
        }
        case 5: {                                                       // one action repeated hundreds of times
            int k = Rand(lines > 0 ? lines : 1), len = 0;
            const char *l = NthLine(trace, k, &len);
            char one[64];
            snprintf(one, sizeof(one), "%.*s\n", len < 60 ? len : 60, l ? l : "0 M");
            for (int i = 0; i < 400; i++) at = Append(at, end, one);
            Append(at, end, trace);
            break;
        }
        case 6: case 7:                                                 // the whole battery: move, turn, move, turn...
            for (int i = 0; i < 3000; i++) at = Append(at, end, (i & 1) ? "0 L\n" : (i % 6 == 0) ? "0 X\n" : "0 M\n");
            Append(at, end, kind == 6 ? "END WIN\n" : "END FAIL The battery ran out.\n");
            break;
        case 8:                                                         // noise
            for (int i = 0; i < 300 && at < end - 1; i++) { char c = (char)(1 + Rand(126)); *at++ = c; }
            *at = '\0';
            break;
        case 9: break;                                                  // nothing at all
        case 10: {                                                      // two lines swapped
            int a = Rand(lines + 1), b = Rand(lines + 1);
            for (int i = 0; i <= lines; i++) {
                int len;
                const char *l = NthLine(trace, i == a ? b : i == b ? a : i, &len);
                if (!l) continue;
                char one[400];
                snprintf(one, sizeof(one), "%.*s\n", len < 380 ? len : 380, l);
                at = Append(at, end, one);
            }
            break;
        }
        case 11:                                                        // a crowd of holograms
            for (int i = 1; i < 24; i++) { char one[32]; snprintf(one, sizeof(one), "COPY %d %d\n%d M\n", i % 9, Rand(3), i % 9); at = Append(at, end, one); }
            Append(at, end, trace);
            break;
        case 12: at = Append(at, end, trace); at = Append(at, end, "END FAIL first\n"); Append(at, end, "END WIN\n"); break;
        default: at = Append(at, end, "END WIN\n0 M\n0 R\n"); Append(at, end, trace); break;
    }
}

// Every test of a game mission: the real run's output and trace, then hundreds of changed
// versions of both, each checked against the runner's own verdict
static void FuzzGame(const LevelDef *lv, const MissionInfo *m, const RunResult *real, int changes)
{
    static char trace[40000], mut[BIG], norm[OUTPUT_CAP];
    for (int t = 0; t < real->testCount; t++) {
        const TestCase *tc = &m->tests[t];
        char what[128];
        ParseRunTrace(lv, tc, NULL, RUN_NOT_STARTED, false, "");
        CHECK(ev.preview && ev.result == EV_SPAWN, "%s test %d: preview", m->id, t + 1);
        for (int k = 0; k < changes; k++) {
            int tk = k < 14 * 6 ? k % 14 : Rand(14);
            MutateTrace(real->events[t], trace, sizeof(trace), tk);
            const char *out = real->output[t];
            if (k % 3 == 1) {                                         // the output goes wrong too
                Mutate(tc->expected ? tc->expected : real->output[t], mut, Rand(23));
                NormalizeOutput(mut, norm, sizeof(norm));
                out = norm;
            }
            RunStatus status = k % 5 == 3 ? RUN_CRASHED : k % 5 == 4 ? RUN_TIMEOUT : RUN_OK;
            bool passed = status == RUN_OK && (!tc->expected || SameOutput(out, tc->expected)) && TraceWon(trace);
            snprintf(what, sizeof(what), "%s test %d, change %d (trace kind %d, status %d)", m->id, t + 1, k, tk, status);
            ParseRunTrace(lv, tc, out, status, passed, trace);
            CheckEvents(passed, what);
        }
    }
}

// One turn of a 1v1 game: a program for the side to move, which should (or shouldn't) be played
static void DuelCase(const char *what, const char *fen, const char *body, bool shouldPlay, const char *traceHas)
{
    static char input[200], code[800];
    snprintf(input, sizeof(input), "FEN %s\nEND\n", fen);
    snprintf(code, sizeof(code), "#include <stdio.h>\nint main(void)\n{\n    %s\n    return 0;\n}\n", body);
    TestCase tc = { input, NULL };
    RunRequest req = { code, LANG_C, "#define CFD_DUEL\n#include \"cfd_chess.h\"\n", "game", true, &tc, 1 };
    RunProgram(&req, &res);
    CHECK(res.compiled, "1v1 %s: doesn't compile:\n%s", what, res.compileLog);
    CHECK(res.passed[0] == shouldPlay, "1v1 %s: the move should%s be played (trace: %s)", what, shouldPlay ? "" : "n't", res.events[0]);
    if (traceHas) CHECK(strstr(res.events[0], traceHas) != NULL, "1v1 %s: the trace should have \"%s\" (it is: %s)", what, traceHas, res.events[0]);
}

static void CheckGameMission(const LevelDef *lv, const MissionInfo *m, bool quick)
{
    static RunResult real;
    int before = failures;
    RunRequest req = { m->solution, m->lang, m->prelude, "game", true, m->tests, m->testCount };
    RunProgram(&req, &real);
    CHECK(real.compiled, "%s: the solution doesn't compile:\n%s", m->id, real.compileLog);
    CHECK(real.allPassed, "%s: the solution should pass every test", m->id);
    for (int t = 0; real.compiled && t < real.testCount; t++) {
        char what[96];
        snprintf(what, sizeof(what), "%s test %d: the solution's run", m->id, t + 1);
        ParseRunTrace(lv, &m->tests[t], real.output[t], real.status[t], real.passed[t], real.events[t]);
        CheckEvents(real.passed[t], what);
    }
    if (!quick) {
        RunRequest sreq = { m->starter, m->lang, m->prelude, "game", true, m->tests, m->testCount };
        RunProgram(&sreq, &res);
        CHECK(!res.compiled || !res.allPassed, "%s: the starter code should fail a test", m->id);
        for (int t = 0; res.compiled && t < res.testCount; t++) {
            char what[96];
            snprintf(what, sizeof(what), "%s test %d: the starter's run", m->id, t + 1);
            ParseRunTrace(lv, &m->tests[t], res.output[t], res.status[t], res.passed[t], res.events[t]);
            CheckEvents(res.passed[t], what);
        }
    }
    if (real.compiled) FuzzGame(lv, m, &real, 400);
    printf("  %-24s %s\n", m->id, failures == before ? "ok" : "FAILED");
}

int main(int argc, char **argv)
{
    bool quick = argc > 1 && strcmp(argv[1], "--quick") == 0;
    printf("Projects (solutions must pass, starters must fail):\n");
    for (int i = 0; i < TutorialCount(); i++) {
        const Tutorial *tu = GetTutorial(i);
        const LevelDef *lv = LevelFor(tu->id);
        if (quick && !lv) continue;
        int before = failures;

        RunPractice(tu->solution, tu->lang, tu->tests, tu->testCount, &res);
        CHECK(res.compiled, "%s: the solution doesn't compile:\n%s", tu->id, res.compileLog);
        CHECK(res.allPassed, "%s: the solution should pass every test (statuses: %d %d %d %d %d %d - 1 crashed, 2 timed out, 4 couldn't start)",
              tu->id, res.status[0], res.status[1], res.status[2], res.status[3], res.status[4], res.status[5]);
        if (lv && res.compiled)
            for (int t = 0; t < res.testCount; t++) {
                char what[96];
                snprintf(what, sizeof(what), "%s test %d: the solution's output", tu->id, t + 1);
                ParseRun(lv, &tu->tests[t], res.output[t], res.status[t], res.passed[t]);
                CheckEvents(res.passed[t], what);
            }

        RunPractice(tu->starterCode, tu->lang, tu->tests, tu->testCount, &res);
        CHECK(!res.allPassed, "%s: the starter code should fail a test", tu->id);
        if (lv && res.compiled)
            for (int t = 0; t < res.testCount; t++) {
                char what[96];
                snprintf(what, sizeof(what), "%s test %d: the starter's output", tu->id, t + 1);
                ParseRun(lv, &tu->tests[t], res.output[t], res.status[t], res.passed[t]);
                CheckEvents(res.passed[t], what);
            }
        printf("  %-24s %s\n", tu->id, failures == before ? "ok" : "FAILED");
    }

    printf("Bug hunts (every bug placed, the broken code compiles but fails, one per level):\n");
    MissionsInit();
    int hunts[64] = { 0 };
    for (int t = 0; t < MissionTableCount(); t++) {
        const Mission *m = GetMissionTable(t);
        if (m->mode != MODE_BUGS) continue;
        int mi = FindMission(m->id);
        CHECK(mi >= 0, "%s: not in the registry", m->id);
        if (mi < 0) continue;
        const MissionInfo *info = GetMission(mi);
        if (info->tut >= 0 && info->tut < 64) hunts[info->tut]++;
        if (quick && !LevelFor(GetTutorial(info->tut)->id)) continue;
        int before = failures;
        CHECK(info->bugCount == m->bugCount, "%s: only %d of %d bugs were found in the solution", m->id, info->bugCount, m->bugCount);
        RunPractice(info->starter, info->lang, info->tests, info->testCount, &res);
        // (some hunts break the build on purpose - a typo in an #include is a bug too)
        CHECK(!res.compiled || !res.allPassed, "%s: the broken code should fail a test", m->id);
        printf("  %-24s %s\n", m->id, failures == before ? "ok" : "FAILED");
    }
    for (int i = 0; i < TutorialCount() && i < 64; i++) CHECK(hunts[i] == 1, "%s has %d bug hunts (should be 1)", GetTutorial(i)->id, hunts[i]);

    printf("Level parsers on wrong output:\n");
    for (int i = 0; i < LevelTableCount(); i++) {
        const LevelDef *lv = LevelTableAt(i);
        const Tutorial *tu = NULL;
        for (int t = 0; t < TutorialCount(); t++) if (strcmp(GetTutorial(t)->id, lv->tutId) == 0) tu = GetTutorial(t);
        CHECK(tu != NULL, "level %s: no tutorial %s", lv->name, lv->tutId);
        if (tu) FuzzLevel(lv, tu);
    }

    printf("The Assembly Line (Factory Contracts and the factory boss):\n");
    for (int i = 0; i < ProjectCount(); i++) {
        const Project *p = GetProject(i);
        int before = failures;
        for (int s = 0; s < p->stepCount; s++) {
            const ProjectStep *st = &p->steps[s];
            char id[96];
            snprintf(id, sizeof(id), "%s step %d", p->id, s + 1);
            // each step's solution passes; where the student starts it (the step before) is
            // parsed too, whatever it does
            if (!quick) {
                CheckProgram(&LEVEL_FACTORY, id, st->solution, p->lang, st->tests, st->testCount, true);
                CheckProgram(&LEVEL_FACTORY, id, s > 0 ? p->steps[s - 1].solution : p->starterCode, p->lang, st->tests, st->testCount, false);
            }
            FuzzTests(&LEVEL_FACTORY, id, st->tests, st->testCount, 60);
        }
        printf("  %-24s %s\n", p->id, failures == before ? "ok" : "FAILED");
    }
    for (int i = 0; i < MissionCount(); i++) {
        const MissionInfo *m = GetMission(i);
        if (m->mode != MODE_FACTORY || (m->tut >= 0 && m->role == ROLE_MAIN)) continue;
        int before = failures;
        if (!quick) {
            CheckProgram(&LEVEL_FACTORY, m->id, m->solution, m->lang, m->tests, m->testCount, true);
            CheckProgram(&LEVEL_FACTORY, m->id, m->starter, m->lang, m->tests, m->testCount, false);
        }
        FuzzTests(&LEVEL_FACTORY, m->id, m->tests, m->testCount, 400);
        printf("  %-24s %s\n", m->id, failures == before ? "ok" : "FAILED");
    }

    printf("Robot Rescue (the robot library's trace, replayed):\n");
    for (int i = 0; i < MissionCount(); i++)
        if (GetMission(i)->mode == MODE_ROBOT) CheckGameMission(&LEVEL_ROBOT, GetMission(i), quick);
    printf("Memory Heist (the heist library's trace, replayed):\n");
    for (int i = 0; i < MissionCount(); i++)
        if (GetMission(i)->mode == MODE_HEIST) CheckGameMission(&LEVEL_HEIST, GetMission(i), quick);

    // The chess library's rules, counted the standard way: the number of move sequences of
    // each length from well-known positions (castling, en passant, promotion and pins included)
    printf("Chess Puzzles (the rules, then every puzzle):\n");
    {
        static const struct { const char *fen, *counts; } PERFT[] = {
            { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", "20\n400\n8902\n" },
            { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", "48\n2039\n97862\n" },
            { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", "14\n191\n2812\n" },
            { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", "6\n264\n9467\n" },
            { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", "44\n1486\n62379\n" },
        };
        static char inputs[5][160];
        TestCase tests[5];
        for (int i = 0; i < 5; i++) {
            snprintf(inputs[i], sizeof(inputs[i]), "FEN %s\nGOAL MATE 1\nEND\n", PERFT[i].fen);
            tests[i] = (TestCase){ inputs[i], PERFT[i].counts };
        }
        const char *code = "#include <stdio.h>\n"
                           "int main(void)\n"
                           "{\n"
                           "    for (int d = 1; d <= 3; d++) printf(\"%ld\\n\", cfd_perft(&cfd_pos, d));\n"
                           "    return 0;\n"
                           "}\n";
        RunRequest req = { code, LANG_C, "#define CFD_PERFT\n#include \"cfd_chess.h\"\n", "game", true, tests, 5 };
        int before = failures;
        RunProgram(&req, &res);
        CHECK(res.compiled, "chess perft: doesn't compile:\n%s", res.compileLog);
        for (int i = 0; res.compiled && i < 5; i++)
            CHECK(res.outputOk[i], "chess perft position %d: got %s", i + 1, res.output[i]);
        printf("  %-24s %s\n", "the rules (perft)", failures == before ? "ok" : "FAILED");
    }
    for (int i = 0; i < MissionCount(); i++)
        if (GetMission(i)->mode == MODE_CHESS) CheckGameMission(&LEVEL_CHESS, GetMission(i), quick);

    printf("1v1 chess (one coded move per turn, the game rules, the computer):\n");
    {
        int before = failures;
        DuelCase("White's first move", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "Square *p = at(\"e2\"); move(p, p + 16);", true, "W e2 e4 - -");
        DuelCase("Black's answer", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                 "Square *p = at(\"e7\"); move(p, p - 16);", true, "B e7 e5 - -");
        DuelCase("Black moving a white piece", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                 "move(at(\"e4\"), at(\"e5\"));", false, "I e4 e5");
        DuelCase("two moves in one turn", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "move(at(\"e2\"), at(\"e4\")); move(at(\"d2\"), at(\"d4\"));", false, "END FAIL One move per turn");
        DuelCase("no move at all", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "printf(\"hmm\\n\");", false, "END FAIL");
        DuelCase("Black promotes (and mates)", "8/8/8/8/8/7k/p7/7K b - - 0 1", "promote(at(\"a2\"), at(\"a1\"), 'Q');", true, "B a2 a1 Q #");
        DuelCase("Black castles", "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", "move(at(\"e8\"), at(\"g8\"));", true, "B e8 g8 - c");
        DuelCase("Black en passant", "8/8/8/8/3Pp3/8/8/K6k b - d3 0 1", "move(at(\"e4\"), at(\"d3\"));", true, "B e4 d3 - e");
        DuelCase("a black pawn moving backwards", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                 "Square *p = at(\"d7\"); move(p - 16, p - 8);", false, "END FAIL");
        // the second way to move: pick up a piece and point hand at its new square
        DuelCase("way 2, hand = at()", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "pick_up(at(\"e2\")); hand = at(\"e4\");", true, "W e2 e4 - -");
        DuelCase("way 2, a knight with hand += 15", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "pick_up(at(\"g1\")); hand += 15;", true, "W g1 f3 - -");
        DuelCase("way 2, Black with hand -= 16", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                 "pick_up(at(\"e7\")); hand -= 16;", true, "B e7 e5 - -");
        DuelCase("way 2, put_down() then end", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "pick_up(at(\"d2\")); hand += 16; put_down();", true, "W d2 d4 - -");
        DuelCase("way 2, hand off the board at the end", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "pick_up(at(\"h2\")); hand += 64;", false, "hand points outside the board");
        DuelCase("way 2, put straight back", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "pick_up(at(\"e2\"));", false, "straight back");
        DuelCase("way 2, the other side's piece", "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
                 "pick_up(at(\"e4\"));", false, "only pick up your own pieces");
        DuelCase("way 2, a blocked bishop", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "pick_up(at(\"f1\")); hand += 9 * 2;", false, "in the way");
        DuelCase("way 2, hand without pick_up", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                 "hand = at(\"e4\");", false, "nothing was picked up");
        printf("  %-24s %s\n", "the library, both sides", failures == before ? "ok" : "FAILED");

        // puzzles solved the second way, several moves in one program
        before = failures;
        static const struct { const char *id, *body; } WAY2[] = {
            { "chess-backrank", "pick_up(at(\"a1\")); hand += 8 * 7;" },
            { "chess-fork", "pick_up(at(\"b5\")); hand += 17; put_down();\n    pick_up(at(\"c7\")); hand = find('q');" },
            { "chess-roller", "pick_up(at(\"b2\")); hand += 8 * 5; put_down();\n    pick_up(at(\"a1\")); hand = at(\"a8\");" },
            { "chess-pawnrace", "for (Square *p = board; p < board + 64; p++) {\n"
                                "        if (p->piece != 'P') continue;\n"
                                "        int clear = 1;\n"
                                "        for (Square *s = p; s->rank < 7; ) { s += 8; if (s->piece != '.') clear = 0; }\n"
                                "        if (!clear) continue;\n"
                                "        for (Square *pawn = p; pawn->rank < 7; pawn += 8) { pick_up(pawn); hand += 8; put_down(); }\n"
                                "        return 0;\n"
                                "    }" },
        };
        for (int i = 0; i < (int)(sizeof(WAY2) / sizeof(WAY2[0])); i++) {
            int mi = FindMission(WAY2[i].id);
            CHECK(mi >= 0, "%s: not found", WAY2[i].id);
            if (mi < 0) continue;
            const MissionInfo *m = GetMission(mi);
            static char code[1200];
            snprintf(code, sizeof(code), "#include <stdio.h>\nint main(void)\n{\n    %s\n    return 0;\n}\n", WAY2[i].body);
            RunRequest req = { code, m->lang, m->prelude, "game", true, m->tests, m->testCount };
            RunProgram(&req, &res);
            CHECK(res.compiled && res.allPassed, "%s solved with pick_up() and hand should pass (%s)", WAY2[i].id, res.compiled ? res.events[0] : res.compileLog);
        }
        printf("  %-24s %s\n", "puzzles, the second way", failures == before ? "ok" : "FAILED");

        before = failures;
        static ChessGame g;
        char fen[100], flags[8];
        ChessGameStart(&g);
        ChessGameFen(&g, fen, sizeof(fen));
        CHECK(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == 0, "start FEN: %s", fen);
        // the fool's mate: f3 e5 g4 Qh4#
        static const int fool[4][2] = { { 13, 21 }, { 52, 36 }, { 14, 30 }, { 59, 31 } };
        for (int i = 0; i < 4; i++) {
            ChessMove m;
            CHECK(ChessGameFind(&g, fool[i][0], fool[i][1], 0, &m), "fool's mate move %d isn't legal", i + 1);
            ChessGamePlay(&g, m, flags, sizeof(flags));
            if (i == 0) { ChessGameFen(&g, fen, sizeof(fen)); CHECK(strcmp(fen, "rnbqkbnr/pppppppp/8/8/8/5P2/PPPPP1PP/RNBQKBNR b KQkq - 0 1") == 0, "after f3: %s", fen); }
        }
        CHECK(ChessGameState(&g) == GAME_CHECKMATE, "the fool's mate should be checkmate (state %d)", ChessGameState(&g));
        CHECK(strcmp(g.notation[3], "Qd8-h4#") == 0 && strcmp(flags, "#") == 0, "fool's mate notation %s flags %s", g.notation[3], flags);
        ChessGameFromFen(&g, "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
        CHECK(ChessGameState(&g) == GAME_STALEMATE, "stalemate (state %d)", ChessGameState(&g));
        ChessGameFromFen(&g, "8/8/8/4k3/8/8/8/2B1K3 w - - 0 1");
        CHECK(ChessGameState(&g) == GAME_MATERIAL, "king and bishop against a king is a draw");
        ChessGameFromFen(&g, "8/8/8/4k3/8/8/8/R3K3 w - - 0 1");
        CHECK(ChessGameState(&g) == GAME_ON, "a rook can still mate");
        ChessGameFromFen(&g, "8/8/8/4k3/8/8/8/R3K3 w - - 100 80");
        CHECK(ChessGameState(&g) == GAME_FIFTY, "the fifty-move rule");
        ChessGameStart(&g);
        static const int shuffle[4][2] = { { 6, 21 }, { 62, 45 }, { 21, 6 }, { 45, 62 } };
        for (int rep = 0; rep < 2; rep++)
            for (int i = 0; i < 4; i++) {
                ChessMove m;
                if (ChessGameFind(&g, shuffle[i][0], shuffle[i][1], 0, &m)) ChessGamePlay(&g, m, flags, sizeof(flags));
            }
        CHECK(ChessGameState(&g) == GAME_REPETITION, "threefold repetition (state %d)", ChessGameState(&g));
        printf("  %-24s %s\n", "the game rules", failures == before ? "ok" : "FAILED");

        // the computer: legal moves, mate in one for either side, a free queen, whole games, and speed
        before = failures;
        static const char *MATES[] = { "6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1", "6rk/6pp/8/6N1/8/8/8/6K1 w - - 0 1", "7k/Q7/6K1/8/8/8/8/8 w - - 0 1",
                                       "k7/pp6/8/8/8/8/8/4R1K1 w - - 0 1", "r5k1/8/8/8/8/8/5PPP/6K1 b - - 0 1" };
        for (int level = 0; level < 3; level++) {
            for (int i = 0; i < 5; i++) {
                ChessGameFromFen(&g, MATES[i]);
                ChessMove m = ChessComputerMove(&g, level), found;
                CHECK(ChessGameFind(&g, m.from, m.to, m.promo, &found), "level %d, mate %d: an illegal move", level, i + 1);
                ChessGamePlay(&g, m, flags, sizeof(flags));
                CHECK(ChessGameState(&g) == GAME_CHECKMATE, "level %d should find the mate in position %d", level, i + 1);
            }
            ChessGameFromFen(&g, "6k1/5ppp/8/3q4/8/2N5/5PPP/6K1 w - - 0 1");
            ChessMove m = ChessComputerMove(&g, level);
            if (level > 0) CHECK(m.from == 18 && m.to == 35, "level %d should take the free queen", level);
        }
        clock_t t0 = clock();
        ChessGameFromFen(&g, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");   // a busy middlegame
        ChessMove first = ChessComputerMove(&g, 2), ok;
        double secs = (double)(clock() - t0) / CLOCKS_PER_SEC;
        CHECK(ChessGameFind(&g, first.from, first.to, first.promo, &ok), "hard: an illegal move");
        CHECK(secs < 4.0, "hard takes %.1f seconds for a middlegame move", secs);
        for (int game = 0; game < 3; game++) {
            ChessGameStart(&g);
            int wl = game == 0 ? 1 : game == 1 ? 0 : 2, bl = game == 0 ? 0 : game == 1 ? 1 : 1, plies = 0;
            while (ChessGameState(&g) == GAME_ON && plies < 160) {
                ChessMove m = ChessComputerMove(&g, g.white ? wl : bl), legal;
                if (!ChessGameFind(&g, m.from, m.to, m.promo, &legal)) { CHECK(false, "game %d: an illegal move at ply %d", game + 1, plies); break; }
                ChessGamePlay(&g, legal, flags, sizeof(flags));
                plies++;
            }
            checks++;
        }
        printf("  %-24s %s  (hard, a middlegame move: %.2f s)\n", "the computer player", failures == before ? "ok" : "FAILED", secs);
    }

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
