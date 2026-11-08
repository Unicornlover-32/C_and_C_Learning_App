/* Robot Rescue - the robot's half of the game.
 *
 * The app puts this file in front of your code, which is why you can call move(),
 * turn_left() and the rest without writing them. It reads the maze from the input,
 * writes every action to cfd_events.txt so the app can animate it, and decides at
 * the end whether the rescue worked.
 *
 * Maze format (the test's input):
 *     <width> <height>
 *     one line per row:  #  wall    .  floor    C  coin    E  exit
 *                        L R S  signposts       ^ > v <  a robot facing north/east/south/west
 *     ...followed by anything your own program reads with scanf or cin.
 *
 * Options the level can switch on before including this file:
 *     CFD_NO_TURN_RIGHT  you write turn_right() yourself
 *     CFD_MOVE_N         move_n(steps) is provided
 *     CFD_SIDE_SENSORS   wall_left() and wall_right() are provided
 *     CFD_SIGNS          read_sign() is provided
 *     CFD_ROVER          (C++) a Robot object called rover
 *     CFD_FLEET          (C++) two Robot objects called red and blue
 */
#ifndef CFD_ROBOT_H
#define CFD_ROBOT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFD_MAXW    40
#define CFD_MAXH    30
#define CFD_MAXBOTS 8          /* real robots plus hologram copies */
#define CFD_BATTERY 2000       /* moves and turns before the battery runs out */

typedef struct { int x, y, dir, ghost, alive, coins, moved; } CfdBot;

static char   cfd_map[CFD_MAXH][CFD_MAXW + 1];
static int    cfd_w, cfd_h;
static CfdBot cfd_bot[CFD_MAXBOTS];
static int    cfd_nbots, cfd_nreal;
static int    cfd_coinsLeft, cfd_actions, cfd_over;
static FILE  *cfd_ev;
static const int cfd_dx[4] = { 0, 1, 0, -1 };   /* facing north, east, south, west */
static const int cfd_dy[4] = { -1, 0, 1, 0 };

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

static int cfd_wall_at(int x, int y)
{
    return x < 0 || y < 0 || x >= cfd_w || y >= cfd_h || cfd_map[y][x] == '#';
}

static void cfd_spend(void)
{
    if (++cfd_actions > CFD_BATTERY)
        cfd_stop("The battery ran out after 2000 moves and turns. Is a loop going round forever?");
}

static void cfd_move_bot(int b)
{
    char line[200];
    CfdBot *r = &cfd_bot[b];
    int nx, ny;
    if (cfd_over || !r->alive) return;
    cfd_spend();
    nx = r->x + cfd_dx[r->dir];
    ny = r->y + cfd_dy[r->dir];
    if (cfd_wall_at(nx, ny)) {
        snprintf(line, sizeof line, "%d X", b);
        cfd_log(line);
        if (r->ghost) { r->alive = 0; return; }
        snprintf(line, sizeof line, "CRASH! The robot drove into a wall on action %d. Check wall_ahead() before you move.", cfd_actions);
        cfd_stop(line);
    }
    r->x = nx;
    r->y = ny;
    r->moved++;
    snprintf(line, sizeof line, "%d M", b);
    cfd_log(line);
    if (!r->ghost && cfd_map[ny][nx] == 'C') {
        cfd_map[ny][nx] = '.';
        r->coins++;
        cfd_coinsLeft--;
    }
}

static void cfd_turn_bot(int b, int right)
{
    char line[32];
    CfdBot *r = &cfd_bot[b];
    if (cfd_over || !r->alive) return;
    cfd_spend();
    r->dir = (r->dir + (right ? 1 : 3)) % 4;
    snprintf(line, sizeof line, "%d %c", b, right ? 'R' : 'L');
    cfd_log(line);
}

/* Is there a wall next to robot b? turn: 0 ahead, 1 to the right, 3 to the left */
static int cfd_sense(int b, int turn)
{
    CfdBot *r = &cfd_bot[b];
    int d = (r->dir + turn) % 4;
    return cfd_wall_at(r->x + cfd_dx[d], r->y + cfd_dy[d]);
}

static int cfd_on_exit(int b)
{
    return cfd_map[cfd_bot[b].y][cfd_bot[b].x] == 'E';
}

/* Copying a Robot object makes a hologram: it can walk around, but it isn't the real robot. */
static int cfd_copy_bot(int src)
{
    char line[48];
    int g = cfd_nbots < CFD_MAXBOTS ? cfd_nbots++ : CFD_MAXBOTS - 1;
    cfd_bot[g] = cfd_bot[src];
    cfd_bot[g].ghost = 1;
    cfd_bot[g].alive = 1;
    cfd_bot[g].moved = 0;
    cfd_bot[g].coins = 0;
    snprintf(line, sizeof line, "COPY %d %d", g, src);
    cfd_log(line);
    return g;
}

static void cfd_finish(void)
{
    char reason[256];
    int i, ghostMoved = 0, home = 1;
    if (cfd_over) return;
    for (i = 0; i < cfd_nbots; i++)
        if (cfd_bot[i].ghost && cfd_bot[i].moved) ghostMoved = 1;
    for (i = 0; i < cfd_nreal; i++)
        if (!cfd_on_exit(i)) home = 0;
    if (!home) {
        if (ghostMoved)
            snprintf(reason, sizeof reason, "The real robot never reached the exit - a hologram copy did the walking. "
                                           "Pass the robot by reference (Robot &r) so the function moves the real one.");
        else if (cfd_nreal > 1)
            snprintf(reason, sizeof reason, "Your program ended, but not every robot is standing on an exit.");
        else
            snprintf(reason, sizeof reason, "Your program ended, but the robot isn't standing on the exit.");
        cfd_end(0, reason);
        return;
    }
    if (cfd_coinsLeft > 0) {
        snprintf(reason, sizeof reason, "The robot reached the exit, but %d coin%s still in the maze. Collect every coin.",
                 cfd_coinsLeft, cfd_coinsLeft == 1 ? " is" : "s are");
        cfd_end(0, reason);
        return;
    }
    cfd_end(1, "");
}

static int cfd_load(void)
{
    char row[256];
    int x, y;
    if (scanf("%d %d", &cfd_w, &cfd_h) != 2) return 0;
    if (cfd_w < 1 || cfd_h < 1 || cfd_w > CFD_MAXW || cfd_h > CFD_MAXH) return 0;
    for (y = 0; y < cfd_h; y++) {
        int len;
        if (scanf("%255s", row) != 1) return 0;
        len = (int)strlen(row);
        for (x = 0; x < cfd_w; x++) {
            char c = x < len ? row[x] : '#';
            int dir = c == '^' ? 0 : c == '>' ? 1 : c == 'v' ? 2 : c == '<' ? 3 : -1;
            if (dir >= 0) {
                if (cfd_nbots < CFD_MAXBOTS) {
                    CfdBot *r = &cfd_bot[cfd_nbots++];
                    memset(r, 0, sizeof *r);
                    r->x = x;
                    r->y = y;
                    r->dir = dir;
                    r->alive = 1;
                }
                c = '.';
            }
            if (c == 'C') cfd_coinsLeft++;
            cfd_map[y][x] = c;
        }
        cfd_map[y][cfd_w] = '\0';
    }
    cfd_nreal = cfd_nbots;
    return cfd_nbots > 0;
}

/* ------------------------------------------------------------------ what you can call */

void move(void)       { cfd_move_bot(0); }
void turn_left(void)  { cfd_turn_bot(0, 0); }
#ifndef CFD_NO_TURN_RIGHT
void turn_right(void) { cfd_turn_bot(0, 1); }
#endif
int  wall_ahead(void) { return cfd_sense(0, 0); }
#ifdef CFD_SIDE_SENSORS
int  wall_left(void)  { return cfd_sense(0, 3); }
int  wall_right(void) { return cfd_sense(0, 1); }
#endif
int  at_exit(void)    { return cfd_on_exit(0); }
int  coins(void)
{
    int i, n = 0;
    for (i = 0; i < cfd_nreal; i++) n += cfd_bot[i].coins;
    return n;
}
#ifdef CFD_SIGNS
char read_sign(void)
{
    char c = cfd_map[cfd_bot[0].y][cfd_bot[0].x];
    return (c == 'L' || c == 'R' || c == 'S') ? c : '.';
}
#endif
#ifdef CFD_MOVE_N
void move_n(int steps)
{
    int i;
    for (i = 0; i < steps; i++) cfd_move_bot(0);
}
#endif

#ifdef __cplusplus
class Robot {
public:
    explicit Robot(int index) : bot(index) {}
    Robot(const Robot &other) : bot(cfd_copy_bot(other.bot)) {}
    Robot &operator=(const Robot &other) { bot = cfd_copy_bot(other.bot); return *this; }

    void move()            { cfd_move_bot(bot); }
    void turnLeft()        { cfd_turn_bot(bot, 0); }
    void turnRight()       { cfd_turn_bot(bot, 1); }
    bool wallAhead() const { return cfd_sense(bot, 0) != 0; }
    bool wallLeft() const  { return cfd_sense(bot, 3) != 0; }
    bool wallRight() const { return cfd_sense(bot, 1) != 0; }
    bool atExit() const    { return cfd_on_exit(bot) != 0; }
    int  coins() const     { return cfd_bot[bot].coins; }

private:
    int bot;
};
#ifdef CFD_ROVER
Robot rover(0);
#endif
#ifdef CFD_FLEET
Robot red(0);
Robot blue(1);
#endif
#endif

/* ------------------------------------------------------------------ the real main */

int cfd_student_main();

int main(void)
{
    cfd_ev = fopen("cfd_events.txt", "w");
    if (!cfd_load()) {
        cfd_end(0, "The maze could not be loaded - this is a problem with the level, not your code.");
        return 1;
    }
    atexit(cfd_finish);
    return cfd_student_main();
}

#endif
