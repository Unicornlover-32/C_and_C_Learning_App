// Sort Three -> "Memory Heist"
//
// Memory is a street: x, y and z are three vaults side by side, four bytes apart, each
// holding the loot read from the input. sortThree's pointers a, b and c glow over the
// street with arrows to the vaults they point at. "Before:" shows the loot. "After:" plays
// the swaps that turn the printed Before into the printed After: swap(int *a, int *b)
// lifts one vault's loot into a temp bag, moves the other across, then empties the bag -
// the swaps are worked out from the two printed lines. Loot that wasn't there before
// (lost or doubled values) or a wrong order sets off the alarm.
#include "levels.h"
#include "stage.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { SW_VAULT = 1, SW_BEFORE, SW_SWAP, SW_AFTER };
enum { SWF_BEFORE = 1, SWF_ORDER, SWF_LOOT, SWF_GARBLED, SWF_EXTRA };

// ---------------------------------------------------------------- the parser

// Adjacent swaps that turn `from` into `to` (both 3 long); false if `to` isn't a reordering
static bool PlanSwaps(Parse *p, const int *from, const int *to)
{
    int cur[3] = { from[0], from[1], from[2] }, swaps[8][2], n = 0;
    for (int i = 0; i < 3; i++) {
        int j = i;
        while (j < 3 && cur[j] != to[i]) j++;
        if (j == 3) return false;
        for (; j > i && n < 8; j--) {
            int t = cur[j]; cur[j] = cur[j - 1]; cur[j - 1] = t;
            swaps[n][0] = j - 1; swaps[n][1] = j; n++;
        }
    }
    for (int k = 0; k < n; k++) EvAct(p, SW_SWAP, swaps[k][0], swaps[k][1], 0);
    return true;
}

static void ParseSwap(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    int in[3] = { 0, 0, 0 };
    InputInts(run->input, in, 3);
    for (int i = 0; i < 3; i++) EvSpawn(&p, SW_VAULT, i, 0, in[i]);
    if (ParsePreview(&p)) return;

    char note[120];
    int before[3] = { in[0], in[1], in[2] };
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al, want[4], wn = 0, got[4], gn = 0;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        bool isBefore = ScanAfterInts(E, el, "Before:", want, 4, &wn);
        const char *prefix = isBefore ? "Before:" : "After:";
        if (!isBefore) ScanAfterInts(E, el, "After:", want, 4, &wn);
        if (wn != 3) { ParseTake(&p, e); continue; }
        if (ParseTake(&p, e)) {
            if (isBefore) { for (int i = 0; i < 3; i++) { before[i] = want[i]; EvAct(&p, SW_BEFORE, i, 0, want[i]); } }
            else { PlanSwaps(&p, before, want); for (int i = 0; i < 3; i++) EvAct(&p, SW_AFTER, i, 0, want[i]); }
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, isBefore ? SWF_BEFORE : SWF_ORDER, e, 0, 0, 0, NULL); break; }
        if (!ScanAfterInts(A, al, prefix, got, 4, &gn) || gn != 3) {
            ParseFail(&p, FAIL_GARBLED, SWF_GARBLED, e, 0, 0, 0, isBefore ? "Expected Before: and three numbers" : "Expected After: and three numbers");
            break;
        }
        ParseShow(&p);
        if (isBefore) {
            int k = 0;
            while (k < 3 && got[k] == want[k]) k++;
            ParseFail(&p, FAIL_WRONG, SWF_BEFORE, e, k < 3 ? k : 0, 0, got[k < 3 ? k : 0], "Before: should show the three numbers as they were read");
            break;
        }
        if (PlanSwaps(&p, before, got)) {
            for (int i = 0; i < 3; i++) EvAct(&p, SW_AFTER, i, 0, got[i]);
            if (got[0] == before[0] && got[1] == before[1] && got[2] == before[2])
                snprintf(note, sizeof(note), "Nothing moved: swap must change the values the pointers point at (*a, *b)");
            else
                snprintf(note, sizeof(note), "Not in order yet: after sortThree, *a <= *b <= *c (%d %d %d)", want[0], want[1], want[2]);
            ParseFail(&p, FAIL_WRONG, SWF_ORDER, e, 0, 0, 0, note);
        } else {
            int k = 0;              // the first loot that wasn't in the vaults before
            while (k < 3 && (got[k] == before[0] || got[k] == before[1] || got[k] == before[2])) k++;
            snprintf(note, sizeof(note), "Loot appeared or vanished: swap needs a temp to keep the first value safe");
            GameEvent *f = ParseFail(&p, FAIL_WRONG, SWF_LOOT, e, k < 3 ? k : 0, 0, 0, note);
            if (f) snprintf(f->text, EV_TEXT, "%d %d %d", got[0], got[1], got[2]);
        }
        break;
    }
    ParseEnd(&p, SWF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_SWAP_A, DO_SWAP_B, DO_SWAP_C, DO_ALARM, DO_CHEER, DO_FAKE };

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time;
    int   loot[3];                 // what each vault holds
    bool  shown[3];                // printed
    Vector3 lootPos[3];            // where each vault's loot is drawn: it glides home after a move
    bool  empty[3];                // its loot has been taken out (mid-swap)
    int   bag;                     // the temp bag's loot
    bool  bagFull;
    Vector3 bagPos;                // where the bag's loot is drawn: it glides into the bag
    int   swapI, swapJ;            // the vaults the swap's pointers are on (-1 none)
    float swapGlow;
    bool  alarm, fake[3], won;
    char  want[48];
} W;

static float VaultX(int i) { return -2.4f + i * 2.4f; }
static Vector3 LootHome(int i) { return V3(VaultX(i), 0.02f, 0.95f); }
static Vector3 BagHome(void) { return V3(1.2f, 0.02f, 2.25f); }

static void SwapReset(const EventList *ev)
{
    memset(&W, 0, sizeof(W));
    W.ev = ev;
    W.swapI = W.swapJ = -1;
    for (int i = 0; i < 3; i++) W.lootPos[i] = LootHome(i);
    W.bagPos = BagHome();
    ShotOrbit(V3(0, 1.1f, 0.6f), 9.4f, 24, 0, 38, 0);
}

static float SwapEvent(const GameEvent *e)
{
    int i = e->a, j = e->b;
    switch (e->type) {
        case EV_SPAWN: if (i >= 0 && i < 3) W.loot[i] = (int)e->value; return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == SW_BEFORE && i >= 0 && i < 3) {
                W.loot[i] = (int)e->value;
                W.shown[i] = true;
                FxSparkle(V3(VaultX(i), 0.6f, 0.95f), 8, (Color){ 255, 220, 120, 255 });
                return 0.35f;
            }
            if (e->code == SW_SWAP && i >= 0 && i < 3 && j >= 0 && j < 3) {
                W.swapI = i; W.swapJ = j;
                W.swapGlow = 1;
                CueAt(&W.cues, W.time + 0.3f, DO_SWAP_A, i, j, V3(0, 0, 0), 0);
                CueAt(&W.cues, W.time + 1.0f, DO_SWAP_B, i, j, V3(0, 0, 0), 0);
                CueAt(&W.cues, W.time + 1.7f, DO_SWAP_C, i, j, V3(0, 0, 0), 0);
                return 2.4f;
            }
            if (e->code == SW_AFTER && i >= 0 && i < 3) {
                W.swapI = W.swapJ = -1;
                W.loot[i] = (int)e->value;
                return 0.2f;
            }
            return 0;
        case EV_FAIL:
            if (W.ev) {
                const char *x = W.ev->expected;
                StrCopy(W.want, sizeof(W.want), x);
            }
            if (e->code == SWF_LOOT) {
                int g[3];
                if (sscanf(e->text, "%d %d %d", &g[0], &g[1], &g[2]) == 3)
                    for (int k = 0; k < 3; k++) {
                        bool known = g[k] == W.loot[0] || g[k] == W.loot[1] || g[k] == W.loot[2];
                        CueAt(&W.cues, W.time + 0.3f + k * 0.3f, DO_FAKE, k, g[k], V3(known ? 0 : 1, 0, 0), 0);
                    }
            }
            CueAt(&W.cues, W.time + 1.3f, DO_ALARM, 0, 0, V3(0, 0, 0), 0);
            return 3.0f;
        case EV_CRASH:
            CueAt(&W.cues, W.time + 0.2f, DO_ALARM, 0, 0, V3(0, 0, 0), 0);
            return 2.4f;
        case EV_WIN:
            W.won = true;
            CueAt(&W.cues, W.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void SwapUpdate(float dt)
{
    W.time += dt;
    PopsUpdate(&W.pops, dt);
    W.swapGlow = Smooth(W.swapGlow, W.swapI >= 0 ? 1.0f : 0.0f, 5, dt);
    Cue c;
    while (CueDue(&W.cues, W.time, &c)) {
        switch (c.what) {
            case DO_SWAP_A:         // temp = *a: a's loot goes into the bag
                W.bag = W.loot[c.a];
                W.bagFull = true;
                W.bagPos = LootHome(c.a);
                W.empty[c.a] = true;
                PopAdd(&W.pops, "temp = *a", V3(0, 1.0f, 2.0f), LBL_GOLD);
                break;
            case DO_SWAP_B: {       // *a = *b: b's loot moves over to a
                W.loot[c.a] = W.loot[c.b];
                W.lootPos[c.a] = LootHome(c.b);
                W.empty[c.a] = false;
                W.empty[c.b] = true;
                PopAdd(&W.pops, "*a = *b", V3(VaultX(c.a), 1.8f, 1.0f), LBL_GOLD);
                break;
            }
            case DO_SWAP_C:         // *b = temp: the bag empties into b
                W.loot[c.b] = W.bag;
                W.lootPos[c.b] = BagHome();
                W.empty[c.b] = false;
                W.bagFull = false;
                PopAdd(&W.pops, "*b = temp", V3(VaultX(c.b), 1.8f, 1.0f), LBL_GOLD);
                FxSparkle(V3(VaultX(c.b), 0.6f, 0.95f), 10, (Color){ 255, 220, 120, 255 });
                break;
            case DO_FAKE:
                W.loot[c.a] = c.b;
                W.fake[c.a] = c.v.x > 0;
                if (W.fake[c.a]) FxSparks(V3(VaultX(c.a), 0.7f, 1.0f), 24, (Color){ 255, 90, 90, 255 });
                break;
            case DO_ALARM:
                W.alarm = true;
                for (int k = 0; k < 3; k++) FxSparks(V3(VaultX(k), 1.6f, 0.4f), 12, (Color){ 255, 70, 70, 255 });
                PlaybackShake(0.25f);
                break;
            case DO_CHEER:
                FxConfetti(V3(0, 1.6f, 0.8f), PlaybackFinale() ? 150 : 70, 2.0f);
                for (int k = 0; k < 3; k++) FxSparkle(V3(VaultX(k), 0.6f, 1.0f), 12, (Color){ 255, 220, 120, 255 });
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
    // loot glides to where it now belongs
    for (int i = 0; i < 3; i++) W.lootPos[i] = LerpV3(W.lootPos[i], LootHome(i), 1 - expf(-5 * dt));
    W.bagPos = LerpV3(W.bagPos, BagHome(), 1 - expf(-5 * dt));
}

static void SwapDraw(void)
{
    KitStage(V3(0, 0, 0.6f), 4.6f, 2.2f);
    KitRoad(V3(-4.4f, 0, 1.55f), 8.8f);
    const char *names[3] = { "x", "y", "z" }, *ptrs[3] = { "a", "b", "c" };
    for (int i = 0; i < 3; i++) {
        Color light = W.alarm ? (Color){ 235, 60, 60, 255 } : W.won ? (Color){ 80, 220, 130, 255 } : (Color){ 86, 156, 255, 255 };
        KitVault(V3(VaultX(i), 0, 0), W.alarm ? 0 : 0.85f, light, W.alarm ? 0.6f + 0.4f * Pulse(W.time, 10) : 0.5f);
        Vector3 lp = W.lootPos[i];
        int bars = W.loot[i] > 0 ? (W.loot[i] > 6 ? 6 : W.loot[i]) : 0;
        if (W.empty[i]) Part(MESH_CYLINDER, V3(LootHome(i).x, 0.02f, LootHome(i).z), V3(0.5f, 0.03f, 0.5f), (Color){ 60, 64, 84, 255 }, 0);
        else if (W.fake[i]) Part(MESH_CUBE, V3(lp.x, 0.12f, lp.z), V3(0.5f, 0.2f, 0.3f), (Color){ 235, 80, 80, 255 }, 0.6f);
        else if (bars > 0) KitGold(lp, bars, 0.35f);
        else Part(MESH_CYLINDER, V3(lp.x, 0.02f, lp.z), V3(0.5f, 0.03f, 0.5f), (Color){ 90, 96, 120, 255 }, 0);
    }
    // the temp bag, and the loot going into it
    KitBag(BagHome(), W.bagFull ? 0.55f : 0.4f, (Color){ 150, 110, 70, 255 });
    if (W.bagFull && Vector3Distance(W.bagPos, BagHome()) > 0.15f) KitGold(W.bagPos, W.bag > 0 ? (W.bag > 6 ? 6 : W.bag) : 1, 0.35f);
    // the pointers: badges over the street with arrows down to the vaults they point at
    for (int i = 0; i < 3; i++) {
        bool hot = (i == W.swapI || i == W.swapJ) && W.swapGlow > 0.1f;
        Vector3 badge = V3(VaultX(i) + 0.9f, 2.9f, 1.0f);
        Color col = hot ? (Color){ 255, 210, 90, 255 } : (Color){ 160, 120, 255, 255 };
        PartRot(MESH_CYLINDER, badge, V3(0.42f, 0.08f, 0.42f), V3(90, 0, 0), col, hot ? 0.5f : 0.2f);
        KitArc(V3(badge.x, badge.y - 0.15f, badge.z), V3(VaultX(i), 1.45f, 0.1f), 0.25f, 1, col, hot ? 0.9f : 0.4f);
    }

    for (int i = 0; i < 3; i++) {
        Vector3 badge = V3(VaultX(i) + 0.9f, 2.9f, 1.0f);
        Label3D(V3(badge.x + 0.45f, badge.y, badge.z + 0.1f), TextFormat("int *%s", ptrs[i]), 14, WHITE, LBL_DIM, LABEL_CODE);
        Label3D(V3(VaultX(i), 1.75f, 0.0f), names[i], 16, LBL_SOFT, LBL_DIM, LABEL_CODE);
        Label3D(V3(VaultX(i), 0.02f, 1.95f), TextFormat("&%s = 0x100%d", names[i], i * 4), 12, (Color){ 170, 150, 255, 255 }, LBL_DIM, LABEL_CODE);
        Vector3 lp = W.lootPos[i];
        Color fg = W.fake[i] ? WHITE : W.shown[i] ? LBL_GOLD : LBL_SOFT, bg = W.fake[i] ? (Color){ 200, 50, 55, 235 } : LBL_DIM;
        if (!W.empty[i]) Label3D(V3(lp.x, lp.y + 0.55f, lp.z), TextFormat("%d", W.loot[i]), 17, fg, bg, LABEL_CODE);
    }
    if (W.swapI >= 0) Label3D(V3((VaultX(W.swapI) + VaultX(W.swapJ)) / 2 + 0.9f, 3.45f, 1.0f), TextFormat("swap(%s, %s)", ptrs[W.swapI], ptrs[W.swapJ]), 15,
                              LBL_GOLD, LBL_DIM, LABEL_CODE);
    Label3D(V3(BagHome().x, 0.75f, BagHome().z), W.bagFull ? TextFormat("temp = %d", W.bag) : "temp", 13, (Color){ 230, 200, 160, 255 }, LBL_DIM, LABEL_CODE);
    if (W.alarm && W.want[0]) Label3D(V3(0, 2.35f, 0.2f), TextFormat("should be: %s", W.want), 14, LBL_GOOD, LBL_DIM, LABEL_CODE);
    PopsDraw(&W.pops);
}

const LevelDef LEVEL_SWAP = {
    "c-pointers-2", "Memory Heist", "Your pointers lead the thieves: swap() moves the loot between the vaults.",
    { 170, 130, 255, 255 },
    ParseSwap, SwapReset, SwapEvent, SwapUpdate, SwapDraw, NULL,
};
