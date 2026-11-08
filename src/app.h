#ifndef APP_H
#define APP_H

// Shared pieces of the app: which screen is showing, the helpers every screen uses,
// and how to open each screen.
#include "raylib.h"
#include "ui.h"
#include "art.h"
#include "tutorials.h"
#include "projects.h"
#include "missions.h"
#include "progress.h"
#include "gamify.h"
#include "runner.h"

typedef enum {
    SCREEN_MENU,
    SCREEN_MAP,
    SCREEN_LEVEL,
    SCREEN_BRIEFING,
    SCREEN_RUSH,
    SCREEN_MISSION,
    SCREEN_CONTRACTS,
    SCREEN_CONTRACT,
    SCREEN_PROFILE,
} Screen;

typedef struct {
    Screen screen;
    bool   quit;
    int    tut;          // the level being looked at (tutorial index)
} App;

extern App app;

// ---------------------------------------------------------------- shared UI (main.c)
void      DrawTopBar(const char *title);
void      TopBarRight(const char *text, Color color);
bool      BackButton(const char *label);
Rectangle ContentPanel(void);
float     NavY(void);
void      DrawPlayerChip(float right, float y);      // level, XP bar and streak, right-aligned
const char *GameDir(void);                            // where the game headers live
bool      CtrlDown(void);

// ---------------------------------------------------------------- screens
void OpenMenu(void);
void ScreenMenu(void);
void OpenProfile(void);
void ScreenProfile(void);

void OpenMap(void);
void ScreenMap(void);
int  NextLevel(void);                                 // the first level whose main mission isn't cleared, or -1

void OpenLevel(int tut);
void ScreenLevel(void);
void OpenBriefing(int tut, int page);
void ScreenBriefing(void);

void OpenRush(int tut);
void OpenWarmup(void);
void ScreenRush(void);

void MissionInit(void);
void MissionShutdown(void);
void OpenMission(int mission);
void OpenContractStep(int proj, int step);
void MissionLeave(void);                              // saves the code
void ScreenMission(void);

void OpenContracts(int region);                       // -1 shows every region
void ScreenContracts(void);
void OpenContract(int proj);
void ScreenContract(void);

#endif
