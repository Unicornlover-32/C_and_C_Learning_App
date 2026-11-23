// How to move pieces: the chess commands in seven short topics (left), and the pointer
// explorer (right) - a board where you click a square to see its board[] index and how far
// every other square is from it, pick a piece to see where it can go, and hover a square
// to see that move written both ways: with move(), and with pick_up() and hand.
// It opens from the Chess screen, from any chess puzzle and from 1v1 chess, and Back
// returns to wherever it was opened from.
#include "app.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct { bool code; const char *text; } Block;
typedef struct { const char *title; Block blocks[8]; } Topic;

static const Topic TOPICS[] = {
    { "The board", {
        { false, "The board is an array of 64 squares, Square board[64]. board[0] is a1 (White's bottom-left corner), board[1] is b1, "
                 "board[7] is h1, board[8] is a2 ... and board[63] is h8. So one square to the right is +1, and one rank up the "
                 "board (towards Black) is +8." },
        { true,  "typedef struct {\n"
                 "    char piece;   // 'K' 'Q' 'R' 'B' 'N' 'P' White,\n"
                 "                  // 'k' 'q' 'r' 'b' 'n' 'p' Black, '.' empty\n"
                 "    int  file;    // 0 to 7: the a-file to the h-file\n"
                 "    int  rank;    // 0 to 7: rank 1 to rank 8\n"
                 "} Square;\n"
                 "\n"
                 "Square board[64];" },
        { false, "Your code never changes the board itself - you read it through pointers, and move pieces with one of the two "
                 "ways in the next topics. Click any square on the board on the right to see its index, and how far every other "
                 "square is from it." },
    } },
    { "Finding squares", {
        { false, "Every command works with pointers to squares. Get one by naming the square, by looking for a piece, or by "
                 "stepping from another pointer." },
        { true,  "Square *e4 = at(\"e4\");     // the square e4 (NULL if there's no such square)\n"
                 "Square *n = find('N');      // the first square from a1 with a White knight, or NULL\n"
                 "Square *up = e4 + 8;        // one rank up: e5\n"
                 "\n"
                 "char what = e4->piece;      // what's on it ('.' for nothing)\n"
                 "int file = e4->file;        // 4 (a=0 ... h=7)\n"
                 "\n"
                 "is_mine(p)    is_theirs(p)  // one of your pieces / the other side's\n"
                 "is_white(p)   is_black(p)" },
        { false, "To look at every square, walk a pointer along the array:" },
        { true,  "for (Square *p = board; p < board + 64; p++)\n"
                 "    if (is_mine(p))\n"
                 "        printf(\"%c on board[%d]\\n\", p->piece, (int)(p - board));" },
    } },
    { "Way 1: move()", {
        { false, "Give move() two pointers: the square the piece is on, and the square it goes to. Name the squares, or step "
                 "there with pointer arithmetic - both give a pointer to the same square, so use whichever is easier." },
        { true,  "move(at(\"e2\"), at(\"e4\"));       // name both squares\n"
                 "\n"
                 "Square *pawn = at(\"e2\");\n"
                 "move(pawn, pawn + 16);           // or step: two ranks up\n"
                 "\n"
                 "Square *knight = at(\"g1\");\n"
                 "move(knight, at(\"f3\"));          // or mix them" },
        { false, "The steps, from any square:  +1 right,  -1 left,  +8 up (towards Black),  -8 down (towards White),  "
                 "+9 up-right,  +7 up-left,  -7 down-right,  -9 down-left.  A knight jumps +17 +15 +10 +6 or -6 -10 -15 -17. "
                 "Pick a piece on the board on the right to see its steps from any square." },
    } },
    { "Way 2: pick_up() and hand", {
        { false, "hand is a pointer the board gives you. pick_up() puts it over one of your pieces - then you reassign hand to "
                 "point at the square the piece should go to. When your program ends, the piece is put down wherever hand points." },
        { true,  "pick_up(at(\"g1\"));     // hand now points at g1, where the knight is\n"
                 "hand = at(\"f3\");       // reassign the pointer: the knight goes to f3" },
        { false, "Steps work on hand too, so you can walk it across the board:" },
        { true,  "pick_up(at(\"a1\"));     // a rook\n"
                 "hand += 8;             // one rank up...\n"
                 "hand += 8;             // ...and another: the rook goes to a3\n"
                 "\n"
                 "pick_up(at(\"c1\"));     // a bishop\n"
                 "hand = hand + 9 * 3;   // three squares up-right: f4" },
        { false, "Several moves in one program (in the puzzles): put_down() plays the move straight away, then pick up the next "
                 "piece. Don't declare your own variable called hand - use the board's." },
        { true,  "pick_up(at(\"b2\"));  hand += 8 * 5;  put_down();   // b7, played now\n"
                 "pick_up(at(\"a1\"));  hand = at(\"a8\");             // played when the program ends" },
    } },
    { "Special moves", {
        { false, "Castling: move the king two squares towards a rook - move(at(\"e1\"), at(\"g1\")) - and the rook jumps over "
                 "by itself. Only if neither has moved, nothing stands between them, and the king isn't in check, doesn't pass "
                 "through an attacked square and doesn't land on one." },
        { false, "En passant: when the other side's pawn jumps two squares and lands right beside your pawn, your pawn can take "
                 "it on the very next move, by moving diagonally onto the square it skipped." },
        { false, "Promotion: a pawn that reaches the last rank becomes a queen with move() or with hand. To turn it into "
                 "something else, use promote():" },
        { true,  "promote(at(\"e7\"), at(\"e8\"), 'N');   // a knight instead of a queen\n"
                 "                                     // 'Q' 'R' 'B' or 'N'" },
    } },
    { "Helpers for smart code", {
        { false, "These let your program look before it moves - in 1v1 you can write code that chooses its own move." },
        { true,  "legal(from, to)      // 1 if that move is allowed right now\n"
                 "mates(from, to)      // 1 if that move would be checkmate\n"
                 "their_from()         // the other side's last move: where it came from...\n"
                 "their_to()           // ...and where it went (NULL before it has moved)\n"
                 "moves_left()         // your moves left (in a puzzle; 1 in a 1v1 turn)" },
        { false, "For example, a program that finds a checkmate in one, wherever it is:" },
        { true,  "for (Square *from = board; from < board + 64; from++) {\n"
                 "    if (!is_mine(from)) continue;\n"
                 "    for (Square *to = board; to < board + 64; to++)\n"
                 "        if (mates(from, to)) { move(from, to); return 0; }\n"
                 "}" },
    } },
    { "What gets refused", {
        { false, "Every move follows the real rules of chess. A move is refused - and isn't played - if the piece doesn't move "
                 "that way, if something is in the way (only knights jump over pieces), if it lands on one of your own pieces, "
                 "or if it would leave your own king in check." },
        { false, "Pointers are checked too: NULL, or a pointer outside board[0] to board[63], is refused. So is moving the "
                 "other side's piece, and writing to the board yourself - read it through your pointers, never write to it." },
        { false, "In a 1v1 turn your program makes exactly one move. In a puzzle you have a set number of moves, and the board "
                 "answers each one." },
        { false, "When a move is refused you're told exactly why - fix the code and run it again." },
    } },
};

#define TOPIC_COUNT ((int)(sizeof(TOPICS) / sizeof(TOPICS[0])))

static Screen back = SCREEN_CHESS;
static int    topic, selected = 12, kind = 1, hovered = -1;
static float  scroll;

void OpenChessHelp(Screen from)
{
    back = from;
    scroll = 0;
    app.screen = SCREEN_CHESS_HELP;
}

// ---------------------------------------------------------------- the pointer explorer

static const char *KINDS[6] = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
static const char KIND_LETTERS[6] = { 'P', 'N', 'B', 'R', 'Q', 'K' };

static void Name(int sq, char *out) { out[0] = (char)('a' + sq % 8); out[1] = (char)('1' + sq / 8); out[2] = '\0'; }

// Where a White piece on an empty board can go from `from`: 1 a move, 2 a pawn's capture
static void Reach(int from, int k, unsigned char *out)
{
    memset(out, 0, 64);
    int f = from % 8, r = from / 8;
    if (k == 0) {
        if (r < 7) out[from + 8] = 1;
        if (r == 1) out[from + 16] = 1;
        if (r < 7 && f > 0) out[from + 7] = 2;
        if (r < 7 && f < 7) out[from + 9] = 2;
        return;
    }
    if (k == 1) {
        static const int kn[8][2] = { { 1, 2 }, { 2, 1 }, { 2, -1 }, { 1, -2 }, { -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 } };
        for (int i = 0; i < 8; i++) {
            int nf = f + kn[i][0], nr = r + kn[i][1];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) out[nr * 8 + nf] = 1;
        }
        return;
    }
    static const int dir[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
    int a = k == 2 ? 4 : 0, b = k == 3 ? 4 : 8;
    for (int i = a; i < b; i++) {
        int nf = f + dir[i][0], nr = r + dir[i][1];
        while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
            out[nr * 8 + nf] = 1;
            if (k == 5) break;
            nf += dir[i][0];
            nr += dir[i][1];
        }
    }
}

static void DrawExplorer(Rectangle r)
{
    DrawRectangleRounded(r, 0.03f, 8, COL_PANEL);
    float x = r.x + 18, w = r.width - 36, y = r.y + 14;
    UiText("Pointer explorer", x, y, 20, COL_TEXT);
    y += 26;
    UiText("Click a square, pick a piece, then hover where it can go.", x, y, 14, COL_MUTED);
    y += 26;

    // the piece to try
    float cw = (w - 5 * 6) / 6;
    for (int i = 0; i < 6; i++) {
        Rectangle c = { x + i * (cw + 6), y, cw, 30 };
        bool hover = false;
        if (UiClickArea(c, &hover)) kind = i;
        DrawRectangleRounded(c, 0.4f, 6, kind == i ? ColorAlpha(COL_CHESS, 0.3f) : hover ? COL_PANEL_HI : (Color){ 36, 39, 51, 255 });
        if (kind == i) DrawRectangleRoundedLinesEx(c, 0.4f, 6, 2, COL_CHESS);
        UiTextCentered(KINDS[i], c.x + c.width / 2, c.y + 6, 15, kind == i ? COL_TEXT : COL_MUTED);
    }
    y += 42;

    // the board: rank 8 at the top, like the 3D board
    float infoH = 132, size = fminf(w, r.y + r.height - infoH - 16 - y);
    float s = floorf(size / 8);
    size = s * 8;
    float bx = x + (w - size) / 2, by = y;
    unsigned char reach[64];
    Reach(selected, kind, reach);
    hovered = -1;
    Vector2 mouse = GetMousePosition();
    for (int sq = 0; sq < 64; sq++) {
        int f = sq % 8, rk = sq / 8;
        Rectangle q = { bx + f * s, by + (7 - rk) * s, s, s };
        bool light = (f + rk) % 2 == 1;
        Color c = light ? (Color){ 200, 178, 140, 255 } : (Color){ 128, 92, 64, 255 };
        if (reach[sq] == 1) c = light ? (Color){ 130, 205, 150, 255 } : (Color){ 70, 150, 100, 255 };
        if (reach[sq] == 2) c = light ? (Color){ 235, 180, 110, 255 } : (Color){ 190, 125, 60, 255 };
        if (sq == selected) c = (Color){ 86, 156, 255, 255 };
        bool hover = CheckCollisionPointRec(mouse, q);
        if (hover) { hovered = sq; c = ColorBrightness(c, 0.15f); }
        DrawRectangleRec(q, c);
        if (hover && UiClickArea(q, NULL)) selected = sq;
        char nm[3];
        Name(sq, nm);
        Color ink = (Color){ 30, 26, 22, 200 };
        UiText(nm, q.x + 3, q.y + 1, fmaxf(10, s * 0.2f), ink);
        UiText(TextFormat("[%d]", sq), q.x + s - UiTextWidth(TextFormat("[%d]", sq), fmaxf(10, s * 0.18f)) - 3, q.y + s - fmaxf(10, s * 0.18f) - 3,
               fmaxf(10, s * 0.18f), ink);
        if (sq == selected) {
            char letter[2] = { KIND_LETTERS[kind], '\0' };
            UiTextCentered(letter, q.x + s / 2, q.y + s / 2 - s * 0.2f, s * 0.42f, WHITE);
        } else if (reach[sq]) {
            UiTextCentered(TextFormat("%+d", sq - selected), q.x + s / 2, q.y + s / 2 - s * 0.14f, s * 0.28f, (Color){ 20, 30, 24, 255 });
        }
    }
    DrawRectangleLinesEx((Rectangle){ bx - 2, by - 2, size + 4, size + 4 }, 2, (Color){ 70, 52, 36, 255 });

    // what the hovered square means
    float iy = by + size + 12;
    char from[3], to[3];
    Name(selected, from);
    if (hovered < 0 || hovered == selected) {
        UiWrapped(TextFormat("%s is board[%d]. Hover over another square to see the pointer step from %s to it%s.", from, selected, from,
                             kind == 0 ? " (these are White's pawn moves - Black's go the other way, -8)" : ""),
                  x, iy, w, 15, COL_MUTED);
        return;
    }
    Name(hovered, to);
    int step = hovered - selected;
    UiText(TextFormat("%s is board[%d]  -  from %s that's %d - %d = %+d", to, hovered, from, hovered, selected, step), x, iy, 15, COL_TEXT);
    iy += 24;
    if (!reach[hovered]) {
        UiWrapped(TextFormat("A %s on %s can't get to %s in one move.", KINDS[kind], from, to), x, iy, w, 15, COL_MUTED);
        return;
    }
    UiCode(TextFormat("move(at(\"%s\"), at(\"%s\"));\n"
                      "move(p, p %c %d);              // p = at(\"%s\")\n"
                      "pick_up(at(\"%s\"));  hand %s %d;",
                      from, to, step < 0 ? '-' : '+', step < 0 ? -step : step, from, from, step < 0 ? "-=" : "+=", step < 0 ? -step : step),
           x, iy, 15, COL_CODE_TEXT, w);
    if (reach[hovered] == 2) UiText("(a pawn only moves this way when it captures)", x, iy + 66, 14, COL_GOLD);
}

// ---------------------------------------------------------------- the screen

void ScreenChessHelp(void)
{
    if (IsKeyPressed(KEY_ESCAPE)) { app.screen = back; return; }
    DrawTopBar("How to move pieces");
    if (BackButton("< Back")) { app.screen = back; return; }

    float top = 84, bottom = SCREEN_H - 20.0f;
    float navW = 250, exW = fminf(560, fmaxf(400, SCREEN_W * 0.34f));
    Rectangle nav = { 20, top, navW, bottom - top };
    Rectangle ex = { SCREEN_W - 20 - exW, top, exW, bottom - top };
    Rectangle body = { nav.x + navW + 16, top, ex.x - 16 - (nav.x + navW + 16), bottom - top };

    // the topics
    DrawRectangleRounded(nav, 0.04f, 8, COL_PANEL);
    float y = nav.y + 14;
    UiText("Commands", nav.x + 16, y, 15, COL_MUTED);
    y += 28;
    for (int i = 0; i < TOPIC_COUNT; i++) {
        Rectangle b = { nav.x + 10, y, navW - 20, 44 };
        bool hover = false;
        if (UiClickArea(b, &hover) && topic != i) { topic = i; scroll = 0; }
        DrawRectangleRounded(b, 0.3f, 6, topic == i ? ColorAlpha(COL_CHESS, 0.25f) : hover ? COL_PANEL_HI : (Color){ 0, 0, 0, 0 });
        UiText(TextFormat("%d.  %s", i + 1, TOPICS[i].title), b.x + 12, b.y + 12, 17, topic == i ? COL_TEXT : COL_MUTED);
        y += 50;
    }

    // the topic itself, scrolling when it's long
    DrawRectangleRounded(body, 0.03f, 8, COL_PANEL);
    const Topic *t = &TOPICS[topic];
    float x = body.x + 24, w = body.width - 48;
    if (CheckCollisionPointRec(GetMousePosition(), body)) scroll -= GetMouseWheelMove() * 40;
    if (scroll < 0) scroll = 0;
    BeginScissorMode((int)body.x, (int)body.y + 4, (int)body.width, (int)body.height - 80);   // (the buttons sit underneath)
    y = body.y + 20 - scroll;
    UiText(t->title, x, y, 26, COL_TEXT);
    y += 44;
    for (int i = 0; i < 8 && t->blocks[i].text; i++) {
        if (t->blocks[i].code) y += UiCodeBlock(t->blocks[i].text, x, y, w, 15) + 14;
        else y += UiWrapped(t->blocks[i].text, x, y, w, 17, COL_TEXT) + 14;
    }
    float used = y + scroll - body.y;
    EndScissorMode();
    if (scroll > used - body.height + 90) scroll = fmaxf(0, used - body.height + 90);

    // next / previous
    if (topic + 1 < TOPIC_COUNT && UiButton((Rectangle){ body.x + body.width - 180, body.y + body.height - 60, 164, 44 }, "Next  >", COL_PANEL_HI)) {
        topic++;
        scroll = 0;
    }
    if (topic > 0 && UiButton((Rectangle){ body.x + 16, body.y + body.height - 60, 164, 44 }, "<  Previous", COL_PANEL_HI)) {
        topic--;
        scroll = 0;
    }

    DrawExplorer(ex);
}
