#include "gamify.h"
#include "tutorials.h"
#include "projects.h"
#include <stdio.h>
#include <time.h>

static const Badge BADGES[BADGE_COUNT] = {
    { "First Steps",     "Clear your first mission.",                              { 120, 200, 255, 255 } },
    { "Escape Artist",   "Clear every Robot Rescue level on C Island.",             {  86, 156, 255, 255 } },
    { "Pointer Pro",     "Clear every Memory Heist mission.",                      { 200, 120, 255, 255 } },
    { "Zero Bugs",       "Squash every bug in a Bug Hunt on your first run.",      {  80, 200, 120, 255 } },
    { "Flawless",        "Clear an Output Rush without a single mistake.",         { 255, 200,  80, 255 } },
    { "Speed Reader",    "Answer 10 questions correctly in one Warm-up.",          { 255, 140,  80, 255 } },
    { "Factory Foreman", "Complete every step of a Factory Contract.",             { 240, 180,  60, 255 } },
    { "Boss Slayer",     "Defeat a boss.",                                         { 235,  90,  90, 255 } },
    { "Lean Coder",      "Clear a Robot Rescue or Memory Heist within par.",       {  90, 220, 220, 255 } },
    { "Self-Made",       "Earn all three stars on five missions.",                 { 255, 215,  90, 255 } },
    { "Star Collector",  "Collect 50 stars.",                                      { 255, 230, 120, 255 } },
    { "On Fire",         "Make progress three days in a row.",                     { 255, 120,  60, 255 } },
    { "Week Warrior",    "Make progress seven days in a row.",                     { 255,  90,  40, 255 } },
    { "Bookworm",        "Read 10 briefings to the last page.",                    { 160, 200, 120, 255 } },
    { "Island Hopper",   "Clear a main mission in all four regions.",              {  70, 190, 170, 255 } },
    { "Completionist",   "Clear the main mission of every level.",                 { 255, 255, 255, 255 } },
};

const Badge *GetBadge(int i) { return &BADGES[i < 0 || i >= BADGE_COUNT ? 0 : i]; }
bool HasBadge(int i) { return (prog.badges >> i) & 1u; }

// Level n needs 100 * (1 + 2 + ... + (n - 1)) XP: 100, 300, 600, 1000...
int LevelXp(int level) { return 50 * (level - 1) * level; }

int PlayerLevel(int xp)
{
    int level = 1;
    while (xp >= LevelXp(level + 1)) level++;
    return level;
}

const char *PlayerTitle(int level)
{
    static const char *titles[] = { "Beginner", "Novice", "Apprentice", "Coder", "Programmer",
                                    "Hacker", "Engineer", "Architect", "Wizard", "Legend" };
    int i = level - 1;
    if (i > 9) i = 9;
    return titles[i];
}

void SaveProgress(void) { ProgressSave(&prog, PROGRESS_FILE); }

bool MissionCleared(int mission) { return mission >= 0 && (prog.stars[mission] & STAR_CLEAR); }

bool LevelCleared(int tut) { return MissionCleared(MainMissionOf(tut)); }

int TotalStars(void)
{
    int n = 0;
    for (int i = 0; i < MissionCount() && i < MAX_MISSIONS; i++)
        if (GetMission(i)->role != ROLE_SIDE) n += StarCount(prog.stars[i]);
    for (int t = 0; t < TutorialCount() && t < MAX_TUTORIALS; t++) n += prog.rushStars[t];
    return n;
}

int SideStars(void)
{
    int n = 0;
    for (int i = 0; i < MissionCount() && i < MAX_MISSIONS; i++)
        if (GetMission(i)->role == ROLE_SIDE) n += StarCount(prog.stars[i]);
    return n;
}

static int SideCount(void)
{
    int n = 0;
    for (int i = 0; i < MissionCount(); i++) if (GetMission(i)->role == ROLE_SIDE) n++;
    return n;
}

int MaxSideStars(void) { return 3 * SideCount(); }

int MaxStars(void) { return 3 * (MissionCount() - SideCount() + TutorialCount()); }

static int Today(void)
{
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    return (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
}

static int Yesterday(void)
{
    time_t now = time(NULL) - 24 * 60 * 60;
    struct tm *tm = localtime(&now);
    return (tm->tm_year + 1900) * 10000 + (tm->tm_mon + 1) * 100 + tm->tm_mday;
}

static void TouchStreak(void)
{
    int today = Today();
    if (prog.lastDay == today) return;
    prog.streak = (prog.lastDay == Yesterday()) ? prog.streak + 1 : 1;
    prog.lastDay = today;
    if (prog.streak > prog.bestStreak) prog.bestStreak = prog.streak;
    if (prog.streak > 1)
        ToastPush(TextFormat("%d-day streak!", prog.streak), "You've made progress every day. Keep it going!",
                  (Color){ 255, 140, 60, 255 }, TOAST_STREAK);
}

static void GiveBadge(int b)
{
    if (HasBadge(b)) return;
    prog.badges |= 1u << b;
    ToastPush(TextFormat("Badge: %s", BADGES[b].name), BADGES[b].desc, BADGES[b].color, TOAST_BADGE);
}

static void GiveXp(int amount)
{
    if (amount <= 0) return;
    int before = PlayerLevel(prog.xp);
    prog.xp += amount;
    int after = PlayerLevel(prog.xp);
    if (after > before)
        ToastPush(TextFormat("Level %d!", after), TextFormat("You're now a %s.", PlayerTitle(after)),
                  (Color){ 255, 215, 90, 255 }, TOAST_LEVEL);
}

// Badges that depend on the overall picture
static void CheckBadges(void)
{
    int cleared = 0, perfect = 0;
    bool allRobots = true, anyRobot = false, allHeists = true, anyHeist = false, allMains = true;
    bool regionDone[MAX_REGIONS] = { false };

    for (int i = 0; i < MissionCount() && i < MAX_MISSIONS; i++) {
        const MissionInfo *m = GetMission(i);
        bool done = (prog.stars[i] & STAR_CLEAR) != 0;
        if (done) cleared++;
        if (StarCount(prog.stars[i]) == 3) perfect++;
        if (m->mode == MODE_ROBOT && m->region == 0) { anyRobot = true; if (!done) allRobots = false; }
        if (m->mode == MODE_HEIST) { anyHeist = true; if (!done) allHeists = false; }
        if (m->role == ROLE_MAIN) {
            if (!done) allMains = false;
            else if (m->region >= 0 && m->region < MAX_REGIONS) regionDone[m->region] = true;
        }
        if (done && m->role == ROLE_BOSS) GiveBadge(BADGE_BOSS_SLAYER);
        if ((prog.stars[i] & STAR_MASTER) && (m->mode == MODE_ROBOT || m->mode == MODE_HEIST)) GiveBadge(BADGE_LEAN_CODER);
        if ((prog.stars[i] & STAR_MASTER) && m->mode == MODE_BUGS) GiveBadge(BADGE_ZERO_BUGS);
    }
    if (cleared >= 1) GiveBadge(BADGE_FIRST_STEPS);
    if (anyRobot && allRobots) GiveBadge(BADGE_ESCAPE_ARTIST);
    if (anyHeist && allHeists) GiveBadge(BADGE_POINTER_PRO);
    if (perfect >= 5) GiveBadge(BADGE_SELF_MADE);
    if (TotalStars() >= 50) GiveBadge(BADGE_STAR_COLLECTOR);
    if (allMains) GiveBadge(BADGE_COMPLETIONIST);

    bool everyRegion = RegionCount() > 0;
    for (int r = 0; r < RegionCount(); r++) if (!regionDone[r]) everyRegion = false;
    if (everyRegion) GiveBadge(BADGE_ISLAND_HOPPER);

    int read = 0;
    for (int t = 0; t < TutorialCount() && t < MAX_TUTORIALS; t++) if (prog.lessonRead[t]) read++;
    if (read >= 10) GiveBadge(BADGE_BOOKWORM);

    for (int p = 0; p < ProjectCount() && p < MAX_PROJECTS; p++)
        if (prog.projSteps[p] >= GetProject(p)->stepCount) GiveBadge(BADGE_FOREMAN);

    if (prog.streak >= 3) GiveBadge(BADGE_ON_FIRE);
    if (prog.streak >= 7) GiveBadge(BADGE_WEEK_WARRIOR);
}

static int Finish(int xp)
{
    TouchStreak();
    GiveXp(xp);
    CheckBadges();
    SaveProgress();
    return xp;
}

int AwardMission(int mission, unsigned char bits)
{
    if (mission < 0 || mission >= MAX_MISSIONS) return 0;
    unsigned char before = prog.stars[mission];
    unsigned char now = before | bits;
    if (now == before) return Finish(0);
    prog.stars[mission] = now;

    const MissionInfo *m = GetMission(mission);
    int xp = 0;
    if (!(before & STAR_CLEAR) && (now & STAR_CLEAR)) xp += (m->role == ROLE_BOSS) ? 200 : (m->role == ROLE_MAIN) ? 80 : 50;
    xp += 25 * (StarCount(now) - StarCount(before));
    return Finish(xp);
}

int AwardRush(int tut, int stars, int mistakes)
{
    if (tut < 0 || tut >= MAX_TUTORIALS) return 0;
    int before = prog.rushStars[tut];
    int xp = 0;
    if (before == 0) xp += 30;
    if (stars > before) {
        xp += 10 * (stars - before);
        prog.rushStars[tut] = (unsigned char)stars;
    }
    if (mistakes == 0) GiveBadge(BADGE_FLAWLESS);
    return Finish(xp);
}

int AwardWarmup(int score, int correct)
{
    if (score > prog.warmupBest) prog.warmupBest = score;
    if (correct >= 10) GiveBadge(BADGE_SPEED_READER);
    return Finish(correct * 5);
}

int AwardLesson(int tut)
{
    if (tut < 0 || tut >= MAX_TUTORIALS || prog.lessonRead[tut]) return 0;
    prog.lessonRead[tut] = true;
    return Finish(10);
}

int AwardContractStep(int proj, int stepsBefore, int stepsNow, int stepCount)
{
    (void)proj;
    if (stepsNow <= stepsBefore) return Finish(0);
    int xp = 40 * (stepsNow - stepsBefore);
    if (stepsNow >= stepCount && stepsBefore < stepCount) xp += 100;
    return Finish(xp);
}
