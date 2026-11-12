#ifndef MISSIONS_H
#define MISSIONS_H

#include "runner.h"

#define MAX_BUGS     4
#define MAX_REGIONS  4

// The four games. Robot Rescue and Memory Heist run the student's code with a game
// library in front of it; Code Factory and Bug Hunter run it on its own and check the
// output, like the original projects.
typedef enum { MODE_FACTORY, MODE_ROBOT, MODE_HEIST, MODE_BUGS } GameMode;

// Where a mission sits on the map: the main game of a level (its own project), a bonus
// round (Bug Hunter), the boss at the end of a region, or an optional side quest (Robot
// Rescue and Memory Heist), whose stars are counted separately.
typedef enum { ROLE_MAIN, ROLE_BONUS, ROLE_BOSS, ROLE_SIDE } MissionRole;

// A Bug Hunter bug: the line as it should be (found in the base solution) and the
// broken version that replaces it. Either may span several lines.
typedef struct {
    const char *fixedText;
    const char *buggyText;
    const char *note;          // what was wrong, shown once the bug is squashed
} BugDef;

typedef struct {
    const char *id;            // stable name used in the save file - never change it once released
    const char *level;         // id of the tutorial (level) it belongs to; NULL for bosses
    int         region;        // bosses: which region they guard (0-3)
    MissionRole role;
    GameMode    mode;
    Language    lang;
    const char *title;
    const char *brief;
    const char *api;           // robot and heist: what the game gives you, one item per line
    const char *config;        // robot and heist: #defines that switch the game's features on
    const char *starterCode;   // bug hunts: NULL, the buggy code is built from the solution
    const char *hint;
    const char *solution;      // bug hunts: NULL means "the level's own project solution"
    int         parLines;      // robot and heist: the lines of code for the third star
    int         timeLimit;     // bug hunts: seconds on the clock
    int         bugCount;
    BugDef      bugs[MAX_BUGS];
    int         testCount;     // bug hunts: 0 means "the level's own project tests"
    TestCase    tests[MAX_TESTS];
} Mission;

int            MissionTableCount(void);
const Mission *GetMissionTable(int index);

// ---------------------------------------------------------------- the registry
//
// Every playable mission, resolved: the game missions above plus each level's own
// project, which becomes a Code Factory mission. Progress is stored by these indexes.

#define MAX_MISSIONS 96

typedef struct {
    int         bugLine;       // 0-based line the bug sits on in the buggy code
    const char *buggyLine;     // that line's text (the bug stays on it while it is unchanged)
    const char *note;
} BugSpot;

typedef struct {
    const char     *id;
    GameMode        mode;
    MissionRole     role;
    Language        lang;
    int             tut;       // the level it belongs to, or -1 for bosses
    int             region;
    const char     *title;
    const char     *brief;
    const char     *api;
    const char     *prelude;   // the text put in front of the student's code (robot and heist)
    const char     *starter;
    const char     *hint;
    const char     *solution;
    int             parLines;
    int             timeLimit;
    int             bugCount;
    BugSpot         bugs[MAX_BUGS];
    const TestCase *tests;
    int             testCount;
} MissionInfo;

void               MissionsInit(void);
int                MissionCount(void);
const MissionInfo *GetMission(int index);
int                FindMission(const char *id);

int  MainMissionOf(int tut);              // the level's headline mission
int  LevelMissions(int tut, int *out);    // main first, then side quests, then bonus rounds; returns how many
int  BossOf(int region);                  // -1 if the region has none

int  RegionCount(void);
int  RegionOf(int tut);
int  RegionFirstTut(int region);
int  RegionTutCount(int region);

const char *ModeName(GameMode mode);      // "Robot Rescue", "Memory Heist"...
int  CountCodeLines(const char *code);    // the lines that count towards par

#endif
