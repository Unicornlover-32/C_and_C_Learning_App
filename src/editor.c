#include "editor.h"
#include "highlight.h"
#include "ui.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define KEY_HIT(k) (IsKeyPressed(k) || IsKeyPressedRepeat(k))
#define PAD     10.0f
#define GUTTER  50.0f
#define TAB_W   4

enum { EDIT_NONE, EDIT_TYPE, EDIT_DELETE, EDIT_OTHER };

static char clipBuf[EDITOR_CAP];

static float LineH(const Editor *e) { return e->fontSize * 1.3f; }
static int   MinI(int a, int b)     { return a < b ? a : b; }
static int   MaxI(int a, int b)     { return a > b ? a : b; }

// ---------------------------------------------------------------- text helpers

static int LineStart(const Editor *e, int i) { while (i > 0 && e->text[i - 1] != '\n') i--; return i; }
static int LineEnd(const Editor *e, int i)   { while (i < e->len && e->text[i] != '\n') i++; return i; }

static int LineOf(const Editor *e, int index)
{
    int line = 0;
    for (int i = 0; i < index; i++) if (e->text[i] == '\n') line++;
    return line;
}

static int LineCount(const Editor *e) { return LineOf(e, e->len) + 1; }

static int IndexOfLine(const Editor *e, int line)
{
    int i = 0, l = 0;
    while (l < line && i < e->len) {
        if (e->text[i] == '\n') l++;
        i++;
    }
    return i;
}

static bool HasSel(const Editor *e) { return e->anchor != e->cursor; }
static int  SelMin(const Editor *e) { return MinI(e->anchor, e->cursor); }
static int  SelMax(const Editor *e) { return MaxI(e->anchor, e->cursor); }

static void InsertAt(Editor *e, int pos, const char *s, int n)
{
    if (e->len + n >= EDITOR_CAP) n = EDITOR_CAP - 1 - e->len;
    if (n <= 0) return;
    memmove(e->text + pos + n, e->text + pos, e->len - pos + 1);   // +1 keeps the '\0'
    memcpy(e->text + pos, s, n);
    e->len += n;
    if (e->cursor >= pos) e->cursor += n;
    if (e->anchor > pos) e->anchor += n;
}

static void Insert(Editor *e, const char *s, int n)
{
    InsertAt(e, e->cursor, s, n);
    e->anchor = e->cursor;
}

static void Erase(Editor *e, int from, int to)   // removes [from, to)
{
    if (from < 0) from = 0;
    if (to > e->len) to = e->len;
    if (from >= to) return;
    memmove(e->text + from, e->text + to, e->len - to + 1);
    e->len -= (to - from);
    if (e->cursor >= to) e->cursor -= (to - from); else if (e->cursor > from) e->cursor = from;
    if (e->anchor >= to) e->anchor -= (to - from); else if (e->anchor > from) e->anchor = from;
}

static void DeleteSelection(Editor *e)
{
    if (!HasSel(e)) return;
    int a = SelMin(e), b = SelMax(e);
    Erase(e, a, b);
    e->cursor = e->anchor = a;
}

static bool OnlySpacesBefore(const Editor *e, int ls, int cursor)
{
    for (int i = ls; i < cursor; i++) if (e->text[i] != ' ') return false;
    return true;
}

// ---------------------------------------------------------------- undo / redo

static void FreeSnap(EditSnapshot *s) { free(s->text); s->text = NULL; }

static void PushSnap(EditSnapshot *stack, int *count, const Editor *e)
{
    if (*count == UNDO_MAX) {
        FreeSnap(&stack[0]);
        memmove(stack, stack + 1, sizeof(EditSnapshot) * (UNDO_MAX - 1));
        (*count)--;
    }
    EditSnapshot *s = &stack[(*count)++];
    s->text = malloc((size_t)e->len + 1);
    if (s->text) memcpy(s->text, e->text, (size_t)e->len + 1);
    s->len = e->len;
    s->cursor = e->cursor;
}

static void Restore(Editor *e, EditSnapshot *s)
{
    if (s->text) {
        memcpy(e->text, s->text, (size_t)s->len + 1);
        e->len = s->len;
    }
    e->cursor = e->anchor = MinI(s->cursor, e->len);
    FreeSnap(s);
}

// Call before every change. Quick runs of typing (or deleting) share one undo step.
static void BeforeEdit(Editor *e, int kind)
{
    double now = GetTime();
    if (kind != EDIT_OTHER && kind == e->lastEditKind && now - e->lastEditTime < 1.0) {
        e->lastEditTime = now;
        return;
    }
    PushSnap(e->undo, &e->undoCount, e);
    for (int i = 0; i < e->redoCount; i++) FreeSnap(&e->redo[i]);
    e->redoCount = 0;
    e->lastEditKind = kind;
    e->lastEditTime = now;
}

void EditorUndo(Editor *e)
{
    if (e->undoCount == 0) return;
    PushSnap(e->redo, &e->redoCount, e);
    Restore(e, &e->undo[--e->undoCount]);
    e->lastEditKind = EDIT_NONE;
}

void EditorRedo(Editor *e)
{
    if (e->redoCount == 0) return;
    PushSnap(e->undo, &e->undoCount, e);
    Restore(e, &e->redo[--e->redoCount]);
    e->lastEditKind = EDIT_NONE;
}

// ---------------------------------------------------------------- public setup

void EditorInit(Editor *e, int fontSize)
{
    memset(e, 0, sizeof(*e));
    e->fontSize = fontSize;
}

void EditorFree(Editor *e)
{
    for (int i = 0; i < e->undoCount; i++) FreeSnap(&e->undo[i]);
    for (int i = 0; i < e->redoCount; i++) FreeSnap(&e->redo[i]);
    e->undoCount = e->redoCount = 0;
}

static void CopyIn(Editor *e, const char *text)
{
    size_t n = strlen(text);
    if (n >= EDITOR_CAP) n = EDITOR_CAP - 1;
    memcpy(e->text, text, n);
    e->text[n] = '\0';
    e->len = (int)n;
    e->cursor = e->anchor = 0;
    e->scrollY = 0;
    e->blink = 0;
}

void EditorSetText(Editor *e, const char *text)
{
    EditorFree(e);
    CopyIn(e, text);
    e->lastEditKind = EDIT_NONE;
}

void EditorReplaceAll(Editor *e, const char *text)
{
    BeforeEdit(e, EDIT_OTHER);
    CopyIn(e, text);
}

// ---------------------------------------------------------------- geometry

static float PrefixWidth(const Editor *e, int lineStart, int count)
{
    if (count <= 0) return 0;
    char buf[512];
    if (count > 511) count = 511;
    memcpy(buf, e->text + lineStart, count);
    buf[count] = '\0';
    float sp = UiCodeSpacing((float)e->fontSize);
    return MeasureTextEx(gCodeFont, buf, (float)e->fontSize, sp).x + sp;
}

static int ColumnFromX(const Editor *e, int ls, int le, float x)
{
    float prev = 0;
    for (int c = 1; c <= le - ls; c++) {
        float w = PrefixWidth(e, ls, c);
        if (w > x) return (x - prev < w - x) ? c - 1 : c;
        prev = w;
    }
    return le - ls;
}

static int IndexFromPoint(const Editor *e, Rectangle b, Vector2 m)
{
    float lh = LineH(e);
    int line = (int)floorf((m.y - (b.y + PAD) + e->scrollY) / lh);
    int lines = LineCount(e);
    if (line < 0) return 0;
    if (line >= lines) return e->len;
    int ls = IndexOfLine(e, line);
    int le = LineEnd(e, ls);
    return ls + ColumnFromX(e, ls, le, m.x - (b.x + GUTTER + PAD));
}

// Moves the cursor. With shift held the selection grows, otherwise it is cleared.
static void MoveTo(Editor *e, int pos, bool shift)
{
    e->cursor = pos;
    if (!shift) e->anchor = pos;
    e->lastEditKind = EDIT_NONE;
}

// Indents (or un-indents) every line from line `first` to line `last`.
static void IndentLines(Editor *e, int first, int last, bool dedent)
{
    for (int l = first; l <= last; l++) {
        int ls = IndexOfLine(e, l);
        if (dedent) {
            int k = 0;
            while (k < TAB_W && ls + k < e->len && e->text[ls + k] == ' ') k++;
            Erase(e, ls, ls + k);
        } else {
            InsertAt(e, ls, "    ", TAB_W);
        }
    }
}

static void CopySelection(const Editor *e)
{
    int a = SelMin(e), b = SelMax(e);
    memcpy(clipBuf, e->text + a, b - a);
    clipBuf[b - a] = '\0';
    SetClipboardText(clipBuf);
}

// ---------------------------------------------------------------- update

void EditorUpdate(Editor *e, Rectangle b)
{
    Vector2 m = GetMousePosition();
    bool inside = CheckCollisionPointRec(m, b);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    float lh = LineH(e);
    bool moved = false;

    if (inside) UiSetCursor(MOUSE_CURSOR_IBEAM);

    // Mouse: click to place the cursor, drag to select, shift+click to extend
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        e->focused = inside;
        if (inside) {
            MoveTo(e, IndexFromPoint(e, b, m), shift);
            e->dragging = true;
            e->blink = 0;
        }
    }
    if (e->dragging) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (m.y < b.y) e->scrollY -= lh * 0.5f;              // drag past the edge to scroll
            if (m.y > b.y + b.height) e->scrollY += lh * 0.5f;
            e->cursor = IndexFromPoint(e, b, m);
            e->blink = 0;
        } else {
            e->dragging = false;
        }
    }

    if (inside) e->scrollY -= GetMouseWheelMove() * lh * 3;

    if (e->focused) {
        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                    IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);

        // Typed characters
        int ch;
        while ((ch = GetCharPressed()) != 0) {
            if (ch < 32 || ch > 126 || ctrl) continue;
            BeforeEdit(e, EDIT_TYPE);
            DeleteSelection(e);
            if (ch == '}') {
                // Auto-dedent a closing brace typed on an otherwise empty line
                int ls = LineStart(e, e->cursor);
                if (OnlySpacesBefore(e, ls, e->cursor) && e->cursor - ls >= TAB_W)
                    Erase(e, e->cursor - TAB_W, e->cursor);
            }
            char c = (char)ch;
            Insert(e, &c, 1);
            moved = true;
        }

        int ls  = LineStart(e, e->cursor);
        int col = e->cursor - ls;

        if (ctrl) {
            if (IsKeyPressed(KEY_Z) && shift) { EditorRedo(e); moved = true; }
            else if (KEY_HIT(KEY_Z))          { EditorUndo(e); moved = true; }
            else if (KEY_HIT(KEY_Y))          { EditorRedo(e); moved = true; }
            else if (IsKeyPressed(KEY_A))     { e->anchor = 0; e->cursor = e->len; }
            else if (IsKeyPressed(KEY_C) && HasSel(e)) CopySelection(e);
            else if (IsKeyPressed(KEY_X) && HasSel(e)) {
                CopySelection(e);
                BeforeEdit(e, EDIT_OTHER);
                DeleteSelection(e);
                moved = true;
            }
            else if (IsKeyPressed(KEY_V)) {
                const char *clip = GetClipboardText();
                if (clip && *clip) {
                    BeforeEdit(e, EDIT_OTHER);
                    DeleteSelection(e);
                    for (const char *p = clip; *p; p++) {
                        if (*p == '\r') continue;
                        if (*p == '\t') { Insert(e, "    ", TAB_W); continue; }
                        if (*p == '\n' || (*p >= 32 && *p < 127)) Insert(e, p, 1);
                    }
                }
                moved = true;
            }
        }
        else if (KEY_HIT(KEY_ENTER) || KEY_HIT(KEY_KP_ENTER)) {
            BeforeEdit(e, EDIT_OTHER);
            DeleteSelection(e);
            ls = LineStart(e, e->cursor);
            // Keep the indentation of the current line, add a level after '{'
            int indent = 0;
            while (ls + indent < e->cursor && e->text[ls + indent] == ' ') indent++;
            int k = e->cursor - 1;
            while (k >= ls && e->text[k] == ' ') k--;
            if (k >= ls && e->text[k] == '{') indent += TAB_W;

            char buf[128];
            int n = 0;
            buf[n++] = '\n';
            for (int i = 0; i < indent && n < (int)sizeof(buf); i++) buf[n++] = ' ';
            Insert(e, buf, n);
            moved = true;
        }
        else if (KEY_HIT(KEY_BACKSPACE)) {
            if (HasSel(e)) {
                BeforeEdit(e, EDIT_OTHER);
                DeleteSelection(e);
            } else if (e->cursor > 0) {
                BeforeEdit(e, EDIT_DELETE);
                int n = 1;
                if (col > 0 && OnlySpacesBefore(e, ls, e->cursor)) {
                    n = col % TAB_W;          // soft tab: back to the previous tab stop
                    if (n == 0) n = TAB_W;
                }
                Erase(e, e->cursor - n, e->cursor);
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_DELETE)) {
            if (HasSel(e)) {
                BeforeEdit(e, EDIT_OTHER);
                DeleteSelection(e);
            } else if (e->cursor < e->len) {
                BeforeEdit(e, EDIT_DELETE);
                Erase(e, e->cursor, e->cursor + 1);
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_TAB)) {
            int firstLine = LineOf(e, SelMin(e)), lastLine = LineOf(e, SelMax(e));
            if (shift || firstLine != lastLine) {
                // Indent / un-indent whole lines, keeping them selected
                BeforeEdit(e, EDIT_OTHER);
                IndentLines(e, firstLine, lastLine, shift);
                if (firstLine != lastLine) {
                    e->anchor = IndexOfLine(e, firstLine);
                    e->cursor = LineEnd(e, IndexOfLine(e, lastLine));
                }
            } else {
                BeforeEdit(e, EDIT_OTHER);
                DeleteSelection(e);
                col = e->cursor - LineStart(e, e->cursor);
                Insert(e, "        ", TAB_W - (col % TAB_W));
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_LEFT)) {
            if (HasSel(e) && !shift) MoveTo(e, SelMin(e), false);
            else if (e->cursor > 0) MoveTo(e, e->cursor - 1, shift);
            moved = true;
        }
        else if (KEY_HIT(KEY_RIGHT)) {
            if (HasSel(e) && !shift) MoveTo(e, SelMax(e), false);
            else if (e->cursor < e->len) MoveTo(e, e->cursor + 1, shift);
            moved = true;
        }
        else if (KEY_HIT(KEY_UP)) {
            if (ls > 0) {
                int pls = LineStart(e, ls - 1);
                MoveTo(e, pls + MinI(col, (ls - 1) - pls), shift);
            } else {
                MoveTo(e, 0, shift);
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_DOWN)) {
            int le = LineEnd(e, e->cursor);
            if (le < e->len) {
                int nls = le + 1;
                MoveTo(e, nls + MinI(col, LineEnd(e, nls) - nls), shift);
            } else {
                MoveTo(e, e->len, shift);
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_HOME)) {
            // First press goes to the first non-space character, second to column 0
            int first = ls;
            while (first < e->len && e->text[first] == ' ') first++;
            MoveTo(e, (e->cursor == first) ? ls : first, shift);
            moved = true;
        }
        else if (KEY_HIT(KEY_END)) {
            MoveTo(e, LineEnd(e, e->cursor), shift);
            moved = true;
        }
    }

    // Keep the cursor on screen and clamp scrolling
    float viewH = b.height - PAD * 2;
    if (moved) {
        float cy = LineOf(e, e->cursor) * lh;
        if (cy < e->scrollY) e->scrollY = cy;
        if (cy + lh > e->scrollY + viewH) e->scrollY = cy + lh - viewH;
        e->blink = 0;
    }
    float maxScroll = LineCount(e) * lh - viewH;
    if (maxScroll < 0) maxScroll = 0;
    if (e->scrollY > maxScroll) e->scrollY = maxScroll;
    if (e->scrollY < 0) e->scrollY = 0;

    e->blink += GetFrameTime();
}

// ---------------------------------------------------------------- draw

void EditorDraw(const Editor *e, Rectangle b)
{
    float fs = (float)e->fontSize;
    float lh = LineH(e);
    float sp = UiCodeSpacing(fs);
    float numSize = fs * 0.85f;
    float textX = b.x + GUTTER + PAD;
    float textY = b.y + PAD - e->scrollY;
    int cursorLine = LineOf(e, e->cursor);
    int selA = SelMin(e), selB = SelMax(e);
    float spaceW = MeasureTextEx(gCodeFont, " ", fs, sp).x;
    int hlState = 0;

    DrawRectangleRec(b, COL_CODE_BG);
    DrawRectangle((int)b.x, (int)b.y, (int)GUTTER, (int)b.height, (Color){ 24, 26, 33, 255 });

    BeginScissorMode((int)b.x, (int)b.y, (int)b.width, (int)b.height);

    int line = 0, ls = 0;
    for (;;) {
        int le = LineEnd(e, ls);
        float y = textY + line * lh;
        bool visible = (y + lh >= b.y && y <= b.y + b.height);

        if (visible) {
            if (line == cursorLine && e->focused && !HasSel(e))
                DrawRectangle((int)(b.x + GUTTER), (int)(y - 2), (int)(b.width - GUTTER), (int)lh,
                              (Color){ 255, 255, 255, 10 });

            // Selection on this line
            if (selA != selB && selA <= le && selB >= ls) {
                int a = MaxI(selA, ls), z = MinI(selB, le);
                float x1 = textX + PrefixWidth(e, ls, a - ls);
                float x2 = textX + PrefixWidth(e, ls, z - ls);
                if (selB > le) x2 += spaceW;               // selection continues past the line end
                DrawRectangle((int)x1, (int)(y - 2), (int)(x2 - x1), (int)lh, (Color){ 86, 156, 255, 70 });
            }

            const char *num = TextFormat("%d", line + 1);
            float nw = MeasureTextEx(gCodeFont, num, numSize, sp).x;
            DrawTextEx(gCodeFont, num, (Vector2){ b.x + GUTTER - 10 - nw, y + fs * 0.1f }, numSize, sp,
                       line == cursorLine ? COL_TEXT : COL_MUTED);
        }

        HlLine(gCodeFont, e->text + ls, le - ls, textX, y, fs, sp, &hlState, visible);

        if (visible && line == cursorLine && e->focused && fmodf(e->blink, 1.0f) < 0.6f) {
            float cx = textX + PrefixWidth(e, ls, e->cursor - ls);
            DrawRectangle((int)cx, (int)y, 2, (int)fs, COL_ACCENT);
        }

        if (le >= e->len) break;
        ls = le + 1;
        line++;
    }

    EndScissorMode();
    DrawRectangleLinesEx(b, 1, e->focused ? COL_ACCENT : COL_BORDER);
}
