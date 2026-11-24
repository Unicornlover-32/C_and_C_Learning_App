#ifndef UI_H
#define UI_H

#include "raylib.h"

// The app runs full screen (F11 toggles a window). The designs are laid out for
// 1280 x 720; on a bigger screen the whole UI (text, boxes, editor) is zoomed by
// UiScale() so it stays readable, and SCREEN_W / SCREEN_H are the zoomed-out size
// every layout works in. Ctrl + / Ctrl - / Ctrl 0 adjust the zoom.
float UiScale(void);
#define SCREEN_W ((int)(GetScreenWidth() / UiScale()))
#define SCREEN_H ((int)(GetScreenHeight() / UiScale()))
#define BASE_W   1280
#define BASE_H   720
#define UI_TEXT_SCALE 1.0f   // extra text-only scale on top of the whole-UI zoom (see UiScale)

// Colour palette
#define COL_BG        (Color){  22,  24,  31, 255 }
#define COL_PANEL     (Color){  32,  35,  45, 255 }
#define COL_PANEL_HI  (Color){  44,  48,  62, 255 }
#define COL_BORDER    (Color){  58,  63,  80, 255 }
#define COL_TEXT      (Color){ 226, 229, 236, 255 }
#define COL_MUTED     (Color){ 148, 154, 172, 255 }
#define COL_ACCENT    (Color){  86, 156, 255, 255 }
#define COL_GOOD      (Color){  80, 200, 120, 255 }
#define COL_GOOD_DARK (Color){  38, 140,  80, 255 }
#define COL_BAD       (Color){ 235,  90,  90, 255 }
#define COL_BAD_DARK  (Color){ 150,  50,  55, 255 }
#define COL_CODE_BG   (Color){  15,  16,  21, 255 }
#define COL_CODE_TEXT (Color){ 205, 220, 255, 255 }

// Fonts: a UI font and a monospace code font. Loaded from the system
// (or from assets/ui.ttf and assets/code.ttf) and fall back to raylib's default.
extern Font gUiFont;
extern Font gCodeFont;

void  UiInit(void);
void  UiShutdown(void);
void  UiBeginFrame(void);
void  UiEndFrame(void);
void  UiSetCursor(int cursor);

// Starts drawing with the UI zoom. Called after BeginDrawing, and again after any
// render-texture pass (which resets the transform).
void  UiApplyScale(void);
// Scissor rectangle in layout coordinates (BeginScissorMode wants real pixels).
void  UiBeginScissor(float x, float y, float width, float height);

float UiSpacing(float size);
float UiCodeSpacing(float size);

void  UiText(const char *text, float x, float y, float size, Color color);
void  UiTextCentered(const char *text, float centerX, float y, float size, Color color);
float UiTextWidth(const char *text, float size);

// Word-wrapped paragraph text. Returns the height used.
float UiWrapped(const char *text, float x, float y, float maxWidth, float size, Color color);
float UiWrappedHeight(const char *text, float maxWidth, float size);   // measures without drawing

// Multi-line monospace text. maxWidth <= 0 means no wrapping. Returns the height used.
float UiCode(const char *text, float x, float y, float size, Color color, float maxWidth);

// A code snippet drawn in a dark rounded box, with syntax colours. Returns the height used.
float UiCodeBlock(const char *code, float x, float y, float width, float size);

// While blocked (a pop-up card is showing on top), click areas and buttons ignore the mouse.
void  UiSetBlocked(bool blocked);

// Clickable area / button. Only one click is accepted per frame.
bool  UiClickArea(Rectangle r, bool *hover);
bool  UiButton(Rectangle r, const char *label, Color base);

#endif
