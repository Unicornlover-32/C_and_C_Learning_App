// Output Rush: the quiz questions fall like blocks, and you pick the answer before each
// one lands. Wrong answers and blocks that land pile up at the bottom (and the question
// comes back later). A level's Rush uses its own quiz; the Warm-up mixes every level's
// questions for two minutes.
#include "app.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define RUSH_QUEUE 96
#define PILE_MAX   12
#define WARMUP_SECONDS 120

typedef struct { int tut, q; } QRef;

static struct {
    bool   warmup;
    int    tut;
    QRef   queue[RUSH_QUEUE];
    int    head, count;
    int    total, cleared;

    bool   active;
    QRef   cur;
    float  y, fallTime, blockH;
    bool   wrong[MAX_OPTIONS];
    bool   dropping;              // answered wrong: on its way to the pile
    float  shake;

    QRef   pile[PILE_MAX];
    int    pileCount;

    int    score, combo, mistakes, correct;
    double startTime;
    bool   over, won;
    double overAt;
    int    xp;

    char   banner[400];
    bool   bannerGood;
    double bannerAt;
    double burstAt;
    float  burstY;
} rs;

static const QuizQuestion *Q(QRef r) { return &GetTutorial(r.tut)->quiz[r.q]; }

static void Shuffle(QRef *items, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = GetRandomValue(0, i);
        QRef t = items[i];
        items[i] = items[j];
        items[j] = t;
    }
}

static void Push(QRef r)
{
    if (rs.count < RUSH_QUEUE) rs.queue[(rs.head + rs.count++) % RUSH_QUEUE] = r;
}

static bool Pop(QRef *r)
{
    if (rs.count == 0) return false;
    *r = rs.queue[rs.head];
    rs.head = (rs.head + 1) % RUSH_QUEUE;
    rs.count--;
    return true;
}

static void Begin(void)
{
    rs.cleared = rs.pileCount = 0;
    rs.score = rs.combo = rs.mistakes = rs.correct = 0;
    rs.active = false;
    rs.over = rs.won = false;
    rs.banner[0] = '\0';
    rs.startTime = GetTime();
    rs.fallTime = rs.warmup ? 14 : 20;
    rs.xp = 0;
    app.screen = SCREEN_RUSH;
}

void OpenRush(int tut)
{
    const Tutorial *t = GetTutorial(tut);
    memset(&rs, 0, sizeof(rs));
    rs.tut = tut;
    QRef items[MAX_QUIZ];
    for (int i = 0; i < t->quizCount; i++) items[i] = (QRef){ tut, i };
    Shuffle(items, t->quizCount);
    for (int i = 0; i < t->quizCount; i++) Push(items[i]);
    rs.total = t->quizCount;
    Begin();
}

static void FillWarmup(void)
{
    static QRef all[RUSH_QUEUE];
    int n = 0;
    for (int t = 0; t < TutorialCount(); t++)
        for (int q = 0; q < GetTutorial(t)->quizCount && n < RUSH_QUEUE; q++) all[n++] = (QRef){ t, q };
    Shuffle(all, n);
    for (int i = 0; i < n && rs.count < RUSH_QUEUE; i++) Push(all[i]);
}

void OpenWarmup(void)
{
    memset(&rs, 0, sizeof(rs));
    rs.warmup = true;
    rs.tut = -1;
    FillWarmup();
    Begin();
}

static void Banner(const char *text, bool good)
{
    snprintf(rs.banner, sizeof(rs.banner), "%s", text);
    rs.bannerGood = good;
    rs.bannerAt = GetTime();
}

static void Finish(bool won)
{
    rs.over = true;
    rs.won = won;
    rs.overAt = GetTime();
    rs.active = false;
    if (rs.warmup) {
        rs.xp = AwardWarmup(rs.score, rs.correct);
    } else if (won) {
        int stars = rs.mistakes == 0 ? 3 : rs.mistakes == 1 ? 2 : 1;
        rs.xp = AwardRush(rs.tut, stars, rs.mistakes);
        ParticlesBurst(SCREEN_W / 2.0f, SCREEN_H * 0.4f, 120, 460);
    }
}

static void NextBlock(void)
{
    if (rs.warmup && rs.count == 0) FillWarmup();
    if (!Pop(&rs.cur)) {
        if (!rs.warmup) Finish(true);
        return;
    }
    rs.active = true;
    rs.dropping = false;
    rs.shake = 0;
    memset(rs.wrong, 0, sizeof(rs.wrong));
    rs.y = -1;   // placed when drawn, once its height is known
}

// The block failed (wrong answer or it landed): onto the pile, and the question comes back later
static void ToPile(const char *why)
{
    rs.mistakes++;
    rs.combo = 0;
    if (rs.pileCount < PILE_MAX) rs.pile[rs.pileCount++] = rs.cur;
    if (!rs.warmup) Push(rs.cur);
    Banner(why, false);
    rs.active = false;
}

static void Answer(int option)
{
    const QuizQuestion *q = Q(rs.cur);
    if (option == q->correct) {
        rs.combo++;
        rs.correct++;
        int bonus = (int)fmaxf(0, 50 * (1 - rs.y / 600.0f));
        rs.score += 100 * (1 + rs.combo / 3) + bonus;
        rs.burstAt = GetTime();
        rs.burstY = rs.y;
        ParticlesBurst(SCREEN_W / 2.0f, rs.y + 60, 50, 320);
        Banner(TextFormat("Correct! %s", q->explanation), true);
        rs.active = false;
        if (!rs.warmup) rs.cleared++;
        rs.fallTime = fmaxf(rs.warmup ? 5 : 9, rs.fallTime * 0.88f);
    } else {
        rs.wrong[option] = true;
        rs.dropping = true;
        rs.shake = 0.4f;
    }
}

// ---------------------------------------------------------------- drawing

static float BlockHeight(const QuizQuestion *q, float w)
{
    float h = 24 + UiWrappedHeight(q->question, w - 40, 22);
    if (q->code) {
        int lines = 1;
        for (const char *c = q->code; *c; c++) if (*c == '\n') lines++;
        h += 12 + lines * 16 * 1.3f + 32;
    }
    return h + 20;
}

static void DrawBlock(Rectangle r, const QuizQuestion *q, Color border)
{
    DrawRectangleRounded((Rectangle){ r.x + 4, r.y + 6, r.width, r.height }, 0.08f, 8, ColorAlpha(BLACK, 0.35f));
    DrawRectangleRounded(r, 0.08f, 8, (Color){ 40, 44, 62, 255 });
    DrawRectangleRoundedLinesEx(r, 0.08f, 8, 3, border);
    float y = r.y + 18;
    y += UiWrapped(q->question, r.x + 20, y, r.width - 40, 22, COL_TEXT);
    if (q->code) UiCodeBlock(q->code, r.x + 20, y + 8, r.width - 40, 16);
}

void ScreenRush(void)
{
    bool modal = rs.over;
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (rs.warmup) OpenMenu(); else OpenLevel(rs.tut);
        return;
    }

    const char *title = rs.warmup ? "Warm-up: Output Rush" : TextFormat("Output Rush: %s", GetTutorial(rs.tut)->title);
    DrawTopBar(title);
    DrawPlayerChip(SCREEN_W - 24, 14);
    UiSetBlocked(modal);
    if (BackButton(rs.warmup ? "< Menu" : "< Level")) {
        UiSetBlocked(false);
        if (rs.warmup) OpenMenu(); else OpenLevel(rs.tut);
        return;
    }

    float sideW = SCREEN_W >= 1500 ? 300 : 220;
    float playW = fminf(900, SCREEN_W - 2 * sideW - 80);
    float playX = (SCREEN_W - playW) / 2;
    float answersH = 140;
    Rectangle play = { playX, 110, playW, SCREEN_H - 110 - answersH - 30 };
    float time = (float)GetTime();
    float dt = GetFrameTime();

    // the shaft the blocks fall down
    DrawRectangleRounded(play, 0.03f, 8, (Color){ 26, 28, 38, 255 });
    BeginScissorMode((int)play.x, (int)play.y, (int)play.width, (int)play.height);
    for (float gy = play.y + fmodf(time * 30, 40) - 40; gy < play.y + play.height; gy += 40)
        DrawLine((int)play.x, (int)gy, (int)(play.x + play.width), (int)gy, (Color){ 255, 255, 255, 8 });
    EndScissorMode();

    // the pile at the bottom
    float brickH = 34;
    float floorY = play.y + play.height - rs.pileCount * brickH;
    for (int i = 0; i < rs.pileCount; i++) {
        Rectangle b = { play.x + 6, play.y + play.height - (i + 1) * brickH, play.width - 12, brickH - 4 };
        DrawRectangleRounded(b, 0.3f, 6, (Color){ 110, 45, 55, 255 });
        char text[90];
        snprintf(text, sizeof(text), "x  %.70s", Q(rs.pile[i])->question);
        BeginScissorMode((int)b.x, (int)b.y, (int)b.width, (int)b.height);
        UiText(text, b.x + 12, b.y + 6, 16, (Color){ 255, 200, 200, 255 });
        EndScissorMode();
    }

    // timers
    float elapsed = (float)(GetTime() - rs.startTime);
    if (!rs.over && rs.warmup && elapsed >= WARMUP_SECONDS) Finish(true);

    // the falling block
    if (!rs.over && !rs.active && GetTime() - rs.bannerAt > (rs.bannerGood ? 0.5 : 0.9)) NextBlock();
    if (rs.active) {
        const QuizQuestion *q = Q(rs.cur);
        float bw = play.width - 40;
        rs.blockH = BlockHeight(q, bw);
        float landY = floorY - rs.blockH - 6;
        if (rs.y < 0) rs.y = play.y + 8;   // starts fully in view, so the question can be read straight away
        if (landY < play.y) {
            // no room left: the pile reached the top
            if (rs.warmup) Finish(true);
            else { rs.over = true; rs.won = false; rs.overAt = GetTime(); rs.active = false; }
        } else if (!modal) {
            float speed = rs.dropping ? 1400 : fmaxf(20, landY - (play.y + 8)) / rs.fallTime;
            rs.y += speed * dt;
            if (rs.y >= landY) {
                rs.y = landY;
                ToPile(rs.dropping ? "Not quite - that one's on the pile, and it'll come back later."
                                   : "Too slow! It landed on the pile, and it'll come back later.");
            }
        }
        if (rs.active) {
            float shakeX = rs.shake > 0 ? sinf(time * 60) * 8 * rs.shake : 0;
            if (rs.shake > 0) rs.shake -= dt;
            Color border = rs.dropping ? COL_BAD : COL_RUSH;
            BeginScissorMode((int)play.x, (int)play.y, (int)play.width, (int)play.height);
            DrawBlock((Rectangle){ play.x + 20 + shakeX, rs.y, bw, rs.blockH }, q, border);
            EndScissorMode();
        }
    }

    // feedback banner
    if (rs.banner[0] && GetTime() - rs.bannerAt < 6) {
        float a = fminf(1, (float)(6 - (GetTime() - rs.bannerAt)));
        Rectangle b = { play.x, 74, play.width, 30 };
        DrawRectangleRounded(b, 0.4f, 6, ColorAlpha(rs.bannerGood ? COL_GOOD_DARK : COL_BAD_DARK, 0.9f * a));
        BeginScissorMode((int)b.x, (int)b.y, (int)b.width, (int)b.height);
        UiText(rs.banner, b.x + 14, b.y + 6, 16, ColorAlpha(WHITE, a));
        EndScissorMode();
    }

    // the answers
    float ay = play.y + play.height + 14;
    float aw = (play.width - 12) / 2, ah = (answersH - 12) / 2;
    if (rs.active && !rs.dropping) {
        const QuizQuestion *q = Q(rs.cur);
        for (int i = 0; i < q->optionCount; i++) {
            Rectangle r = { play.x + (i % 2) * (aw + 12), ay + (i / 2) * (ah + 12), aw, ah };
            bool hover = false;
            bool clicked = !rs.wrong[i] && UiClickArea(r, &hover);
            if (!rs.wrong[i] && !modal && IsKeyPressed(KEY_ONE + i)) clicked = true;
            Color fill = rs.wrong[i] ? COL_BAD_DARK : hover ? ColorBrightness(COL_PANEL_HI, 0.2f) : COL_PANEL_HI;
            DrawRectangleRounded(r, 0.25f, 8, fill);
            DrawCircleV((Vector2){ r.x + 26, r.y + r.height / 2 }, 15, COL_RUSH);
            UiTextCentered(TextFormat("%d", i + 1), r.x + 26, r.y + r.height / 2 - 10, 19, WHITE);
            BeginScissorMode((int)r.x, (int)r.y, (int)r.width, (int)r.height);
            float th = UiWrappedHeight(q->options[i], r.width - 64, 18);
            UiWrapped(q->options[i], r.x + 52, r.y + (r.height - th) / 2 + 2, r.width - 64, 18, COL_TEXT);
            EndScissorMode();
            if (clicked) { Answer(i); break; }
        }
    } else if (!rs.over) {
        UiTextCentered("Get ready...", SCREEN_W / 2.0f, ay + answersH / 2 - 14, 22, COL_MUTED);
    }

    // sidebars: progress, score, combo, time
    float lx = playX - sideW - 20, rx = playX + playW + 20;
    if (lx > 10) {
        float y = 120;
        UiText(rs.warmup ? "WARM-UP" : "OUTPUT RUSH", lx, y, 20, COL_RUSH);
        y += 34;
        if (rs.warmup) {
            float left = fmaxf(0, WARMUP_SECONDS - elapsed);
            UiText(TextFormat("%d:%02d", (int)left / 60, (int)left % 60), lx, y, 44, left < 15 ? COL_BAD : COL_TEXT);
            y += 60;
            UiText(TextFormat("Best: %d", prog.warmupBest), lx, y, 18, COL_MUTED);
        } else {
            UiText(TextFormat("%d / %d", rs.cleared, rs.total), lx, y, 44, COL_TEXT);
            y += 56;
            UiText("questions cleared", lx, y, 17, COL_MUTED);
            y += 40;
            DrawStarRow(lx + 34, y + 10, 12, 0, rs.mistakes == 0 ? 3 : rs.mistakes == 1 ? 2 : 1);
            UiText("if you finish now", lx + 80, y, 15, COL_MUTED);
        }
        y += 60;
        UiWrapped("Click an answer or press 1-4 before the block lands. Wrong answers pile up and come back.",
                  lx, y, sideW, 16, COL_MUTED);
    }
    if (rx + 100 < SCREEN_W) {
        float y = 120;
        UiText("SCORE", rx, y, 18, COL_MUTED);
        UiText(TextFormat("%d", rs.score), rx, y + 24, 44, COL_GOLD);
        y += 90;
        UiText("COMBO", rx, y, 18, COL_MUTED);
        UiText(TextFormat("x%d", rs.combo), rx, y + 24, 36, rs.combo >= 3 ? COL_RUSH : COL_TEXT);
        y += 80;
        UiText("MISTAKES", rx, y, 18, COL_MUTED);
        UiText(TextFormat("%d", rs.mistakes), rx, y + 24, 36, rs.mistakes ? COL_BAD : COL_GOOD);
    }

    UiSetBlocked(false);

    // the end
    if (rs.over && GetTime() - rs.overAt > 0.4) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, ColorAlpha(BLACK, 0.55f));
        Rectangle card = { SCREEN_W / 2.0f - 280, SCREEN_H / 2.0f - 170, 560, 340 };
        DrawRectangleRounded(card, 0.08f, 8, COL_PANEL);
        DrawRectangleRoundedLinesEx(card, 0.08f, 8, 2, COL_RUSH);
        float cx = card.x + card.width / 2, y = card.y + 28;
        if (rs.warmup) {
            UiTextCentered("Time!", cx, y, 38, COL_TEXT);
            y += 60;
            UiTextCentered(TextFormat("Score %d  -  %d correct, %d mistake%s", rs.score, rs.correct, rs.mistakes, rs.mistakes == 1 ? "" : "s"),
                           cx, y, 20, COL_TEXT);
            y += 36;
            UiTextCentered(rs.score >= prog.warmupBest && rs.score > 0 ? "A new best score!" : TextFormat("Best: %d", prog.warmupBest),
                           cx, y, 18, COL_GOLD);
        } else if (rs.won) {
            UiTextCentered("Rush cleared!", cx, y, 38, COL_TEXT);
            y += 64;
            DrawStarRow(cx, y + 12, 24, 0, rs.mistakes == 0 ? 3 : rs.mistakes == 1 ? 2 : 1);
            y += 54;
            UiTextCentered(rs.mistakes == 0 ? "Flawless - not a single mistake!" : TextFormat("%d mistake%s", rs.mistakes, rs.mistakes == 1 ? "" : "s"),
                           cx, y, 18, COL_MUTED);
        } else {
            UiTextCentered("The pile reached the top!", cx, y, 34, COL_TEXT);
            y += 60;
            UiTextCentered("Re-read the briefing, then have another go.", cx, y, 18, COL_MUTED);
            y += 36;
        }
        y += 34;
        UiTextCentered(rs.xp > 0 ? TextFormat("+%d XP", rs.xp) : "", cx, y, 22, COL_GOLD);
        Rectangle a = { cx - 255, card.y + card.height - 70, 245, 50 }, b = { cx + 10, card.y + card.height - 70, 245, 50 };
        if (UiButton(a, "Play again", COL_PANEL_HI)) {
            if (rs.warmup) OpenWarmup(); else OpenRush(rs.tut);
            return;
        }
        if (rs.warmup) {
            if (UiButton(b, "Main menu", COL_ACCENT)) OpenMenu();
        } else if (rs.won) {
            int main = MainMissionOf(rs.tut);
            if (UiButton(b, "To the mission >", COL_ACCENT)) {
                if (main >= 0) OpenMission(main); else OpenLevel(rs.tut);
            }
        } else {
            if (UiButton(b, "Read the briefing", COL_ACCENT)) OpenBriefing(rs.tut, 0);
        }
    }
}
