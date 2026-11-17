// Chess Puzzles: the 3D board for the chess puzzle missions.
//
// The student's program moves White's pieces with pointers into the board array, and the
// chess library (game/cfd_chess.h) checks every move against the real rules, plays Black's
// answers and writes them all to a trace:
//     W e2 e4 - -     White's move: from, to, promotion ('-' or Q R B N), flags
//     B e7 e5 - -     Black's answer          flags: x capture, e en passant, c castling,
//     I e2 e5         a move that was refused        + check, # checkmate, = stalemate
//     END WIN / END FAIL <why>
// The position comes from the test input (FEN), the trace is replayed piece by piece, each
// of White's moves drawn as a pointer arrow labelled with its step (+8, +17...), and the
// verdict is read exactly the way the runner reads it.
//
// The board and the pieces come in several looks, unlocked with stars (ChessSetStyle).
#include "levels.h"
#include "stage.h"
#include "rlgl.h"
#include "../ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { CH_PIECE = 1, CH_GOAL, CH_MOVE, CH_REPLY, CH_BAD, CH_SKIP, CH_SET, CH_HOME, CH_SAY };
enum { CHF_GAME = 1, CHF_LINE, CHF_EXTRA };
enum { GOAL_MATE = 1, GOAL_TAKE, GOAL_PROMOTE };

// ---------------------------------------------------------------- the looks

typedef struct { const char *name; int stars; } Look;

static const Look PIECE_SETS[CHESS_PIECE_SETS] = {
    { "Classic", 0 }, { "Robots", 15 }, { "Crystal", 40 }, { "Gold & Onyx", 70 }, { "Neon", 110 },
};
static const Look BOARDS[CHESS_BOARDS] = {
    { "Wood", 0 }, { "Marble", 10 }, { "Neon Grid", 30 }, { "Ice", 55 }, { "Lava", 85 }, { "Galaxy", 130 },
};

static int pieceSet, boardLook;

const char *ChessPieceSetName(int i) { return (i >= 0 && i < CHESS_PIECE_SETS) ? PIECE_SETS[i].name : ""; }
int         ChessPieceSetStars(int i) { return (i >= 0 && i < CHESS_PIECE_SETS) ? PIECE_SETS[i].stars : 0; }
const char *ChessBoardName(int i) { return (i >= 0 && i < CHESS_BOARDS) ? BOARDS[i].name : ""; }
int         ChessBoardStars(int i) { return (i >= 0 && i < CHESS_BOARDS) ? BOARDS[i].stars : 0; }

void ChessSetStyle(int pieces, int board)
{
    pieceSet = (pieces >= 0 && pieces < CHESS_PIECE_SETS) ? pieces : 0;
    boardLook = (board >= 0 && board < CHESS_BOARDS) ? board : 0;
}

const char *ChessDifficultyName(int d)
{
    static const char *names[4] = { "EASY", "MEDIUM", "HARD", "VERY HARD" };
    return names[d < 0 ? 0 : d > 3 ? 3 : d];
}

Color ChessDifficultyColor(int d)
{
    static const Color colors[4] = { { 110, 220, 140, 255 }, { 120, 190, 255, 255 }, { 255, 180, 80, 255 }, { 255, 100, 110, 255 } };
    return colors[d < 0 ? 0 : d > 3 ? 3 : d];
}

// ---------------------------------------------------------------- the parser

static bool Space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }

static bool Word(const char **p, char *out, int cap)
{
    while (Space(**p)) (*p)++;
    if (!**p) return false;
    int n = 0;
    while (**p && !Space(**p)) { if (n < cap - 1) out[n++] = **p; (*p)++; }
    out[n] = '\0';
    return true;
}

static int Sq(const char *s, int len)
{
    if (len != 2 || s[0] < 'a' || s[0] > 'h' || s[1] < '1' || s[1] > '8') return -1;
    return (s[1] - '1') * 8 + (s[0] - 'a');
}

static bool IsPiece(char c) { return c && strchr("KQRBNPkqrbnp", c) != NULL; }

// The words of one trace line (at most `max`)
static int Words(const char *s, int len, const char **w, int *wl, int max)
{
    int n = 0, i = 0;
    while (n < max) {
        while (i < len && s[i] == ' ') i++;
        if (i >= len) break;
        w[n] = s + i;
        while (i < len && s[i] != ' ') i++;
        wl[n] = (int)(s + i - w[n]);
        n++;
    }
    return n;
}

// Moves a piece on the parser's own board (for skipping ahead)
static void Apply(char *b, int from, int to, char promo, const char *flags, bool white)
{
    char c = b[from];
    if (strchr(flags, 'e')) b[to + (white ? -8 : 8)] = '.';
    if (strchr(flags, 'c')) {
        if (to > from && from + 3 < 64) { b[from + 1] = b[from + 3]; b[from + 3] = '.'; }
        else if (to < from && from - 4 >= 0) { b[from - 1] = b[from - 4]; b[from - 4] = '.'; }
    }
    b[to] = promo ? (char)(white ? promo : promo - 'A' + 'a') : c;
    b[from] = '.';
}

static void ParseChess(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);

    // the position and the goal, read the way the library reads them
    char sim[64];
    memset(sim, '.', sizeof(sim));
    int goal = GOAL_MATE, limit = 1;
    char target = 'q', w[100];
    const char *in = run->input ? run->input : "";
    for (int guard = 0; Word(&in, w, sizeof(w)) && guard < 200; guard++) {
        if (strcmp(w, "END") == 0) break;
        if (strcmp(w, "FEN") == 0 && Word(&in, w, sizeof(w))) {
            memset(sim, '.', sizeof(sim));
            int r = 7, f = 0;
            for (int i = 0; w[i]; i++) {
                char c = w[i];
                if (c == '/') { r--; f = 0; }
                else if (c >= '1' && c <= '8') f += c - '0';
                else { if (r >= 0 && f < 8 && IsPiece(c)) sim[r * 8 + f] = c; f++; }
            }
        } else if (strcmp(w, "GOAL") == 0 && Word(&in, w, sizeof(w))) {
            goal = strcmp(w, "TAKE") == 0 ? GOAL_TAKE : strcmp(w, "PROMOTE") == 0 ? GOAL_PROMOTE : GOAL_MATE;
            if (goal == GOAL_TAKE && Word(&in, w, sizeof(w))) target = w[0];
            if (Word(&in, w, sizeof(w))) { limit = 0; for (int i = 0; w[i] >= '0' && w[i] <= '9' && i < 4; i++) limit = limit * 10 + (w[i] - '0'); }
        }
    }
    for (int s = 0; s < 64; s++) if (sim[s] != '.') EvSpawn(&p, CH_PIECE, s, sim[s], 0);
    GameEvent *g = EvSpawn(&p, CH_GOAL, goal, limit, 0);
    if (g) { char t[2] = { IsPiece(target) ? target : 'q', '\0' }; EvText(g, t); }
    if (ParsePreview(&p)) return;

    // the moves, up to the first END line (a run too long for the event list skips ahead)
    const char *t = run->trace ? run->trace : "";
    int budget = EV_MAX - 2 * PARSE_LINES - 64 - 40, skipped = 0;
    bool skipping = false;
    const char *q = t;
    for (int lines = 0; *q && lines < 40000; lines++) {
        const char *e = strchr(q, '\n');
        int len = e ? (int)(e - q) : (int)strlen(q);
        while (len > 0 && (q[len - 1] == '\r' || q[len - 1] == ' ')) len--;
        if (len >= 4 && memcmp(q, "END ", 4) == 0) break;
        const char *wd[6];
        int wl[6], n = Words(q, len, wd, wl, 6);
        if (n == 5 && wl[0] == 1 && (wd[0][0] == 'W' || wd[0][0] == 'B')) {
            int from = Sq(wd[1], wl[1]), to = Sq(wd[2], wl[2]);
            char promo = (wl[3] == 1 && strchr("QRBN", wd[3][0])) ? wd[3][0] : 0;
            char flags[8];
            int k = 0;
            for (int i = 0; i < wl[4] && k < 7; i++) if (strchr("xec+#=", wd[4][i])) flags[k++] = wd[4][i];
            flags[k] = '\0';
            if (from >= 0 && to >= 0 && from != to && (wl[3] == 1) && (promo || wd[3][0] == '-')) {
                Apply(sim, from, to, promo, flags, wd[0][0] == 'W');
                if (skipping) skipped++;
                else {
                    GameEvent *a = EvAct(&p, wd[0][0] == 'W' ? CH_MOVE : CH_REPLY, from, to, promo);
                    EvText(a, flags);
                }
            }
        } else if (n >= 1 && wl[0] == 1 && wd[0][0] == 'I') {
            int from = n >= 2 ? Sq(wd[1], wl[1]) : -1, to = n >= 3 ? Sq(wd[2], wl[2]) : -1;
            if (!skipping) EvAct(&p, CH_BAD, from, to, 0);
        }
        if (!skipping && ev->count >= budget) skipping = true;
        if (!e) break;
        q = e + 1;
    }
    if (skipping) {
        EvAct(&p, CH_SKIP, skipped, 0, 0);
        for (int s = 0; s < 64; s++) EvAct(&p, CH_SET, s, sim[s], 0);
    }
    // the game's verdict; after a win, what the program printed
    ParseGameEnd(&p, CH_HOME, CH_SAY, CHF_GAME, CHF_LINE, CHF_EXTRA);
}

// ---------------------------------------------------------------- drawing the looks

static bool White(char c) { return c >= 'A' && c <= 'Z'; }
static char Kind(char c) { return (char)(c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c); }

static float PieceHeight(char k)
{
    switch (k) {
        case 'P': return 0.75f;
        case 'R': return 0.9f;
        case 'N': return 1.0f;
        case 'B': return 1.05f;
        case 'Q': return 1.15f;
        default:  return 1.3f;
    }
}

// Classic, Gold & Onyx and Neon: turned pieces, built from cylinders, cones and spheres
static void RoundPiece(char k, Color body, Color trim, float glow, float tglow, float yaw)
{
    Part(MESH_CYLINDER, V3(0, 0.05f, 0), V3(0.64f, 0.1f, 0.64f), body, glow);
    Part(MESH_CYLINDER, V3(0, 0.12f, 0), V3(0.52f, 0.05f, 0.52f), trim, tglow);
    switch (k) {
        case 'P':
            Part(MESH_CONE, V3(0, 0.35f, 0), V3(0.38f, 0.42f, 0.38f), body, glow);
            Part(MESH_CYLINDER, V3(0, 0.52f, 0), V3(0.3f, 0.04f, 0.3f), trim, tglow);
            Part(MESH_SPHERE, V3(0, 0.64f, 0), V3(0.27f, 0.27f, 0.27f), body, glow);
            break;
        case 'R':
            Part(MESH_CYLINDER, V3(0, 0.42f, 0), V3(0.38f, 0.56f, 0.38f), body, glow);
            Part(MESH_CYLINDER, V3(0, 0.67f, 0), V3(0.42f, 0.03f, 0.42f), trim, tglow);
            Part(MESH_CYLINDER, V3(0, 0.75f, 0), V3(0.48f, 0.14f, 0.48f), body, glow);
            for (int i = 0; i < 4; i++) {
                float a = (i * 90 + 45) * DEG2RAD;
                Part(MESH_CUBE, V3(cosf(a) * 0.17f, 0.87f, sinf(a) * 0.17f), V3(0.13f, 0.1f, 0.13f), body, glow);
            }
            break;
        case 'N':
            Part(MESH_CYLINDER, V3(0, 0.3f, 0), V3(0.4f, 0.36f, 0.4f), body, glow);
            rlPushMatrix();
            rlRotatef(yaw, 0, 1, 0);
            PartRot(MESH_CUBE, V3(0, 0.62f, -0.03f), V3(0.24f, 0.5f, 0.32f), V3(-14, 0, 0), body, glow);
            PartRot(MESH_CUBE, V3(0, 0.8f, 0.13f), V3(0.22f, 0.2f, 0.42f), V3(22, 0, 0), body, glow);
            Part(MESH_CONE, V3(-0.07f, 0.96f, -0.06f), V3(0.07f, 0.14f, 0.07f), body, glow);
            Part(MESH_CONE, V3(0.07f, 0.96f, -0.06f), V3(0.07f, 0.14f, 0.07f), body, glow);
            PartRot(MESH_CUBE, V3(0, 0.72f, -0.19f), V3(0.07f, 0.44f, 0.08f), V3(-14, 0, 0), trim, tglow);
            Part(MESH_SPHERE, V3(-0.12f, 0.84f, 0.12f), V3(0.05f, 0.05f, 0.05f), trim, tglow + 0.3f);
            Part(MESH_SPHERE, V3(0.12f, 0.84f, 0.12f), V3(0.05f, 0.05f, 0.05f), trim, tglow + 0.3f);
            rlPopMatrix();
            break;
        case 'B':
            Part(MESH_CONE, V3(0, 0.44f, 0), V3(0.4f, 0.62f, 0.4f), body, glow);
            Part(MESH_CYLINDER, V3(0, 0.64f, 0), V3(0.3f, 0.04f, 0.3f), trim, tglow);
            Part(MESH_SPHERE, V3(0, 0.81f, 0), V3(0.27f, 0.34f, 0.27f), body, glow);
            PartRot(MESH_CUBE, V3(0.04f, 0.86f, 0.06f), V3(0.03f, 0.16f, 0.2f), V3(0, 45, 30), trim, tglow);
            Part(MESH_SPHERE, V3(0, 1.0f, 0), V3(0.09f, 0.09f, 0.09f), trim, tglow);
            break;
        case 'Q':
            Part(MESH_CONE, V3(0, 0.49f, 0), V3(0.44f, 0.72f, 0.44f), body, glow);
            Part(MESH_CYLINDER, V3(0, 0.78f, 0), V3(0.36f, 0.05f, 0.36f), trim, tglow);
            Part(MESH_SPHERE, V3(0, 0.9f, 0), V3(0.3f, 0.24f, 0.3f), body, glow);
            for (int i = 0; i < 6; i++) {
                float a = i * 60 * DEG2RAD;
                Part(MESH_SPHERE, V3(cosf(a) * 0.15f, 1.02f, sinf(a) * 0.15f), V3(0.08f, 0.08f, 0.08f), trim, tglow);
            }
            Part(MESH_SPHERE, V3(0, 1.1f, 0), V3(0.1f, 0.1f, 0.1f), trim, tglow);
            break;
        default:   // the king
            Part(MESH_CONE, V3(0, 0.52f, 0), V3(0.46f, 0.78f, 0.46f), body, glow);
            Part(MESH_CYLINDER, V3(0, 0.84f, 0), V3(0.38f, 0.05f, 0.38f), trim, tglow);
            Part(MESH_CYLINDER, V3(0, 0.96f, 0), V3(0.32f, 0.2f, 0.32f), body, glow);
            Part(MESH_CUBE, V3(0, 1.18f, 0), V3(0.07f, 0.28f, 0.07f), trim, tglow);
            Part(MESH_CUBE, V3(0, 1.2f, 0), V3(0.2f, 0.07f, 0.07f), trim, tglow);
            break;
    }
}

// Robots: every piece is a little robot; its hat says what it is
static void RobotPiece(char k, Color body, Color eyes)
{
    Color dark = { 40, 44, 58, 255 };
    float h = k == 'P' ? 0.26f : k == 'K' || k == 'Q' ? 0.44f : 0.36f;
    Part(MESH_CYLINDER, V3(0, 0.05f, 0), V3(0.56f, 0.1f, 0.56f), dark, 0);
    Part(MESH_CUBE, V3(0, 0.1f + h / 2, 0), V3(0.42f, h, 0.34f), body, 0);
    Part(MESH_CUBE, V3(0, 0.1f + h * 0.55f, 0.175f), V3(0.24f, h * 0.4f, 0.02f), dark, 0);
    float hy = 0.1f + h + 0.15f;
    Part(MESH_CUBE, V3(0, hy, 0), V3(0.36f, 0.26f, 0.32f), body, 0);
    Part(MESH_CUBE, V3(0, hy, 0.16f), V3(0.28f, 0.14f, 0.02f), dark, 0);
    Part(MESH_SPHERE, V3(-0.07f, hy, 0.175f), V3(0.07f, 0.07f, 0.04f), eyes, 1.0f);
    Part(MESH_SPHERE, V3(0.07f, hy, 0.175f), V3(0.07f, 0.07f, 0.04f), eyes, 1.0f);
    float top = hy + 0.13f;
    switch (k) {
        case 'P':
            Part(MESH_CYLINDER, V3(0, top + 0.09f, 0), V3(0.03f, 0.18f, 0.03f), dark, 0);
            Part(MESH_SPHERE, V3(0, top + 0.2f, 0), V3(0.08f, 0.08f, 0.08f), eyes, 0.9f);
            break;
        case 'R':
            for (int i = 0; i < 4; i++)
                Part(MESH_CUBE, V3((i % 2 ? 0.12f : -0.12f), top + 0.05f, (i / 2 ? 0.1f : -0.1f)), V3(0.1f, 0.1f, 0.1f), body, 0);
            break;
        case 'N':
            Part(MESH_CUBE, V3(-0.12f, top + 0.08f, 0), V3(0.06f, 0.16f, 0.1f), body, 0);
            Part(MESH_CUBE, V3(0.12f, top + 0.08f, 0), V3(0.06f, 0.16f, 0.1f), body, 0);
            Part(MESH_CUBE, V3(0, hy - 0.02f, 0.24f), V3(0.2f, 0.12f, 0.16f), body, 0);
            break;
        case 'B':
            Part(MESH_CONE, V3(0, top + 0.13f, 0), V3(0.26f, 0.26f, 0.26f), body, 0);
            Part(MESH_SPHERE, V3(0, top + 0.28f, 0), V3(0.07f, 0.07f, 0.07f), eyes, 0.9f);
            break;
        case 'Q':
            for (int i = 0; i < 5; i++) {
                float a = i * 72 * DEG2RAD;
                Part(MESH_CONE, V3(cosf(a) * 0.12f, top + 0.07f, sinf(a) * 0.1f), V3(0.07f, 0.14f, 0.07f), eyes, 0.7f);
            }
            break;
        default:
            Part(MESH_CYLINDER, V3(0, top + 0.05f, 0), V3(0.3f, 0.1f, 0.3f), eyes, 0.6f);
            Part(MESH_CUBE, V3(0, top + 0.2f, 0), V3(0.05f, 0.2f, 0.05f), eyes, 0.8f);
            Part(MESH_CUBE, V3(0, top + 0.22f, 0), V3(0.16f, 0.05f, 0.05f), eyes, 0.8f);
            break;
    }
}

// Crystal: faceted spikes that glow
static void CrystalPiece(char k, Color c, float glow)
{
    Color base = { (unsigned char)(c.r * 0.35f), (unsigned char)(c.g * 0.35f), (unsigned char)(c.b * 0.35f), 255 };
    float h = PieceHeight(k) - 0.2f;
    Part(MESH_PYRAMID, V3(0, 0, 0), V3(0.66f, 0.14f, 0.66f), base, 0.2f);
    Part(MESH_PYRAMID, V3(0, 0.1f, 0), V3(0.4f, h * 0.75f, 0.4f), c, glow);
    PartRot(MESH_PYRAMID, V3(0, 0.1f + h, 0), V3(0.3f, h * 0.35f, 0.3f), V3(180, 45, 0), c, glow + 0.1f);
    float top = 0.1f + h;
    switch (k) {
        case 'P': Part(MESH_SPHERE, V3(0, top + 0.06f, 0), V3(0.16f, 0.16f, 0.16f), c, 0.9f); break;
        case 'R': Part(MESH_CUBE, V3(0, top + 0.07f, 0), V3(0.24f, 0.14f, 0.24f), c, 0.8f); break;
        case 'N': PartRot(MESH_CUBE, V3(0, top + 0.06f, 0.05f), V3(0.14f, 0.14f, 0.3f), V3(30, 0, 0), c, 0.8f); break;
        case 'B': Part(MESH_PYRAMID, V3(0, top, 0), V3(0.14f, 0.3f, 0.14f), c, 0.9f); break;
        case 'Q':
            for (int i = 0; i < 5; i++) {
                float a = i * 72 * DEG2RAD;
                Part(MESH_PYRAMID, V3(cosf(a) * 0.1f, top, sinf(a) * 0.1f), V3(0.07f, 0.2f, 0.07f), c, 0.9f);
            }
            break;
        default:
            Part(MESH_CUBE, V3(0, top + 0.12f, 0), V3(0.06f, 0.26f, 0.06f), c, 1.0f);
            Part(MESH_CUBE, V3(0, top + 0.15f, 0), V3(0.2f, 0.06f, 0.06f), c, 1.0f);
            break;
    }
}

// One piece standing at `at` (0 on the floor), turned over by `tilt` degrees and shrunk by `scale`
static void DrawPiece(char piece, Vector3 at, float tilt, float scale)
{
    bool w = White(piece);
    char k = Kind(piece);
    BlobShadow(V3(at.x, 0.003f, at.z), 0.34f * scale, 1);
    rlPushMatrix();
    rlTranslatef(at.x, at.y, at.z);
    if (tilt != 0) rlRotatef(tilt, 1, 0, 0);
    rlScalef(scale, scale, scale);
    float yaw = w ? 180 : 0;            // knights look at the other side
    switch (pieceSet) {
        case 1:
            RobotPiece(k, w ? (Color){ 225, 230, 240, 255 } : (Color){ 70, 74, 92, 255 }, w ? (Color){ 90, 200, 255, 255 } : (Color){ 255, 90, 90, 255 });
            break;
        case 2:
            CrystalPiece(k, w ? (Color){ 80, 200, 255, 255 } : (Color){ 210, 90, 255, 255 }, w ? 0.3f : 0.5f);
            break;
        case 3:
            if (w) RoundPiece(k, (Color){ 236, 190, 80, 255 }, (Color){ 255, 240, 190, 255 }, 0.1f, 0.3f, yaw);
            else   RoundPiece(k, (Color){ 34, 30, 42, 255 }, (Color){ 225, 175, 60, 255 }, 0, 0.35f, yaw);
            break;
        case 4:
            if (w) RoundPiece(k, (Color){ 60, 220, 255, 255 }, (Color){ 230, 255, 255, 255 }, 0.4f, 0.9f, yaw);
            else   RoundPiece(k, (Color){ 255, 70, 190, 255 }, (Color){ 255, 220, 245, 255 }, 0.4f, 0.9f, yaw);
            break;
        default:
            if (w) RoundPiece(k, (Color){ 236, 230, 215, 255 }, (Color){ 196, 184, 160, 255 }, 0, 0, yaw);
            else   RoundPiece(k, (Color){ 52, 50, 62, 255 }, (Color){ 105, 100, 120, 255 }, 0, 0, yaw);
            break;
    }
    rlPopMatrix();
}

static Vector3 SquarePos(int sq) { return V3((sq % 8) - 3.5f, 0, 3.5f - (sq / 8)); }

// The board: 64 squares on a frame, in the chosen look
static void DrawBoard(float time)
{
    Color light, dark, frame, trim;
    float lg = 0, dg = 0, tg = 0.3f;
    switch (boardLook) {
        case 1: light = (Color){ 236, 236, 240, 255 }; dark = (Color){ 118, 124, 142, 255 }; frame = (Color){ 66, 68, 80, 255 }; trim = (Color){ 200, 200, 214, 255 }; tg = 0.15f; break;
        case 2: light = (Color){ 34, 38, 58, 255 }; dark = (Color){ 16, 18, 30, 255 }; frame = (Color){ 14, 16, 24, 255 }; trim = (Color){ 60, 230, 255, 255 }; tg = 0.9f; break;
        case 3: light = (Color){ 168, 204, 230, 255 }; dark = (Color){ 70, 112, 156, 255 }; frame = (Color){ 96, 136, 176, 255 }; trim = (Color){ 190, 235, 255, 255 }; lg = 0.03f; dg = 0.02f; tg = 0.45f; break;
        case 4: light = (Color){ 78, 70, 68, 255 }; dark = (Color){ 34, 26, 26, 255 }; frame = (Color){ 40, 22, 18, 255 }; trim = (Color){ 255, 110, 40, 255 }; tg = 0.45f; break;
        case 5: light = (Color){ 92, 80, 168, 255 }; dark = (Color){ 30, 20, 72, 255 }; frame = (Color){ 24, 16, 52, 255 }; trim = (Color){ 190, 140, 255, 255 }; lg = 0.08f; tg = 0.8f; break;
        default: light = (Color){ 226, 190, 140, 255 }; dark = (Color){ 140, 94, 58, 255 }; frame = (Color){ 86, 56, 36, 255 }; trim = (Color){ 190, 150, 100, 255 }; tg = 0; break;
    }
    Part(MESH_CUBE, V3(0, -0.2f, 0), V3(9.0f, 0.28f, 9.0f), frame, 0);
    Part(MESH_CUBE, V3(0, -0.08f, 0), V3(8.3f, 0.06f, 8.3f), trim, tg);
    for (int s = 0; s < 64; s++) {
        Vector3 p = SquarePos(s);
        bool isLight = ((s % 8) + (s / 8)) % 2 == 1;
        Color c = isLight ? light : dark;
        float g = isLight ? lg : dg;
        if (boardLook == 4 && !isLight) {                 // lava glowing through the cracks
            float k = Pulse(time * 0.6f + Hash01(s) * 6, 1);
            c = (Color){ (unsigned char)(40 + 80 * k), (unsigned char)(26 + 22 * k), 24, 255 };
            g = 0.06f + 0.24f * k;
        }
        Part(MESH_CUBE, V3(p.x, -0.06f, p.z), V3(1.0f, 0.12f, 1.0f), c, g);
    }
    if (boardLook == 2 || boardLook == 4 || boardLook == 5) {   // glowing lines between the squares
        Color line = boardLook == 2 ? (Color){ 60, 230, 255, 255 } : boardLook == 4 ? (Color){ 255, 110, 30, 255 } : (Color){ 170, 130, 255, 255 };
        float g = boardLook == 2 ? 0.8f : boardLook == 4 ? 0.3f : 0.5f;
        for (int i = 0; i <= 8; i++) {
            Part(MESH_CUBE, V3(i - 4.0f, 0.003f, 0), V3(0.035f, 0.01f, 8.0f), line, g);
            Part(MESH_CUBE, V3(0, 0.003f, i - 4.0f), V3(8.0f, 0.01f, 0.035f), line, g);
        }
    }
    for (int i = 0; i < 8; i++) {                         // a-h and 1-8 round the edge
        char f[2] = { (char)('a' + i), '\0' }, r[2] = { (char)('1' + i), '\0' };
        Label3D(V3(i - 3.5f, 0.02f, 4.32f), f, 12, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
        Label3D(V3(-4.32f, 0.02f, 3.5f - i), r, 12, LBL_SOFT, (Color){ 0, 0, 0, 0 }, LABEL_CODE);
    }
}

// ---------------------------------------------------------------- the scene

enum { DO_LAND, DO_CHEER, DO_TOPPLE };

typedef struct {
    char    piece;
    int     sq;                // -1: captured
    Vector3 pos, from, to;
    float   t, dur, arc;       // the move: progress, length, hop height
    bool    moving;
    float   gone;              // captured: flies off as this goes 0 -> 1
    Vector3 vel;
    float   spin, tilt, sparkle;
    char    becomes;           // a pawn promoting: what it turns into when it lands
} Actor;

#define ACTORS 64

typedef struct { char text[EV_TEXT]; int state; } Bubble;

static struct {
    const EventList *ev;
    Cues   cues;
    Pops   pops;
    float  time, step;
    Actor  a[ACTORS];
    int    count;
    int    at[64];             // the actor on each square, -1 for none
    int    goal, limit, made;
    char   target;
    int    lastFrom, lastTo, lastWhite;  // the pointer arrow for White's last move
    float  arrowT;
    int    badFrom, badTo;
    float  badT;
    int    checkSq;            // a king in check (or mated), -1 for none
    bool   mated, won, failed;
    float  toppleAt;           // when the mated king started to fall
    Bubble say[3];
    int    says;
    char   want[FAIL_TEXT];
} C;

static void ChessReset(const EventList *ev)
{
    memset(&C, 0, sizeof(C));
    C.ev = ev;
    for (int i = 0; i < 64; i++) C.at[i] = -1;
    C.lastFrom = C.lastTo = C.badFrom = C.badTo = C.checkSq = -1;
    int moves = 0;
    for (int i = 0; ev && i < ev->count; i++) if (ev->ev[i].type == EV_ACT && (ev->ev[i].code == CH_MOVE || ev->ev[i].code == CH_REPLY)) moves++;
    C.step = moves <= 20 ? 1.0f : fmaxf(0.15f, 20.0f / moves);
    ShotOrbit(V3(0, 0, 0.15f), 13.8f, 54, 0, 38, 0);
}

static void Place(int sq, char piece)
{
    if (sq < 0 || sq >= 64) return;
    if (C.at[sq] >= 0) { C.a[C.at[sq]].sq = -1; C.a[C.at[sq]].gone = 1; C.at[sq] = -1; }
    if (piece == '.' || !IsPiece(piece) || C.count >= ACTORS) return;
    Actor *a = &C.a[C.count];
    memset(a, 0, sizeof(*a));
    a->piece = piece;
    a->sq = sq;
    a->pos = a->from = a->to = SquarePos(sq);
    C.at[sq] = C.count++;
}

// Knocks the piece on a square off the board
static void Capture(int sq)
{
    if (sq < 0 || sq >= 64 || C.at[sq] < 0) return;
    Actor *a = &C.a[C.at[sq]];
    a->sq = -1;
    a->gone = 0.001f;
    float side = a->pos.x >= 0 ? 1.0f : -1.0f;
    a->vel = V3(side * (2.5f + Hash01(sq) * 1.5f), 4.5f, (White(a->piece) ? 1.0f : -1.0f) * 1.2f);
    a->spin = 0;
    C.at[sq] = -1;
    FxSparks(V3(a->pos.x, 0.6f, a->pos.z), 30, White(a->piece) ? (Color){ 255, 240, 200, 255 } : (Color){ 255, 120, 90, 255 });
}

// Moves the piece on `from` to `to` over `dur` seconds
static void Slide(int from, int to, float dur, bool hop)
{
    if (from < 0 || from >= 64 || to < 0 || to >= 64 || C.at[from] < 0) return;
    int i = C.at[from];
    Actor *a = &C.a[i];
    a->from = a->pos;
    a->to = SquarePos(to);
    a->t = 0;
    a->dur = dur;
    a->moving = true;
    a->arc = hop ? 1.0f : 0.25f;
    a->sq = to;
    C.at[from] = -1;
    C.at[to] = i;
}

static int KingOf(bool white)
{
    for (int s = 0; s < 64; s++) if (C.at[s] >= 0 && C.a[C.at[s]].piece == (white ? 'K' : 'k')) return s;
    return -1;
}

static void AddBubble(const char *text, int state)
{
    if (C.says == 3) { C.say[0] = C.say[1]; C.say[1] = C.say[2]; C.says = 2; }
    StrCopy(C.say[C.says].text, EV_TEXT, text);
    C.say[C.says].state = state;
    C.says++;
}

static float PlayMove(const GameEvent *e, bool white)
{
    int from = e->a, to = e->b;
    char promo = (char)e->value;
    const char *flags = e->text;
    if (from < 0 || from >= 64 || to < 0 || to >= 64 || C.at[from] < 0) return 0.1f;
    char piece = C.a[C.at[from]].piece;
    bool knight = Kind(piece) == 'N';
    float dur = (knight ? 0.62f : 0.5f) * C.step;
    if (strchr(flags, 'e')) Capture(to + (white ? -8 : 8));
    if (C.at[to] >= 0) {
        Capture(to);
        PopAdd(&C.pops, white ? "captured!" : "lost one!", V3(SquarePos(to).x, 1.5f, SquarePos(to).z), white ? LBL_GOLD : LBL_BAD);
    }
    Slide(from, to, dur, knight);
    if (strchr(flags, 'c')) {                                 // castling: the rook hops over the king
        if (to > from) Slide(from + 3, from + 1, dur, true);
        else Slide(from - 4, from - 1, dur, true);
        PopAdd(&C.pops, "castles!", V3(SquarePos(to).x, 1.6f, SquarePos(to).z), LBL_SOFT);
    }
    if (promo && C.at[to] >= 0) C.a[C.at[to]].becomes = (char)(white ? promo : promo - 'A' + 'a');
    CueAt(&C.cues, C.time + dur, DO_LAND, to, white, V3(0, 0, 0), 0);
    C.checkSq = -1;
    if (strchr(flags, '+') || strchr(flags, '#')) C.checkSq = KingOf(!white);
    if (strchr(flags, '#')) {
        C.mated = true;
        CueAt(&C.cues, C.time + dur + 0.25f, DO_TOPPLE, C.checkSq, 0, V3(0, 0, 0), 0);
    }
    if (white) {
        C.lastFrom = from;
        C.lastTo = to;
        C.lastWhite = 1;
        C.arrowT = 0;
        C.made++;
    }
    return dur + (white ? 0.55f : 0.35f) * C.step + (strchr(flags, '#') ? 0.8f : 0);
}

static float ChessEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN:
            if (e->code == CH_PIECE) Place(e->a, (char)e->b);
            else if (e->code == CH_GOAL) { C.goal = e->a; C.limit = e->b; C.target = e->text[0]; }
            return 0;
        case EV_PRINT:
            return 0;
        case EV_ACT:
            switch (e->code) {
                case CH_MOVE:  return PlayMove(e, true);
                case CH_REPLY: return PlayMove(e, false);
                case CH_BAD:
                    C.badFrom = e->a;
                    C.badTo = e->b;
                    C.badT = 0;
                    if (e->a >= 0 && C.at[e->a] >= 0) C.a[C.at[e->a]].spin = 0.6f;     // a shake: "no!"
                    PlaybackShake(0.25f);
                    return 0.6f;
                case CH_SKIP:
                    PopAdd(&C.pops, TextFormat("fast-forward: %d more moves", e->a), V3(0, 2.2f, 0), LBL_GOLD);
                    return 0.5f;
                case CH_SET: {
                    if (e->a == 0) { C.count = 0; for (int i = 0; i < 64; i++) C.at[i] = -1; }
                    if (e->a >= 0 && e->a < 64 && e->b != '.') Place(e->a, (char)e->b);
                    return 0;
                }
                case CH_HOME:
                    return 0.3f;
                case CH_SAY:
                    AddBubble(e->text, e->b ? 3 : 1);
                    return 0.8f;
            }
            return 0;
        case EV_FAIL:
            C.failed = true;
            if (e->code != CHF_GAME && C.ev) {
                AddBubble(C.ev->actual[0] ? C.ev->actual : "(nothing)", 2);
                StrCopy(C.want, sizeof(C.want), C.ev->fail == FAIL_EXTRA ? "(nothing more)" : C.ev->expected);
            }
            PlaybackShake(0.2f);
            return 2.8f;
        case EV_CRASH:
            C.failed = true;
            PopAdd(&C.pops, C.ev && C.ev->fail == FAIL_TIMEOUT ? "stuck in a loop?" : "program crashed!", V3(0, 2.0f, 0), LBL_BAD);
            PlaybackShake(0.3f);
            return 2.6f;
        case EV_WIN:
            C.won = true;
            CueAt(&C.cues, C.time + 0.15f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void ChessUpdate(float dt)
{
    C.time += dt;
    C.arrowT += dt;
    C.badT += dt;
    for (int i = 0; i < C.count; i++) {
        Actor *a = &C.a[i];
        if (a->moving) {
            a->t += dt;
            float k = Clamp01(a->t / fmaxf(0.01f, a->dur)), e = EaseInOutQuad(k);
            a->pos = LerpV3(a->from, a->to, e);
            a->pos.y = sinf(k * PI) * a->arc;
            if (k >= 1) { a->moving = false; a->pos = a->to; }
        }
        if (a->gone > 0 && a->gone < 1) {
            a->gone = fminf(1, a->gone + dt * 0.9f);
            a->vel.y -= 14 * dt;
            a->pos = Vector3Add(a->pos, Vector3Scale(a->vel, dt));
            a->tilt += dt * 420;
        }
        if (a->spin > 0) a->spin -= dt;
        if (a->sparkle > 0) a->sparkle -= dt;
    }
    if (boardLook == 5 && Hash01((int)(C.time * 20)) > 0.92f)      // the galaxy board twinkles
        FxSparkle(V3((Hash01((int)(C.time * 7)) - 0.5f) * 9, 0.4f + Hash01((int)(C.time * 13)), (Hash01((int)(C.time * 11)) - 0.5f) * 9), 1,
                  (Color){ 210, 190, 255, 255 });
    PopsUpdate(&C.pops, dt);
    Cue c;
    while (CueDue(&C.cues, C.time, &c)) {
        if (c.what == DO_LAND && c.a >= 0 && c.a < 64 && C.at[c.a] >= 0) {
            Actor *a = &C.a[C.at[c.a]];
            FxDust(SquarePos(c.a), 6);
            if (a->becomes) {
                a->piece = a->becomes;
                a->becomes = 0;
                a->sparkle = 1;
                FxSparkle(V3(a->pos.x, 0.8f, a->pos.z), 40, (Color){ 255, 230, 140, 255 });
                PopAdd(&C.pops, "promoted!", V3(a->pos.x, 1.8f, a->pos.z), LBL_GOLD);
            }
        } else if (c.what == DO_TOPPLE) {
            C.toppleAt = C.time;
            PlaybackShake(0.25f);
            if (c.a >= 0) FxSparks(V3(SquarePos(c.a).x, 0.8f, SquarePos(c.a).z), 40, (Color){ 255, 90, 90, 255 });
        } else if (c.what == DO_CHEER) {
            bool finale = PlaybackFinale();
            FxConfetti(V3(0, 1.8f, 0), finale ? 170 : 80, 4.0f);
            if (finale) PlaybackSlowMo(0.3f, 1.3f);
        }
    }
}

static void Plate(int sq, Color c, float glow)
{
    if (sq < 0 || sq >= 64) return;
    Vector3 p = SquarePos(sq);
    Part(MESH_CUBE, V3(p.x, 0.006f, p.z), V3(0.94f, 0.012f, 0.94f), c, glow);
}

static void ChessDraw(void)
{
    DrawBoard(C.time);

    // the last move, the king in check, a refused move
    if (C.lastFrom >= 0) {
        Plate(C.lastFrom, (Color){ 100, 170, 230, 255 }, 0.08f);
        Plate(C.lastTo, (Color){ 110, 190, 255, 255 }, 0.25f);
    }
    if (C.checkSq >= 0) Plate(C.checkSq, COL3_BAD, 0.5f + 0.4f * Pulse(C.time, 4));
    if (C.badT < 10 && (C.badFrom >= 0 || C.badTo >= 0)) {
        float k = 1 - Clamp01((C.badT - 9) / 1.0f);
        if (C.badTo >= 0) {
            Vector3 p = SquarePos(C.badTo);
            PartRot(MESH_CUBE, V3(p.x, 0.05f, p.z), V3(0.9f, 0.04f, 0.12f), V3(0, 45, 0), COL3_BAD, 0.9f * k);
            PartRot(MESH_CUBE, V3(p.x, 0.05f, p.z), V3(0.9f, 0.04f, 0.12f), V3(0, -45, 0), COL3_BAD, 0.9f * k);
        }
        if (C.badFrom >= 0 && C.badTo >= 0)
            KitArc(V3(SquarePos(C.badFrom).x, 0.3f, SquarePos(C.badFrom).z), V3(SquarePos(C.badTo).x, 0.3f, SquarePos(C.badTo).z), 1.2f,
                   Clamp01(C.badT * 3), COL3_BAD, 0.8f * k);
    }

    // the pieces
    for (int i = 0; i < C.count; i++) {
        const Actor *a = &C.a[i];
        if (a->gone >= 1) continue;
        Vector3 p = a->pos;
        if (a->spin > 0) p.x += sinf(C.time * 50) * 0.06f * (a->spin / 0.6f);
        float tilt = a->tilt;
        if (a->sq >= 0 && a->sq == C.checkSq && C.toppleAt > 0) tilt = 82 * EaseOutBounce(Clamp01((C.time - C.toppleAt) / 0.7f));
        float scale = a->gone > 0 ? 1 - a->gone * 0.6f : 1 + a->sparkle * 0.15f * sinf(C.time * 20);
        DrawPiece(a->piece, p, tilt, scale);
    }

    // White's last move as a pointer: from + step = to
    if (C.lastFrom >= 0 && C.lastTo >= 0) {
        Vector3 a = SquarePos(C.lastFrom), b = SquarePos(C.lastTo);
        float grow = Clamp01(C.arrowT * 2.5f);
        KitArc(V3(a.x, 0.15f, a.z), V3(b.x, 0.15f, b.z), 1.6f, grow, (Color){ 120, 200, 255, 255 }, 0.8f);
        if (grow >= 1) {
            Vector3 mid = V3((a.x + b.x) / 2, 1.25f, (a.z + b.z) / 2 + 0.3f);
            Label3D(mid, TextFormat("board[%d] %+d = board[%d]", C.lastFrom, C.lastTo - C.lastFrom, C.lastTo), 13, WHITE,
                    (Color){ 30, 70, 120, 220 }, LABEL_CODE);
        }
    }

    // what the program printed
    for (int k = C.want[0] && C.says ? C.says - 1 : 0; k < C.says; k++) {
        Color fg = C.say[k].state == 1 ? LBL_GOOD : C.say[k].state == 2 ? LBL_BAD : WHITE;
        Label3D(V3(0, 2.4f + (C.says - 1 - k) * 0.45f, -2.5f), C.say[k].text, 13, fg, LBL_DIM, LABEL_CODE);
    }
    if (C.want[0]) Label3D(V3(0, 2.85f, -2.5f), TextFormat("expected: %.40s", C.want), 12, LBL_GOOD, LBL_DIM, LABEL_CODE);
    if (C.mated) Label3D(V3(0, 2.0f, 0), "CHECKMATE!", 18, LBL_GOOD, LBL_DIM, LABEL_CODE | LABEL_BIG);
    PopsDraw(&C.pops);
}

// The goal and the moves used, under the test's name
static void ChessOverlay(Rectangle view)
{
    if (!C.ev) return;
    const char *what = C.goal == GOAL_TAKE ? (C.target == 'q' ? "Take Black's queen" : C.target == 'r' ? "Take Black's rook" :
                                              C.target == 'b' ? "Take Black's bishop" : C.target == 'n' ? "Take Black's knight" : "Take Black's pawn")
                     : C.goal == GOAL_PROMOTE ? "Promote a pawn" : "Checkmate";
    const char *text = C.ev->preview ? TextFormat("Goal: %s in %d move%s", what, C.limit, C.limit == 1 ? "" : "s")
                                     : TextFormat("Goal: %s  -  move %d of %d", what, C.made, C.limit);
    float size = view.height > 500 ? 17 : 15, w = UiTextWidth(text, size) + 24;
    Rectangle box = { view.x + 12, view.y + 46, w, size + 12 };
    DrawRectangleRounded(box, 0.5f, 6, (Color){ 12, 14, 22, 210 });
    UiText(text, box.x + 12, box.y + 6, size, C.won ? (Color){ 140, 255, 180, 255 } : C.failed ? (Color){ 255, 130, 130, 255 } : (Color){ 255, 222, 130, 255 });
}

const LevelDef LEVEL_CHESS = {
    "", "Chess Puzzles", "Move the pieces with pointers into the board - every move follows the real rules of chess.",
    { 110, 225, 200, 255 },
    ParseChess, ChessReset, ChessEvent, ChessUpdate, ChessDraw, ChessOverlay,
};

// ---------------------------------------------------------------- the showcase on the puzzle screen

void ChessPreview(Rectangle view, float time, float dt)
{
    static const char *start = "RNBQKBNRPPPPPPPP................................pppppppprnbqkbnr";
    float yaw = sinf(time * 0.15f) * 28;
    float aspect = view.height > 0 ? view.width / view.height : 1.6f;      // a narrow view needs the camera further back
    ShotOrbit(V3(0, 0.2f, 0.2f), 13.4f * fmaxf(1.0f, 1.45f / aspect), 42, yaw, 38, 0);
    CamUpdate(dt);
    FxUpdate(dt);
    if (boardLook == 5 && Hash01((int)(time * 20)) > 0.9f)      // the galaxy board twinkles
        FxSparkle(V3((Hash01((int)(time * 7)) - 0.5f) * 9, 0.6f + Hash01((int)(time * 13)), (Hash01((int)(time * 11)) - 0.5f) * 9), 1,
                  (Color){ 210, 190, 255, 255 });
    SceneBegin(view);
    DrawBoard(time);
    for (int s = 0; s < 64; s++) if (start[s] != '.') DrawPiece(start[s], SquarePos(s), 0, 1);
    FxDraw();
    SceneEnd(view);
}

// ---------------------------------------------------------------- the 1v1 game's board
//
// The 1v1 screen drives the same board: it sets up a position, then tells it each move.

void ChessSceneStart(const char *sq, float yaw)
{
    ChessReset(NULL);
    for (int s = 0; s < 64; s++) Place(s, sq[s]);
    ShotOrbit(V3(0, 0, 0.15f), 13.8f, 54, yaw, 38, 0);
}

void ChessSceneTurn(float yaw) { ShotOrbit(V3(0, 0, 0.15f), 13.8f, 54, yaw, 38, 0.9f); }

void ChessSceneMove(int from, int to, char promo, const char *flags, bool white)
{
    GameEvent e;
    memset(&e, 0, sizeof(e));
    e.type = EV_ACT;
    e.code = white ? CH_MOVE : CH_REPLY;
    e.a = from;
    e.b = to;
    e.value = promo;
    StrCopy(e.text, EV_TEXT, flags ? flags : "");
    PlayMove(&e, white);
    C.lastFrom = from;                  // both sides' moves get the pointer arrow
    C.lastTo = to;
    C.arrowT = 0;
}

void ChessSceneRefused(int from, int to)
{
    C.badFrom = from;
    C.badTo = to;
    C.badT = 0;
    if (from >= 0 && from < 64 && C.at[from] >= 0) C.a[C.at[from]].spin = 0.6f;
    PlaybackShake(0.25f);
}

bool ChessSceneBusy(void)
{
    for (int i = 0; i < C.count; i++) if (C.a[i].moving) return true;
    return false;
}

void ChessSceneDraw(Rectangle view, float dt)
{
    CamUpdate(dt);
    FxUpdate(dt);
    ChessUpdate(dt);
    SceneBegin(view);
    ChessDraw();
    FxDraw();
    SceneEnd(view);
}
