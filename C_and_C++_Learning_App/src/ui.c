#include "ui.h"
#include <string.h>

Font gUiFont;
Font gCodeFont;

static bool uiCustom = false;
static bool codeCustom = false;
static bool clickConsumed = false;
static int wantedCursor = MOUSE_CURSOR_DEFAULT;
static int currentCursor = MOUSE_CURSOR_DEFAULT;

#define COUNT_OF(a) ((int)(sizeof(a) / sizeof((a)[0])))

// Put your own fonts in assets/ to override the system ones.
static const char *UI_FONT_PATHS[] = {
    "assets/ui.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
};

static const char *CODE_FONT_PATHS[] = {
    "assets/code.ttf",
    "C:/Windows/Fonts/consola.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
    "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",
    "/System/Library/Fonts/Supplemental/Courier New.ttf",
};

static bool TryLoadFont(Font *out, const char *const *paths, int count, int baseSize)
{
    for (int i = 0; i < count; i++) {
        if (!FileExists(paths[i])) continue;
        Font f = LoadFontEx(paths[i], baseSize, NULL, 0);
        if (f.texture.id != 0 && f.texture.id != GetFontDefault().texture.id) {
            SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);
            *out = f;
            return true;
        }
    }
    *out = GetFontDefault();
    return false;
}

void UiInit(void)
{
    uiCustom   = TryLoadFont(&gUiFont, UI_FONT_PATHS, COUNT_OF(UI_FONT_PATHS), 48);
    codeCustom = TryLoadFont(&gCodeFont, CODE_FONT_PATHS, COUNT_OF(CODE_FONT_PATHS), 20);
}

void UiShutdown(void)
{
    if (uiCustom) UnloadFont(gUiFont);
    if (codeCustom) UnloadFont(gCodeFont);
}

void UiBeginFrame(void)
{
    clickConsumed = false;
    wantedCursor = MOUSE_CURSOR_DEFAULT;
}

void UiEndFrame(void)
{
    if (wantedCursor != currentCursor) {
        SetMouseCursor(wantedCursor);
        currentCursor = wantedCursor;
    }
}

void UiSetCursor(int cursor) { wantedCursor = cursor; }

float UiSpacing(float size)     { return uiCustom ? 0.5f : size / 10.0f; }
float UiCodeSpacing(float size) { return codeCustom ? 0.0f : size / 10.0f; }

void UiText(const char *text, float x, float y, float size, Color color)
{
    DrawTextEx(gUiFont, text, (Vector2){ x, y }, size, UiSpacing(size), color);
}

float UiTextWidth(const char *text, float size)
{
    return MeasureTextEx(gUiFont, text, size, UiSpacing(size)).x;
}

void UiTextCentered(const char *text, float centerX, float y, float size, Color color)
{
    UiText(text, centerX - UiTextWidth(text, size) / 2.0f, y, size, color);
}

float UiWrapped(const char *text, float x, float y, float maxWidth, float size, Color color)
{
    float lineH = size * 1.35f;
    float cy = y;
    char line[1024];
    char cand[1024];
    int lineLen = 0;
    const char *p = text;

    for (;;) {
        if (*p == '\0' || *p == '\n') {
            line[lineLen] = '\0';
            UiText(line, x, cy, size, color);
            cy += lineH;
            lineLen = 0;
            if (*p == '\0') break;
            p++;
            continue;
        }

        // Grab the next word
        int wl = 0;
        while (p[wl] && p[wl] != ' ' && p[wl] != '\n') wl++;

        int n = 0;
        if (lineLen > 0) {
            memcpy(cand, line, lineLen);
            n = lineLen;
            cand[n++] = ' ';
        }
        if (n + wl >= (int)sizeof(cand)) wl = (int)sizeof(cand) - 1 - n;
        memcpy(cand + n, p, wl);
        n += wl;
        cand[n] = '\0';

        if (lineLen > 0 && UiTextWidth(cand, size) > maxWidth) {
            line[lineLen] = '\0';
            UiText(line, x, cy, size, color);
            cy += lineH;
            memcpy(line, p, wl);
            lineLen = wl;
        } else {
            memcpy(line, cand, n);
            lineLen = n;
        }

        p += wl;
        while (*p == ' ') p++;
    }
    return cy - y;
}

float UiCode(const char *text, float x, float y, float size, Color color, float maxWidth)
{
    float lineH = size * 1.3f;
    float sp = UiCodeSpacing(size);
    float charW = MeasureTextEx(gCodeFont, "M", size, sp).x + sp;
    int maxChars = (maxWidth > 0 && charW > 0) ? (int)(maxWidth / charW) : 100000;
    if (maxChars < 1) maxChars = 1;

    char line[512];
    int n = 0;
    float cy = y;

    for (const char *p = text;; p++) {
        bool endOfText = (*p == '\0');
        bool flush = endOfText || *p == '\n' || n >= maxChars || n >= (int)sizeof(line) - 1;
        if (flush) {
            line[n] = '\0';
            DrawTextEx(gCodeFont, line, (Vector2){ x, cy }, size, sp, color);
            cy += lineH;
            n = 0;
            if (endOfText) break;
            if (*p != '\n') line[n++] = *p;   // wrapped: keep this character on the next line
            continue;
        }
        line[n++] = *p;
    }
    return cy - y;
}

float UiCodeBlock(const char *code, float x, float y, float width, float size)
{
    float pad = 16.0f;
    float lineH = size * 1.3f;
    int lines = 1;
    for (const char *p = code; *p; p++) if (*p == '\n') lines++;
    float h = lines * lineH + pad * 2;

    DrawRectangleRounded((Rectangle){ x, y, width, h }, 0.04f, 8, COL_CODE_BG);
    DrawRectangle((int)x, (int)(y + 8), 4, (int)(h - 16), COL_ACCENT);
    UiCode(code, x + pad + 6, y + pad, size, COL_CODE_TEXT, width - pad * 2 - 6);
    return h;
}

bool UiClickArea(Rectangle r, bool *hover)
{
    bool h = CheckCollisionPointRec(GetMousePosition(), r);
    if (hover) *hover = h;
    if (h) UiSetCursor(MOUSE_CURSOR_POINTING_HAND);
    if (h && !clickConsumed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clickConsumed = true;
        return true;
    }
    return false;
}

bool UiButton(Rectangle r, const char *label, Color base)
{
    bool hover;
    bool clicked = UiClickArea(r, &hover);

    Color fill = base;
    if (hover) fill = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? ColorBrightness(base, -0.12f)
                                                          : ColorBrightness(base, 0.15f);
    DrawRectangleRounded(r, 0.3f, 8, fill);

    float size = 21.0f;
    float tw = UiTextWidth(label, size);
    UiText(label, r.x + (r.width - tw) / 2.0f, r.y + (r.height - size) / 2.0f, size, WHITE);
    return clicked;
}
