#include "highlight.h"
#include "ui.h"
#include <ctype.h>
#include <string.h>

#define COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))

static const char *KEYWORDS[] = {
    "if", "else", "for", "while", "do", "switch", "case", "default", "break", "continue",
    "return", "goto", "sizeof", "typedef", "struct", "union", "enum", "class", "public",
    "private", "protected", "namespace", "using", "new", "delete", "template", "typename",
    "this", "virtual", "override", "const", "static", "extern", "inline", "true", "false",
    "NULL", "nullptr", "operator", "friend",
};

static const char *TYPES[] = {
    "int", "char", "float", "double", "void", "bool", "long", "short", "unsigned", "signed",
    "auto", "size_t", "FILE", "string", "vector", "std",
};

static bool IsWordChar(char c) { return isalnum((unsigned char)c) || c == '_'; }

static bool InList(const char *s, int n, const char *const *list, int count)
{
    for (int i = 0; i < count; i++)
        if ((int)strlen(list[i]) == n && strncmp(list[i], s, n) == 0) return true;
    return false;
}

static const char *FindEnd(const char *s, int from, int len)   // finds "*/"
{
    for (int i = from; i + 1 < len; i++)
        if (s[i] == '*' && s[i + 1] == '/') return s + i;
    return NULL;
}

// Draws line[start, end) at the right x position for its place in the line.
static void Piece(Font font, const char *line, int start, int end, float x, float y,
                  float size, float sp, Color c)
{
    char buf[512];
    float px = x;
    if (start > 0) {
        int n = start < 511 ? start : 511;
        memcpy(buf, line, n);
        buf[n] = '\0';
        px += MeasureTextEx(font, buf, size, sp).x + sp;
    }
    int n = end - start;
    if (n > 511) n = 511;
    if (n <= 0) return;
    memcpy(buf, line + start, n);
    buf[n] = '\0';
    DrawTextEx(font, buf, (Vector2){ px, y }, size, sp, c);
}

void HlLine(Font font, const char *s, int len, float x, float y, float size, float sp,
            int *state, bool draw)
{
    int first = 0;
    while (first < len && s[first] == ' ') first++;

    // Preprocessor lines: #include, #define ...
    if (*state == 0 && first < len && s[first] == '#') {
        if (draw) Piece(font, s, 0, len, x, y, size, sp, HL_PREPROC);
        return;
    }

    int i = 0;
    while (i < len) {
        int start = i;
        Color c = COL_CODE_TEXT;

        if (*state == 1) {                                     // inside /* ... */
            const char *e = FindEnd(s, i, len);
            if (e) { i = (int)(e - s) + 2; *state = 0; } else i = len;
            c = HL_COMMENT;
        } else if (s[i] == '/' && i + 1 < len && s[i + 1] == '/') {   // line comment
            i = len;
            c = HL_COMMENT;
        } else if (s[i] == '/' && i + 1 < len && s[i + 1] == '*') {   // block comment starts
            const char *e = FindEnd(s, i + 2, len);
            if (e) i = (int)(e - s) + 2; else { i = len; *state = 1; }
            c = HL_COMMENT;
        } else if (s[i] == '"' || s[i] == '\'') {              // string or char
            char q = s[i++];
            while (i < len && s[i] != q) {
                if (s[i] == '\\' && i + 1 < len) i++;
                i++;
            }
            if (i < len) i++;
            c = HL_STRING;
        } else if (isdigit((unsigned char)s[i])) {             // number
            while (i < len && (isalnum((unsigned char)s[i]) || s[i] == '.')) i++;
            c = HL_NUMBER;
        } else if (IsWordChar(s[i])) {                         // word
            while (i < len && IsWordChar(s[i])) i++;
            if (InList(s + start, i - start, KEYWORDS, COUNT_OF(KEYWORDS))) c = HL_KEYWORD;
            else if (InList(s + start, i - start, TYPES, COUNT_OF(TYPES))) c = HL_TYPE;
        } else {                                               // spaces and punctuation
            i++;
            while (i < len && !IsWordChar(s[i]) && s[i] != '"' && s[i] != '\'' &&
                   !(s[i] == '/' && i + 1 < len && (s[i + 1] == '/' || s[i + 1] == '*')))
                i++;
        }

        if (draw) Piece(font, s, start, i, x, y, size, sp, c);
    }
}

float HlCode(Font font, const char *code, float x, float y, float size, float sp, float lineH)
{
    int state = 0;
    float cy = y;
    const char *p = code;
    for (;;) {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        HlLine(font, p, len, x, cy, size, sp, &state, true);
        cy += lineH;
        if (!nl) break;
        p = nl + 1;
    }
    return cy - y;
}
