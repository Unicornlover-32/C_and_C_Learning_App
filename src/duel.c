// 1v1 Chess: a whole game of chess where every move is a program. On your turn you write C
// that makes ONE move for your side - pointers into Square board[64], the same chess library
// as the puzzles - and press Play my move. The code is compiled and run against the real
// position: if the move is legal it is played on the 3D board, if not the library says why
// and it's still your turn. Your code stays in the editor between turns, so you can change
// one line, or write a little program that picks a move by itself.
//
// Play the computer (easy, medium or hard, as White or Black), or a friend on the same
// computer: each player keeps their own program, and the board turns to face whoever moves.
#include "app.h"
#include "editor.h"
#include "game/levels.h"
#include "game/gfx.h"
#include "game/chess_duel.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { PHASE_SETUP, PHASE_PLAY };
enum { VS_COMPUTER, VS_FRIEND };

static Editor    editor;
static bool      editorReady;
static RunResult run;

static struct {
    int        phase;
    int        mode, level, humanWhite;     // the set-up choices
    bool       started;                      // there is a game to resume
    ChessGame  game;
    char       code[2][EDITOR_CAP];          // [1] White's program, [0] Black's
    int        codeSide;                     // whose program is in the editor
    bool       runPending, thinking, over, confirmResign, cardHidden;
    double     thinkAt, resignAt;
    ChessState state;
    int        winner;                       // 1 White, 0 Black, -1 a draw
    bool       resigned;
    char       message[400];
    Color      messageColor;
    char       output[1200];                 // what the program printed, or the compiler's errors
    bool       compileError;
    float      apiScroll;
    int        xp;
} D = { .mode = VS_COMPUTER, .level = 1, .humanWhite = 1 };

static const char *API =
    "Each turn your program makes ONE move for your side.\n"
    "\n"
    "Square board[64]        a1 is board[0] ... h8 is board[63]\n"
    "Square *at(\"e4\")        a pointer to a square\n"
    "Square *find('N')       the first square with that piece\n"
    "p->piece p->file p->rank   'K'..'P' White, 'k'..'p' Black, '.'\n"
    "Way 1: move(from, to)   your move\n"
    "Way 2: pick_up(square)  pick up a piece, then point hand\n"
    "       hand = at(\"e4\");  at its new square (or hand += 16;)\n"
    "promote(from, to, 'Q')  a pawn to the last rank\n"
    "legal(from, to)         1 if that move is allowed\n"
    "mates(from, to)         1 if that move is checkmate\n"
    "is_mine(p)  is_theirs(p)\n"
    "their_from()  their_to()   the other side's last move\n"
    "\n"
    "+1 right   -1 left   +8 up the board (towards Black)\n"
    "-8 down (towards White)   +9 +7 -7 -9 the diagonals";

static const char *Template(bool white)
{
    return white ? "#include <stdio.h>\n"
                   "\n"
                   "// You are White. Two ways to move (Commands shows them all):\n"
                   "//   1. move(at(\"e2\"), at(\"e4\"));    or with steps: move(p, p + 16);\n"
                   "//   2. pick_up(at(\"e2\"));  hand = at(\"e4\");    or: hand += 16;\n"
                   "// +8 is one rank up the board (towards Black).\n"
                   "\n"
                   "int main(void)\n"
                   "{\n"
                   "    Square *pawn = at(\"e2\");\n"
                   "    move(pawn, pawn + 16);     // e2 to e4: two ranks up\n"
                   "    return 0;\n"
                   "}\n"
                 : "#include <stdio.h>\n"
                   "\n"
                   "// You are Black. Two ways to move (Commands shows them all):\n"
                   "//   1. move(at(\"e7\"), at(\"e5\"));    or with steps: move(p, p - 16);\n"
                   "//   2. pick_up(at(\"e7\"));  hand = at(\"e5\");    or: hand -= 16;\n"
                   "// -8 is one rank down the board (towards White).\n"
                   "\n"
                   "int main(void)\n"
                   "{\n"
                   "    Square *pawn = at(\"e7\");\n"
                   "    move(pawn, pawn - 16);     // e7 to e5: two ranks down\n"
                   "    return 0;\n"
                   "}\n";
}

static const char *LevelName(int level) { return level <= 0 ? "Easy" : level == 1 ? "Medium" : "Hard"; }
static const char *SideName(int white) { return white ? "White" : "Black"; }

static bool HumanTurn(void) { return !D.over && (D.mode == VS_FRIEND || D.game.white == D.humanWhite); }

// The camera looks from the side of whoever is coding
static float Yaw(void)
{
    if (D.mode == VS_COMPUTER) return D.humanWhite ? 0 : 180;
    return D.game.white ? 0 : 180;
}

static void Say(const char *text, Color color)
{
    snprintf(D.message, sizeof(D.message), "%s", text);
    D.messageColor = color;
}

static void SaveCode(void) { memcpy(D.code[D.codeSide], editor.text, (size_t)editor.len + 1); }

static void LoadCode(int side)
{
    D.codeSide = side;
    EditorSetText(&editor, D.code[side]);
    editor.focused = true;
}

static void YourTurnMessage(void)
{
    if (D.mode == VS_FRIEND)
        Say(TextFormat("%s to move - %s, write your move and press Play my move.", SideName(D.game.white), SideName(D.game.white)), COL_ACCENT);
    else
        Say("Your move: write it in code, then press Play my move (Ctrl+Enter).", COL_ACCENT);
}

static void StartGame(void)
{
    ChessGameStart(&D.game);
    D.started = true;
    D.phase = PHASE_PLAY;
    D.over = D.resigned = D.thinking = D.runPending = D.confirmResign = D.cardHidden = false;
    D.state = GAME_ON;
    D.winner = -1;
    D.xp = 0;
    D.output[0] = '\0';
    D.compileError = false;
    snprintf(D.code[1], EDITOR_CAP, "%s", Template(true));
    snprintf(D.code[0], EDITOR_CAP, "%s", Template(false));
    LoadCode(D.mode == VS_COMPUTER ? D.humanWhite : 1);
    ChessSceneStart(D.game.sq, Yaw());
    FxClear();
    if (D.mode == VS_COMPUTER && !D.humanWhite) {
        D.thinking = true;
        D.thinkAt = GetTime() + 0.8;
        Say("The computer plays White and moves first...", COL_MUTED);
    } else {
        YourTurnMessage();
    }
}

static void EndGame(ChessState state, bool moverWasWhite)
{
    D.over = true;
    D.state = state;
    D.winner = state == GAME_CHECKMATE ? (moverWasWhite ? 1 : 0) : -1;
    if (D.winner >= 0) FxConfetti((Vector3){ 0, 2.0f, 0 }, 170, 4.0f);
    if (D.mode == VS_COMPUTER && D.winner == D.humanWhite) D.xp = AwardDuel(D.level);
    if (state == GAME_CHECKMATE) Say(TextFormat("Checkmate! %s wins.", SideName(D.winner)), COL_GOOD);
    else Say(TextFormat("A draw: %s.", ChessStateText(state)), COL_GOLD);
}

static void Play(ChessMove m)
{
    bool white = D.game.white != 0;
    char flags[8];
    ChessGamePlay(&D.game, m, flags, sizeof(flags));
    ChessSceneMove(m.from, m.to, m.promo, flags, white);
    ChessState st = ChessGameState(&D.game);
    if (st != GAME_ON) { EndGame(st, white); return; }

    if (D.mode == VS_FRIEND) {                   // the next player gets their own program, and the board turns
        SaveCode();
        LoadCode(D.game.white);
        ChessSceneTurn(Yaw());
        YourTurnMessage();
        if (ChessGameInCheck(&D.game)) Say(TextFormat("Check! %s must get the king out of check.", SideName(D.game.white)), COL_BAD);
    } else if (D.game.white != D.humanWhite) {
        D.thinking = true;
        D.thinkAt = GetTime() + 0.9;             // let the move land first
        Say("The computer is thinking...", COL_MUTED);
    } else {
        YourTurnMessage();
        if (ChessGameInCheck(&D.game)) Say("Check! Your king is attacked - your move has to get it out of check.", COL_BAD);
    }
}

static int SquareAt(const char *s) { return (s[0] >= 'a' && s[0] <= 'h' && s[1] >= '1' && s[1] <= '8') ? (s[1] - '1') * 8 + (s[0] - 'a') : -1; }

// Compiles and runs the program for the side to move, and plays its move if it is legal
static void RunMove(void)
{
    SaveCode();
    char fen[100], input[200];
    ChessGameFen(&D.game, fen, sizeof(fen));
    int n = snprintf(input, sizeof(input), "FEN %s\n", fen);
    if (D.game.plies > 0) {
        ChessMove last = D.game.moves[D.game.plies - 1];
        n += snprintf(input + n, sizeof(input) - (size_t)n, "LAST %c%c%c%c\n", 'a' + last.from % 8, '1' + last.from / 8, 'a' + last.to % 8, '1' + last.to / 8);
    }
    snprintf(input + n, sizeof(input) - (size_t)n, "END\n");
    TestCase test = { input, NULL };
    RunRequest req = { editor.text, LANG_C, "#define CFD_DUEL\n#include \"cfd_chess.h\"\n", GameDir(), true, &test, 1 };
    RunProgram(&req, &run);

    D.compileError = !run.compiled;
    if (!run.compiled) {
        snprintf(D.output, sizeof(D.output), "%.1100s", run.compileLog);
        Say("Your code didn't compile - fix the first error below, then try again. It's still your move.", COL_BAD);
        return;
    }
    snprintf(D.output, sizeof(D.output), "%.1100s", run.output[0]);
    char reason[300];
    int ending = TraceEnding(run.events[0], reason, sizeof(reason));

    // the move line ("W e2 e4 - -"), or the refused one ("I e2 e5")
    int from = -1, to = -1;
    char promo = 0, kind = 0;
    for (const char *l = run.events[0]; l && *l; ) {
        if ((l[0] == 'W' || l[0] == 'B' || l[0] == 'I') && l[1] == ' ') {
            kind = l[0];
            from = SquareAt(l + 2);
            to = strlen(l) > 6 ? SquareAt(l + 5) : -1;
            promo = (kind != 'I' && strlen(l) > 9 && strchr("QRBN", l[8])) ? l[8] : 0;
        }
        l = strchr(l, '\n');
        if (l) l++;
    }

    if (ending == 1 && run.status[0] == RUN_OK && (kind == 'W' || kind == 'B')) {
        ChessMove m;
        if (from >= 0 && to >= 0 && ChessGameFind(&D.game, from, to, promo, &m)) { Play(m); return; }
        Say("Something went wrong reading your move - please try again.", COL_BAD);
        return;
    }
    if (ending == 0) {
        Say(TextFormat("Not played: %s It's still your move.", reason), COL_BAD);
        if (kind == 'I') ChessSceneRefused(from, to);
        return;
    }
    if (run.status[0] == RUN_TIMEOUT || run.status[0] == RUN_TOO_MUCH_OUTPUT)
        Say("Your program ran too long (is a loop going round forever?) - no move was played.", COL_BAD);
    else if (run.status[0] != RUN_OK)
        Say("Your program crashed before it finished - no move was played. Check your pointers aren't NULL or off the board.", COL_BAD);
    else
        Say("Your program ended without the game checking the move - try again.", COL_BAD);
}

void OpenDuel(void)
{
    if (!editorReady) { EditorInit(&editor, 20); editorReady = true; }
    ChessSetStyle(prog.chessPieces, prog.chessBoard);
    app.screen = SCREEN_DUEL;
    if (D.started && D.phase == PHASE_PLAY) ChessSceneStart(D.game.sq, Yaw());   // the board is shared with the puzzles
    else D.phase = PHASE_SETUP;
}

void DuelShutdown(void)
{
    if (editorReady) EditorFree(&editor);
    editorReady = false;
}

static void Leave(void)
{
    if (D.phase == PHASE_PLAY && D.started) SaveCode();
    OpenChess();
}

// ---------------------------------------------------------------- the set-up

static bool Choice(Rectangle r, const char *title, const char *sub, bool on)
{
    bool hover = false, click = UiClickArea(r, &hover);
    DrawRectangleRounded(r, 0.2f, 8, on ? ColorAlpha(COL_CHESS, 0.25f) : hover ? COL_PANEL_HI : (Color){ 36, 39, 51, 255 });
    if (on) DrawRectangleRoundedLinesEx(r, 0.2f, 8, 2, COL_CHESS);
    UiText(title, r.x + 16, r.y + (sub ? 12 : r.height / 2 - 10), 19, on ? COL_TEXT : COL_MUTED);
    if (sub) UiText(sub, r.x + 16, r.y + 40, 14, COL_MUTED);
    return click;
}

static void DrawSetup(void)
{
    float top = 84, bottom = SCREEN_H - 20.0f, leftW = floorf((SCREEN_W - 60) * 0.5f);
    Rectangle view = { 20, top, leftW, bottom - top };
    ChessPreview(view, (float)GetTime(), GetFrameTime());
    Rectangle r = { 40 + leftW, top, SCREEN_W - 60 - leftW, bottom - top };
    DrawRectangleRounded(r, 0.03f, 8, COL_PANEL);
    float x = r.x + 24, w = r.width - 48, y = r.y + 20;
    IconPawn((Vector2){ x + 16, y + 18 }, 38, COL_CHESS);
    UiText("1v1 Chess", x + 44, y, 30, COL_TEXT);
    y += 50;
    y += UiWrapped("A real game of chess - but every move is a program. On your turn, write C that moves one of your pieces "
                   "with pointers into the board, and press Play my move. If the move breaks the rules it isn't played, and "
                   "the game tells you why. Your code stays in the editor, so next turn you can change a line - or write a "
                   "program clever enough to choose the move by itself.", x, y, w, 16, COL_MUTED) + 18;

    UiText("Who do you want to play?", x, y, 18, COL_TEXT);
    y += 30;
    float cw = (w - 12) / 2;
    if (Choice((Rectangle){ x, y, cw, 66 }, "The computer", "it answers each of your moves", D.mode == VS_COMPUTER)) D.mode = VS_COMPUTER;
    if (Choice((Rectangle){ x + cw + 12, y, cw, 66 }, "A friend", "take turns on this computer", D.mode == VS_FRIEND)) D.mode = VS_FRIEND;
    y += 84;
    if (D.mode == VS_COMPUTER) {
        UiText("How strong?", x, y, 18, COL_TEXT);
        y += 30;
        float bw = (w - 24) / 3;
        static const char *subs[3] = { "looks one move ahead", "looks two moves ahead", "looks five moves ahead" };
        for (int i = 0; i < 3; i++)
            if (Choice((Rectangle){ x + i * (bw + 12), y, bw, 66 }, LevelName(i), subs[i], D.level == i)) D.level = i;
        y += 84;
        UiText("You play", x, y, 18, COL_TEXT);
        y += 30;
        if (Choice((Rectangle){ x, y, cw, 48 }, "White (you move first)", NULL, D.humanWhite)) D.humanWhite = 1;
        if (Choice((Rectangle){ x + cw + 12, y, cw, 48 }, "Black", NULL, !D.humanWhite)) D.humanWhite = 0;
        y += 66;
        UiText(TextFormat("Beat it for %d XP.", D.level <= 0 ? 40 : D.level == 1 ? 80 : 150), x, y, 16, COL_GOLD);
    } else {
        UiWrapped("White writes a program for White's moves and Black for Black's - each player keeps their own code, "
                  "and the board turns to face whoever is moving.", x, y, w, 16, COL_MUTED);
    }

    float by = r.y + r.height - 76;
    bool resume = D.started && !D.over;
    if (resume && UiButton((Rectangle){ x, by, cw, 56 }, "Resume the game", COL_PANEL_HI)) {
        D.phase = PHASE_PLAY;
        ChessSceneStart(D.game.sq, Yaw());
        return;
    }
    if (UiButton(resume ? (Rectangle){ x + cw + 12, by, cw, 56 } : (Rectangle){ x, by, w, 56 }, "Start a new game", (Color){ 40, 120, 105, 255 }))
        StartGame();
}

// ---------------------------------------------------------------- the game

static void DrawMoves(Rectangle r)
{
    DrawRectangleRounded(r, 0.06f, 8, COL_PANEL);
    UiText("Moves", r.x + 14, r.y + 8, 15, COL_MUTED);
    float lh = 20, y0 = r.y + 32;
    int rows = (D.game.plies + 1) / 2, fit = (int)((r.height - 40) / lh);
    int first = rows > fit ? rows - fit : 0;
    BeginScissorMode((int)r.x, (int)y0, (int)r.width, (int)(r.height - 36));
    for (int row = first; row < rows; row++) {
        float y = y0 + (row - first) * lh;
        UiText(TextFormat("%d.", row + 1), r.x + 14, y, 15, COL_MUTED);
        UiCode(D.game.notation[row * 2], r.x + 50, y, 15, COL_TEXT, 0);
        if (row * 2 + 1 < D.game.plies) UiCode(D.game.notation[row * 2 + 1], r.x + 50 + (r.width - 60) / 2, y, 15, COL_TEXT, 0);
    }
    EndScissorMode();
    if (D.game.plies == 0) UiText("No moves yet.", r.x + 14, y0, 15, COL_MUTED);
}

static void DrawOutput(Rectangle r)
{
    DrawRectangleRounded(r, 0.04f, 8, COL_PANEL);
    float x = r.x + 14, w = r.width - 28, y = r.y + 10;
    y += UiWrapped(D.message, x, y, w, 16, D.messageColor) + 8;
    if (!D.output[0]) return;
    UiText(D.compileError ? "Compiler errors:" : "Your program printed:", x, y, 14, COL_MUTED);
    y += 20;
    BeginScissorMode((int)x, (int)y, (int)w, (int)(r.y + r.height - 8 - y));
    UiCode(D.output, x, y, 14, D.compileError ? (Color){ 255, 170, 170, 255 } : COL_CODE_TEXT, w);
    EndScissorMode();
}

static void DrawEndCard(Rectangle view)
{
    Rectangle card = { view.x + view.width / 2 - 260, view.y + view.height / 2 - 130, 520, 260 };
    DrawRectangleRounded(card, 0.08f, 8, (Color){ 22, 25, 34, 240 });
    DrawRectangleRoundedLinesEx(card, 0.08f, 8, 2, D.winner >= 0 ? COL_GOOD : COL_GOLD);
    float cx = card.x + card.width / 2, y = card.y + 22;
    const char *title;
    if (D.winner < 0) title = "It's a draw";
    else if (D.mode == VS_COMPUTER) title = D.winner == D.humanWhite ? "You win!" : "The computer wins";
    else title = TextFormat("%s wins!", SideName(D.winner));
    UiTextCentered(title, cx, y, 34, COL_TEXT);
    y += 50;
    const char *why = D.resigned ? TextFormat("%s resigned.", SideName(!D.winner)) : D.state == GAME_CHECKMATE ? "Checkmate." : ChessStateText(D.state);
    UiTextCentered(why, cx, y, 17, COL_MUTED);
    y += 30;
    UiTextCentered(TextFormat("%d moves", (D.game.plies + 1) / 2), cx, y, 17, COL_MUTED);
    y += 30;
    if (D.xp > 0) UiTextCentered(TextFormat("+%d XP", D.xp), cx, y, 22, COL_GOLD);
    Rectangle a = { card.x + 20, card.y + card.height - 66, 230, 48 }, b = { card.x + 270, card.y + card.height - 66, 230, 48 };
    if (UiButton(a, "Look at the board", COL_PANEL_HI)) D.cardHidden = true;
    if (UiButton(b, "New game", (Color){ 40, 120, 105, 255 })) { D.phase = PHASE_SETUP; D.started = false; }
}

void ScreenDuel(void)
{
    if (D.phase == PHASE_SETUP) {
        if (IsKeyPressed(KEY_ESCAPE)) { Leave(); return; }
        DrawTopBar("1v1 Chess");
        DrawPlayerChip(SCREEN_W - 24, 14);
        if (BackButton("< Chess")) { Leave(); return; }
        DrawSetup();
        return;
    }

    // a run was asked for last frame, after "Compiling..." was drawn; the computer moves once
    // the last move has landed
    if (D.runPending) { D.runPending = false; RunMove(); }
    if (D.thinking && !D.over && GetTime() >= D.thinkAt && !ChessSceneBusy()) {
        D.thinking = false;
        Play(ChessComputerMove(&D.game, D.level));
    }
    if (D.confirmResign && GetTime() - D.resignAt > 3) D.confirmResign = false;

    bool endCard = D.over && !D.cardHidden;
    if (IsKeyPressed(KEY_ESCAPE)) { Leave(); return; }
    if (!endCard && HumanTurn() && !D.thinking && CtrlDown() && IsKeyPressed(KEY_ENTER)) D.runPending = true;

    DrawTopBar("1v1 Chess");
    TopBarRight(D.mode == VS_COMPUTER ? TextFormat("vs the computer (%s)  -  you are %s", LevelName(D.level), SideName(D.humanWhite))
                                      : "two players", COL_MUTED);
    UiSetBlocked(endCard);
    if (BackButton("< Chess")) { UiSetBlocked(false); Leave(); return; }

    float colW = SCREEN_W * 0.41f;
    if (colW < 470) colW = 470;
    if (colW > 760) colW = 760;
    Rectangle col = { 20, 84, colW, SCREEN_H - 104.0f };
    Rectangle right = { col.x + colW + 16, 84, SCREEN_W - (col.x + colW + 16) - 20, SCREEN_H - 104.0f };

    // the left: whose turn, what the library gives you, the editor and the buttons
    Rectangle head = { col.x, col.y, col.width, 56 };
    DrawRectangleRounded(head, 0.14f, 8, COL_PANEL);
    const char *chip = D.mode == VS_COMPUTER ? TextFormat("VS COMPUTER - %s", LevelName(D.level)) : "2 PLAYERS";
    float chw = UiTextWidth(chip, 15) + 24;
    DrawRectangleRounded((Rectangle){ head.x + 14, head.y + 15, chw, 26 }, 0.5f, 6, ColorAlpha(COL_CHESS, 0.2f));
    UiText(chip, head.x + 26, head.y + 20, 15, COL_CHESS);
    const char *turn = D.over ? "Game over"
                     : D.thinking ? "The computer is thinking..."
                     : D.mode == VS_FRIEND ? TextFormat("Move %d - %s's code", D.game.fullmove, SideName(D.game.white))
                     : TextFormat("Move %d - your code (%s)", D.game.fullmove, SideName(D.humanWhite));
    UiText(turn, head.x + 26 + chw, head.y + 16, 20, COL_TEXT);
    if (UiButton((Rectangle){ head.x + head.width - 132, head.y + 12, 120, 32 }, "Commands", COL_PANEL_HI)) {
        UiSetBlocked(false);
        SaveCode();
        OpenChessHelp(SCREEN_DUEL);
        return;
    }

    float bh = 46, gap = 8, by = col.y + col.height - bh;
    float apiH = 220;
    Rectangle api = { col.x, head.y + head.height + 8, col.width, apiH };
    DrawRectangleRounded(api, 0.05f, 8, COL_PANEL);
    if (CheckCollisionPointRec(GetMousePosition(), api)) D.apiScroll -= GetMouseWheelMove() * 30;
    if (D.apiScroll < 0) D.apiScroll = 0;
    BeginScissorMode((int)api.x, (int)api.y + 6, (int)api.width, (int)api.height - 12);
    float used = UiCode(API, api.x + 14, api.y + 10 - D.apiScroll, 14, COL_CODE_TEXT, api.width - 28);
    EndScissorMode();
    if (D.apiScroll > used - api.height + 20) D.apiScroll = fmaxf(0, used - api.height + 20);

    Rectangle ed = { col.x, api.y + api.height + 8, col.width, by - 8 - (api.y + api.height + 8) };
    bool canPlay = HumanTurn() && !D.thinking && !D.runPending;
    if (!endCard && canPlay) EditorUpdate(&editor, ed);
    EditorDraw(&editor, ed);
    if (!canPlay && !D.over) {
        DrawRectangleRec(ed, (Color){ 10, 12, 18, 150 });
        UiTextCentered(D.thinking ? "The computer is thinking..." : "Compiling your move...", ed.x + ed.width / 2, ed.y + ed.height / 2 - 12, 20, COL_MUTED);
    }

    float playW = col.width * 0.44f, small = (col.width - playW - gap * 2) / 2;
    Rectangle playBtn = { col.x, by, playW, bh }, resetBtn = { col.x + playW + gap, by, small, bh }, resignBtn = { resetBtn.x + small + gap, by, small, bh };
    if (UiButton(playBtn, D.runPending ? "Compiling..." : D.over ? "Game over" : "Play my move", canPlay ? COL_GOOD_DARK : COL_PANEL_HI) && canPlay) {
        D.runPending = true;
        Say("Compiling your move...", COL_ACCENT);
    }
    if (UiButton(resetBtn, "Reset code", COL_PANEL_HI) && !D.over) {
        EditorReplaceAll(&editor, Template(D.codeSide));
        Say("Your code is back to the example move - Ctrl+Z brings yours back.", COL_MUTED);
    }
    if (UiButton(resignBtn, D.over ? "New game" : D.confirmResign ? "Sure?" : "Resign", D.confirmResign ? COL_BAD_DARK : COL_PANEL_HI)) {
        if (D.over) { D.phase = PHASE_SETUP; D.started = false; UiSetBlocked(false); return; }
        if (!D.confirmResign) { D.confirmResign = true; D.resignAt = GetTime(); }
        else if (!D.thinking) {
            int loser = D.mode == VS_COMPUTER ? D.humanWhite : D.game.white;
            D.over = true;
            D.resigned = true;
            D.state = GAME_CHECKMATE;
            D.winner = !loser;
            D.confirmResign = false;
            Say(TextFormat("%s resigned - %s wins.", SideName(loser), SideName(!loser)), COL_GOLD);
        }
    }

    // the right: the board, the moves and what the program said
    float bottomH = 170;
    Rectangle view = { right.x, right.y, right.width, right.height - bottomH - 10 };
    ChessSceneDraw(view, GetFrameTime());
    const char *who = D.over ? (D.winner < 0 ? "Draw" : TextFormat("%s wins", SideName(D.winner))) : TextFormat("%s to move", SideName(D.game.white));
    float ww = UiTextWidth(who, 17) + 24;
    DrawRectangleRounded((Rectangle){ view.x + 12, view.y + 12, ww, 30 }, 0.5f, 6, D.game.white ? (Color){ 230, 226, 214, 230 } : (Color){ 40, 40, 52, 230 });
    UiText(who, view.x + 24, view.y + 18, 17, D.game.white ? (Color){ 30, 30, 40, 255 } : (Color){ 235, 235, 245, 255 });
    if (!D.over && ChessGameInCheck(&D.game)) {
        DrawRectangleRounded((Rectangle){ view.x + 20 + ww, view.y + 12, 90, 30 }, 0.5f, 6, COL_BAD_DARK);
        UiText("CHECK!", view.x + 36 + ww, view.y + 18, 17, WHITE);
    }
    float half = (right.width - 10) / 2;
    DrawMoves((Rectangle){ right.x, view.y + view.height + 10, half * 0.8f, bottomH });
    DrawOutput((Rectangle){ right.x + half * 0.8f + 10, view.y + view.height + 10, right.width - half * 0.8f - 10, bottomH });

    UiSetBlocked(false);
    if (endCard) DrawEndCard(view);
}
