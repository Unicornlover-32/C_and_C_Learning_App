#include "ui.h"
#include "highlight.h"
#include "rlgl.h"
#include <math.h>
#include <string.h>

Font gUiFont;
Font gCodeFont;

static bool uiCustom = false;
static bool codeCustom = false;
static bool clickConsumed = false;
static bool blocked = false;
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

// ---------------------------------------------------------------- zoom

static float uiScale = 1.0f;     // layout units -> real pixels
static float userZoom = 1.0f;    // Ctrl +/- on top of the automatic fit
static float fontScale = 0.0f;   // the zoom the fonts were last loaded for

float UiScale(void) { return uiScale; }

static float FitScale(void)
{
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    // fit the 1280 x 720 design to the screen, then apply the user's zoom
    float s = fminf(w / BASE_W, h / BASE_H) * userZoom;
    // never zoom so far that the layouts get less room than they need
    float most = fminf(w / 1100.0f, h / 640.0f);
    if (s > most) s = most;
    if (s < 0.75f) s = 0.75f;
    return s;
}

// Fonts are rasterised at the zoomed size so big text stays sharp.
static void LoadFonts(float scale)
{
    if (uiCustom) UnloadFont(gUiFont);
    if (codeCustom) UnloadFont(gCodeFont);
    uiCustom   = TryLoadFont(&gUiFont, UI_FONT_PATHS, COUNT_OF(UI_FONT_PATHS), (int)(48 * scale + 0.5f));
    codeCustom = TryLoadFont(&gCodeFont, CODE_FONT_PATHS, COUNT_OF(CODE_FONT_PATHS), (int)(32 * scale + 0.5f));
    fontScale = scale;
}

void UiInit(void)
{
    uiScale = FitScale();
    LoadFonts(uiScale);
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

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (ctrl && (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)))      userZoom = fminf(userZoom + 0.1f, 1.6f);
    if (ctrl && (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT))) userZoom = fmaxf(userZoom - 0.1f, 0.6f);
    if (ctrl && (IsKeyPressed(KEY_ZERO)  || IsKeyPressed(KEY_KP_0)))        userZoom = 1.0f;

    uiScale = FitScale();
    if (fabsf(uiScale - fontScale) > 0.01f) LoadFonts(uiScale);
    SetMouseScale(1.0f / uiScale, 1.0f / uiScale);   // the mouse reports layout coordinates
}

void UiApplyScale(void)
{
    rlDrawRenderBatchActive();
    rlLoadIdentity();
    rlScalef(uiScale, uiScale, 1.0f);
}

void UiBeginScissor(float x, float y, float width, float height)
{
    float s = uiScale;
    int x0 = (int)floorf(x * s), y0 = (int)floorf(y * s);
    int x1 = (int)ceilf((x + width) * s), y1 = (int)ceilf((y + height) * s);
    BeginScissorMode(x0, y0, x1 - x0, y1 - y0);
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
    float scaled = size * UI_TEXT_SCALE;
    DrawTextEx(gUiFont, text, (Vector2){ x, y }, scaled, UiSpacing(scaled), color);
}

float UiTextWidth(const char *text, float size)
{
    float scaled = size * UI_TEXT_SCALE;
    return MeasureTextEx(gUiFont, text, scaled, UiSpacing(scaled)).x;
}

void UiTextCentered(const char *text, float centerX, float y, float size, Color color)
{
    UiText(text, centerX - UiTextWidth(text, size) / 2.0f, y, size, color);
}

static float Wrap(const char *text, float x, float y, float maxWidth, float size, Color color, bool draw);

float UiWrapped(const char *text, float x, float y, float maxWidth, float size, Color color)
{
    return Wrap(text, x, y, maxWidth, size, color, true);
}

float UiWrappedHeight(const char *text, float maxWidth, float size)
{
    return Wrap(text, 0, 0, maxWidth, size, BLANK, false);
}

static float Wrap(const char *text, float x, float y, float maxWidth, float size, Color color, bool draw)
{
    float lineH = size * UI_TEXT_SCALE * 1.35f;
    float cy = y;
    char line[1024];
    char cand[1024];
    int lineLen = 0;
    const char *p = text;

    for (;;) {
        if (*p == '\0' || *p == '\n') {
            line[lineLen] = '\0';
            if (draw) UiText(line, x, cy, size, color);
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
            if (draw) UiText(line, x, cy, size, color);
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
    float scaled = size * UI_TEXT_SCALE;
    float lineH = scaled * 1.3f;
    float sp = UiCodeSpacing(scaled);
    float charW = MeasureTextEx(gCodeFont, "M", scaled, sp).x + sp;
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
            DrawTextEx(gCodeFont, line, (Vector2){ x, cy }, scaled, sp, color);
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
    float scaled = size * UI_TEXT_SCALE;
    float lineH = scaled * 1.3f;
    int lines = 1;
    for (const char *p = code; *p; p++) if (*p == '\n') lines++;
    float h = lines * lineH + pad * 2;

    DrawRectangleRounded((Rectangle){ x, y, width, h }, 0.04f, 8, COL_CODE_BG);
    DrawRectangle((int)x, (int)(y + 8), 4, (int)(h - 16), COL_ACCENT);
    UiBeginScissor(x, y, width, h);
    HlCode(gCodeFont, code, x + pad + 6, y + pad, scaled, UiCodeSpacing(scaled), lineH);
    EndScissorMode();
    return h;
}

void UiSetBlocked(bool b) { blocked = b; }

bool UiClickArea(Rectangle r, bool *hover)
{
    bool h = !blocked && CheckCollisionPointRec(GetMousePosition(), r);
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

    // shrink the label if it would run past the button's edges
    float size = 21.0f;
    float tw = UiTextWidth(label, size);
    float room = r.width - 16.0f;
    if (tw > room && room > 0) {
        size = fmaxf(13.0f, size * room / tw);
        tw = UiTextWidth(label, size);
    }
    float scaled = size * UI_TEXT_SCALE;
    UiText(label, r.x + (r.width - tw) / 2.0f, r.y + (r.height - scaled) / 2.0f, size, WHITE);
    return clicked;
}
