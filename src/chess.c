// The Chess Puzzles screen: the board in the player's chosen look (left, with the way into
// 1v1 chess), the pieces and boards that stars unlock (below it), and the puzzles from easy
// to very hard (right).
// Each puzzle opens on the mission screen, where the student's C code moves the pieces
// with pointers into the board and the 3D board plays it out.
#include "app.h"
#include "game/levels.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

void OpenChess(void)
{
    ChessSetStyle(prog.chessPieces, prog.chessBoard);
    app.screen = SCREEN_CHESS;
}

// One choice in the shop. Returns true when an unlocked one is clicked.
static bool LookChip(Rectangle r, const char *name, int need, bool selected, int have)
{
    bool unlocked = have >= need, hover = false;
    bool clicked = UiClickArea(r, &hover) && unlocked;
    Color fill = selected ? ColorAlpha(COL_CHESS, 0.28f) : (hover && unlocked) ? COL_PANEL_HI : (Color){ 36, 39, 51, 255 };
    DrawRectangleRounded(r, 0.25f, 6, fill);
    if (selected) DrawRectangleRoundedLinesEx(r, 0.25f, 6, 2, COL_CHESS);
    float cx = r.x + r.width / 2;
    UiTextCentered(name, cx, r.y + 9, 16, unlocked ? COL_TEXT : COL_MUTED);
    if (!unlocked) {
        const char *need_ = TextFormat("%d", need);
        float w = 16 + 6 + UiTextWidth(need_, 15);
        DrawStar((Vector2){ cx - w / 2 + 8, r.y + 42 }, 8, COL_GOLD);
        UiText(need_, cx - w / 2 + 22, r.y + 33, 15, COL_GOLD);
        IconLock((Vector2){ r.x + r.width - 14, r.y + 14 }, 14, COL_MUTED);
    } else {
        UiTextCentered(selected ? "in use" : hover ? "use this" : "unlocked", cx, r.y + 33, 14, selected ? COL_CHESS : COL_MUTED);
    }
    return clicked;
}

static void DrawShop(Rectangle r)
{
    DrawRectangleRounded(r, 0.06f, 8, COL_PANEL);
    float x = r.x + 16, w = r.width - 32, y = r.y + 12;
    int have = AllStars();
    UiText("Your board", x, y, 20, COL_TEXT);
    const char *stars = TextFormat("%d", have);
    float sw = UiTextWidth(stars, 22);
    DrawStar((Vector2){ r.x + r.width - 26 - sw - 14, y + 13 }, 11, COL_GOLD);
    UiText(stars, r.x + r.width - 16 - sw, y, 22, COL_GOLD);
    UiText(TextFormat("levels %d  +  side quests %d  +  puzzles %d", TotalStars(), SideStars(), ChessStars()), x, y + 26, 14, COL_MUTED);
    y += 52;

    bool changed = false;
    for (int row = 0; row < 2; row++) {
        int n = row == 0 ? CHESS_PIECE_SETS : CHESS_BOARDS;
        UiText(row == 0 ? "Pieces" : "Board", x, y + 18, 16, COL_MUTED);
        float lx = x + 64, gap = 8, cw = (w - 64 - gap * (n - 1)) / n;
        for (int i = 0; i < n; i++) {
            Rectangle c = { lx + i * (cw + gap), y, cw, 58 };
            const char *name = row == 0 ? ChessPieceSetName(i) : ChessBoardName(i);
            int need = row == 0 ? ChessPieceSetStars(i) : ChessBoardStars(i);
            int *cur = row == 0 ? &prog.chessPieces : &prog.chessBoard;
            if (LookChip(c, name, need, *cur == i, have) && *cur != i) { *cur = i; changed = true; }
        }
        y += 66;
    }
    if (changed) {
        ChessSetStyle(prog.chessPieces, prog.chessBoard);
        SaveProgress();
    }

    // what the next star unlocks
    int next = -1;
    const char *what = "";
    for (int i = 0; i < CHESS_PIECE_SETS; i++)
        if (ChessPieceSetStars(i) > have && (next < 0 || ChessPieceSetStars(i) < next)) { next = ChessPieceSetStars(i); what = TextFormat("%s pieces", ChessPieceSetName(i)); }
    for (int i = 0; i < CHESS_BOARDS; i++)
        if (ChessBoardStars(i) > have && (next < 0 || ChessBoardStars(i) < next)) { next = ChessBoardStars(i); what = TextFormat("the %s board", ChessBoardName(i)); }
    if (next >= 0) UiText(TextFormat("Next unlock: %s at %d stars - %d to go. Stars from every part of the game count.", what, next, next - have),
                          x, r.y + r.height - 26, 14, COL_MUTED);
    else UiText("Every piece set and board is unlocked - well played!", x, r.y + r.height - 26, 14, COL_CHESS);
}

// The first sentence of a brief, for the puzzle cards
static const char *Teaser(const char *brief)
{
    static char out[160];
    int n = 0;
    while (brief[n] && brief[n] != '\n' && n < (int)sizeof(out) - 1) {
        out[n] = brief[n];
        n++;
        if (brief[n - 1] == '.' || brief[n - 1] == '!') break;
    }
    out[n] = '\0';
    return out;
}

static void DrawPuzzles(Rectangle r)
{
    DrawRectangleRounded(r, 0.03f, 8, COL_PANEL);
    float x = r.x + 20, w = r.width - 40, y = r.y + 14;
    IconPawn((Vector2){ x + 14, y + 16 }, 34, COL_CHESS);
    UiText("Chess Puzzles", x + 38, y, 26, COL_TEXT);
    const char *score = TextFormat("%d / %d", ChessStars(), MaxChessStars());
    float sw = UiTextWidth(score, 20);
    DrawStar((Vector2){ r.x + r.width - 30 - sw - 12, y + 13 }, 10, COL_GOLD);
    UiText(score, r.x + r.width - 20 - sw, y + 2, 20, COL_GOLD);
    y += 36;
    y += UiWrapped("Solve each puzzle with C code: pointers into Square board[64] move your pieces (+8 is one rank up), "
                   "every move follows the real rules of chess, and Black answers straight away.", x, y, w, 15, COL_MUTED) + 12;

    int cols = r.width > 760 ? 3 : 2;
    float gap = 12, cw = (w - gap * (cols - 1)) / cols, ch = 98;
    for (int d = 0; d < 4; d++) {
        Color dc = ChessDifficultyColor(d);
        const char *dn = ChessDifficultyName(d);
        float tw = UiTextWidth(dn, 14) + 22;
        DrawRectangleRounded((Rectangle){ x, y, tw, 22 }, 0.5f, 6, ColorAlpha(dc, 0.2f));
        UiText(dn, x + 11, y + 3, 14, dc);
        y += 28;
        int k = 0;
        for (int i = 0; i < MissionCount(); i++) {
            const MissionInfo *m = GetMission(i);
            if (m->role != ROLE_CHESS || m->region != d) continue;
            Rectangle c = { x + (k % cols) * (cw + gap), y + (k / cols) * (ch + gap), cw, ch };
            bool hover = false;
            if (UiClickArea(c, &hover)) { OpenMission(i); return; }
            DrawRectangleRounded(c, 0.12f, 8, hover ? COL_PANEL_HI : (Color){ 36, 39, 51, 255 });
            DrawRectangle((int)c.x, (int)(c.y + 10), 4, (int)(c.height - 20), dc);
            UiBeginScissor((int)c.x, (int)c.y, (int)c.width, (int)c.height);
            UiText(m->title, c.x + 16, c.y + 10, 19, COL_TEXT);
            DrawStarRow(c.x + c.width - 44, c.y + 21, 9, prog.stars[i], -1);
            UiWrapped(Teaser(m->brief), c.x + 16, c.y + 40, c.width - 30, 14, COL_MUTED);
            EndScissorMode();
            k++;
        }
        y += ((k + cols - 1) / cols) * (ch + gap) + 6;
    }
}

void ScreenChess(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { OpenMenu(); return; }
    DrawTopBar("Chess Puzzles");
    DrawPlayerChip(SCREEN_W - 24, 14);
    if (BackButton("< Menu")) { OpenMenu(); return; }

    float top = 84, bottom = SCREEN_H - 20.0f;
    float leftW = floorf((SCREEN_W - 60) * 0.45f);
    Rectangle left = { 20, top, leftW, bottom - top };
    Rectangle right = { 40 + leftW, top, SCREEN_W - 60 - leftW, bottom - top };

    float shopH = 236;
    Rectangle view = { left.x, left.y, left.width, left.height - shopH - 12 };
    ChessSetStyle(prog.chessPieces, prog.chessBoard);
    ChessPreview(view, (float)GetTime(), GetFrameTime());
    const char *look = TextFormat("%s pieces on the %s board", ChessPieceSetName(prog.chessPieces), ChessBoardName(prog.chessBoard));
    float lw = UiTextWidth(look, 15) + 20;
    DrawRectangleRounded((Rectangle){ view.x + 10, view.y + 10, lw, 26 }, 0.5f, 6, (Color){ 12, 14, 22, 200 });
    UiText(look, view.x + 20, view.y + 14, 15, COL_CHESS);

    Rectangle duel = { view.x + view.width - 230, view.y + view.height - 62, 218, 50 };
    if (UiButton(duel, "Play 1v1 chess  >", (Color){ 40, 120, 105, 255 })) { OpenDuel(); return; }
    Rectangle help = { view.x + 12, view.y + view.height - 62, 218, 50 };
    if (UiButton(help, "How to move pieces", COL_PANEL_HI)) { OpenChessHelp(SCREEN_CHESS); return; }

    DrawShop((Rectangle){ left.x, view.y + view.height + 12, left.width, shopH });
    DrawPuzzles(right);
}
