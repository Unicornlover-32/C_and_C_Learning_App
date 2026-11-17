// The chess puzzles. Every puzzle is played with the chess library (game/cfd_chess.h) in front
// of the student's code: White moves with pointers into the board, Black answers, and the
// library checks the goal (see the top of cfd_chess.h for the input format).
// region is the difficulty: 0 easy, 1 medium, 2 hard, 3 very hard.
// make test runs every solution (it must solve every test) and starter (it must not).
#include "missions.h"
#include <stddef.h>

#define CHESS_API \
    "Square board[64]             a1 is board[0] ... h8 is board[63]\n" \
    "Square *at(\"e4\")             a pointer to the square e4\n" \
    "Square *find('N')            the first square holding that piece, or NULL\n" \
    "p->piece                     'K' 'Q' 'R' 'B' 'N' 'P' White, lower case Black, '.' empty\n" \
    "p->file  p->rank             0 to 7 (a-h, ranks 1-8)\n" \
    "Way 1: move(from, to)        your move - Black answers at once\n" \
    "Way 2: pick_up(square)       pick up one of your pieces, then point\n" \
    "       hand = at(\"e4\");      the pointer hand at its new square\n" \
    "       hand += 16;           (steps work too) - played when your\n" \
    "       put_down();           program ends, or now with put_down()\n" \
    "promote(from, to, 'Q')       a pawn to the last rank: 'Q' 'R' 'B' or 'N'\n" \
    "is_white(p)  is_black(p)     whose piece is on that square\n" \
    "legal(from, to)              1 if that move is allowed right now\n" \
    "mates(from, to)              1 if that move is checkmate\n" \
    "their_from()  their_to()     Black's last answer (NULL before it moves)\n" \
    "moves_left()                 how many moves you have left\n" \
    "pointer steps:  +1 right  -1 left  +8 up  -8 down  +9 / +7 up the diagonals"

const Mission CHESS_PUZZLES[] = {
{
    .id = "chess-backrank", .level = NULL, .region = 0,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Back Rank",
    .brief =
        "Black's king is trapped behind its own pawns. Checkmate it in one move.\n"
        "\n"
        "at(\"a1\") gives you a pointer to the square a1, where your rook stands. Move the rook all the way up the a-file to a8: move(at(\"a1\"), at(\"a8\"));\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "One line: move(at(\"a1\"), at(\"a8\")); - the rook checks along the 8th rank and the pawns block every escape.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    move(at(\"a1\"), at(\"a8\"));\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 6k1/5ppp/8/8/8/8/8/R5K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-file", .level = NULL, .region = 0,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Up the File",
    .brief =
        "Black left a rook on d6, right in front of your queen. Take it.\n"
        "\n"
        "This time, use pointer arithmetic. find('Q') gives you a pointer to your queen's square. Every +8 is one rank up the board, so the rook is 5 ranks up: queen + 8 * 5.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Square *queen = find('Q');  then  move(queen, queen + 8 * 5);  - d1 is board[3], and d6 is board[43].",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *queen = find('Q');\n"
        "    move(queen, queen + 8 * 5);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 6k1/5ppp/3r4/8/8/8/5PPP/3Q2K1 w - -\n"
          "GOAL TAKE r 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-knight", .level = NULL, .region = 0,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Knight's Leap",
    .brief =
        "Black's queen is on d5, and your knight on c3 can reach it.\n"
        "\n"
        "A knight jumps in an L: two squares one way, then one to the side. In pointer steps that's +6, +10, +15 or +17 (or minus those). Two ranks up (+16) and one file right (+1) is +17. Find your knight with find('N') and jump it onto the queen.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Square *knight = find('N');  move(knight, knight + 17);",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *knight = find('N');\n"
        "    move(knight, knight + 17);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 6k1/5ppp/8/3q4/8/2N5/5PPP/6K1 w - -\n"
          "GOAL TAKE q 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-smothered", .level = NULL, .region = 1,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Smothered",
    .brief =
        "Black's king is boxed in by its own rook and pawns. One knight jump finishes it: a smothered mate.\n"
        "\n"
        "Your knight is on g5. Which of its jumps checks the king on h8? Work out the pointer step and checkmate in one move.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "g5 is board[38] and f7 is board[53]: a step of +15 (two ranks up, one file left). From f7 the knight attacks h8.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *knight = find('N');\n"
        "    move(knight, knight + 15);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 6rk/6pp/8/6N1/8/8/8/6K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-fork", .level = NULL, .region = 1,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Royal Fork",
    .brief =
        "Win Black's queen in two moves.\n"
        "\n"
        "Jump your knight from b5 to c7. From there it checks the king on e8 AND attacks the queen on a8 at the same time - a fork. Black has to save its king, so on your second move the knight takes the queen.\n"
        "\n"
        "Keep a pointer to your knight, and remember it has moved: after the first jump it is at knight + 17.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Square *knight = at(\"b5\");  move(knight, knight + 17);  then  move(knight + 17, find('q'));",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *knight = at(\"b5\");\n"
        "    move(knight, knight + 17);        // c7: check, and the queen is attacked\n"
        "    knight = knight + 17;\n"
        "    move(knight, find('q'));          // the king had to move - take the queen\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN q3k3/8/8/1N6/8/8/5PPP/6K1 w - -\n"
          "GOAL TAKE q 2\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-roller", .level = NULL, .region = 1,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Rook Roller",
    .brief =
        "Checkmate in two moves with your two rooks.\n"
        "\n"
        "First lift the rook on b2 up to b7: it seals off the 7th rank, so Black's king can only shuffle along the 8th. Then the other rook goes all the way up the a-file to a8 - checkmate.\n"
        "\n"
        "Work out both moves as pointer steps from where each rook starts.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Square *low = at(\"b2\"), *high = at(\"a1\");  move(low, low + 8 * 5);  move(high, high + 8 * 7);",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *low = at(\"b2\"), *high = at(\"a1\");\n"
        "    move(low, low + 8 * 5);      // b7: the king can't leave the 8th rank\n"
        "    move(high, high + 8 * 7);    // a8: checkmate\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 7k/8/8/8/8/8/1R6/R5K1 w - -\n"
          "GOAL MATE 2\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-promote", .level = NULL, .region = 2,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Queen Me",
    .brief =
        "Your pawn on e7 is one step from the last rank. A pawn that reaches the last rank is promoted: it becomes a queen, rook, bishop or knight.\n"
        "\n"
        "Use promote(from, to, 'Q') to push it to e8 and make a queen - it checkmates Black at once along the back rank.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Square *pawn = find('P');  promote(pawn, pawn + 8, 'Q');",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *pawn = find('P');\n"
        "    promote(pawn, pawn + 8, 'Q');\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 6k1/4Pppp/8/8/8/8/8/6K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-enpassant", .level = NULL, .region = 2,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Sneaky Pawn",
    .brief =
        "Black's pawn has just jumped two squares, from d7 to d5, to slip past your pawn on e5. Capture it anyway!\n"
        "\n"
        "A pawn that jumps two squares can be taken 'en passant' (in passing) on the very next move, as if it had only moved one: your pawn moves diagonally to d6, the square it skipped, and the black pawn is removed.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "d6 is one rank up and one file left of e5: a step of +7.  Square *pawn = at(\"e5\");  move(pawn, pawn + 7);",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *pawn = at(\"e5\");\n"
        "    move(pawn, pawn + 7);     // to d6, taking the pawn on d5 en passant\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "FEN 6k1/8/8/3pP3/8/8/8/6K1 w - d6\n"
          "GOAL TAKE p 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-hungry", .level = NULL, .region = 2,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Hungry Pieces",
    .brief =
        "Black's queen is hanging - but in every test it's on a different square, and a different one of your pieces can take it. Write code that works for any position.\n"
        "\n"
        "find('q') points at the queen. Then walk a pointer over the whole board, from board to board + 64, and move the first white piece that can legally take it:\n"
        "\n"
        "for (Square *p = board; p < board + 64; p++)\n"
        "    if (is_white(p) && legal(p, queen)) ...\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Inside the loop: if (is_white(p) && legal(p, queen)) { move(p, queen); return 0; }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    Square *queen = find('q');\n"
        "    for (Square *p = board; p < board + 64; p++) {\n"
        "        if (is_white(p) && legal(p, queen)) {\n"
        "            move(p, queen);\n"
        "            return 0;\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "FEN 6k1/6q1/8/8/8/8/1B3PPP/6K1 w - -\n"
          "GOAL TAKE q 1\n"
          "END\n", NULL },
        { "FEN 6k1/3q1ppp/8/8/8/8/5PPP/3R2K1 w - -\n"
          "GOAL TAKE q 1\n"
          "END\n", NULL },
        { "FEN 6k1/5ppp/8/8/2q5/8/1N3PPP/6K1 w - -\n"
          "GOAL TAKE q 1\n"
          "END\n", NULL },
        { "FEN 6k1/5ppp/8/8/8/4q3/3P1PPP/6K1 w - -\n"
          "GOAL TAKE q 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-matefinder", .level = NULL, .region = 3,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Mate Finder",
    .brief =
        "Every test is a different position with a checkmate in one move somewhere - a rook, a knight, a queen, a pawn that promotes... Write ONE program that finds it every time.\n"
        "\n"
        "Use two pointers: from walks over all 64 squares looking for your pieces, and for each one to walks over all 64 squares too. mates(from, to) tells you if that move is checkmate.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "for (Square *from = board; from < board + 64; from++) - skip squares that aren't white - then for (Square *to = board; to < board + 64; to++) if (mates(from, to)) { move(from, to); return 0; }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    for (Square *from = board; from < board + 64; from++) {\n"
        "        if (!is_white(from)) continue;\n"
        "        for (Square *to = board; to < board + 64; to++) {\n"
        "            if (mates(from, to)) {\n"
        "                move(from, to);\n"
        "                return 0;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 5,
    .tests = {
        { "FEN 6k1/5ppp/8/8/8/8/8/R5K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
        { "FEN 6rk/6pp/8/6N1/8/8/8/6K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
        { "FEN 7k/Q7/6K1/8/8/8/8/8 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
        { "FEN k7/pp6/8/8/8/8/8/4R1K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
        { "FEN 6k1/4Pppp/8/8/8/8/8/6K1 w - -\n"
          "GOAL MATE 1\n"
          "END\n", NULL },
    },
},

{
    .id = "chess-pawnrace", .level = NULL, .region = 3,
    .role = ROLE_CHESS, .mode = MODE_CHESS, .lang = LANG_C,
    .title = "Pawn Race",
    .brief =
        "Promote a pawn - but in every test only one of your pawns has a clear road to the last rank. The others are blocked by a piece somewhere in front of them.\n"
        "\n"
        "For each white pawn ('P'), walk a pointer up its file, +8 at a time, until rank 7 (p->rank == 7) and check every square on the way is empty ('.'). When you find a clear road, push that pawn one square at a time until it promotes. Remember the pawn has moved: after move(pawn, pawn + 8) it is at pawn + 8.\n"
        "\n"
        "You play White. There are two ways to move: move(from, to), or pick_up() a piece and point hand at its new square - press Commands to see them all. Every piece moves in its normal pattern (anything else is refused, with the reason), and Black answers each of your moves straight away.",
    .api = CHESS_API,
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // your moves go here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Checking the road: for (Square *s = p; s->rank < 7; ) { s += 8; if (s->piece != '.') clear = 0; }  Pushing: while (pawn->rank < 7) { move(pawn, pawn + 8); pawn += 8; }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    for (Square *p = board; p < board + 64; p++) {\n"
        "        if (p->piece != 'P') continue;\n"
        "\n"
        "        int clear = 1;\n"
        "        for (Square *s = p; s->rank < 7; ) {\n"
        "            s += 8;\n"
        "            if (s->piece != '.') clear = 0;\n"
        "        }\n"
        "        if (!clear) continue;\n"
        "\n"
        "        Square *pawn = p;\n"
        "        while (pawn->rank < 7) {\n"
        "            move(pawn, pawn + 8);\n"
        "            pawn += 8;\n"
        "        }\n"
        "        return 0;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "FEN 7k/6pp/8/8/8/p7/P1P5/4K3 w - -\n"
          "GOAL PROMOTE 6\n"
          "END\n", NULL },
        { "FEN 7k/6pp/8/1p2P3/1P1p4/3P4/8/4K3 w - -\n"
          "GOAL PROMOTE 3\n"
          "END\n", NULL },
        { "FEN 7k/B5pp/P1P5/5n2/8/8/5P2/4K3 w - -\n"
          "GOAL PROMOTE 2\n"
          "END\n", NULL },
    },
},

};

int ChessPuzzleCount(void) { return (int)(sizeof(CHESS_PUZZLES) / sizeof(CHESS_PUZZLES[0])); }
