#ifndef GAME_CHESS_DUEL_H
#define GAME_CHESS_DUEL_H

// A 1v1 chess game (chess_duel.c): the position, the moves so far, every way a game can end,
// and a computer player. It uses the very same rules as the chess library the students'
// programs run with (game/cfd_chess.h), so the two can never disagree about a move.
#include <stdbool.h>

#define DUEL_MAX_PLIES 600

typedef struct { int from, to; char promo; } ChessMove;     // promo: 'Q' 'R' 'B' 'N', or 0

typedef struct {
    char      sq[64];            // a1 = sq[0] ... h8 = sq[63]; 'K'..'P' White, 'k'..'p' Black, '.' empty
    int       white;             // 1: White to move
    int       castle, ep;        // castling rights (1 K, 2 Q, 4 k, 8 q); the en passant square or -1
    int       halfmove, fullmove;
    int       plies;             // moves played so far (both sides)
    ChessMove moves[DUEL_MAX_PLIES];
    char      notation[DUEL_MAX_PLIES][12];
    char      seen[DUEL_MAX_PLIES + 1][72];   // every position so far, for threefold repetition
} ChessGame;

typedef enum {
    GAME_ON, GAME_CHECKMATE, GAME_STALEMATE, GAME_MATERIAL, GAME_FIFTY, GAME_REPETITION, GAME_FULL
} ChessState;

void       ChessGameStart(ChessGame *g);
bool       ChessGameFromFen(ChessGame *g, const char *fen);
void       ChessGameFen(const ChessGame *g, char *out, int cap);
// The legal move from -> to (promo 0 means a queen for a pawn reaching the last rank); false if none
bool       ChessGameFind(const ChessGame *g, int from, int to, char promo, ChessMove *out);
// Plays a legal move. flags gets what happened, like the library's trace: x capture,
// e en passant, c castling, + check, # checkmate, = stalemate
void       ChessGamePlay(ChessGame *g, ChessMove m, char *flags, int cap);
ChessState ChessGameState(const ChessGame *g);
const char *ChessStateText(ChessState s);           // "checkmate", "stalemate", ...
bool       ChessGameInCheck(const ChessGame *g);    // the side to move is in check
int        ChessGameKing(const ChessGame *g, bool white);
int        ChessGameLegalCount(const ChessGame *g);
// The computer's move for the side to move: level 0 easy, 1 medium, 2 hard
ChessMove  ChessComputerMove(const ChessGame *g, int level);

#endif
