#ifndef HEISTVIEW_H
#define HEISTVIEW_H

#include "raylib.h"
#include "runner.h"

// Memory Heist: draws the street of memory and replays what the thief did.

void HeistLoad(const TestCase *tests, int count);
void HeistPlay(int test, const char *events, bool won, bool crashed);
void HeistReplay(void);
void HeistSetSpeed(float eventsPerSecond);
bool HeistPlaying(void);
void HeistDraw(Rectangle area, int test);

#endif
