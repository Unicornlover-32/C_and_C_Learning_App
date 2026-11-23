#ifndef GAME_KITS_H
#define GAME_KITS_H

// Scene kits: the props levels are built from, so levels that share a world look alike.
//   kit_robot.c  the robot world: a stage, floor tiles, coins, number blocks, gates and doors
#include "gfx.h"

// ---------------------------------------------------------------- the robot (kit_robot.c)
enum { MOOD_IDLE, MOOD_HAPPY, MOOD_SAD, MOOD_CONFUSED };

typedef struct {
    Vector3 pos, from, to;
    float   moveT, moveDur;
    float   yaw, yawTo;
    float   time, blinkIn, blink;
    Spring  squash;
    int     mood;
    float   shake;          // seconds of head shaking left
    float   cheer;          // seconds of celebrating left
    float   look;           // seconds of looking around left
    float   wheel, speed;
    float   power;          // 1 on, 0 powered down
    bool    off;
    Color   body, accent;   // zero: the usual white and blue
    float   scale;          // zero: full size
} Bot;

void    BotInit(Bot *b, Vector3 pos, float yaw);
void    BotMoveTo(Bot *b, Vector3 to, float seconds);   // turns towards where it's going and drives there
void    BotFace(Bot *b, float yaw);
bool    BotMoving(const Bot *b);
void    BotUpdate(Bot *b, float dt);
void    BotDraw(const Bot *b);
void    BotMood(Bot *b, int mood);
void    BotShakeHead(Bot *b);
void    BotLookAround(Bot *b);
void    BotBump(Bot *b);                 // hits something: squash, sparks
void    BotCheer(Bot *b);
void    BotPowerDown(Bot *b);            // crashed: sparks, eyes go dark
Vector3 BotHead(const Bot *b);           // just above its head, for labels

// ---------------------------------------------------------------- robot-world props
void KitStage(Vector3 center, float halfW, float halfD);              // the floor the level stands on
void KitTile(Vector3 pos, float size, float pop, Color top, float glow); // pop 0..1 rises into place
void KitCoin(Vector3 pos, float flip, float spin, float glow, bool bad);// flip 0 lies face down, 1 stands up
void KitBlock(Vector3 pos, float size, Color color, float glow, float yaw);
void KitWall(Vector3 from, Vector3 to);                                // a low wall along x or z
void KitGate(Vector3 pos, float yaw, float open, Color lamp, float lampGlow);
void KitDoor(Vector3 pos, float yaw, float open, Color light, float lightGlow);

// ---------------------------------------------------------------- the factory (kit_factory.c)
#define BELT_Y 0.62f                                                   // the top of every conveyor
void    KitBelt(Vector3 left, float length, float scroll);           // a conveyor along +x
void    KitCrate(Vector3 pos, float size, Color color, float glow, float yaw);   // pos: the middle of its bottom
void    KitMachine(Vector3 pos, Vector3 size, Color body, Color light, float glow, float busy);  // the belt runs through it
void    KitScreen(Vector3 pos, float w, float h, Color tint, float glow);         // a display on a pole, facing +z
Vector3 KitScreenCenter(Vector3 pos, float h);
void    KitChute(Vector3 pos, Color color, float glow);              // a bin under a funnel
void    KitGauge(Vector3 pos, float radius, float k, Color needle, float glow);  // a dial facing +z; k 0..1 (more overshoots)
void    KitLamp(Vector3 pos, Color color, float glow);
void    KitPanel(Vector3 pos, float w, float h, Color color, float glow);         // a flat sign facing +z

// ---------------------------------------------------------------- memory street (kit_street.c)
void KitHouse(Vector3 pos, Color wall, Color roof, float glow);      // a little house facing +z
void KitVault(Vector3 pos, float open, Color light, float glow);     // a bank vault facing +z
void KitArc(Vector3 from, Vector3 to, float lift, float grow, Color color, float glow);  // a glowing pointer arrow
void KitGold(Vector3 pos, int bars, float glow);                     // a stack of gold bars
void KitBag(Vector3 pos, float size, Color color);                   // a sack
void KitRoad(Vector3 left, float length);                            // a road along +x
void KitLot(Vector3 pos, float w, float d, Color color, float glow); // an outlined plot of land (the heap)

#endif
