// A 1v1 chess game: the position and history, every way a game ends (checkmate, stalemate,
// too little material, the fifty-move rule, threefold repetition), move notation, and the
// computer player. The rules are the chess library's own (game/cfd_chess.h, rules only),
// the same code the students' programs run with, checked by the move-count tests.
#define CFD_RULES_ONLY
#include "../../game/cfd_chess.h"
#include "chess_duel.h"
#include <stdio.h>
#include <string.h>

static CfdPos ToPos(const ChessGame *g)
{
    CfdPos p;
    memcpy(p.sq, g->sq, 64);
    p.white = g->white;
    p.castle = g->castle;
    p.ep = g->ep;
    return p;
}

static void Key(const ChessGame *g, char *out)
{
    memcpy(out, g->sq, 64);
    out[64] = g->white ? 'w' : 'b';
    out[65] = (char)('a' + g->castle);
    out[66] = (char)(g->ep >= 0 ? '0' + g->ep : '-');   // (any distinct character is fine)
    out[67] = '\0';
}

void ChessGameStart(ChessGame *g)
{
    ChessGameFromFen(g, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool ChessGameFromFen(ChessGame *g, const char *fen)
{
    memset(g, 0, sizeof(*g));
    memset(g->sq, '.', 64);
    int r = 7, f = 0, i = 0;
    for (; fen[i] && fen[i] != ' '; i++) {
        char c = fen[i];
        if (c == '/') { r--; f = 0; }
        else if (c >= '1' && c <= '8') f += c - '0';
        else { if (r >= 0 && f < 8 && strchr("KQRBNPkqrbnp", c)) g->sq[r * 8 + f] = c; f++; }
    }
    if (fen[i] != ' ') return false;
    i++;
    g->white = fen[i] != 'b';
    while (fen[i] && fen[i] != ' ') i++;
    while (fen[i] == ' ') i++;
    for (; fen[i] && fen[i] != ' '; i++)
        g->castle |= fen[i] == 'K' ? 1 : fen[i] == 'Q' ? 2 : fen[i] == 'k' ? 4 : fen[i] == 'q' ? 8 : 0;
    while (fen[i] == ' ') i++;
    g->ep = -1;
    if (fen[i] >= 'a' && fen[i] <= 'h' && fen[i + 1] >= '1' && fen[i + 1] <= '8') g->ep = (fen[i + 1] - '1') * 8 + (fen[i] - 'a');
    while (fen[i] && fen[i] != ' ') i++;
    g->fullmove = 1;
    if (sscanf(fen + i, "%d %d", &g->halfmove, &g->fullmove) < 2 && g->fullmove < 1) g->fullmove = 1;
    if (g->fullmove < 1) g->fullmove = 1;
    Key(g, g->seen[0]);
    return true;
}

void ChessGameFen(const ChessGame *g, char *out, int cap)
{
    char place[80];
    int n = 0;
    for (int r = 7; r >= 0; r--) {
        int empty = 0;
        for (int f = 0; f < 8; f++) {
            char c = g->sq[r * 8 + f];
            if (c == '.') { empty++; continue; }
            if (empty) { place[n++] = (char)('0' + empty); empty = 0; }
            place[n++] = c;
        }
        if (empty) place[n++] = (char)('0' + empty);
        if (r) place[n++] = '/';
    }
    place[n] = '\0';
    char castle[5];
    int c = 0;
    if (g->castle & 1) castle[c++] = 'K';
    if (g->castle & 2) castle[c++] = 'Q';
    if (g->castle & 4) castle[c++] = 'k';
    if (g->castle & 8) castle[c++] = 'q';
    if (!c) castle[c++] = '-';
    castle[c] = '\0';
    char ep[3] = "-";
    if (g->ep >= 0) cfd_name(g->ep, ep);
    snprintf(out, (size_t)cap, "%s %c %s %s %d %d", place, g->white ? 'w' : 'b', castle, ep, g->halfmove, g->fullmove);
}

int ChessGameLegalCount(const ChessGame *g)
{
    CfdPos p = ToPos(g);
    CfdMove list[CFD_MAXMOVES];
    return cfd_legal_moves(&p, list);
}

bool ChessGameFind(const ChessGame *g, int from, int to, char promo, ChessMove *out)
{
    CfdPos p = ToPos(g);
    CfdMove list[CFD_MAXMOVES];
    int n = cfd_legal_moves(&p, list);
    for (int i = 0; i < n; i++) {
        if (list[i].from != from || list[i].to != to) continue;
        if (list[i].promo && list[i].promo != (promo ? promo : 'Q')) continue;
        *out = (ChessMove){ list[i].from, list[i].to, list[i].promo };
        return true;
    }
    return false;
}

void ChessGamePlay(ChessGame *g, ChessMove m, char *flags, int cap)
{
    CfdPos p = ToPos(g);
    CfdMove mv = { m.from, m.to, m.promo };
    char piece = p.sq[m.from], took = p.sq[m.to], k = cfd_kind(piece), f[8], a[3], b[3];
    int n = 0;
    bool ep = k == 'P' && m.to == p.ep && took == '.' && m.to % 8 != m.from % 8;
    bool castle = k == 'K' && (m.to - m.from == 2 || m.from - m.to == 2);
    if (ep) f[n++] = 'e';
    else if (took != '.') f[n++] = 'x';
    if (castle) f[n++] = 'c';
    cfd_apply(&p, mv);
    CfdMove list[CFD_MAXMOVES];
    int left = cfd_legal_moves(&p, list);
    bool check = cfd_in_check(&p, p.white);
    if (left == 0) f[n++] = check ? '#' : '=';
    else if (check) f[n++] = '+';
    f[n] = '\0';
    if (flags && cap > 0) snprintf(flags, (size_t)cap, "%s", f);

    // the notation: Ng1-f3, e4xd5, O-O, e7-e8=Q+
    if (g->plies < DUEL_MAX_PLIES) {
        char *s = g->notation[g->plies];
        cfd_name(m.from, a);
        cfd_name(m.to, b);
        if (castle) snprintf(s, 12, "%s", m.to > m.from ? "O-O" : "O-O-O");
        else snprintf(s, 12, "%s%s%c%s", k == 'P' ? "" : (k == 'N' ? "N" : k == 'B' ? "B" : k == 'R' ? "R" : k == 'Q' ? "Q" : "K"), a,
                      (ep || took != '.') ? 'x' : '-', b);
        size_t len = strlen(s);
        if (m.promo && len + 2 < 12) { s[len++] = '='; s[len++] = m.promo; s[len] = '\0'; }
        if ((check) && len + 1 < 12) { s[len++] = left == 0 ? '#' : '+'; s[len] = '\0'; }
        g->moves[g->plies] = m;
    }

    bool reset = k == 'P' || ep || took != '.';
    memcpy(g->sq, p.sq, 64);
    g->castle = p.castle;
    g->ep = p.ep;
    g->halfmove = reset ? 0 : g->halfmove + 1;
    if (!g->white) g->fullmove++;
    g->white = p.white;
    if (g->plies < DUEL_MAX_PLIES) g->plies++;
    Key(g, g->seen[g->plies]);
}

bool ChessGameInCheck(const ChessGame *g)
{
    CfdPos p = ToPos(g);
    return cfd_in_check(&p, p.white) != 0;
}

int ChessGameKing(const ChessGame *g, bool white)
{
    CfdPos p = ToPos(g);
    return cfd_king(&p, white);
}

// Neither side can ever checkmate: kings alone, or a king and one bishop or knight against a
// lone king, or bishops that all stand on the same colour of square
static bool TooLittle(const ChessGame *g)
{
    int minors = 0, bishopColour[2] = { 0, 0 };
    for (int s = 0; s < 64; s++) {
        char k = cfd_kind(g->sq[s]);
        if (k == 'P' || k == 'R' || k == 'Q') return false;
        if (k == 'N') minors++;
        if (k == 'B') { minors++; bishopColour[((s % 8) + (s / 8)) % 2]++; }
    }
    if (minors <= 1) return true;
    bool knights = false;
    for (int s = 0; s < 64; s++) if (cfd_kind(g->sq[s]) == 'N') knights = true;
    return !knights && (bishopColour[0] == 0 || bishopColour[1] == 0);
}

ChessState ChessGameState(const ChessGame *g)
{
    CfdPos p = ToPos(g);
    CfdMove list[CFD_MAXMOVES];
    if (cfd_legal_moves(&p, list) == 0) return cfd_in_check(&p, p.white) ? GAME_CHECKMATE : GAME_STALEMATE;
    if (TooLittle(g)) return GAME_MATERIAL;
    if (g->halfmove >= 100) return GAME_FIFTY;
    int same = 0;
    for (int i = 0; i <= g->plies; i++) if (strcmp(g->seen[i], g->seen[g->plies]) == 0) same++;
    if (same >= 3) return GAME_REPETITION;
    if (g->plies >= DUEL_MAX_PLIES) return GAME_FULL;
    return GAME_ON;
}

const char *ChessStateText(ChessState s)
{
    switch (s) {
        case GAME_CHECKMATE:  return "checkmate";
        case GAME_STALEMATE:  return "stalemate - the side to move has no legal move but isn't in check";
        case GAME_MATERIAL:   return "not enough pieces left for either side to checkmate";
        case GAME_FIFTY:      return "the fifty-move rule - fifty moves each with no capture and no pawn move";
        case GAME_REPETITION: return "threefold repetition - the same position three times";
        case GAME_FULL:       return "the game is too long";
        default:              return "";
    }
}

// ---------------------------------------------------------------- the computer player
//
// Negamax with alpha-beta pruning and iterative deepening, then a capture-only search at the
// leaves so it doesn't stop counting halfway through an exchange. The score is material plus
// piece-square tables (pieces like the centre, pawns like to advance, the king hides until
// the endgame). Easy looks one move ahead and plays a little loosely; Medium looks two; Hard
// looks five, with a bigger budget (each stops early when the budget runs out).

#define INF   1000000
#define MATE  100000

static const int PST_P[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,  50, 50, 50, 50, 50, 50, 50, 50,  10, 10, 20, 30, 30, 20, 10, 10,   5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,   5, -5,-10,  0,  0,-10, -5,  5,   5, 10, 10,-20,-20, 10, 10,  5,   0,  0,  0,  0,  0,  0,  0,  0 };
static const int PST_N[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50, -40,-20,  0,  0,  0,  0,-20,-40, -30,  0, 10, 15, 15, 10,  0,-30, -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30, -30,  5, 10, 15, 15, 10,  5,-30, -40,-20,  0,  5,  5,  0,-20,-40, -50,-40,-30,-30,-30,-30,-40,-50 };
static const int PST_B[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20, -10,  0,  0,  0,  0,  0,  0,-10, -10,  0,  5, 10, 10,  5,  0,-10, -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0, 10, 10, 10, 10,  0,-10, -10, 10, 10, 10, 10, 10, 10,-10, -10,  5,  0,  0,  0,  0,  5,-10, -20,-10,-10,-10,-10,-10,-10,-20 };
static const int PST_R[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,   5, 10, 10, 10, 10, 10, 10,  5,  -5,  0,  0,  0,  0,  0,  0, -5,  -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,  -5,  0,  0,  0,  0,  0,  0, -5,  -5,  0,  0,  0,  0,  0,  0, -5,   0,  0,  0,  5,  5,  0,  0,  0 };
static const int PST_Q[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20, -10,  0,  0,  0,  0,  0,  0,-10, -10,  0,  5,  5,  5,  5,  0,-10,  -5,  0,  5,  5,  5,  5,  0, -5,
     0,  0,  5,  5,  5,  5,  0, -5, -10,  5,  5,  5,  5,  5,  0,-10, -10,  0,  5,  0,  0,  0,  0,-10, -20,-10,-10, -5, -5,-10,-10,-20 };
static const int PST_K[64] = {
   -30,-40,-40,-50,-50,-40,-40,-30, -30,-40,-40,-50,-50,-40,-40,-30, -30,-40,-40,-50,-50,-40,-40,-30, -30,-40,-40,-50,-50,-40,-40,-30,
   -20,-30,-30,-40,-40,-30,-30,-20, -10,-20,-20,-20,-20,-20,-20,-10,  20, 20,  0,  0,  0,  0, 20, 20,  20, 30, 10,  0,  0, 10, 30, 20 };
static const int PST_KE[64] = {
   -50,-40,-30,-20,-20,-30,-40,-50, -30,-20,-10,  0,  0,-10,-20,-30, -30,-10, 20, 30, 30, 20,-10,-30, -30,-10, 30, 40, 40, 30,-10,-30,
   -30,-10, 30, 40, 40, 30,-10,-30, -30,-10, 20, 30, 30, 20,-10,-30, -30,-30,  0,  0,  0,  0,-30,-30, -50,-30,-30,-30,-30,-30,-30,-50 };

static int Value(char k)
{
    switch (k) {
        case 'P': return 100;
        case 'N': return 320;
        case 'B': return 330;
        case 'R': return 500;
        case 'Q': return 900;
    }
    return 0;
}

// The score for the side to move
static int Evaluate(const CfdPos *p)
{
    int heavy = 0, score = 0;
    for (int s = 0; s < 64; s++) {
        char k = cfd_kind(p->sq[s]);
        if (k != 'P' && k != 'K' && k != '.') heavy += Value(k);
    }
    bool endgame = heavy <= 1300;
    for (int s = 0; s < 64; s++) {
        char c = p->sq[s];
        if (c == '.') continue;
        char k = cfd_kind(c);
        bool w = cfd_isw(c) != 0;
        int idx = w ? (7 - s / 8) * 8 + s % 8 : s;           // the tables are drawn from White's side, rank 8 first
        const int *t = k == 'P' ? PST_P : k == 'N' ? PST_N : k == 'B' ? PST_B : k == 'R' ? PST_R : k == 'Q' ? PST_Q
                     : endgame ? PST_KE : PST_K;
        int v = Value(k) + t[idx];
        score += w ? v : -v;
    }
    return p->white ? score : -score;
}

typedef struct { long nodes, limit; } Budget;

// Captures and promotions first, the most valuable victim taken by the cheapest attacker
static int OrderScore(const CfdPos *p, CfdMove m)
{
    int s = 0;
    char victim = p->sq[m.to];
    if (victim != '.') s += 10000 + 10 * Value(cfd_kind(victim)) - Value(cfd_kind(p->sq[m.from])) / 10;
    if (m.promo) s += 9000 + Value(m.promo);
    return s;
}

static void Sort(const CfdPos *p, CfdMove *list, int n, int *keys)
{
    for (int i = 0; i < n; i++) keys[i] = OrderScore(p, list[i]);
    for (int i = 1; i < n; i++) {
        CfdMove m = list[i];
        int k = keys[i], j = i - 1;
        while (j >= 0 && keys[j] < k) { list[j + 1] = list[j]; keys[j + 1] = keys[j]; j--; }
        list[j + 1] = m;
        keys[j + 1] = k;
    }
}

// Only captures (and promotions) from here on - unless the side to move is in check: then
// every move is tried, and having none is checkmate
static int Quiesce(const CfdPos *p, int alpha, int beta, int depth, int ply, Budget *b)
{
    b->nodes++;
    bool check = cfd_in_check(p, p->white) != 0;
    if (!check) {
        int stand = Evaluate(p);
        if (stand >= beta) return beta;
        if (stand > alpha) alpha = stand;
        if (depth <= 0 || b->nodes > b->limit) return alpha;
    }
    CfdMove list[CFD_MAXMOVES];
    int keys[CFD_MAXMOVES], n = cfd_pseudo(p, list), legal = 0, best = -INF;
    Sort(p, list, n, keys);
    for (int i = 0; i < n && (check || keys[i] > 0); i++) {
        CfdPos q = *p;
        cfd_apply(&q, list[i]);
        if (cfd_in_check(&q, p->white)) continue;
        legal++;
        int v = (check && depth <= 0) ? -Evaluate(&q) : -Quiesce(&q, -beta, -alpha, depth - 1, ply + 1, b);
        if (v > best) best = v;
        if (v >= beta) return beta;
        if (v > alpha) alpha = v;
    }
    if (check) return legal == 0 ? -MATE + ply : best;
    return alpha;
}

static int Negamax(const CfdPos *p, int depth, int alpha, int beta, int ply, Budget *b)
{
    if (depth <= 0) return Quiesce(p, alpha, beta, 6, ply, b);
    b->nodes++;
    CfdMove list[CFD_MAXMOVES];
    int keys[CFD_MAXMOVES], n = cfd_pseudo(p, list), legal = 0, best = -INF;
    Sort(p, list, n, keys);
    for (int i = 0; i < n; i++) {
        CfdPos q = *p;
        cfd_apply(&q, list[i]);
        if (cfd_in_check(&q, p->white)) continue;
        legal++;
        int v = -Negamax(&q, depth - 1, -beta, -alpha, ply + 1, b);
        if (v > best) best = v;
        if (v > alpha) alpha = v;
        if (alpha >= beta || b->nodes > b->limit) break;
    }
    if (!legal) return cfd_in_check(p, p->white) ? -MATE + ply : 0;
    return best;
}

// A small, repeatable bit of noise for the easy computer
static int Noise(const ChessGame *g, CfdMove m)
{
    unsigned h = (unsigned)(g->plies * 7919 + m.from * 131 + m.to * 31 + (unsigned char)m.promo) * 2654435761u;
    return (int)((h >> 16) % 121) - 60;
}

ChessMove ChessComputerMove(const ChessGame *g, int level)
{
    CfdPos p = ToPos(g);
    CfdMove list[CFD_MAXMOVES];
    int n = cfd_legal_moves(&p, list);
    if (n == 0) return (ChessMove){ -1, -1, 0 };
    int depthMax = level <= 0 ? 1 : level == 1 ? 2 : 5;
    Budget b = { 0, level <= 0 ? 20000 : level == 1 ? 150000 : 1500000 };
    int keys[CFD_MAXMOVES];
    Sort(&p, list, n, keys);

    CfdMove best = list[0];
    for (int depth = 1; depth <= depthMax; depth++) {
        int alpha = -INF, bestScore = -INF, bestIndex = 0;
        bool complete = true;
        for (int i = 0; i < n; i++) {
            CfdPos q = p;
            cfd_apply(&q, list[i]);
            // (easy gets each move's exact score, so its bit of noise can't lift a bound over a real mate)
            int v = -Negamax(&q, depth - 1, -INF, level <= 0 ? INF : -alpha, 1, &b);
            if (level <= 0 && v < MATE - 1000 && v > -MATE + 1000) v += Noise(g, list[i]);
            if (v > bestScore) { bestScore = v; bestIndex = i; }
            if (v > alpha) alpha = v;
            if (b.nodes > b.limit) { complete = false; break; }
        }
        if (complete || depth == 1) {
            best = list[bestIndex];
            // search the best move first next time
            CfdMove t = list[bestIndex];
            memmove(&list[1], &list[0], sizeof(CfdMove) * (size_t)bestIndex);
            list[0] = t;
        }
        if (!complete || bestScore > MATE - 100) break;
    }
    return (ChessMove){ best.from, best.to, best.promo };
}
