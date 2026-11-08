#ifndef GAMIFY_H
#define GAMIFY_H

#include "raylib.h"
#include "progress.h"

// XP, player levels, badges and the daily streak. Every Award function updates the
// progress, saves it and pops up toasts for anything new.

typedef enum {
    BADGE_FIRST_STEPS, BADGE_ESCAPE_ARTIST, BADGE_POINTER_PRO, BADGE_ZERO_BUGS,
    BADGE_FLAWLESS, BADGE_SPEED_READER, BADGE_FOREMAN, BADGE_BOSS_SLAYER,
    BADGE_LEAN_CODER, BADGE_SELF_MADE, BADGE_STAR_COLLECTOR, BADGE_ON_FIRE,
    BADGE_WEEK_WARRIOR, BADGE_BOOKWORM, BADGE_ISLAND_HOPPER, BADGE_COMPLETIONIST,
    BADGE_COUNT
} BadgeId;

typedef struct {
    const char *name;
    const char *desc;
    Color       color;
} Badge;

extern Progress prog;

const Badge *GetBadge(int i);
bool         HasBadge(int i);

int         PlayerLevel(int xp);           // 1, 2, 3...
int         LevelXp(int level);            // XP needed to reach that level
const char *PlayerTitle(int level);

int  TotalStars(void);                     // stars from missions and Output Rushes
int  MaxStars(void);
bool MissionCleared(int mission);
bool LevelCleared(int tut);                // its main mission is cleared

// Returns the XP gained.
int  AwardMission(int mission, unsigned char bits);
int  AwardRush(int tut, int stars, int mistakes);
int  AwardWarmup(int score, int correct);
int  AwardLesson(int tut);
int  AwardContractStep(int proj, int stepsBefore, int stepsNow, int stepCount);

void SaveProgress(void);

// Implemented by the UI: a little card that slides in at the top right.
void ToastPush(const char *title, const char *text, Color color, int icon);
enum { TOAST_XP, TOAST_BADGE, TOAST_LEVEL, TOAST_STREAK };

#endif
