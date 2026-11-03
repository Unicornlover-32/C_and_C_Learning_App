#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "raylib.h"
#include <stdbool.h>

// Syntax colours for C and C++ code.
#define HL_KEYWORD (Color){ 198, 120, 221, 255 }
#define HL_TYPE    (Color){  86, 182, 194, 255 }
#define HL_STRING  (Color){ 152, 195, 121, 255 }
#define HL_NUMBER  (Color){ 209, 154, 102, 255 }
#define HL_COMMENT (Color){ 112, 120, 138, 255 }
#define HL_PREPROC (Color){ 224, 108, 117, 255 }

// Draws one line of code in colour. `state` carries a /* block comment */ from
// one line to the next: start it at 0 for the first line.
// With draw == false it only updates `state` (for lines scrolled off screen).
void  HlLine(Font font, const char *line, int len, float x, float y, float size, float spacing,
             int *state, bool draw);

// Draws multi-line code in colour (no wrapping). Returns the height used.
float HlCode(Font font, const char *code, float x, float y, float size, float spacing, float lineHeight);

#endif
