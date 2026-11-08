// Factory Contracts: the bigger projects, built one step at a time. Each region's factory
// on the map lists its own contracts; every step is a Code Factory mission.
#include "app.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static int    region = -1;       // -1: every region
static int    proj;
static float  listScroll;
static double restartConfirmAt;

static const char *RegionName(int r)
{
    static const char *names[] = { "Robot Rescue Bay", "Memory City", "Class Town", "STL Harbour" };
    return (r >= 0 && r < 4) ? names[r] : "Every region";
}

static bool ContractDone(int i) { return prog.projSteps[i] >= GetProject(i)->stepCount; }

void OpenContracts(int r)
{
    region = r;
    listScroll = 0;
    app.screen = SCREEN_CONTRACTS;
}

void OpenContract(int p)
{
    proj = p;
    restartConfirmAt = 0;
    app.screen = SCREEN_CONTRACT;
}

void ScreenContracts(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { OpenMap(); return; }

    const float top = 66, cardH = 96, gap = 14, headH = 64, introH = 80;
    int count = ProjectCount();

    float contentH = 24 + introH;
    int lastRegion = -2;
    for (int i = 0; i < count; i++) {
        const Project *p = GetProject(i);
        if (region >= 0 && p->region != region) continue;
        if (p->region != lastRegion) { contentH += headH; lastRegion = p->region; }
        contentH += cardH + gap;
    }
    float viewH = SCREEN_H - top;
    float maxScroll = contentH - viewH;
    if (maxScroll < 0) maxScroll = 0;
    listScroll -= GetMouseWheelMove() * 50;
    if (listScroll < 0) listScroll = 0;
    if (listScroll > maxScroll) listScroll = maxScroll;

    float cardW = SCREEN_W - 160.0f;
    if (cardW > 1400) cardW = 1400;
    float cardX = (SCREEN_W - cardW) / 2;

    BeginScissorMode(0, (int)top, SCREEN_W, (int)viewH);
    float y = top + 22 - listScroll;
    IconFactory((Vector2){ cardX + 30, y + 26 }, 52, COL_FACTORY);
    UiWrapped("Bigger programs, built one step at a time - the kind every coding course has you make. Each step is a new order for the "
              "factory: your code carries on from one step to the next, and every test is a crate on the belt.",
              cardX + 76, y, cardW - 80, 19, COL_MUTED);
    y += introH;

    lastRegion = -2;
    for (int i = 0; i < count; i++) {
        const Project *p = GetProject(i);
        if (region >= 0 && p->region != region) continue;
        if (p->region != lastRegion) {
            lastRegion = p->region;
            UiText(RegionName(p->region), cardX + 4, y + 14, 32, COL_FACTORY);
            UiText(p->lang == LANG_CPP ? "C++ contracts" : "C contracts", cardX + 4 + UiTextWidth(RegionName(p->region), 32) + 18,
                   y + 26, 19, COL_MUTED);
            y += headH;
        }

        Rectangle card = { cardX, y, cardW, cardH };
        bool hover = false;
        bool clicked = GetMouseY() > top && UiClickArea(card, &hover);
        DrawRectangleRounded(card, 0.18f, 8, hover ? COL_PANEL_HI : COL_PANEL);

        Vector2 c = { card.x + 52, card.y + cardH / 2 };
        DrawCircleV(c, 30, ContractDone(i) ? COL_GOOD_DARK : ColorAlpha(COL_FACTORY, 0.25f));
        IconGear(c, 40, (float)GetTime() * (hover ? 2 : 0.3f), ContractDone(i) ? WHITE : COL_FACTORY);

        UiText(p->title, card.x + 100, card.y + 18, 26, COL_TEXT);
        if (p->lecture) {
            const char *lec = TextFormat("Lecture: %s", p->lecture);
            UiText(lec, card.x + card.width - 30 - UiTextWidth(lec, 15), card.y + 12, 15, COL_ACCENT);
        }
        UiText(p->summary, card.x + 100, card.y + 56, 18, COL_MUTED);

        // a row of step lights
        int done = prog.projSteps[i];
        for (int s = 0; s < p->stepCount; s++) {
            Vector2 d = { card.x + card.width - 30 - (p->stepCount - 1 - s) * 22.0f, card.y + 44 };
            DrawCircleV(d, 7, s < done ? COL_GOOD : COL_PANEL_HI);
        }
        const char *status = ContractDone(i) ? "Completed" : done > 0 ? TextFormat("Step %d of %d  >", done + 1, p->stepCount)
                                                                   : TextFormat("%d steps  >", p->stepCount);
        UiText(status, card.x + card.width - 30 - UiTextWidth(status, 18), card.y + 60, 18,
               ContractDone(i) ? COL_GOOD : hover ? COL_ACCENT : COL_MUTED);

        if (clicked) { OpenContract(i); EndScissorMode(); return; }
        y += cardH + gap;
    }
    EndScissorMode();

    DrawTopBar(region >= 0 ? TextFormat("%s Factory", RegionName(region)) : "Factory Contracts");
    DrawPlayerChip(SCREEN_W - 24, 14);
    if (BackButton("< Map")) OpenMap();
}

void ScreenContract(void)
{
    const Project *p = GetProject(proj);
    int stepsDone = prog.projSteps[proj];
    bool started = stepsDone > 0 || prog.projHasDraft[proj];

    if (IsKeyPressed(KEY_ESCAPE)) { OpenContracts(region); return; }
    if (restartConfirmAt > 0 && GetTime() - restartConfirmAt > 3.0) restartConfirmAt = 0;

    DrawTopBar(TextFormat("Contract: %s", p->title));
    TopBarRight(p->lang == LANG_CPP ? "C++" : "C", COL_MUTED);
    if (BackButton("< Factory")) { OpenContracts(region); return; }

    Rectangle panel = ContentPanel();
    float left = panel.x, right = panel.x + panel.width, navY = NavY();
    DrawRectangleRounded(panel, 0.04f, 8, COL_PANEL);
    float x = panel.x + 40, w = panel.width - 80;
    float y = panel.y + 32;

    IconFactory((Vector2){ x + 26, y + 20 }, 48, COL_FACTORY);
    UiText(p->title, x + 64, y, 36, COL_TEXT);
    y += 60;
    y += UiWrapped(p->intro, x, y, w, 21, COL_TEXT) + 6;
    if (p->lecture) y += UiWrapped(TextFormat("Based on the lecture: %s", p->lecture), x, y, w, 18, COL_ACCENT) + 2;
    y += UiWrapped(TextFormat("Best after these levels: %s", p->needs), x, y, w, 18, COL_MUTED) + 18;

    // the steps: finished ones and the next one can be opened directly
    for (int i = 0; i < p->stepCount; i++) {
        Rectangle row = { x, y, w, 44 };
        bool unlocked = i <= stepsDone;
        bool hover = false;
        bool clicked = unlocked && UiClickArea(row, &hover);
        if (hover) DrawRectangleRounded(row, 0.3f, 8, COL_PANEL_HI);
        Color circle = COL_PANEL_HI;
        if (i < stepsDone) circle = COL_GOOD_DARK;
        else if (i == stepsDone) circle = COL_ACCENT;
        DrawCircle((int)(row.x + 24), (int)(row.y + 22), 15, circle);
        UiTextCentered(TextFormat("%d", i + 1), row.x + 24, row.y + 12, 19, unlocked ? WHITE : COL_MUTED);
        UiText(p->steps[i].title, row.x + 54, row.y + 11, 21, unlocked ? COL_TEXT : COL_MUTED);
        const char *status = i < stepsDone ? "Shipped" : i == stepsDone ? "Next order" : "";
        UiText(status, row.x + row.width - 16 - UiTextWidth(status, 18), row.y + 13, 18, i < stepsDone ? COL_GOOD : COL_ACCENT);
        if (clicked) { OpenContractStep(proj, i); return; }
        y += 50;
    }

    const char *label;
    if (ContractDone(proj)) label = "Open Contract >";
    else if (started)       label = TextFormat("Continue: Step %d >", stepsDone + 1);
    else                    label = "Start Contract >";
    if (UiButton((Rectangle){ right - 260, navY, 260, 50 }, label, ContractDone(proj) ? COL_ACCENT : COL_GOOD_DARK)) {
        OpenContractStep(proj, stepsDone);
        return;
    }

    if (started) {
        bool confirming = restartConfirmAt > 0;
        if (UiButton((Rectangle){ left, navY, 200, 50 }, confirming ? "Sure? Click again" : "Start Over",
                     confirming ? COL_BAD_DARK : COL_PANEL_HI)) {
            if (!confirming) {
                restartConfirmAt = GetTime();
            } else {
                prog.projSteps[proj] = 0;
                prog.projHasDraft[proj] = false;
                prog.projDrafts[proj][0] = '\0';
                restartConfirmAt = 0;
                SaveProgress();
            }
        }
        if (confirming) UiText("This deletes your code for this contract.", left + 216, navY + 15, 18, COL_BAD);
    }
}
