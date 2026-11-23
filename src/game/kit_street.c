// Memory street: houses with addresses, vaults, pointer arrows, gold, bags and heap lots.
#include "kits.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

#define METAL (Color){ 78, 86, 112, 255 }
#define DARK  (Color){ 22, 24, 34, 255 }

void KitHouse(Vector3 pos, Color wall, Color roof, float glow)
{
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.45f, pos.z }, (Vector3){ 1.0f, 0.9f, 0.9f }, wall, 0);
    PartRot(MESH_PYRAMID, (Vector3){ pos.x, pos.y + 1.2f, pos.z }, (Vector3){ 1.48f, 0.62f, 1.36f }, (Vector3){ 0, 45, 0 }, roof, 0);
    Part(MESH_CUBE, (Vector3){ pos.x - 0.18f, pos.y + 0.27f, pos.z + 0.455f }, (Vector3){ 0.26f, 0.46f, 0.02f }, DARK, 0);
    Part(MESH_CUBE, (Vector3){ pos.x + 0.22f, pos.y + 0.56f, pos.z + 0.455f }, (Vector3){ 0.24f, 0.2f, 0.02f },
         (Color){ 255, 214, 130, 255 }, 0.35f + glow);
    Part(MESH_CUBE, (Vector3){ pos.x + 0.3f, pos.y + 1.36f, pos.z - 0.15f }, (Vector3){ 0.14f, 0.3f, 0.14f }, roof, 0);
}

void KitVault(Vector3 pos, float open, Color light, float glow)
{
    Color steel = { 112, 122, 148, 255 };
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.6f, pos.z }, (Vector3){ 1.2f, 1.2f, 1.0f }, steel, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 1.24f, pos.z }, (Vector3){ 1.3f, 0.1f, 1.1f }, METAL, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 1.3f, pos.z + 0.5f }, (Vector3){ 0.9f, 0.05f, 0.02f }, light, glow);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.6f, pos.z + 0.49f }, (Vector3){ 0.8f, 0.8f, 0.02f }, DARK, 0);
    // the round door swings open on its left hinge
    rlPushMatrix();
    rlTranslatef(pos.x - 0.38f, pos.y + 0.6f, pos.z + 0.53f);
    rlRotatef(-Clamp01(open) * 105, 0, 1, 0);
    PartRot(MESH_CYLINDER, (Vector3){ 0.38f, 0, 0 }, (Vector3){ 0.74f, 0.08f, 0.74f }, (Vector3){ 90, 0, 0 }, (Color){ 140, 150, 176, 255 }, 0);
    PartRot(MESH_CYLINDER, (Vector3){ 0.38f, 0, 0.05f }, (Vector3){ 0.3f, 0.04f, 0.3f }, (Vector3){ 90, 0, 0 }, METAL, 0);
    for (int i = 0; i < 3; i++)
        PartRot(MESH_CUBE, (Vector3){ 0.38f, 0, 0.07f }, (Vector3){ 0.4f, 0.04f, 0.03f }, (Vector3){ 0, 0, i * 60.0f }, METAL, 0);
    rlPopMatrix();
}

static Vector3 Bezier(Vector3 a, Vector3 c, Vector3 b, float t)
{
    float u = 1 - t;
    return (Vector3){ u * u * a.x + 2 * u * t * c.x + t * t * b.x, u * u * a.y + 2 * u * t * c.y + t * t * b.y,
                      u * u * a.z + 2 * u * t * c.z + t * t * b.z };
}

void KitArc(Vector3 from, Vector3 to, float lift, float grow, Color color, float glow)
{
    float g = Clamp01(grow);
    if (g <= 0) return;
    Vector3 ctrl = { (from.x + to.x) / 2, fmaxf(from.y, to.y) + lift, (from.z + to.z) / 2 };
    const int N = 18;
    int segs = (int)ceilf(N * g);
    for (int i = 0; i < segs; i++) {
        float t0 = (float)i / N, t1 = fminf((float)(i + 1) / N, g);
        Vector3 a = Bezier(from, ctrl, to, t0), b = Bezier(from, ctrl, to, t1);
        Vector3 d = Vector3Subtract(b, a);
        float len = Vector3Length(d);
        if (len < 0.001f) continue;
        float yaw = atan2f(-d.z, d.x) * RAD2DEG, pitch = atan2f(d.y, sqrtf(d.x * d.x + d.z * d.z)) * RAD2DEG;
        rlPushMatrix();
        rlTranslatef((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);
        rlRotatef(yaw, 0, 1, 0);
        rlRotatef(pitch, 0, 0, 1);
        Part(MESH_CUBE, (Vector3){ 0, 0, 0 }, (Vector3){ len * 1.15f, 0.07f, 0.07f }, color, glow);
        rlPopMatrix();
    }
    if (g >= 1)       // the arrow head, pointing down at the target
        PartRot(MESH_CONE, (Vector3){ to.x, to.y + 0.12f, to.z }, (Vector3){ 0.24f, 0.26f, 0.24f }, (Vector3){ 180, 0, 0 }, color, glow);
}

void KitGold(Vector3 pos, int bars, float glow)
{
    Color gold = { 255, 200, 70, 255 };
    int shown = bars > 6 ? 6 : bars;
    static const float lx[6] = { -0.2f, 0.0f, 0.2f, -0.1f, 0.1f, 0.0f };
    static const float ly[6] = { 0, 0, 0, 1, 1, 2 };
    for (int i = 0; i < shown; i++)
        Part(MESH_CUBE, (Vector3){ pos.x + lx[i], pos.y + 0.055f + ly[i] * 0.11f, pos.z }, (Vector3){ 0.18f, 0.1f, 0.3f }, gold, glow);
}

void KitBag(Vector3 pos, float size, Color color)
{
    Part(MESH_SPHERE, (Vector3){ pos.x, pos.y + size * 0.45f, pos.z }, (Vector3){ size, size * 0.9f, size }, color, 0);
    Part(MESH_SPHERE, (Vector3){ pos.x, pos.y + size * 0.95f, pos.z }, (Vector3){ size * 0.32f, size * 0.26f, size * 0.32f },
         (Color){ (unsigned char)(color.r * 0.7f), (unsigned char)(color.g * 0.7f), (unsigned char)(color.b * 0.7f), 255 }, 0);
}

void KitRoad(Vector3 left, float length)
{
    Part(MESH_CUBE, (Vector3){ left.x + length / 2, left.y + 0.006f, left.z }, (Vector3){ length, 0.012f, 0.9f }, (Color){ 30, 32, 42, 255 }, 0);
    for (float x = left.x + 0.25f; x < left.x + length - 0.2f; x += 0.7f)
        Part(MESH_CUBE, (Vector3){ x + 0.15f, left.y + 0.014f, left.z }, (Vector3){ 0.3f, 0.005f, 0.06f }, (Color){ 220, 220, 200, 255 }, 0.15f);
}

void KitLot(Vector3 pos, float w, float d, Color color, float glow)
{
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.01f, pos.z }, (Vector3){ w, 0.02f, d }, (Color){ 30, 34, 48, 255 }, 0);
    for (int s = -1; s <= 1; s += 2) {
        Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.03f, pos.z + s * d / 2 }, (Vector3){ w, 0.03f, 0.04f }, color, glow);
        Part(MESH_CUBE, (Vector3){ pos.x + s * w / 2, pos.y + 0.03f, pos.z }, (Vector3){ 0.04f, 0.03f, d }, color, glow);
    }
}
