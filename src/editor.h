#ifndef EDITOR_H
#define EDITOR_H

#include "raylib.h"

#define EDITOR_CAP 16384
#define UNDO_MAX   200

typedef struct {
    char *text;
    int   len;
    int   cursor;
} EditSnapshot;

// A multi-line code editor with syntax highlighting.
//   Typing, arrows, Home/End, Tab and auto-indent, Backspace/Delete
//   Selection with the mouse (click and drag) or Shift + arrows/Home/End
//   Ctrl+A select all, Ctrl+C copy, Ctrl+X cut, Ctrl+V paste
//   Ctrl+Z undo, Ctrl+Y or Ctrl+Shift+Z redo
//   Tab / Shift+Tab indent or un-indent the selected lines
typedef struct {
    char  text[EDITOR_CAP];
    int   len;
    int   cursor;
    int   anchor;        // other end of the selection; anchor == cursor means nothing selected
    float scrollY;
    float blink;
    int   fontSize;
    bool  focused;
    bool  dragging;

    EditSnapshot undo[UNDO_MAX];
    int          undoCount;
    EditSnapshot redo[UNDO_MAX];
    int          redoCount;
    int          lastEditKind;
    double       lastEditTime;
} Editor;

void EditorInit(Editor *e, int fontSize);
void EditorFree(Editor *e);
void EditorSetText(Editor *e, const char *text);      // fresh start, clears undo history
void EditorReplaceAll(Editor *e, const char *text);   // like SetText, but can be undone
void EditorUndo(Editor *e);
void EditorRedo(Editor *e);
void EditorUpdate(Editor *e, Rectangle bounds);
void EditorDraw(const Editor *e, Rectangle bounds);

// Where line `line` (counting from 0) is drawn, so other things can be placed beside it
float EditorLineY(const Editor *e, Rectangle bounds, int line);
float EditorLineHeight(const Editor *e);
// Puts the cursor at the end of a line and scrolls it into view
void  EditorGotoLine(Editor *e, Rectangle bounds, int line);
// The line whose text is exactly `text`, nearest to line `near`; -1 if there is none
int   EditorFindLine(const Editor *e, const char *text, int near);

#endif
