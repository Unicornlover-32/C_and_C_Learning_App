#ifndef ART_H
#define ART_H

#include "raylib.h"
#include "missions.h"

// Little hand-drawn pictures used all over the app (no image files needed).

#define COL_GOLD      (Color){ 255, 205,  70, 255 }
#define COL_GOLD_DARK (Color){ 150, 115,  30, 255 }
#define COL_ROBOT     (Color){ 120, 190, 255, 255 }
#define COL_HEIST     (Color){ 190, 130, 255, 255 }
#define COL_FACTORY   (Color){ 255, 170,  70, 255 }
#define COL_BUGS      (Color){ 120, 215, 110, 255 }
#define COL_RUSH      (Color){ 255, 110, 140, 255 }

Color ModeColor(GameMode mode);

void DrawStar(Vector2 c, float r, Color color);
void DrawStarOutline(Vector2 c, float r, float thick, Color color);
// Three stars in a row, filled for each STAR_ bit that is set (or for the first `count` when bits < 0)
void DrawStarRow(float centerX, float y, float r, unsigned char bits, int count);

void IconRobot(Vector2 c, float s, Color color);
void IconMask(Vector2 c, float s, Color color);
void IconGear(Vector2 c, float s, float angle, Color color);
void IconBug(Vector2 c, float s, float angle, Color color);
void IconBook(Vector2 c, float s, Color color);
void IconRush(Vector2 c, float s, Color color);
void IconCrown(Vector2 c, float s, Color color);
void IconFlame(Vector2 c, float s, Color color);
void IconFactory(Vector2 c, float s, Color color);
void IconLock(Vector2 c, float s, Color color);
void IconMedal(Vector2 c, float s, Color color, int index);
void DrawModeIcon(GameMode mode, Vector2 c, float s);

// Confetti: burst some, then call ParticlesDraw() once a frame after everything else
void ParticlesBurst(float x, float y, int count, float speed);
void ParticlesDraw(void);

#endif
