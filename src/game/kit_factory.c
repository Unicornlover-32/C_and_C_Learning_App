// The factory: conveyors, crates, machines, screens, chutes and dials.
#include "kits.h"
#include "rlgl.h"
#include <math.h>

#define METAL  (Color){ 78, 86, 112, 255 }
#define DARK   (Color){ 24, 26, 36, 255 }
#define FRAME  (Color){ 52, 58, 78, 255 }
#define ACCENT (Color){ 86, 156, 255, 255 }

static Color Shade(Color c, float k)
{
    return (Color){ (unsigned char)fminf(255, c.r * k), (unsigned char)fminf(255, c.g * k), (unsigned char)fminf(255, c.b * k), c.a };
}

void KitBelt(Vector3 left, float length, float scroll)
{
    float cx = left.x + length / 2, z = left.z;
    Part(MESH_CUBE, (Vector3){ cx, BELT_Y - 0.1f, z }, (Vector3){ length, 0.14f, 0.9f }, FRAME, 0);
    Part(MESH_CUBE, (Vector3){ cx, BELT_Y - 0.02f, z }, (Vector3){ length - 0.08f, 0.04f, 0.74f }, (Color){ 38, 42, 54, 255 }, 0);
    float step = 0.34f, off = fmodf(scroll, step);
    if (off < 0) off += step;
    for (float x = left.x + 0.06f + off; x < left.x + length - 0.06f; x += step)
        Part(MESH_CUBE, (Vector3){ x, BELT_Y + 0.001f, z }, (Vector3){ 0.05f, 0.012f, 0.72f }, (Color){ 66, 72, 94, 255 }, 0);
    for (int s = -1; s <= 1; s += 2)
        Part(MESH_CUBE, (Vector3){ cx, BELT_Y - 0.03f, z + s * 0.46f }, (Vector3){ length, 0.035f, 0.03f }, ACCENT, 0.45f);
    for (float x = left.x + 0.3f; x <= left.x + length - 0.2f; x += 1.6f)
        for (int s = -1; s <= 1; s += 2)
            Part(MESH_CUBE, (Vector3){ x, (BELT_Y - 0.17f) / 2, z + s * 0.34f }, (Vector3){ 0.1f, BELT_Y - 0.17f, 0.1f }, METAL, 0);
    for (int e = 0; e < 2; e++)
        PartRot(MESH_CYLINDER, (Vector3){ left.x + (e ? length : 0), BELT_Y - 0.1f, z }, (Vector3){ 0.2f, 0.9f, 0.2f }, (Vector3){ 90, 0, 0 }, METAL, 0);
}

void KitCrate(Vector3 pos, float s, Color color, float glow, float yaw)
{
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y + s / 2, pos.z);
    rlRotatef(yaw, 0, 1, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0, 0 }, (Vector3){ s, s, s }, color, glow * 0.5f);
    Color band = Shade(color, 0.68f);
    Part(MESH_CUBE, (Vector3){ 0, s * 0.34f, 0 }, (Vector3){ s * 1.03f, s * 0.1f, s * 1.03f }, band, glow * 0.3f);
    Part(MESH_CUBE, (Vector3){ 0, -s * 0.34f, 0 }, (Vector3){ s * 1.03f, s * 0.1f, s * 1.03f }, band, glow * 0.3f);
    rlPopMatrix();
}

void KitMachine(Vector3 pos, Vector3 size, Color body, Color light, float glow, float busy)
{
    float hx = size.x / 2, hz = size.z / 2;
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + size.y / 2, pos.z }, size, body, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + size.y + 0.05f, pos.z }, (Vector3){ size.x + 0.1f, 0.1f, size.z + 0.1f }, Shade(body, 0.75f), 0);
    // the tunnel the belt runs through
    for (int s = -1; s <= 1; s += 2)
        Part(MESH_CUBE, (Vector3){ pos.x + s * (hx + 0.005f), BELT_Y + 0.3f, pos.z }, (Vector3){ 0.03f, 0.6f, fminf(0.8f, size.z - 0.1f) }, DARK, 0);
    // a front panel with a light strip
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + size.y * 0.62f, pos.z + hz + 0.01f }, (Vector3){ size.x * 0.72f, size.y * 0.34f, 0.02f }, DARK, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + size.y * 0.88f, pos.z + hz + 0.015f }, (Vector3){ size.x * 0.8f, 0.05f, 0.02f }, light, glow);
    // two pistons that pump while it works
    for (int s = -1; s <= 1; s += 2) {
        float h = 0.18f + 0.12f * (0.5f + 0.5f * sinf(busy * 9 + s));
        Part(MESH_CYLINDER, (Vector3){ pos.x + s * hx * 0.45f, pos.y + size.y + 0.1f + h / 2, pos.z }, (Vector3){ 0.16f, h, 0.16f }, METAL, 0);
        Part(MESH_CYLINDER, (Vector3){ pos.x + s * hx * 0.45f, pos.y + size.y + 0.12f + h, pos.z }, (Vector3){ 0.26f, 0.06f, 0.26f }, light, glow * 0.6f);
    }
}

void KitScreen(Vector3 pos, float w, float h, Color tint, float glow)
{
    float pole = 1.15f;
    Part(MESH_CYLINDER, (Vector3){ pos.x, pos.y + pole / 2, pos.z }, (Vector3){ 0.09f, pole, 0.09f }, METAL, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.04f, pos.z }, (Vector3){ 0.4f, 0.08f, 0.3f }, FRAME, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + pole + h / 2, pos.z }, (Vector3){ w + 0.1f, h + 0.1f, 0.1f }, FRAME, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + pole + h / 2, pos.z + 0.055f }, (Vector3){ w, h, 0.01f }, Shade(tint, 0.35f), glow * 0.35f);
}

Vector3 KitScreenCenter(Vector3 pos, float h) { return (Vector3){ pos.x, pos.y + 1.15f + h / 2, pos.z + 0.08f }; }

void KitChute(Vector3 pos, Color color, float glow)
{
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.28f, pos.z }, (Vector3){ 0.84f, 0.56f, 0.84f }, Shade(color, 0.45f), 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y + 0.58f, pos.z }, (Vector3){ 0.9f, 0.06f, 0.9f }, color, glow);
    PartRot(MESH_CONE, (Vector3){ pos.x, pos.y + 1.12f, pos.z }, (Vector3){ 0.9f, 0.55f, 0.9f }, (Vector3){ 180, 0, 0 }, METAL, 0);
    Part(MESH_CYLINDER, (Vector3){ pos.x, pos.y + 1.45f, pos.z }, (Vector3){ 0.92f, 0.1f, 0.92f }, FRAME, 0);
}

void KitGauge(Vector3 pos, float r, float k, Color needle, float glow)
{
    PartRot(MESH_CYLINDER, (Vector3){ pos.x, pos.y, pos.z - 0.02f }, (Vector3){ r * 2.2f, 0.06f, r * 2.2f }, (Vector3){ 90, 0, 0 }, METAL, 0);
    PartRot(MESH_CYLINDER, pos, (Vector3){ r * 2, 0.07f, r * 2 }, (Vector3){ 90, 0, 0 }, (Color){ 28, 30, 42, 255 }, 0);
    for (int i = 0; i <= 6; i++) {
        float a = (-120 + 40 * i) * DEG2RAD;
        Color tick = i == 6 ? (Color){ 235, 90, 90, 255 } : (Color){ 150, 160, 190, 255 };
        Part(MESH_CUBE, (Vector3){ pos.x + sinf(a) * r * 0.82f, pos.y + cosf(a) * r * 0.82f, pos.z + 0.04f }, (Vector3){ 0.04f, 0.04f, 0.02f }, tick, 0.2f);
    }
    float kk = k < -0.15f ? -0.15f : k > 1.2f ? 1.2f : k;
    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z + 0.05f);
    rlRotatef(120 - 240 * kk, 0, 0, 1);
    Part(MESH_CUBE, (Vector3){ 0, r * 0.38f, 0 }, (Vector3){ 0.035f, r * 0.76f, 0.02f }, needle, glow);
    rlPopMatrix();
    Part(MESH_SPHERE, (Vector3){ pos.x, pos.y, pos.z + 0.05f }, (Vector3){ 0.08f, 0.08f, 0.05f }, METAL, 0);
}

void KitLamp(Vector3 pos, Color color, float glow)
{
    Part(MESH_CYLINDER, (Vector3){ pos.x, pos.y + 0.3f, pos.z }, (Vector3){ 0.07f, 0.6f, 0.07f }, METAL, 0);
    Part(MESH_SPHERE, (Vector3){ pos.x, pos.y + 0.66f, pos.z }, (Vector3){ 0.2f, 0.2f, 0.2f }, color, glow);
}

void KitPanel(Vector3 pos, float w, float h, Color color, float glow)
{
    Part(MESH_CUBE, pos, (Vector3){ w + 0.08f, h + 0.08f, 0.06f }, FRAME, 0);
    Part(MESH_CUBE, (Vector3){ pos.x, pos.y, pos.z + 0.035f }, (Vector3){ w, h, 0.01f }, color, glow);
}
