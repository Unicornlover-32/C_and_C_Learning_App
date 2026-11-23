// The robot world: the robot that acts out the program, and the stage it works on.
#include "kits.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

#define FLOOR_COL  (Color){ 44, 49, 68, 255 }
#define RIM_COL    (Color){ 52, 58, 80, 255 }
#define WALL_COL   (Color){ 62, 68, 92, 255 }
#define METAL_COL  (Color){ 78, 86, 112, 255 }
#define DARK_COL   (Color){ 26, 28, 38, 255 }
#define ACCENT_COL (Color){ 86, 156, 255, 255 }

// ---------------------------------------------------------------- the robot

void BotInit(Bot *b, Vector3 pos, float yaw)
{
    *b = (Bot){ 0 };
    b->pos = b->from = b->to = pos;
    b->yaw = b->yawTo = yaw;
    b->blinkIn = 1.5f;
    b->power = 1;
    SpringReset(&b->squash);
}

void BotMoveTo(Bot *b, Vector3 to, float seconds)
{
    b->from = b->pos;
    b->to = to;
    b->moveT = 0;
    b->moveDur = seconds > 0.01f ? seconds : 0.01f;
    float dx = to.x - b->pos.x, dz = to.z - b->pos.z;
    if (dx * dx + dz * dz > 0.0004f) b->yawTo = atan2f(dx, dz) * RAD2DEG;
}

void BotFace(Bot *b, float yaw) { b->yawTo = yaw; }
bool BotMoving(const Bot *b) { return b->moveDur > 0; }
void BotMood(Bot *b, int mood) { b->mood = mood; }
void BotShakeHead(Bot *b) { b->shake = 0.7f; }
void BotLookAround(Bot *b) { b->look = 2.0f; b->mood = MOOD_CONFUSED; }

void BotBump(Bot *b)
{
    SpringKick(&b->squash, 0.62f);
    FxSparks((Vector3){ b->pos.x, b->pos.y + 0.5f, b->pos.z }, 26, (Color){ 255, 190, 80, 255 });
    FxDust(b->pos, 10);
    b->mood = MOOD_SAD;
}

void BotCheer(Bot *b)
{
    b->cheer = 2.2f;
    b->mood = MOOD_HAPPY;
}

void BotPowerDown(Bot *b)
{
    b->off = true;
    b->mood = MOOD_SAD;
    FxSparks((Vector3){ b->pos.x, b->pos.y + 1.0f, b->pos.z }, 40, (Color){ 255, 170, 70, 255 });
    SpringKick(&b->squash, 0.75f);
}

Vector3 BotHead(const Bot *b) { return (Vector3){ b->pos.x, b->pos.y + 1.55f * (b->scale > 0 ? b->scale : 1), b->pos.z }; }

static float WrapAngle(float a)
{
    while (a > 180) a -= 360;
    while (a < -180) a += 360;
    return a;
}

void BotUpdate(Bot *b, float dt)
{
    b->time += dt;
    if (b->moveDur > 0) {
        b->moveT += dt;
        float k = EaseInOutQuad(b->moveT / b->moveDur);
        Vector3 np = LerpV3(b->from, b->to, k);
        float dist = Vector3Distance(np, b->pos);
        b->speed = dt > 0 ? dist / dt : 0;
        b->wheel += dist / 0.16f * RAD2DEG;
        b->pos = np;
        if (b->moveT >= b->moveDur) {
            b->moveDur = 0;
            SpringKick(&b->squash, 0.9f);       // a little settle when it stops
        }
    } else {
        b->speed = Smooth(b->speed, 0, 12, dt);
    }
    b->yaw += WrapAngle(b->yawTo - b->yaw) * (1 - expf(-14 * dt));

    b->blinkIn -= dt;
    if (b->blinkIn <= 0) {
        b->blink += dt * 9;
        if (b->blink >= 2) { b->blink = 0; b->blinkIn = 2.2f + Hash01((int)(b->time * 10)) * 2.5f; }
    }
    if (b->shake > 0) b->shake -= dt;
    if (b->cheer > 0) b->cheer -= dt;
    if (b->look > 0) b->look -= dt;
    b->power = Smooth(b->power, b->off ? 0.0f : 1.0f, 5, dt);
    SpringUpdate(&b->squash, dt);
}

void BotDraw(const Bot *b)
{
    RobotPose p = RobotPoseDefault(b->pos);
    if (b->body.a) p.body = b->body;
    if (b->accent.a) p.accent = b->accent;
    p.scale = b->scale > 0 ? b->scale : 1;
    p.yaw = b->yaw;
    p.squash = b->squash.value;
    p.wheel = b->wheel;
    p.power = b->power;
    p.blink = b->blink > 1 ? 2 - b->blink : b->blink;
    float moving = Clamp01(b->speed / 3.0f);
    p.lean = moving * 7;
    p.bob = fabsf(sinf(b->time * 18)) * 0.025f * moving;
    p.armL = 8 + sinf(b->time * 11) * 22 * moving + sinf(b->time * 2) * 3;
    p.armR = 8 - sinf(b->time * 11) * 22 * moving + sinf(b->time * 2.3f) * 3;
    p.antenna = 0.45f + 0.55f * Pulse(b->time, 3);

    switch (b->mood) {
        case MOOD_HAPPY:    p.eyes = (Color){ 120, 255, 170, 255 }; break;
        case MOOD_SAD:      p.eyes = (Color){ 255, 100, 100, 255 }; p.headTilt = 9; break;
        case MOOD_CONFUSED: p.eyes = (Color){ 255, 214, 90, 255 }; p.headTilt = -10; break;
        default: break;
    }
    if (b->shake > 0) p.headTurn = sinf(b->time * 26) * 24 * Clamp01(b->shake / 0.3f);
    if (b->look > 0) p.headTurn = sinf(b->time * 3.4f) * 40;
    if (b->cheer > 0) {
        float k = Clamp01(b->cheer / 0.4f);
        p.bob = fabsf(sinf(b->time * 7)) * 0.3f * k;
        p.armL = Lerpf(p.armL, 155 + sinf(b->time * 14) * 20, k);
        p.armR = Lerpf(p.armR, 155 - sinf(b->time * 14) * 20, k);
    }
    if (b->off) { p.headTilt = 22 * (1 - b->power); p.lean = -10 * (1 - b->power); p.armL = p.armR = 0; }

    BlobShadow(b->pos, 0.38f * p.scale, 1 - p.bob);
    DrawRobot(&p);
}

// ---------------------------------------------------------------- props

void KitStage(Vector3 c, float halfW, float halfD)
{
    Part(MESH_CUBE, (Vector3){ c.x, -0.16f, c.z }, (Vector3){ halfW * 2 + 0.5f, 0.2f, halfD * 2 + 0.5f }, RIM_COL, 0);
    Part(MESH_CUBE, (Vector3){ c.x, -0.05f, c.z }, (Vector3){ halfW * 2, 0.1f, halfD * 2 }, FLOOR_COL, 0);
    // a glowing trim along the front edge, and lamp posts on the corners
    Part(MESH_CUBE, (Vector3){ c.x, -0.07f, c.z + halfD + 0.26f }, (Vector3){ halfW * 2 + 0.5f, 0.03f, 0.03f }, ACCENT_COL, 0.7f);
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2) {
            Vector3 p = { c.x + sx * (halfW + 0.05f), 0, c.z + sz * (halfD + 0.05f) };
            Part(MESH_CYLINDER, (Vector3){ p.x, 0.35f, p.z }, (Vector3){ 0.1f, 0.7f, 0.1f }, METAL_COL, 0);
            Part(MESH_SPHERE, (Vector3){ p.x, 0.76f, p.z }, (Vector3){ 0.18f, 0.18f, 0.18f }, (Color){ 140, 190, 255, 255 }, 0.9f);
        }
}

void KitTile(Vector3 pos, float size, float pop, Color top, float glow)
{
    if (pop <= 0) return;
    float k = EaseOutBack(pop);
    float drop = (1 - Clamp01(pop * 1.4f)) * 0.8f;
    float s = size * (0.55f + 0.45f * k);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.05f + drop, pos.z }, (Vector3){ s, 0.1f, s }, (Color){ 50, 56, 76, 255 }, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.106f + drop, pos.z }, (Vector3){ s * 0.88f, 0.016f, s * 0.88f }, top, glow);
}

void KitCoin(Vector3 pos, float flip, float spin, float glow, bool bad)
{
    float f = Clamp01(flip);
    Color face = bad ? (Color){ 235, 90, 90, 255 } : (Color){ 255, 205, 70, 255 };
    Color down = (Color){ 120, 96, 44, 255 };
    Color c = { (unsigned char)Lerpf(down.r, face.r, f), (unsigned char)Lerpf(down.g, face.g, f), (unsigned char)Lerpf(down.b, face.b, f), 255 };
    float y = pos.y + 0.05f + sinf(f * PI) * 0.35f + f * 0.26f;
    rlPushMatrix();
    rlTranslatef(pos.x, y, pos.z);
    rlRotatef(spin, 0, 1, 0);
    rlRotatef(f * 90, 1, 0, 0);
    Part(MESH_CYLINDER, (Vector3){ 0, 0, 0 }, (Vector3){ 0.5f, 0.06f, 0.5f }, c, glow);
    Part(MESH_CYLINDER, (Vector3){ 0, 0, 0 }, (Vector3){ 0.34f, 0.075f, 0.34f },
         (Color){ (unsigned char)(c.r * 0.82f), (unsigned char)(c.g * 0.82f), (unsigned char)(c.b * 0.82f), 255 }, glow * 0.8f);
    rlPopMatrix();
}

void KitBlock(Vector3 pos, float size, Color color, float glow, float yaw)
{
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(yaw, 0, 1, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0, 0 }, (Vector3){ size, size, size }, color, glow * 0.5f);
    Part(MESH_CUBE, (Vector3){ 0, size * 0.5f, 0 }, (Vector3){ size * 0.8f, size * 0.04f, size * 0.8f },
         (Color){ 255, 255, 255, 255 }, glow);
    rlPopMatrix();
}

void KitWall(Vector3 a, Vector3 b)
{
    float h = 0.34f, t = 0.14f;
    Vector3 c = { (a.x + b.x) / 2, h / 2, (a.z + b.z) / 2 };
    float lx = fabsf(b.x - a.x), lz = fabsf(b.z - a.z);
    Vector3 size = lx > lz ? (Vector3){ lx + t, h, t } : (Vector3){ t, h, lz + t };
    Part(MESH_CUBE, c, size, WALL_COL, 0);
    Vector3 trim = lx > lz ? (Vector3){ lx + t, 0.03f, t * 0.5f } : (Vector3){ t * 0.5f, 0.03f, lz + t };
    Part(MESH_CUBE, (Vector3){ c.x, h + 0.015f, c.z }, trim, ACCENT_COL, 0.35f);
}

// A checkpoint across a path that runs along local x: a kiosk on the near (+z) side with
// its screen and input slot facing the camera, and a striped boom that swings up to open.
void KitGate(Vector3 pos, float yaw, float open, Color lamp, float lampGlow)
{
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(yaw, 0, 1, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.55f, 0.8f }, (Vector3){ 0.5f, 1.1f, 0.42f }, METAL_COL, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.06f, 0.8f }, (Vector3){ 0.62f, 0.12f, 0.54f }, WALL_COL, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.78f, 1.015f }, (Vector3){ 0.38f, 0.3f, 0.02f }, DARK_COL, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.78f, 1.027f }, (Vector3){ 0.3f, 0.21f, 0.01f },
         (Color){ (unsigned char)(lamp.r * 0.6f), (unsigned char)(lamp.g * 0.6f), (unsigned char)(lamp.b * 0.6f), 255 }, 0.45f * lampGlow);
    Part(MESH_CUBE, (Vector3){ 0, 0.42f, 1.02f }, (Vector3){ 0.26f, 0.06f, 0.03f }, DARK_COL, 0);   // the slot
    Part(MESH_CYLINDER, (Vector3){ 0, 1.16f, 0.8f }, (Vector3){ 0.1f, 0.12f, 0.1f }, METAL_COL, 0);
    Part(MESH_SPHERE, (Vector3){ 0, 1.3f, 0.8f }, (Vector3){ 0.22f, 0.22f, 0.22f }, lamp, lampGlow);
    // the boom: hinged at the kiosk, it reaches across the path and swings up
    Part(MESH_CUBE, (Vector3){ 0, 0.4f, -0.95f }, (Vector3){ 0.12f, 0.8f, 0.12f }, METAL_COL, 0);
    rlPushMatrix();
    rlTranslatef(0, 0.84f, 0.56f);
    rlRotatef(Clamp01(open) * 78, 1, 0, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0, -0.78f }, (Vector3){ 0.09f, 0.09f, 1.56f }, (Color){ 240, 200, 70, 255 }, 0.12f);
    for (int i = 0; i < 4; i++)
        Part(MESH_CUBE, (Vector3){ 0, 0, -0.25f - i * 0.36f }, (Vector3){ 0.095f, 0.095f, 0.14f }, DARK_COL, 0);
    rlPopMatrix();
    rlPopMatrix();
}

// An exit facing local +z: a frame with a light strip, and two halves that slide apart
void KitDoor(Vector3 pos, float yaw, float open, Color light, float lightGlow)
{
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(yaw, 0, 1, 0);
    for (int s = -1; s <= 1; s += 2)
        Part(MESH_CUBE, (Vector3){ s * 0.7f, 0.82f, 0 }, (Vector3){ 0.24f, 1.64f, 0.38f }, METAL_COL, 0);
    Part(MESH_CUBE, (Vector3){ 0, 1.74f, 0 }, (Vector3){ 1.64f, 0.24f, 0.42f }, METAL_COL, 0);
    Part(MESH_CUBE, (Vector3){ 0, 1.74f, 0.215f }, (Vector3){ 1.26f, 0.07f, 0.01f }, light, lightGlow);
    // the dark inside, seen when the doors part
    Part(MESH_CUBE, (Vector3){ 0, 0.8f, -0.16f }, (Vector3){ 1.16f, 1.6f, 0.04f }, (Color){ 14, 16, 26, 255 }, 0);
    float slide = Clamp01(open) * 0.5f;
    for (int s = -1; s <= 1; s += 2) {
        float x = s * (0.29f + slide);
        Part(MESH_CUBE, (Vector3){ x, 0.8f, -0.02f }, (Vector3){ 0.56f, 1.56f, 0.1f }, (Color){ 92, 100, 130, 255 }, 0);
        Part(MESH_CUBE, (Vector3){ x - s * 0.22f, 0.8f, 0.035f }, (Vector3){ 0.04f, 1.2f, 0.01f }, light, lightGlow * 0.7f);
    }
    rlPopMatrix();
}
