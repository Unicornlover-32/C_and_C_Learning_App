#ifndef GAME_LEVELS_H
#define GAME_LEVELS_H

// The level table (levels.c) and the player that shows a level's scene for each test
// of a run (playback.c). A level is a parser plus a scene:
//   parse   turns one test run (input, expected output, what the program printed) into events
//   reset   builds a fresh world for one test
//   event   starts one event's animation and says how long to wait before the next
//   update / draw  animate and draw the world (draw is called inside the 3D scene)
#include "raylib.h"
#include "events.h"

typedef struct {
    const char *tutId;        // the tutorial whose project this level is
    const char *name;         // "Coin Grid"
    const char *tagline;      // one line for the level card
    Color       color;
    LevelParser parse;
    void  (*reset)(const EventList *ev);
    float (*event)(const GameEvent *e);
    void  (*update)(float dt);
    void  (*draw)(void);
    void  (*overlay)(Rectangle view);     // optional 2D drawing on top of the scene
} LevelDef;

const LevelDef *LevelFor(const char *tutId);     // NULL if that project has no 3D level yet

// Levels for the missions that aren't a tutorial's own project
extern const LevelDef LEVEL_FACTORY;             // Factory Contracts and the factory boss: any program's output
extern const LevelDef LEVEL_ROBOT;               // Robot Rescue side quests and bosses: the robot library's trace
extern const LevelDef LEVEL_HEIST;               // Memory Heist side quests and the boss: the heist library's trace
int             LevelTableCount(void);
const LevelDef *LevelTableAt(int i);

// ---------------------------------------------------------------- playback.c
void PlaybackOpen(const LevelDef *level, const TestCase *tests, int testCount);  // shows test 1's world
void PlaybackRun(const RunResult *result);        // after Run & Check: plays every test in order
void PlaybackCompileFailed(void);                  // the code didn't compile
void PlaybackClear(void);                          // forget the last run (the code changed)
void PlaybackUpdate(float frameTime);
void PlaybackDraw(Rectangle view);                 // the scene and its heads-up display
void PlaybackBar(Rectangle bar);                   // test tabs, replay, skip, speed
void PlaybackResults(Rectangle area);              // input, expected and actual output, in step with the scene
bool PlaybackBusy(void);                           // still playing a run
bool PlaybackHasRun(void);
int  PlaybackTest(void);
bool PlaybackTestDone(void);                       // the test being shown has played to its end
void PlaybackSetFixedStep(float dt);               // screenshots: every frame advances exactly dt

// Called by the levels
void PlaybackShake(float amount);
void PlaybackSlowMo(float scale, float seconds);
bool PlaybackFinale(void);                         // the last test of a run where every test passed

#endif
