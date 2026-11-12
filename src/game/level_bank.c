// Bank Account -> "Bank Vault"
//
// The account is a vault object. Its balance is private: it sits behind glass marked "?"
// that nothing outside the class can read. The input's commands play in order through
// the public methods: deposit() sends coins in, withdraw() takes them out - or, when
// the program prints "Declined", the red alarm goes off and nothing moves. At the end
// getBalance() reveals the balance the program printed. Withdrawals let through that
// should have been refused (or refused that should have gone through) are pinned to
// their command.
#include "levels.h"
#include "stage.h"
#include "rlgl.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { BK_START = 1, BK_CMD, BK_DEPOSIT, BK_WITHDRAW, BK_DECLINED, BK_BALANCE };
enum { BKF_ALLOWED = 1, BKF_REFUSED, BKF_BALANCE, BKF_GARBLED, BKF_EXTRA };

#define CMAX 12

// ---------------------------------------------------------------- the parser

static void ParseBank(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    const char *s = run->input ? run->input : "";
    int start = 0, used = 0, n = 0, kind[CMAX], amount[CMAX];
    if (sscanf(s, "%d%n", &start, &used) == 1) {
        s += used;
        char c;
        int a;
        while (n < CMAX && sscanf(s, " %c %d%n", &c, &a, &used) == 2) { kind[n] = (c == 'w'); amount[n] = a; n++; s += used; }
    }
    EvSpawn(&p, BK_START, 0, 0, start);
    for (int i = 0; i < n; i++) EvSpawn(&p, BK_CMD, i, kind[i], amount[i]);
    if (ParsePreview(&p)) return;

    char note[140];
    long bal = start;
    int e = 0;
    for (int i = 0; i < n && !p.stopped; i++) {
        if (!kind[i]) { bal += amount[i]; EvAct(&p, BK_DEPOSIT, i, 0, amount[i]); continue; }
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        bool declineHere = amount[i] > bal;
        if (declineHere) {                       // the program must print Declined now
            if (E && LineIs(E, el, "Declined") && ParseTake(&p, e)) { e++; EvAct(&p, BK_DECLINED, i, 0, amount[i]); continue; }
            snprintf(note, sizeof(note), "Withdrawing %d from %ld must be refused: print Declined and change nothing", amount[i], bal);
            ParseFail(&p, A ? FAIL_WRONG : FAIL_MISSING, BKF_ALLOWED, e, i, 0, amount[i], note);
            break;
        }
        // the withdrawal should go through; a Declined here (when none is due) was refused wrongly
        bool dueDecline = E && LineIs(E, el, "Declined");
        if (A && LineIs(A, al, "Declined") && !dueDecline) {
            snprintf(note, sizeof(note), "There was enough money (%ld) to withdraw %d", bal, amount[i]);
            ParseFail(&p, FAIL_WRONG, BKF_REFUSED, e, i, 0, amount[i], note);
            break;
        }
        bal -= amount[i];
        EvAct(&p, BK_WITHDRAW, i, 0, amount[i]);
    }
    for (; e < p.exp.count && !p.stopped; e++) {
        int el, al, want, got;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ScanAfter(E, el, "Balance:", &want)) {
            if (ParseTake(&p, e)) { EvAct(&p, BK_BALANCE, 0, 0, want); continue; }
            if (!A) { ParseFail(&p, FAIL_MISSING, BKF_BALANCE, e, 0, 0, 0, NULL); break; }
            if (ScanAfter(A, al, "Balance:", &got)) {
                snprintf(note, sizeof(note), "The balance should be %d", want);
                GameEvent *f = ParseFail(&p, FAIL_WRONG, BKF_BALANCE, e, 0, 0, got, note);
                if (f) snprintf(f->text, EV_TEXT, "%d", want);
            } else if (LineIs(A, al, "Declined")) {
                ParseFail(&p, FAIL_WRONG, BKF_REFUSED, e, -1, 0, 0, "A withdrawal was refused that should have gone through");
            } else {
                ParseFail(&p, FAIL_GARBLED, BKF_GARBLED, e, 0, 0, 0, "Expected Balance: and a number");
            }
            break;
        }
        ParseTake(&p, e);
    }
    ParseEnd(&p, BKF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_COINS_IN, DO_COINS_OUT, DO_ALARM, DO_REVEAL, DO_CHEER, DO_BAD };

typedef struct { Vector3 from, to; float t; bool live; } Coin;

static struct {
    const EventList *ev;
    Cues  cues;
    Pops  pops;
    float time;
    int   start, n, kind[CMAX], amount[CMAX];
    int   cur;                   // the command being run (-1 none)
    int   result[CMAX];          // 0 not yet, 1 done, 2 declined, 3 wrong
    Coin  coins[24];
    float alarm;
    bool  revealed, revealBad, won;
    int   balance;
    char  want[EV_TEXT];
    char  note[64];
} B;

static Vector3 VaultPos(void) { return V3(0.6f, 0, -0.8f); }
static Vector3 Teller(void) { return V3(-1.9f, 0.9f, 0.6f); }

static void BankReset(const EventList *ev)
{
    memset(&B, 0, sizeof(B));
    B.ev = ev;
    B.cur = -1;
    ShotOrbit(V3(-0.3f, 1.2f, 0), 9.0f, 22, 0, 38, 0);
}

static void ShootCoins(Vector3 from, Vector3 to, int count)
{
    for (int k = 0, made = 0; k < 24 && made < count; k++)
        if (!B.coins[k].live) {
            B.coins[k] = (Coin){ V3(from.x + (made % 3) * 0.12f, from.y, from.z), to, -made * 0.08f, true };
            made++;
        }
}

static float BankEvent(const GameEvent *e)
{
    int i = e->a;
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == BK_START) B.start = (int)e->value;
            if (e->code == BK_CMD && B.n < CMAX) { B.kind[B.n] = e->b; B.amount[B.n] = (int)e->value; B.n++; }
            return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if ((e->code == BK_DEPOSIT || e->code == BK_WITHDRAW || e->code == BK_DECLINED) && i >= 0 && i < CMAX) {
                B.cur = i;
                if (e->code == BK_DEPOSIT) {
                    B.result[i] = 1;
                    ShootCoins(Teller(), V3(VaultPos().x, 1.0f, VaultPos().z + 0.4f), 3 + (int)fminf(5, e->value / 40));
                    PopAdd(&B.pops, TextFormat("deposit(%d)", B.amount[i]), V3(-1.9f, 1.9f, 0.6f), LBL_GOOD);
                } else if (e->code == BK_WITHDRAW) {
                    B.result[i] = 1;
                    ShootCoins(V3(VaultPos().x, 1.0f, VaultPos().z + 0.4f), Teller(), 3 + (int)fminf(5, e->value / 40));
                    PopAdd(&B.pops, TextFormat("withdraw(%d)", B.amount[i]), V3(-1.9f, 1.9f, 0.6f), LBL_GOLD);
                } else {
                    B.result[i] = 2;
                    CueAt(&B.cues, B.time + 0.3f, DO_ALARM, 0, 0, V3(0, 0, 0), 0);
                    PopAdd(&B.pops, TextFormat("withdraw(%d)", B.amount[i]), V3(-1.9f, 1.9f, 0.6f), LBL_BAD);
                }
                return 1.3f;
            }
            if (e->code == BK_BALANCE) {
                B.revealed = true;
                B.balance = (int)e->value;
                B.cur = -1;
                FxSparkle(V3(VaultPos().x, 1.4f, VaultPos().z + 0.6f), 16, (Color){ 140, 255, 180, 255 });
                return 1.4f;
            }
            return 0;
        case EV_FAIL:
            if (e->code == BKF_BALANCE) {
                B.revealed = B.revealBad = true;
                B.balance = (int)e->value;
                StrCopy(B.want, sizeof(B.want), e->text);
                CueAt(&B.cues, B.time + 0.5f, DO_BAD, 0, 0, V3(VaultPos().x, 1.4f, VaultPos().z + 0.6f), 0);
            } else if ((e->code == BKF_ALLOWED || e->code == BKF_REFUSED) && i >= 0 && i < CMAX) {
                B.cur = i;
                B.result[i] = 3;
                if (e->code == BKF_ALLOWED) {
                    ShootCoins(V3(VaultPos().x, 1.0f, VaultPos().z + 0.4f), Teller(), 4);
                    StrCopy(B.note, sizeof(B.note), "not enough money - this should be Declined");
                } else {
                    CueAt(&B.cues, B.time + 0.3f, DO_ALARM, 0, 0, V3(0, 0, 0), 0);
                    StrCopy(B.note, sizeof(B.note), "there was enough - this should go through");
                }
                CueAt(&B.cues, B.time + 0.7f, DO_BAD, 0, 0, V3(-1.9f, 1.2f, 0.6f), 0);
            } else {
                CueAt(&B.cues, B.time + 0.3f, DO_BAD, 0, 0, V3(VaultPos().x, 1.4f, 0), 0);
            }
            return 2.8f;
        case EV_CRASH:
            CueAt(&B.cues, B.time + 0.1f, DO_BAD, 0, 0, V3(VaultPos().x, 1.4f, 0), 0);
            return 2.4f;
        case EV_WIN:
            B.won = true;
            CueAt(&B.cues, B.time + 0.2f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void BankUpdate(float dt)
{
    B.time += dt;
    PopsUpdate(&B.pops, dt);
    B.alarm = fmaxf(0, B.alarm - dt * 0.6f);
    for (int k = 0; k < 24; k++) {
        if (!B.coins[k].live) continue;
        B.coins[k].t += dt / 0.6f;
        if (B.coins[k].t >= 1) B.coins[k].live = false;
    }
    Cue c;
    while (CueDue(&B.cues, B.time, &c)) {
        if (c.what == DO_ALARM) { B.alarm = 1; FxSparks(V3(VaultPos().x, 2.1f, VaultPos().z), 20, (Color){ 255, 70, 70, 255 }); PlaybackShake(0.15f); }
        else if (c.what == DO_BAD) { FxSparks(c.v, 30, (Color){ 255, 100, 90, 255 }); PlaybackShake(0.2f); }
        else if (c.what == DO_CHEER) { FxConfetti(V3(VaultPos().x, 2.2f, 0), PlaybackFinale() ? 150 : 70, 1.6f); if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f); }
    }
}

static void BankDraw(void)
{
    KitStage(V3(-0.6f, 0, 0), 4.6f, 2.0f);
    Vector3 v = VaultPos();
    Color light = B.alarm > 0.05f ? (Color){ 255, 60, 60, 255 } : B.won ? (Color){ 80, 220, 130, 255 } : (Color){ 86, 156, 255, 255 };
    // a big vault: scale the kit's one up
    rlPushMatrix();
    rlTranslatef(v.x, 0, v.z);
    rlScalef(1.7f, 1.7f, 1.7f);
    KitVault(V3(0, 0, 0), 0, light, B.alarm > 0.05f ? 0.6f + 0.4f * Pulse(B.time, 14) : 0.5f);
    rlPopMatrix();
    // the private balance: a window marked "?" until getBalance() is called
    PartGlass(MESH_CUBE, V3(v.x + 1.45f, 1.5f, v.z + 0.2f), V3(0.9f, 0.6f, 0.05f), (Color){ 150, 200, 255, 70 });
    Part(MESH_CUBE, V3(v.x + 1.45f, 1.5f, v.z + 0.15f), V3(1.0f, 0.7f, 0.04f), (Color){ 30, 34, 48, 255 }, 0);
    // the teller's desk: the public methods
    Part(MESH_CUBE, V3(-1.9f, 0.4f, 0.6f), V3(1.6f, 0.8f, 0.8f), (Color){ 92, 80, 64, 255 }, 0);
    // the alarm light
    Part(MESH_SPHERE, V3(v.x, 2.25f, v.z), V3(0.3f, 0.3f, 0.3f), (Color){ 255, 70, 70, 255 }, B.alarm > 0.05f ? 0.6f + 0.4f * Pulse(B.time, 14) : 0.05f);
    for (int k = 0; k < 24; k++) {
        if (!B.coins[k].live || B.coins[k].t < 0) continue;
        float t = EaseInOutQuad(Clamp01(B.coins[k].t));
        Vector3 p = LerpV3(B.coins[k].from, B.coins[k].to, t);
        p.y += sinf(t * PI) * 1.0f;
        PartRot(MESH_CYLINDER, p, V3(0.22f, 0.04f, 0.22f), V3(90, B.coins[k].t * 400, 0), (Color){ 255, 205, 70, 255 }, 0.6f);
    }

    // the commands, the balance, and the methods
    Label3D(V3(-4.0f, 2.4f, 0.4f), TextFormat("start: %d", B.start), 13, LBL_SOFT, LBL_DIM, LABEL_CODE);
    for (int i = 0; i < B.n && i < 8; i++) {
        Color fg = B.result[i] == 3 ? WHITE : B.result[i] == 2 ? LBL_BAD : B.result[i] == 1 ? LBL_GOOD : LBL_SOFT;
        Color bg = B.result[i] == 3 ? (Color){ 200, 50, 55, 235 } : i == B.cur ? LBL_HI : LBL_DIM;
        const char *t = TextFormat("%c %d%s", B.kind[i] ? 'w' : 'd', B.amount[i], B.result[i] == 2 ? "  Declined" : "");
        Label3D(V3(-4.0f, 2.0f - i * 0.32f, 0.4f), t, 13, fg, bg, LABEL_CODE);
    }
    Label3D(V3(v.x + 1.45f, 1.5f, v.z + 0.3f), B.revealed ? TextFormat("%d", B.balance) : "?", 20, B.revealBad ? WHITE : B.revealed ? LBL_GOOD : LBL_SOFT,
            B.revealBad ? (Color){ 200, 50, 55, 235 } : (Color){ 0, 0, 0, 0 }, LABEL_CODE | LABEL_BIG);
    Label3D(V3(v.x + 1.45f, 2.0f, v.z + 0.3f), B.revealed ? "getBalance()" : "private: balance", 12, (Color){ 200, 180, 255, 255 }, LBL_DIM, LABEL_CODE);
    if (B.revealBad && B.want[0]) Label3D(V3(v.x + 1.45f, 0.95f, v.z + 0.3f), TextFormat("should be %s", B.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (B.alarm > 0.05f && !B.note[0]) Label3D(V3(v.x, 2.7f, v.z), "Declined", 17, WHITE, (Color){ 200, 50, 55, 235 }, LABEL_CODE);
    if (B.note[0]) Label3D(V3(-1.9f, 2.4f, 0.6f), B.note, 13, WHITE, (Color){ 200, 50, 55, 235 }, LABEL_CODE);
    Label3D(V3(-1.9f, 0.95f, 1.05f), "deposit()  withdraw()", 12, LBL_SOFT, LBL_DIM, LABEL_CODE);
    PopsDraw(&B.pops);
}

const LevelDef LEVEL_BANK = {
    "cpp-classes", "Bank Vault", "The balance is private: only the class's methods can move the money - or see it.",
    { 86, 156, 255, 255 },
    ParseBank, BankReset, BankEvent, BankUpdate, BankDraw, NULL,
};
