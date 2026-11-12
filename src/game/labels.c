// Text pinned to points in the 3D world: array indexes, addresses, variable names and
// the values a program printed. Queued while the scene draws, then drawn flat on top of
// the finished picture (so they stay sharp and never bloom), farthest first.
#include "gfx.h"
#include "raymath.h"
#include "../ui.h"
#include <stdlib.h>
#include <string.h>

#define LABEL_MAX 320

typedef struct {
    Vector3 at;
    char    text[44];
    float   size;
    Color   fg, bg;
    int     flags;
    float   depth;
} Label;

static Label labels[LABEL_MAX];
static int   count;

void LabelsClear(void) { count = 0; }

void Label3D(Vector3 at, const char *text, float size, Color fg, Color bg, int flags)
{
    if (count >= LABEL_MAX || !text || !text[0]) return;
    Label *l = &labels[count++];
    l->at = at;
    strncpy(l->text, text, sizeof(l->text) - 1);
    l->text[sizeof(l->text) - 1] = '\0';
    l->size = size;
    l->fg = fg;
    l->bg = bg;
    l->flags = flags;
}

static int FarthestFirst(const void *a, const void *b)
{
    float da = ((const Label *)a)->depth, db = ((const Label *)b)->depth;
    return (da < db) - (da > db);
}

void LabelsDraw(Rectangle view, Camera3D camera)
{
    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    for (int i = 0; i < count; i++) labels[i].depth = Vector3DotProduct(Vector3Subtract(labels[i].at, camera.position), forward);
    qsort(labels, (size_t)count, sizeof(Label), FarthestFirst);

    // labels grow a little with the view, so they read the same in the full-screen view
    float scale = view.height / 400.0f;
    if (scale < 0.85f) scale = 0.85f;
    if (scale > 1.6f) scale = 1.6f;

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
    for (int i = 0; i < count; i++) {
        const Label *l = &labels[i];
        if (l->depth < 0.2f) continue;                       // behind the camera
        Vector2 s = GetWorldToScreenEx(l->at, camera, (int)view.width, (int)view.height);
        float x = view.x + s.x, y = view.y + s.y;
        float size = l->size * scale * ((l->flags & LABEL_BIG) ? 1.4f : 1.0f);
        bool code = !(l->flags & LABEL_PLAIN);
        float w = code ? MeasureTextEx(gCodeFont, l->text, size, UiCodeSpacing(size)).x : UiTextWidth(l->text, size);
        if (l->flags & LABEL_LEFT) x += w / 2;
        if (x + w < view.x - 40 || x - w > view.x + view.width + 40) continue;
        if (l->bg.a > 0) {
            float padX = size * 0.4f, padY = size * 0.16f;
            Rectangle box = { x - w / 2 - padX, y - size / 2 - padY, w + padX * 2, size + padY * 2 };
            DrawRectangleRounded(box, 0.45f, 6, l->bg);
        }
        if (code) DrawTextEx(gCodeFont, l->text, (Vector2){ x - w / 2, y - size / 2 }, size, UiCodeSpacing(size), l->fg);
        else UiText(l->text, x - w / 2, y - size / 2, size, l->fg);
    }
    EndScissorMode();
    count = 0;
}
