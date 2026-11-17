/* Chess Puzzles - the board's half of the game.
 *
 * The app puts this file in front of your code. It sets up the position from the input,
 * plays Black's answers, writes every move to cfd_events.txt for the app to animate, and
 * decides at the end whether the puzzle was solved. Every move follows the real rules of
 * chess: each piece moves in its own pattern, only knights jump over pieces, you can never
 * leave your own king in check, and castling, en passant and promotion all work.
 *
 * The board is an array of 64 squares, and you move pieces with pointers into it:
 *     board[0] is a1, board[1] is b1 ... board[7] is h1, board[8] is a2 ... board[63] is h8
 * so from any square, +1 is one file to the right, +8 is one rank up (towards Black),
 * +9 and +7 are the diagonals up, and a knight jumps +6 +10 +15 +17 (or minus those).
 *
 * Two ways to move a piece - use whichever you like:
 *     1. move(from, to)        e.g. move(at("e2"), at("e4"));  or  move(p, p + 16);
 *     2. pick_up(square), then reassign the pointer hand to the square the piece goes to:
 *            pick_up(at("g1"));  hand = at("f3");        (or hand += 15;)
 *        the piece is put down where hand points when your program ends, or at put_down().
 *
 * Puzzle format (the test's input), one item per line, finished by END:
 *     FEN <placement> <side> <castling> <en passant>   the position (White to move)
 *     GOAL MATE n          checkmate Black within n of your moves
 *     GOAL TAKE p n        capture Black's piece p (q r b n p) within n of your moves
 *     GOAL PROMOTE n       promote a pawn within n of your moves
 *     LINE e2e4 e7e5 ...   (optional) the puzzle's main line: while you follow it, Black
 *                          answers with the moves listed; otherwise Black finds its own
 *
 * Two more modes, switched on before including this file:
 *     CFD_DUEL        a 1v1 game: your program plays whichever side is to move in the FEN
 *                     and makes exactly one move; "LAST e7e5" gives the other side's last move
 *     CFD_RULES_ONLY  just the rules (the app uses them for its own games)
 */
#ifndef CFD_CHESS_H
#define CFD_CHESS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef CFD_RULES_ONLY
typedef struct {
    char piece;   /* 'K' 'Q' 'R' 'B' 'N' 'P' are White's, 'k' 'q' 'r' 'b' 'n' 'p' Black's, '.' empty */
    int  file;    /* 0 to 7: the a-file to the h-file */
    int  rank;    /* 0 to 7: rank 1 to rank 8 */
} Square;

Square board[64];
#endif

/* ------------------------------------------------------------------ the rules */

typedef struct { char sq[64]; int white, castle, ep; } CfdPos;   /* castle bits: 1 K, 2 Q, 4 k, 8 q */
typedef struct { int from, to; char promo; } CfdMove;              /* promo: 'Q' 'R' 'B' 'N', or 0 */

#define CFD_MAXMOVES 256
enum { CFD_GOAL_MATE = 1, CFD_GOAL_TAKE, CFD_GOAL_PROMOTE };

/* (rules are static inline: a file that only needs some of them gets no warnings) */
#define CFD_RULE static inline

static const int cfd_kn[8][2] = { { 1, 2 }, { 2, 1 }, { 2, -1 }, { 1, -2 }, { -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 } };
static const int cfd_dir[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };

CFD_RULE int  cfd_isw(char c) { return c >= 'A' && c <= 'Z'; }
CFD_RULE int  cfd_isb(char c) { return c >= 'a' && c <= 'z'; }
CFD_RULE int  cfd_own(char c, int white) { return white ? cfd_isw(c) : cfd_isb(c); }
CFD_RULE char cfd_kind(char c) { return (char)(cfd_isb(c) ? c - 'a' + 'A' : c); }
CFD_RULE int  cfd_on(int f, int r) { return f >= 0 && f < 8 && r >= 0 && r < 8; }

CFD_RULE void cfd_name(int sq, char *out)
{
    out[0] = (char)('a' + sq % 8);
    out[1] = (char)('1' + sq / 8);
    out[2] = '\0';
}

CFD_RULE const char *cfd_piece_name(char c)
{
    switch (cfd_kind(c)) {
        case 'K': return "king";
        case 'Q': return "queen";
        case 'R': return "rook";
        case 'B': return "bishop";
        case 'N': return "knight";
        case 'P': return "pawn";
    }
    return "piece";
}

/* Is square sq attacked by the given side? */
CFD_RULE int cfd_attacked(const CfdPos *p, int sq, int byWhite)
{
    int f = sq % 8, r = sq / 8, i;
    int pr = byWhite ? r - 1 : r + 1;
    for (i = -1; i <= 1; i += 2)
        if (cfd_on(f + i, pr) && p->sq[pr * 8 + f + i] == (byWhite ? 'P' : 'p')) return 1;
    for (i = 0; i < 8; i++) {
        int nf = f + cfd_kn[i][0], nr = r + cfd_kn[i][1];
        if (cfd_on(nf, nr) && p->sq[nr * 8 + nf] == (byWhite ? 'N' : 'n')) return 1;
    }
    for (i = 0; i < 8; i++) {
        int df = cfd_dir[i][0], dr = cfd_dir[i][1], nf = f + df, nr = r + dr, dist = 1;
        int straight = (df == 0 || dr == 0);
        while (cfd_on(nf, nr)) {
            char c = p->sq[nr * 8 + nf];
            if (c != '.') {
                if (cfd_own(c, byWhite)) {
                    char k = cfd_kind(c);
                    if (k == 'Q' || (straight && k == 'R') || (!straight && k == 'B') || (dist == 1 && k == 'K')) return 1;
                }
                break;
            }
            nf += df;
            nr += dr;
            dist++;
        }
    }
    return 0;
}

CFD_RULE int cfd_king(const CfdPos *p, int white)
{
    int i;
    for (i = 0; i < 64; i++) if (p->sq[i] == (white ? 'K' : 'k')) return i;
    return -1;
}

CFD_RULE int cfd_in_check(const CfdPos *p, int white)
{
    int k = cfd_king(p, white);
    return k >= 0 && cfd_attacked(p, k, !white);
}

CFD_RULE void cfd_add(CfdMove *list, int *n, int from, int to, char promo)
{
    if (*n >= CFD_MAXMOVES) return;
    list[*n].from = from;
    list[*n].to = to;
    list[*n].promo = promo;
    (*n)++;
}

CFD_RULE void cfd_add_pawn(CfdMove *list, int *n, int from, int to, int last)
{
    if (to / 8 == last) {
        cfd_add(list, n, from, to, 'Q');
        cfd_add(list, n, from, to, 'R');
        cfd_add(list, n, from, to, 'B');
        cfd_add(list, n, from, to, 'N');
    } else {
        cfd_add(list, n, from, to, 0);
    }
}

/* Every move the side to move could make, before checking it doesn't leave its king in check */
CFD_RULE int cfd_pseudo(const CfdPos *p, CfdMove *list)
{
    int n = 0, w = p->white, sq, i;
    for (sq = 0; sq < 64; sq++) {
        char c = p->sq[sq], k;
        int f = sq % 8, r = sq / 8;
        if (!cfd_own(c, w)) continue;
        k = cfd_kind(c);
        if (k == 'P') {
            int dir = w ? 1 : -1, start = w ? 1 : 6, last = w ? 7 : 0, r1 = r + dir;
            if (r1 < 0 || r1 > 7) continue;
            if (p->sq[r1 * 8 + f] == '.') {
                cfd_add_pawn(list, &n, sq, r1 * 8 + f, last);
                if (r == start && p->sq[(r + 2 * dir) * 8 + f] == '.') cfd_add(list, &n, sq, (r + 2 * dir) * 8 + f, 0);
            }
            for (i = -1; i <= 1; i += 2) {
                int to;
                char t;
                if (f + i < 0 || f + i > 7) continue;
                to = r1 * 8 + f + i;
                t = p->sq[to];
                if ((t != '.' && !cfd_own(t, w)) || to == p->ep) cfd_add_pawn(list, &n, sq, to, last);
            }
        } else if (k == 'N') {
            for (i = 0; i < 8; i++) {
                int nf = f + cfd_kn[i][0], nr = r + cfd_kn[i][1];
                if (cfd_on(nf, nr) && !cfd_own(p->sq[nr * 8 + nf], w)) cfd_add(list, &n, sq, nr * 8 + nf, 0);
            }
        } else {
            int from = k == 'B' ? 4 : 0, to = k == 'R' ? 4 : 8, slide = k != 'K';
            for (i = from; i < to; i++) {
                int nf = f + cfd_dir[i][0], nr = r + cfd_dir[i][1];
                while (cfd_on(nf, nr)) {
                    char t = p->sq[nr * 8 + nf];
                    if (cfd_own(t, w)) break;
                    cfd_add(list, &n, sq, nr * 8 + nf, 0);
                    if (t != '.' || !slide) break;
                    nf += cfd_dir[i][0];
                    nr += cfd_dir[i][1];
                }
            }
        }
    }
    /* castling: neither piece has moved, nothing in between, and the king doesn't start,
       pass or land on an attacked square */
    {
        int home = w ? 4 : 60, kb = w ? 1 : 4, qb = w ? 2 : 8;
        char K = w ? 'K' : 'k', R = w ? 'R' : 'r';
        if (p->sq[home] == K && !cfd_attacked(p, home, !w)) {
            if ((p->castle & kb) && p->sq[home + 3] == R && p->sq[home + 1] == '.' && p->sq[home + 2] == '.' &&
                !cfd_attacked(p, home + 1, !w) && !cfd_attacked(p, home + 2, !w))
                cfd_add(list, &n, home, home + 2, 0);
            if ((p->castle & qb) && p->sq[home - 4] == R && p->sq[home - 1] == '.' && p->sq[home - 2] == '.' &&
                p->sq[home - 3] == '.' && !cfd_attacked(p, home - 1, !w) && !cfd_attacked(p, home - 2, !w))
                cfd_add(list, &n, home, home - 2, 0);
        }
    }
    return n;
}

CFD_RULE void cfd_apply(CfdPos *p, CfdMove m)
{
    char c = p->sq[m.from], k = cfd_kind(c);
    int w = p->white;
    if (k == 'P' && m.to == p->ep && p->sq[m.to] == '.' && m.to % 8 != m.from % 8)
        p->sq[m.to + (w ? -8 : 8)] = '.';                          /* en passant */
    if (k == 'K' && abs(m.to - m.from) == 2) {                     /* castling: the rook jumps over */
        if (m.to > m.from) { p->sq[m.from + 1] = p->sq[m.from + 3]; p->sq[m.from + 3] = '.'; }
        else               { p->sq[m.from - 1] = p->sq[m.from - 4]; p->sq[m.from - 4] = '.'; }
    }
    p->sq[m.to] = m.promo ? (char)(w ? m.promo : m.promo - 'A' + 'a') : c;
    p->sq[m.from] = '.';
    if (c == 'K') p->castle &= ~3;
    if (c == 'k') p->castle &= ~12;
    if (m.from == 0 || m.to == 0) p->castle &= ~2;
    if (m.from == 7 || m.to == 7) p->castle &= ~1;
    if (m.from == 56 || m.to == 56) p->castle &= ~8;
    if (m.from == 63 || m.to == 63) p->castle &= ~4;
    p->ep = (k == 'P' && abs(m.to - m.from) == 16) ? (m.from + m.to) / 2 : -1;
    p->white = !w;
}

CFD_RULE int cfd_legal_moves(const CfdPos *p, CfdMove *out)
{
    CfdMove list[CFD_MAXMOVES];
    int n = cfd_pseudo(p, list), m = 0, i;
    for (i = 0; i < n; i++) {
        CfdPos next = *p;
        cfd_apply(&next, list[i]);
        if (!cfd_in_check(&next, p->white)) out[m++] = list[i];
    }
    return m;
}

CFD_RULE int cfd_is_mate(const CfdPos *p)
{
    CfdMove list[CFD_MAXMOVES];
    return cfd_legal_moves(p, list) == 0 && cfd_in_check(p, p->white);
}

/* Can the side to move checkmate in one move? */
CFD_RULE int cfd_mate_in_one(const CfdPos *p)
{
    CfdMove list[CFD_MAXMOVES];
    int n = cfd_legal_moves(p, list), i;
    for (i = 0; i < n; i++) {
        CfdPos q = *p;
        cfd_apply(&q, list[i]);
        if (cfd_is_mate(&q)) return 1;
    }
    return 0;
}

#ifdef CFD_PERFT
/* (for the app's own tests) how many move sequences of that length there are */
CFD_RULE long cfd_perft(const CfdPos *p, int depth)
{
    CfdMove list[CFD_MAXMOVES];
    int n = cfd_legal_moves(p, list), i;
    long total = 0;
    if (depth <= 1) return depth == 1 ? n : 1;
    for (i = 0; i < n; i++) {
        CfdPos q = *p;
        cfd_apply(&q, list[i]);
        total += cfd_perft(&q, depth - 1);
    }
    return total;
}
#endif

/* ------------------------------------------------------------------ the game */
#ifndef CFD_RULES_ONLY

static CfdPos  cfd_pos;
static FILE   *cfd_ev;
static int     cfd_over, cfd_ready, cfd_me = 1;   /* cfd_me: the side your program plays (1 White) */
static int     cfd_goal = CFD_GOAL_MATE, cfd_limit = 1, cfd_made;
static char    cfd_target = 'q';
static int     cfd_lastFrom = -1, cfd_lastTo = -1;
static CfdMove cfd_line[32];
static int     cfd_lineN, cfd_kingHome = -1;
#ifndef CFD_DUEL
static int     cfd_onLine = 1;
#endif

static void cfd_log(const char *text)
{
    if (cfd_ev) {
        fputs(text, cfd_ev);
        fputc('\n', cfd_ev);
        fflush(cfd_ev);
    }
}

static void cfd_end(int win, const char *reason)
{
    char line[320];
    if (cfd_over) return;
    cfd_over = 1;
    if (win) snprintf(line, sizeof line, "END WIN");
    else     snprintf(line, sizeof line, "END FAIL %s", reason);
    cfd_log(line);
    if (cfd_ev) { fclose(cfd_ev); cfd_ev = NULL; }
}

/* Something went wrong for good: report it and stop the program. */
static void cfd_stop(const char *reason)
{
    cfd_end(0, reason);
    fflush(stdout);
    exit(0);
}

static void cfd_sync(void)
{
    int i;
    for (i = 0; i < 64; i++) {
        board[i].piece = cfd_pos.sq[i];
        board[i].file = i % 8;
        board[i].rank = i / 8;
    }
}

#define CFD_BY_HAND "ALARM! The board was changed by hand. Pieces only move with move() or pick_up() and hand - read the squares through your pointers, but never write to them."

static int cfd_board_ok(void)
{
    int i;
    for (i = 0; i < 64; i++)
        if (board[i].piece != cfd_pos.sq[i] || board[i].file != i % 8 || board[i].rank != i / 8) return 0;
    return 1;
}

static void cfd_check_board(void)
{
    if (!cfd_board_ok()) cfd_stop(CFD_BY_HAND);
}

/* Which square a pointer points at: -2 NULL, -3 not one of the 64 squares */
static int cfd_index(const Square *s)
{
    uintptr_t a = (uintptr_t)s, b = (uintptr_t)board;
    if (s == NULL) return -2;
    if (a < b || a >= b + sizeof(board) || (a - b) % sizeof(Square) != 0) return -3;
    return (int)((a - b) / sizeof(Square));
}

#ifndef CFD_DUEL
static int cfd_value(char c)
{
    switch (cfd_kind(c)) {
        case 'P': return 1;
        case 'N': case 'B': return 3;
        case 'R': return 5;
        case 'Q': return 9;
    }
    return 0;
}
#endif

/* Plays a move for the side to move, logs it and returns what it captured ('.' for nothing) */
static char cfd_play(char side, CfdMove m)
{
    char flags[8], took = cfd_pos.sq[m.to], a[3], b[3], line[48];
    CfdMove replies[CFD_MAXMOVES];
    int n = 0, left, check;
    char k = cfd_kind(cfd_pos.sq[m.from]);
    if (k == 'P' && m.to == cfd_pos.ep && took == '.' && m.to % 8 != m.from % 8) { took = cfd_pos.white ? 'p' : 'P'; flags[n++] = 'e'; }
    else if (took != '.') flags[n++] = 'x';
    if (k == 'K' && abs(m.to - m.from) == 2) flags[n++] = 'c';
    cfd_apply(&cfd_pos, m);
    left = cfd_legal_moves(&cfd_pos, replies);
    check = cfd_in_check(&cfd_pos, cfd_pos.white);
    if (left == 0) flags[n++] = check ? '#' : '=';
    else if (check) flags[n++] = '+';
    flags[n] = '\0';
    cfd_name(m.from, a);
    cfd_name(m.to, b);
    snprintf(line, sizeof line, "%c %s %s %c %s", side, a, b, m.promo ? m.promo : '-', n ? flags : "-");
    cfd_log(line);
    cfd_sync();
    return took;
}

#ifndef CFD_DUEL
/* Black's answer: the puzzle's main line while White follows it; otherwise a move that doesn't
   allow mate in one, wins the most material, leaves the least to be taken, and keeps the king home */
static CfdMove cfd_reply(void)
{
    CfdMove list[CFD_MAXMOVES], wl[CFD_MAXMOVES];
    int n = cfd_legal_moves(&cfd_pos, list), i, j, best = 0, bestScore = -1000000;
    int ply = 2 * cfd_made - 1;
    if (cfd_onLine && ply < cfd_lineN)
        for (i = 0; i < n; i++)
            if (list[i].from == cfd_line[ply].from && list[i].to == cfd_line[ply].to &&
                (list[i].promo == cfd_line[ply].promo || (!cfd_line[ply].promo && list[i].promo == 'Q')))
                return list[i];
    for (i = 0; i < n; i++) {
        CfdPos q = cfd_pos;
        int score = cfd_value(cfd_pos.sq[list[i].to]) * 100, threat = 0, wn;
        cfd_apply(&q, list[i]);
        if (cfd_mate_in_one(&q)) score -= 100000;
        wn = cfd_legal_moves(&q, wl);
        for (j = 0; j < wn; j++) if (cfd_value(q.sq[wl[j].to]) > threat) threat = cfd_value(q.sq[wl[j].to]);
        score -= threat * 30;
        if (cfd_kind(cfd_pos.sq[list[i].from]) == 'K' && cfd_kingHome >= 0)
            score -= abs(list[i].to % 8 - cfd_kingHome % 8) + abs(list[i].to / 8 - cfd_kingHome / 8);
        else if (cfd_pos.sq[list[i].to] == '.')
            score -= 20;
        if (score > bestScore) { bestScore = score; best = i; }
    }
    return list[best];
}
#endif

/* Why a move isn't allowed, in words */
static void cfd_why(int f, int t, char *out, size_t cap)
{
    char a[3], b[3], c = cfd_pos.sq[f], k = cfd_kind(c), target = cfd_pos.sq[t];
    int df = t % 8 - f % 8, dr = t / 8 - f / 8, adf = abs(df), adr = abs(dr), pattern = 1, blocked = 0;
    cfd_name(f, a);
    cfd_name(t, b);
    if (cfd_own(target, cfd_me)) { snprintf(out, cap, "Your own %s is already on %s - you can't capture your own pieces.", cfd_piece_name(target), b); return; }
    switch (k) {
        case 'K':
            if (adr == 0 && adf == 2) {
                snprintf(out, cap, "You can't castle here: the king and rook must not have moved, the squares between them must be empty, and the king can't castle out of, through or into check.");
                return;
            }
            if (adf > 1 || adr > 1) { snprintf(out, cap, "A king only moves one square in any direction (steps of 1, 7, 8 or 9)."); return; }
            break;
        case 'N':
            if (!((adf == 1 && adr == 2) || (adf == 2 && adr == 1))) {
                snprintf(out, cap, "A knight jumps in an L: two squares one way and one to the side. From any square that's +6, +10, +15 or +17 (or minus those).");
                return;
            }
            break;
        case 'B': pattern = adf == adr; break;
        case 'R': pattern = df == 0 || dr == 0; break;
        case 'Q': pattern = adf == adr || df == 0 || dr == 0; break;
        case 'P': {
            int dir = cfd_me ? 1 : -1, fwd = dr * dir, start = cfd_me ? 1 : 6;
            if (df == 0 && fwd == 1 && target != '.') { snprintf(out, cap, "A pawn can't capture straight ahead - it captures one square diagonally forward."); return; }
            if (df == 0 && fwd == 2 && f / 8 == start) {
                if (cfd_pos.sq[f + 8 * dir] != '.' || target != '.') { snprintf(out, cap, "Something is in the way of the pawn on %s.", a); return; }
                break;
            }
            if (adf == 1 && fwd == 1) {
                if (target == '.' && t != cfd_pos.ep) { snprintf(out, cap, "A pawn only moves diagonally when it captures something there (or captures en passant)."); return; }
                break;
            }
            if (df == 0 && fwd == 1) break;
            snprintf(out, cap, "A pawn moves one square straight forward (two from its starting rank), captures one square diagonally forward, and never moves backwards%s.",
                     cfd_me ? "" : " - Black's pawns move down the board, so that's -8");
            return;
        }
    }
    if (!pattern) {
        if (k == 'B') snprintf(out, cap, "A bishop only moves diagonally - steps of +7, +9, -7 or -9.");
        else if (k == 'R') snprintf(out, cap, "A rook only moves in straight lines along a rank or a file - steps of +1, -1, +8 or -8.");
        else snprintf(out, cap, "A queen moves in a straight line along a rank, a file or a diagonal - like a rook or a bishop.");
        return;
    }
    if (k == 'B' || k == 'R' || k == 'Q') {
        int step = (df > 0) - (df < 0) + 8 * ((dr > 0) - (dr < 0)), s;
        for (s = f + step; s != t; s += step) if (cfd_pos.sq[s] != '.') blocked = 1;
        if (blocked) { snprintf(out, cap, "Something is in the way between %s and %s - only knights can jump over pieces.", a, b); return; }
    }
    snprintf(out, cap, "That move would leave your king in check - it isn't allowed.");
}

static int cfd_viaHand;        /* the move came from pick_up() and hand (for clearer messages) */

/* Tries a move for your side. Plays it and returns 1, or returns 0 with the reason in why and
   nothing played. It never stops the program, so it can also run while the program ends. */
static int cfd_attempt(Square *from, Square *to, char promo, char *why, size_t cap)
{
    char text[300], a[3], b[3], took;
    CfdMove list[CFD_MAXMOVES], m;
    int f, t, i, n, found = -1;
    if (!cfd_board_ok()) { snprintf(why, cap, "%s", CFD_BY_HAND); return 0; }
#ifdef CFD_DUEL
    if (cfd_made >= 1) {
        cfd_log("I - -");
        snprintf(why, cap, "One move per turn - your code tried to make a second move. Make one move, then let your program end.");
        return 0;
    }
#endif
    f = cfd_index(from);
    t = cfd_index(to);
    if (f == -2 || t == -2) {
        cfd_log("I - -");
        snprintf(why, cap, "%s", cfd_viaHand ? "ALARM! hand is NULL - point it at the square the piece goes to, like hand = at(\"e4\");"
                                             : "ALARM! move() was given a NULL pointer. at() and find() give NULL when there is no such square or piece - check before you move.");
        return 0;
    }
    if (f == -3 || t == -3) {
        cfd_log("I - -");
        snprintf(why, cap, "%s", cfd_viaHand ? "ALARM! hand points outside the board. Only board[0] (a1) to board[63] (h8) are squares - did hand += ... go too far?"
                                             : "ALARM! That pointer points outside the board. Only board[0] (a1) to board[63] (h8) are squares - did the pointer arithmetic go too far?");
        return 0;
    }
    cfd_name(f, a);
    cfd_name(t, b);
    snprintf(text, sizeof text, "I %s %s", a, b);
    if (cfd_pos.sq[f] == '.') { cfd_log(text); snprintf(why, cap, "There's no piece on %s - from has to point at one of your pieces.", a); return 0; }
    if (!cfd_own(cfd_pos.sq[f], cfd_me)) {
        cfd_log(text);
        snprintf(why, cap, "That's %s's %s on %s - you play %s (the %s letters).", cfd_me ? "Black" : "White", cfd_piece_name(cfd_pos.sq[f]), a,
                 cfd_me ? "White" : "Black", cfd_me ? "capital" : "lower-case");
        return 0;
    }
    if (f == t) { cfd_log(text); snprintf(why, cap, "from and to both point at %s - a move has to go somewhere.", a); return 0; }
    if (promo >= 'a' && promo <= 'z') promo = (char)(promo - 'a' + 'A');
    if (promo && promo != 'Q' && promo != 'R' && promo != 'B' && promo != 'N') {
        cfd_log(text);
        snprintf(why, cap, "promote() needs the piece to become: 'Q', 'R', 'B' or 'N'.");
        return 0;
    }
    n = cfd_legal_moves(&cfd_pos, list);
    for (i = 0; i < n && found < 0; i++)
        if (list[i].from == f && list[i].to == t && (list[i].promo == 0 || list[i].promo == (promo ? promo : 'Q'))) found = i;
    if (found < 0) {
        cfd_log(text);
        cfd_why(f, t, why, cap);
        return 0;
    }
    m = list[found];
    if (promo && !m.promo) {
        cfd_log(text);
        snprintf(why, cap, "promote() is only for a pawn reaching the last rank - use move() for every other move.");
        return 0;
    }
#ifdef CFD_DUEL
    took = cfd_play(cfd_me ? 'W' : 'B', m);
    cfd_made++;
    (void)took;
    return 1;
}
#else
    if (cfd_onLine && (2 * cfd_made >= cfd_lineN || cfd_line[2 * cfd_made].from != m.from || cfd_line[2 * cfd_made].to != m.to)) cfd_onLine = 0;
    took = cfd_play('W', m);
    cfd_made++;

    /* did that solve the puzzle? */
    if (cfd_is_mate(&cfd_pos)) { cfd_end(1, ""); return 1; }
    if ((cfd_goal == CFD_GOAL_TAKE && took == cfd_target) || (cfd_goal == CFD_GOAL_PROMOTE && m.promo)) { cfd_end(1, ""); return 1; }
    if (cfd_legal_moves(&cfd_pos, list) == 0) {
        cfd_end(0, "Stalemate! Black has no legal move but isn't in check - that's a draw, not a win.");
        return 1;
    }
    if (cfd_made >= cfd_limit) {
        if (cfd_goal == CFD_GOAL_MATE) snprintf(text, sizeof text, "That was your last move (%d of %d) and Black isn't checkmated.", cfd_made, cfd_limit);
        else if (cfd_goal == CFD_GOAL_TAKE) snprintf(text, sizeof text, "That was your last move (%d of %d) and Black's %s is still on the board.", cfd_made, cfd_limit, cfd_piece_name(cfd_target));
        else snprintf(text, sizeof text, "That was your last move (%d of %d) and no pawn has been promoted.", cfd_made, cfd_limit);
        cfd_end(0, text);
        return 1;
    }

    /* Black answers */
    m = cfd_reply();
    cfd_play('B', m);
    cfd_lastFrom = m.from;
    cfd_lastTo = m.to;
    if (cfd_legal_moves(&cfd_pos, list) == 0)
        cfd_end(0, cfd_in_check(&cfd_pos, 1) ? "Black checkmated you! Look at what your moves leave open."
                                            : "Stalemate - you have no legal move left. That's a draw.");
    return 1;
}
#endif

/* ------------------------------------------------------------------ the second way: pick up and point */

Square *hand;                  /* the square your hand is over - reassign it to move the piece you picked up */
static int cfd_holding = -1;   /* where that piece came from (-1: holding nothing) */

/* Puts the piece in your hand down where hand points. 1 if that move was played. */
static int cfd_put(char *why, size_t cap)
{
    int from = cfd_holding, ok;
    Square *to = hand;
    char a[3];
    cfd_holding = -1;
    hand = NULL;
    if (to == &board[from]) {
        cfd_name(from, a);
        snprintf(why, cap, "You picked up the %s on %s and put it straight back - point hand at the square it goes to first, like hand = at(\"e4\"); or hand += 8;",
                 cfd_piece_name(cfd_pos.sq[from]), a);
        return 0;
    }
    cfd_viaHand = 1;
    ok = cfd_attempt(&board[from], to, 0, why, cap);
    cfd_viaHand = 0;
    return ok;
}

static void cfd_my_move(Square *from, Square *to, char promo)
{
    char why[320], a[3];
    if (cfd_over) return;
    if (cfd_holding >= 0) {
        cfd_name(cfd_holding, a);
        snprintf(why, sizeof why, "You're holding the %s from %s - put it down first (put_down(), or point hand and let your program end) before calling move().",
                 cfd_piece_name(cfd_pos.sq[cfd_holding]), a);
        cfd_log("I - -");
        cfd_stop(why);
    }
    if (!cfd_attempt(from, to, promo, why, sizeof why)) cfd_stop(why);
}

/* ------------------------------------------------------------------ what you can call */

/* A pointer to the square with that name ("e4"), or NULL if it isn't a square */
Square *at(const char *name)
{
    cfd_check_board();
    if (!name || name[0] < 'a' || name[0] > 'h' || name[1] < '1' || name[1] > '8' || name[2] != '\0') return NULL;
    return &board[(name[1] - '1') * 8 + (name[0] - 'a')];
}

/* The first square (from a1 upwards) holding that piece, or NULL */
Square *find(char piece)
{
    int i;
    cfd_check_board();
    for (i = 0; i < 64; i++) if (board[i].piece == piece) return &board[i];
    return NULL;
}

int is_white(const Square *s) { return s && cfd_isw(s->piece); }
int is_black(const Square *s) { return s && cfd_isb(s->piece); }
int is_mine(const Square *s) { return s && cfd_own(s->piece, cfd_me); }      /* one of your pieces */
int is_theirs(const Square *s) { return s && cfd_own(s->piece, !cfd_me); }  /* one of the other side's */

/* 1 if White may move from -> to right now (a pawn reaching the last rank becomes a queen) */
int legal(Square *from, Square *to)
{
    CfdMove list[CFD_MAXMOVES];
    int f, t, n, i;
    if (cfd_over) return 0;
    cfd_check_board();
    f = cfd_index(from);
    t = cfd_index(to);
    if (f < 0 || t < 0) return 0;
    n = cfd_legal_moves(&cfd_pos, list);
    for (i = 0; i < n; i++) if (list[i].from == f && list[i].to == t) return 1;
    return 0;
}

/* 1 if that move would checkmate Black */
int mates(Square *from, Square *to)
{
    CfdMove list[CFD_MAXMOVES];
    int f, t, n, i;
    if (cfd_over) return 0;
    cfd_check_board();
    f = cfd_index(from);
    t = cfd_index(to);
    if (f < 0 || t < 0) return 0;
    n = cfd_legal_moves(&cfd_pos, list);
    for (i = 0; i < n; i++) {
        if (list[i].from == f && list[i].to == t && (list[i].promo == 0 || list[i].promo == 'Q')) {
            CfdPos q = cfd_pos;
            cfd_apply(&q, list[i]);
            return cfd_is_mate(&q);
        }
    }
    return 0;
}

/* Your move (a pawn reaching the last rank becomes a queen); in a puzzle, Black answers at once */
void move(Square *from, Square *to) { cfd_my_move(from, to, 0); }

/* A pawn to the last rank, becoming piece: 'Q', 'R', 'B' or 'N' */
void promote(Square *from, Square *to, char piece) { cfd_my_move(from, to, piece ? piece : '?'); }

/* The second way to move: pick up one of your pieces - hand then points at its square. Point
   hand at the square it goes to (hand = at("f3"); or hand += 15;), and the move is played when
   your program ends, or straight away with put_down(). */
void pick_up(Square *square)
{
    char why[320], a[3];
    int i;
    if (cfd_over) return;
    cfd_check_board();
    if (cfd_holding >= 0) {
        cfd_name(cfd_holding, a);
        snprintf(why, sizeof why, "You're already holding the %s from %s - put_down() it before you pick up another piece.", cfd_piece_name(cfd_pos.sq[cfd_holding]), a);
        cfd_log("I - -");
        cfd_stop(why);
    }
    i = cfd_index(square);
    if (i == -2) { cfd_log("I - -"); cfd_stop("ALARM! pick_up() was given a NULL pointer. at() and find() give NULL when there is no such square or piece - check first."); }
    if (i == -3) { cfd_log("I - -"); cfd_stop("ALARM! pick_up() was given a pointer outside the board. Only board[0] (a1) to board[63] (h8) are squares."); }
    cfd_name(i, a);
    if (cfd_pos.sq[i] == '.') {
        snprintf(why, sizeof why, "There's nothing on %s to pick up - pick_up() needs a square with one of your pieces.", a);
        cfd_log("I - -");
        cfd_stop(why);
    }
    if (!cfd_own(cfd_pos.sq[i], cfd_me)) {
        snprintf(why, sizeof why, "That's %s's %s on %s - you can only pick up your own pieces (%s).", cfd_me ? "Black" : "White",
                 cfd_piece_name(cfd_pos.sq[i]), a, cfd_me ? "capital letters" : "lower-case letters");
        cfd_log("I - -");
        cfd_stop(why);
    }
    cfd_holding = i;
    hand = square;
}

/* Puts the piece you are holding down where hand points, now (for several moves in one program) */
void put_down(void)
{
    char why[320];
    if (cfd_over) return;
    if (cfd_holding < 0) { cfd_log("I - -"); cfd_stop("put_down() was called, but you aren't holding a piece - pick_up() one first."); }
    if (!cfd_put(why, sizeof why)) cfd_stop(why);
}

/* The other side's last move: where it came from and where it went (NULL before it has moved) */
Square *their_from(void) { return cfd_lastFrom >= 0 ? &board[cfd_lastFrom] : NULL; }
Square *their_to(void)   { return cfd_lastTo >= 0 ? &board[cfd_lastTo] : NULL; }

/* How many moves you have left for the puzzle */
#ifdef CFD_DUEL
int moves_left(void) { return 1 - cfd_made; }
#else
int moves_left(void) { return cfd_limit - cfd_made; }
#endif

/* ------------------------------------------------------------------ setting up and checking */

static int cfd_square(const char *s)
{
    if (!s || s[0] < 'a' || s[0] > 'h' || s[1] < '1' || s[1] > '8') return -1;
    return (s[1] - '1') * 8 + (s[0] - 'a');
}

static int cfd_load(void)
{
    char key[32];
    while (scanf("%31s", key) == 1) {
        if (strcmp(key, "END") == 0) return cfd_ready;
        if (strcmp(key, "FEN") == 0) {
            char place[100], side[8], castle[8], ep[8];
            int r = 7, f = 0, i;
            if (scanf("%99s %7s %7s %7s", place, side, castle, ep) != 4) return 0;
            for (i = 0; i < 64; i++) cfd_pos.sq[i] = '.';
            for (i = 0; place[i]; i++) {
                char c = place[i];
                if (c == '/') { r--; f = 0; }
                else if (c >= '1' && c <= '8') f += c - '0';
                else { if (r >= 0 && f < 8) cfd_pos.sq[r * 8 + f] = c; f++; }
            }
            cfd_pos.white = side[0] != 'b';
            cfd_pos.castle = 0;
            for (i = 0; castle[i]; i++)
                cfd_pos.castle |= castle[i] == 'K' ? 1 : castle[i] == 'Q' ? 2 : castle[i] == 'k' ? 4 : castle[i] == 'q' ? 8 : 0;
            cfd_pos.ep = cfd_square(ep);
            cfd_kingHome = cfd_king(&cfd_pos, 0);
            cfd_me = cfd_pos.white;
            cfd_ready = 1;
        } else if (strcmp(key, "LAST") == 0) {
            char mv[16];
            if (scanf("%15s", mv) != 1) return 0;
            if (strlen(mv) >= 4) { cfd_lastFrom = cfd_square(mv); cfd_lastTo = cfd_square(mv + 2); }
            if (cfd_lastFrom < 0 || cfd_lastTo < 0) cfd_lastFrom = cfd_lastTo = -1;
        } else if (strcmp(key, "GOAL") == 0) {
            char type[16];
            if (scanf("%15s", type) != 1) return 0;
            if (strcmp(type, "TAKE") == 0) {
                cfd_goal = CFD_GOAL_TAKE;
                if (scanf(" %c", &cfd_target) != 1) return 0;
            } else {
                cfd_goal = strcmp(type, "PROMOTE") == 0 ? CFD_GOAL_PROMOTE : CFD_GOAL_MATE;
            }
            if (scanf("%d", &cfd_limit) != 1 || cfd_limit < 1) cfd_limit = 1;
        } else if (strcmp(key, "LINE") == 0) {
            char rest[400], *tok;
            if (!fgets(rest, sizeof rest, stdin)) return 0;
            for (tok = strtok(rest, " \t\r\n"); tok && cfd_lineN < 32; tok = strtok(NULL, " \t\r\n")) {
                CfdMove m;
                m.from = cfd_square(tok);
                m.to = strlen(tok) >= 4 ? cfd_square(tok + 2) : -1;
                m.promo = strlen(tok) >= 5 ? (char)(tok[4] >= 'a' ? tok[4] - 'a' + 'A' : tok[4]) : 0;
                if (m.from >= 0 && m.to >= 0) cfd_line[cfd_lineN++] = m;
            }
        }
    }
    return 0;
}

static void cfd_finish(void)
{
    char reason[320];
    if (cfd_over) return;
    if (cfd_holding >= 0) {                 /* the second way: the piece goes where hand points */
        if (!cfd_put(reason, sizeof reason)) { cfd_end(0, reason); return; }
        if (cfd_over) return;
    } else if (hand != NULL) {
        cfd_end(0, "hand was moved, but nothing was picked up - start with pick_up(at(\"e2\")); then point hand at the square the piece goes to.");
        return;
    }
#ifdef CFD_DUEL
    (void)reason;
    if (cfd_made == 1) cfd_end(1, "");
    else cfd_end(0, "Your code didn't make a move - point at one of your pieces and call move(from, to).");
    return;
#endif
    if (cfd_made == 0) {
        cfd_end(0, "Your code didn't make a move - call move(from, to) with pointers to two squares.");
        return;
    }
    if (cfd_goal == CFD_GOAL_MATE)
        snprintf(reason, sizeof reason, "Black isn't checkmated yet - you made %d of your %d move%s.", cfd_made, cfd_limit, cfd_limit == 1 ? "" : "s");
    else if (cfd_goal == CFD_GOAL_TAKE)
        snprintf(reason, sizeof reason, "Black's %s is still on the board - you made %d of your %d move%s.", cfd_piece_name(cfd_target), cfd_made,
                 cfd_limit, cfd_limit == 1 ? "" : "s");
    else
        snprintf(reason, sizeof reason, "No pawn has been promoted yet - you made %d of your %d move%s.", cfd_made, cfd_limit, cfd_limit == 1 ? "" : "s");
    cfd_end(0, reason);
}

/* ------------------------------------------------------------------ the real main */

int cfd_student_main();

int main(void)
{
    cfd_ev = fopen("cfd_events.txt", "w");
    if (!cfd_load()) {
        cfd_end(0, "The puzzle could not be set up - this is a problem with the level, not your code.");
        return 1;
    }
    cfd_sync();
    atexit(cfd_finish);
    return cfd_student_main();
}

#endif /* CFD_RULES_ONLY */

#endif
