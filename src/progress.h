#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdbool.h>
#include "editor.h"
#include "missions.h"

#define MAX_TUTORIALS 32
#define MAX_PROJECTS  24
#define PROGRESS_FILE "cfd_progress.txt"

// The three stars a mission can earn
#define STAR_CLEAR  1   // finished it
#define STAR_NOHELP 2   // finished it without the hint or the solution
#define STAR_MASTER 4   // robot and heist: within par lines; the others: passed on the first run

// Everything the app remembers between runs.
typedef struct {
    // Levels (tutorials), indexed like GetTutorial()
    unsigned char rushStars[MAX_TUTORIALS];   // Output Rush: 0 = not cleared yet, else 1-3 stars
    bool          lessonRead[MAX_TUTORIALS];  // the briefing was read to the last page

    // Missions, indexed like GetMission()
    unsigned char stars[MAX_MISSIONS];        // STAR_ bits
    bool          hasDraft[MAX_MISSIONS];
    char          drafts[MAX_MISSIONS][EDITOR_CAP];

    // Factory Contracts (the bigger projects), indexed like GetProject()
    int  projSteps[MAX_PROJECTS];             // how many steps are finished
    bool projHasDraft[MAX_PROJECTS];
    char projDrafts[MAX_PROJECTS][EDITOR_CAP];

    // The player
    int      xp;
    unsigned badges;                          // one bit per badge
    int      streak;                          // days in a row with some progress
    int      bestStreak;
    int      lastDay;                         // the last day with progress, as YYYYMMDD
    int      warmupBest;                      // best Output Rush warm-up score
} Progress;

// Loads the save file if there is one (missing or broken files just mean a fresh start).
// Version 1 files from before the games are converted.
void ProgressLoad(Progress *p, const char *path);
bool ProgressSave(const Progress *p, const char *path);

int StarCount(unsigned char bits);

#endif
