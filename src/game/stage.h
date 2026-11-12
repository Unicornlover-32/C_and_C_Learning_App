#ifndef GAME_STAGE_H
#define GAME_STAGE_H

// Small helpers every level scene uses (stage.c):
//   cues    things that happen a moment later (when the robot arrives, when a crate lands)
//   pops    words that float up and fade ("+6", "\n", "swap!")
//   shots   camera positions given as "look at this point from this direction"
#include "gfx.h"
#include "kits.h"
#include "raymath.h"

#define LBL_DIM   (Color){ 12, 14, 22, 195 }
#define LBL_HI    (Color){ 86, 156, 255, 235 }
#define LBL_GOOD  (Color){ 140, 255, 180, 255 }
#define LBL_BAD   (Color){ 255, 130, 130, 255 }
#define LBL_GOLD  (Color){ 255, 222, 130, 255 }
#define LBL_SOFT  (Color){ 175, 185, 210, 255 }
#define COL3_GOOD (Color){ 80, 220, 130, 255 }
#define COL3_BAD  (Color){ 235, 90, 90, 255 }
#define COL3_WARN (Color){ 255, 160, 60, 255 }

static inline Vector3 V3(float x, float y, float z) { return (Vector3){ x, y, z }; }

// ---------------------------------------------------------------- cues
#define CUE_MAX 48
typedef struct { float at; int what, a, b; Vector3 v; float f; bool live; } Cue;
typedef struct { Cue c[CUE_MAX]; } Cues;

void CuesClear(Cues *q);
void CueAt(Cues *q, float when, int what, int a, int b, Vector3 v, float f);
bool CueDue(Cues *q, float now, Cue *out);          // takes the earliest cue that is due
void CuesDrop(Cues *q, int what);                    // forgets every cue of one kind

// ---------------------------------------------------------------- pops
#define POP_MAX 16
typedef struct { char text[32]; Vector3 at; float t, life; Color color; } PopText;
typedef struct { PopText p[POP_MAX]; int next; } Pops;

void PopsClear(Pops *ps);
void PopAdd(Pops *ps, const char *text, Vector3 at, Color color);
void PopsUpdate(Pops *ps, float dt);
void PopsDraw(const Pops *ps);

// ---------------------------------------------------------------- shots and text
// Points the camera at `look` from `dist` away, `pitch` degrees above the ground and turned
// `yaw` degrees from straight-on (0 = looking towards -z)
void ShotOrbit(Vector3 look, float dist, float pitch, float yaw, float fovy, float seconds);
void StrCopy(char *dst, int cap, const char *src);

// ---------------------------------------------------------------- reading output lines
// "Average: 6.00" -> 6.0 (the number must fill the rest of the line)
bool ScanAfterNumber(const char *s, int len, const char *prefix, double *value);
// "Before: 1 2 3" -> 1, 2, 3 (count may be 0: "Forward:" alone is an empty list)
bool ScanAfterInts(const char *s, int len, const char *prefix, int *out, int max, int *count);
// "Longest: cheese" -> "cheese" (everything after the prefix, without the leading spaces)
bool ScanAfterText(const char *s, int len, const char *prefix, char *out, int cap);

#endif
