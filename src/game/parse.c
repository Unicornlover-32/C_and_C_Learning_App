// Shared pieces for the level parsers. Plain C: no raylib, so the tests can run them.
// Everything here is bounded: lines are capped, numbers are range-checked, and nothing
// loops on a value that came from the program's output.
#include "events.h"
#include <stdio.h>
#include <string.h>

// ---------------------------------------------------------------- lines

void LinesSplit(Lines *l, const char *text)
{
    l->count = 0;
    NormalizeOutput(text ? text : "", l->text, sizeof(l->text));
    if (l->text[0] == '\0') return;
    int i = 0;
    while (l->count < PARSE_LINES) {
        int s = i;
        while (l->text[i] && l->text[i] != '\n') i++;
        l->start[l->count] = s;
        l->len[l->count] = i - s;
        l->count++;
        if (l->text[i] == '\0') break;
        i++;
    }
}

const char *LineAt(const Lines *l, int i, int *len)
{
    if (i < 0 || i >= l->count) { if (len) *len = 0; return NULL; }
    if (len) *len = l->len[i];
    return l->text + l->start[i];
}

bool LineIs(const char *s, int len, const char *text)
{
    if (!s) return false;
    int n = (int)strlen(text);
    return n == len && memcmp(s, text, (size_t)n) == 0;
}

static bool SameLine(const Lines *a, int i, const Lines *b, int j)
{
    int la, lb;
    const char *sa = LineAt(a, i, &la), *sb = LineAt(b, j, &lb);
    return sa && sb && la == lb && memcmp(sa, sb, (size_t)la) == 0;
}

// Copies a line for showing on screen: control characters become '?', long lines get "..."
static void CleanCopy(char *dst, int cap, const char *s, int len)
{
    int n = 0;
    for (int i = 0; s && i < len && n < cap - 1; i++) {
        unsigned char c = (unsigned char)s[i];
        dst[n++] = (c < 32 || c >= 127) ? '?' : (char)c;
    }
    if (s && len > cap - 1 && cap > 4) { dst[cap - 4] = '.'; dst[cap - 3] = '.'; dst[cap - 2] = '.'; n = cap - 1; }
    dst[n] = '\0';
}

// ---------------------------------------------------------------- reading values

static bool IsSpace(char c) { return c == ' ' || c == '\t'; }

// One whole number at s[*i]; at most 9 digits so it can never overflow
static bool ReadInt(const char *s, int len, int *i, int *value)
{
    int k = *i;
    bool neg = false;
    if (k < len && (s[k] == '-' || s[k] == '+')) { neg = (s[k] == '-'); k++; }
    int digits = 0;
    long v = 0;
    while (k < len && s[k] >= '0' && s[k] <= '9') {
        if (digits < 9) v = v * 10 + (s[k] - '0');
        digits++;
        k++;
    }
    if (digits == 0 || digits > 9) return false;
    *value = (int)(neg ? -v : v);
    *i = k;
    return true;
}

bool ScanIntList(const char *s, int len, int *out, int max, int *count)
{
    *count = 0;
    if (!s) return false;
    int i = 0;
    while (true) {
        while (i < len && IsSpace(s[i])) i++;
        if (i >= len) return true;
        int v;
        if (!ReadInt(s, len, &i, &v)) return false;
        if (i < len && !IsSpace(s[i])) return false;      // "12a" or "3,4"
        if (*count < max) out[*count] = v;
        (*count)++;
    }
}

bool ScanAfter(const char *s, int len, const char *prefix, int *value)
{
    if (!s) return false;
    int n = (int)strlen(prefix);
    if (len < n || memcmp(s, prefix, (size_t)n) != 0) return false;
    int i = n;
    while (i < len && IsSpace(s[i])) i++;
    if (!ReadInt(s, len, &i, value)) return false;
    return i == len;
}

int InputInts(const char *input, int *out, int max)
{
    int n = 0;
    if (!input) return 0;
    int len = (int)strlen(input), i = 0;
    while (n < max) {
        while (i < len && (IsSpace(input[i]) || input[i] == '\n' || input[i] == '\r')) i++;
        int v;
        if (i >= len || !ReadInt(input, len, &i, &v)) break;
        out[n++] = v;
    }
    return n;
}

// ---------------------------------------------------------------- building the event list

void ParseBegin(Parse *p, const LevelRun *run, EventList *ev)
{
    // (the events themselves are cleared one by one as they are added)
    ev->count = 0;
    ev->overflow = false;
    ev->fail = FAIL_NONE;
    ev->expected[0] = ev->actual[0] = ev->note[0] = '\0';
    ev->failLine = -1;
    ev->status = run->status;
    ev->preview = (run->output == NULL);
    ev->result = EV_SPAWN;
    p->run = run;
    p->ev = ev;
    p->next = 0;
    p->shown = 0;
    p->stopped = false;
    LinesSplit(&p->exp, run->expected);
    LinesSplit(&p->act, run->output);
    ev->printedLines = p->act.count;
}

bool ParsePreview(const Parse *p) { return p->ev->preview; }

static GameEvent *Add(Parse *p, EventType type, int code, int line)
{
    EventList *ev = p->ev;
    if (ev->count >= EV_MAX - 1) {          // the last slot is kept for the ending
        ev->overflow = true;
        if (type == EV_SPAWN || type == EV_PRINT || type == EV_ACT) return NULL;
        if (ev->count >= EV_MAX) return NULL;
    }
    GameEvent *e = &ev->ev[ev->count++];
    memset(e, 0, sizeof(*e));
    e->type = type;
    e->code = code;
    e->line = line;
    return e;
}

void EvText(GameEvent *e, const char *text)
{
    if (e) CleanCopy(e->text, EV_TEXT, text, text ? (int)strlen(text) : 0);
}

GameEvent *EvSpawn(Parse *p, int code, int a, int b, double value)
{
    GameEvent *e = Add(p, EV_SPAWN, code, -1);
    if (e) { e->a = a; e->b = b; e->value = value; }
    return e;
}

GameEvent *EvAct(Parse *p, int code, int a, int b, double value)
{
    if (p->stopped) return NULL;
    GameEvent *e = Add(p, EV_ACT, code, p->shown - 1);
    if (e) { e->a = a; e->b = b; e->value = value; }
    return e;
}

void ParseShow(Parse *p)
{
    if (p->stopped || p->next >= p->act.count || p->shown > p->next) return;
    Add(p, EV_PRINT, 0, p->next);
    p->shown = p->next + 1;
}

bool ParseTake(Parse *p, int e)
{
    if (p->stopped || !SameLine(&p->exp, e, &p->act, p->next)) return false;
    ParseShow(p);
    p->next++;
    return true;
}

const char *ParsePeek(const Parse *p, int *len)
{
    return LineAt(&p->act, p->next, len);
}

static FailKind StoppedKind(RunStatus s)
{
    return (s == RUN_TIMEOUT || s == RUN_TOO_MUCH_OUTPUT) ? FAIL_TIMEOUT : FAIL_CRASH;
}

FailKind ParseStatusKind(RunStatus status) { return StoppedKind(status); }

GameEvent *ParseFail(Parse *p, FailKind kind, int code, int e, int a, int b, double value, const char *note)
{
    if (p->stopped) return NULL;
    EventList *ev = p->ev;
    bool more = p->next < p->act.count;
    if (kind == FAIL_MISSING && !more && p->run->status != RUN_OK) kind = StoppedKind(p->run->status);
    if (more) ParseShow(p);

    int el, al;
    const char *es = LineAt(&p->exp, e, &el);
    const char *as = LineAt(&p->act, p->next, &al);
    CleanCopy(ev->expected, FAIL_TEXT, es, es ? el : 0);
    CleanCopy(ev->actual, FAIL_TEXT, as, as ? al : 0);
    CleanCopy(ev->note, FAIL_TEXT, note, note ? (int)strlen(note) : 0);
    ev->failLine = more ? p->next : -1;
    ev->fail = kind;

    EventType type = (kind == FAIL_CRASH || kind == FAIL_TIMEOUT) ? EV_CRASH : EV_FAIL;
    ev->result = type;
    GameEvent *f = Add(p, type, code, more ? p->next : -1);
    if (f) { f->a = a; f->b = b; f->value = value; EvText(f, note); }
    p->stopped = true;
    return f;
}

GameEvent *ParseStop(Parse *p, FailKind kind, int code, const char *note)
{
    if (p->stopped) return NULL;
    EventList *ev = p->ev;
    ev->expected[0] = ev->actual[0] = '\0';
    CleanCopy(ev->note, FAIL_TEXT, note, note ? (int)strlen(note) : 0);
    ev->failLine = -1;
    ev->fail = kind;
    EventType type = (kind == FAIL_CRASH || kind == FAIL_TIMEOUT) ? EV_CRASH : EV_FAIL;
    ev->result = type;
    GameEvent *f = Add(p, type, code, -1);
    if (f) EvText(f, note);
    p->stopped = true;
    return f;
}

int TraceEnding(const char *trace, char *reason, int cap)
{
    const char *t = trace ? trace : "", *last = NULL;
    for (const char *q = t; (q = strstr(q, "END ")) != NULL; q++)
        if (q == t || q[-1] == '\n') last = q;
    if (reason && cap > 0) reason[0] = '\0';
    if (!last) return -1;
    if (strncmp(last, "END WIN", 7) == 0) return 1;
    if (strncmp(last, "END FAIL ", 9) != 0) return -1;
    int n = 0;
    for (const char *c = last + 9; reason && *c && *c != '\n' && *c != '\r' && n < cap - 1; c++) reason[n++] = *c;
    if (reason && cap > 0) reason[n] = '\0';
    return 0;
}

void ParseGameEnd(Parse *p, int homeCode, int sayCode, int gameCode, int lineCode, int extraCode)
{
    const LevelRun *run = p->run;
    char reason[FAIL_TEXT];
    int ending = TraceEnding(run->trace, reason, sizeof(reason));
    if (ending == 0) {
        ParseStop(p, FAIL_GAME, gameCode, reason[0] ? reason : "The game says the run failed.");
        return;
    }
    if (ending < 0) {
        if (run->status != RUN_OK) ParseStop(p, StoppedKind(run->status), 0, NULL);
        else ParseStop(p, FAIL_GAME, gameCode, "The game never got to the end - did the program stop partway?");
        return;
    }
    EvAct(p, homeCode, 0, 0, 0);
    if (!run->expected) {                       // any output will do: show what was printed
        for (int i = 0; i < p->act.count; i++) {
            int len;
            const char *s = LineAt(&p->act, i, &len);
            ParseShow(p);
            p->next++;
            GameEvent *e = EvAct(p, sayCode, i, 1, 0);
            if (e) CleanCopy(e->text, EV_TEXT, s, len);
        }
    } else {
        for (int e = 0; e < p->exp.count && !p->stopped; e++) {
            int el, al;
            const char *E = LineAt(&p->exp, e, &el);
            const char *A = ParsePeek(p, &al);
            if (ParseTake(p, e)) {
                GameEvent *g = EvAct(p, sayCode, e, 0, 0);
                if (g) CleanCopy(g->text, EV_TEXT, E, el);
                continue;
            }
            ParseFail(p, A ? FAIL_WRONG : FAIL_MISSING, lineCode, e, e, 0, 0, NULL);
        }
    }
    ParseEnd(p, extraCode);
}

void ParseEnd(Parse *p, int extraCode)
{
    if (p->stopped) return;
    EventList *ev = p->ev;
    const LevelRun *run = p->run;
    if (p->next < p->act.count) {
        ParseFail(p, FAIL_EXTRA, extraCode, -1, 0, 0, 0, "Your program printed more than it should");
        return;
    }
    if (run->status != RUN_OK) {
        FailKind kind = StoppedKind(run->status);
        ev->fail = kind;
        ev->result = EV_CRASH;
        Add(p, EV_CRASH, 0, -1);
        p->stopped = true;
        return;
    }
    if (!run->passed) {      // can't happen while the runner and the parsers agree; the tests check it
        ParseFail(p, FAIL_WRONG, extraCode, -1, 0, 0, 0, "Your output doesn't match");
        return;
    }
    ev->result = EV_WIN;
    Add(p, EV_WIN, 0, -1);
}
