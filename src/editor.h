#ifndef EDITOR_H
#define EDITOR_H

#include "raylib.h"

#define EDITOR_CAP 16384

// A simple multi-line code editor: typing, arrows, Home/End, Tab, auto-indent,
// Backspace/Delete, Ctrl+V paste, mouse click to place the cursor, wheel scroll.
typedef struct {
    char  text[EDITOR_CAP];
    int   len;
    int   cursor;
    float scrollY;
    float blink;
    int   fontSize;
    bool  focused;
} Editor;

void EditorInit(Editor *e, int fontSize);
void EditorSetText(Editor *e, const char *text);
void EditorUpdate(Editor *e, Rectangle bounds);
void EditorDraw(const Editor *e, Rectangle bounds);

#endif
