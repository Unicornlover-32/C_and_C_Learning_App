#include "editor.h"
#include "ui.h"
#include <math.h>
#include <string.h>

#define KEY_HIT(k) (IsKeyPressed(k) || IsKeyPressedRepeat(k))
#define PAD     10.0f
#define GUTTER  50.0f
#define TAB_W   4

static float LineH(const Editor *e) { return e->fontSize * 1.3f; }
static int   MinI(int a, int b)     { return a < b ? a : b; }

void EditorInit(Editor *e, int fontSize)
{
    memset(e, 0, sizeof(*e));
    e->fontSize = fontSize;
}

void EditorSetText(Editor *e, const char *text)
{
    size_t n = strlen(text);
    if (n >= EDITOR_CAP) n = EDITOR_CAP - 1;
    memcpy(e->text, text, n);
    e->text[n] = '\0';
    e->len = (int)n;
    e->cursor = 0;
    e->scrollY = 0;
    e->blink = 0;
}

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

static void Insert(Editor *e, const char *s, int n)
{
    if (e->len + n >= EDITOR_CAP) n = EDITOR_CAP - 1 - e->len;
    if (n <= 0) return;
    memmove(e->text + e->cursor + n, e->text + e->cursor, e->len - e->cursor + 1); // +1 keeps the '\0'
    memcpy(e->text + e->cursor, s, n);
    e->len += n;
    e->cursor += n;
}

static void Erase(Editor *e, int from, int to)  // removes [from, to)
{
    if (from < 0) from = 0;
    if (to > e->len) to = e->len;
    if (from >= to) return;
    memmove(e->text + from, e->text + to, e->len - to + 1);
    e->len -= (to - from);
    if (e->cursor >= to) e->cursor -= (to - from);
    else if (e->cursor > from) e->cursor = from;
}

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

static bool OnlySpacesBefore(const Editor *e, int ls, int cursor)
{
    for (int i = ls; i < cursor; i++) if (e->text[i] != ' ') return false;
    return true;
}

void EditorUpdate(Editor *e, Rectangle b)
{
    Vector2 m = GetMousePosition();
    bool inside = CheckCollisionPointRec(m, b);
    float lh = LineH(e);
    float textX = b.x + GUTTER + PAD;
    float textY = b.y + PAD;
    bool moved = false;

    if (inside) UiSetCursor(MOUSE_CURSOR_IBEAM);

    // Click to focus and place the cursor
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        e->focused = inside;
        if (inside) {
            int line = (int)((m.y - textY + e->scrollY) / lh);
            int lines = LineCount(e);
            if (line < 0) line = 0;
            if (line >= lines) line = lines - 1;
            int ls = IndexOfLine(e, line);
            int le = LineEnd(e, ls);
            e->cursor = ls + ColumnFromX(e, ls, le, m.x - textX);
            e->blink = 0;
        }
    }

    if (inside) e->scrollY -= GetMouseWheelMove() * lh * 3;

    if (e->focused) {
        bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                    IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER);

        // Typed characters
        int ch;
        while ((ch = GetCharPressed()) != 0) {
            if (ch < 32 || ch > 126) continue;
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

        if ((KEY_HIT(KEY_ENTER) || KEY_HIT(KEY_KP_ENTER)) && !ctrl) {
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
        else if (KEY_HIT(KEY_BACKSPACE) && e->cursor > 0) {
            int n = 1;
            if (col > 0 && OnlySpacesBefore(e, ls, e->cursor)) {
                n = col % TAB_W;           // soft-tab: remove back to the previous tab stop
                if (n == 0) n = TAB_W;
            }
            Erase(e, e->cursor - n, e->cursor);
            moved = true;
        }
        else if (KEY_HIT(KEY_DELETE)) {
            Erase(e, e->cursor, e->cursor + 1);
            moved = true;
        }
        else if (KEY_HIT(KEY_TAB)) {
            const char *spaces = "        ";
            Insert(e, spaces, TAB_W - (col % TAB_W));
            moved = true;
        }
        else if (KEY_HIT(KEY_LEFT)) {
            if (e->cursor > 0) e->cursor--;
            moved = true;
        }
        else if (KEY_HIT(KEY_RIGHT)) {
            if (e->cursor < e->len) e->cursor++;
            moved = true;
        }
        else if (KEY_HIT(KEY_UP)) {
            if (ls > 0) {
                int pls = LineStart(e, ls - 1);
                e->cursor = pls + MinI(col, (ls - 1) - pls);
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_DOWN)) {
            int le = LineEnd(e, e->cursor);
            if (le < e->len) {
                int nls = le + 1;
                e->cursor = nls + MinI(col, LineEnd(e, nls) - nls);
            }
            moved = true;
        }
        else if (KEY_HIT(KEY_HOME)) {
            // First press goes to the first non-space character, second to column 0
            int first = ls;
            while (first < e->len && e->text[first] == ' ') first++;
            e->cursor = (e->cursor == first) ? ls : first;
            moved = true;
        }
        else if (KEY_HIT(KEY_END)) {
            e->cursor = LineEnd(e, e->cursor);
            moved = true;
        }
        else if (ctrl && IsKeyPressed(KEY_V)) {
            const char *clip = GetClipboardText();
            if (clip) {
                for (const char *p = clip; *p; p++) {
                    if (*p == '\r') continue;
                    if (*p == '\t') { Insert(e, "    ", TAB_W); continue; }
                    if (*p == '\n' || (*p >= 32 && *p < 127)) Insert(e, p, 1);
                }
            }
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

void EditorDraw(const Editor *e, Rectangle b)
{
    float fs = (float)e->fontSize;
    float lh = LineH(e);
    float sp = UiCodeSpacing(fs);
    float numSize = fs * 0.85f;
    float textX = b.x + GUTTER + PAD;
    float textY = b.y + PAD - e->scrollY;
    int cursorLine = LineOf(e, e->cursor);
    char buf[512];

    DrawRectangleRec(b, COL_CODE_BG);
    DrawRectangle((int)b.x, (int)b.y, (int)GUTTER, (int)b.height, (Color){ 24, 26, 33, 255 });

    BeginScissorMode((int)b.x, (int)b.y, (int)b.width, (int)b.height);

    int line = 0, ls = 0;
    for (;;) {
        int le = LineEnd(e, ls);
        float y = textY + line * lh;

        if (y + lh >= b.y && y <= b.y + b.height) {
            if (line == cursorLine && e->focused)
                DrawRectangle((int)(b.x + GUTTER), (int)(y - 2), (int)(b.width - GUTTER), (int)lh,
                              (Color){ 255, 255, 255, 10 });

            const char *num = TextFormat("%d", line + 1);
            float nw = MeasureTextEx(gCodeFont, num, numSize, sp).x;
            DrawTextEx(gCodeFont, num, (Vector2){ b.x + GUTTER - 10 - nw, y + fs * 0.1f }, numSize, sp,
                       line == cursorLine ? COL_TEXT : COL_MUTED);

            int n = MinI(le - ls, (int)sizeof(buf) - 1);
            memcpy(buf, e->text + ls, n);
            buf[n] = '\0';
            DrawTextEx(gCodeFont, buf, (Vector2){ textX, y }, fs, sp, COL_CODE_TEXT);

            if (line == cursorLine && e->focused && fmodf(e->blink, 1.0f) < 0.6f) {
                float cx = textX + PrefixWidth(e, ls, e->cursor - ls);
                DrawRectangle((int)cx, (int)y, 2, (int)fs, COL_ACCENT);
            }
        }

        if (le >= e->len) break;
        ls = le + 1;
        line++;
    }

    EndScissorMode();
    DrawRectangleLinesEx(b, 1, e->focused ? COL_ACCENT : COL_BORDER);
}
