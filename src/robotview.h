#ifndef ROBOTVIEW_H
#define ROBOTVIEW_H

#include "raylib.h"
#include "runner.h"

// Robot Rescue: draws the mazes and replays what the robot did.

void RobotLoad(const TestCase *tests, int count);    // read every maze from the tests' input
void RobotPlay(int test, const char *events, bool won);
void RobotReplay(void);                               // start the current replay again
void RobotSetSpeed(float eventsPerSecond);
bool RobotPlaying(void);
void RobotDraw(Rectangle area, int test);             // the maze for `test`, animated if it is the one playing

#endif
