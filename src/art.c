#include "art.h"
#include "progress.h"
#include <math.h>

Color ModeColor(GameMode mode)
{
    switch (mode) {
        case MODE_ROBOT: return COL_ROBOT;
        case MODE_HEIST: return COL_HEIST;
        case MODE_BUGS:  return COL_BUGS;
        default:         return COL_FACTORY;
    }
}

static Vector2 Polar(Vector2 c, float r, float a) { return (Vector2){ c.x + cosf(a) * r, c.y + sinf(a) * r }; }

// raylib only draws triangles wound counter-clockwise; drawing both windings means any order works
static void Tri(Vector2 a, Vector2 b, Vector2 c, Color color)
{
    DrawTriangle(a, b, c, color);
    DrawTriangle(a, c, b, color);
}

void DrawStar(Vector2 c, float r, Color color)
{
    float inner = r * 0.45f;
    for (int i = 0; i < 5; i++) {
        float a = -PI / 2 + i * 2 * PI / 5;
        Vector2 tip = Polar(c, r, a);
        Vector2 l = Polar(c, inner, a - PI / 5);
        Vector2 rr = Polar(c, inner, a + PI / 5);
        Tri(tip, l, rr, color);
        Tri(c, rr, l, color);
    }
}

void DrawStarOutline(Vector2 c, float r, float thick, Color color)
{
    float inner = r * 0.45f;
    Vector2 pts[11];
    for (int i = 0; i < 10; i++) {
        float a = -PI / 2 + i * PI / 5;
        pts[i] = Polar(c, (i % 2) ? inner : r, a);
    }
    pts[10] = pts[0];
    for (int i = 0; i < 10; i++) DrawLineEx(pts[i], pts[i + 1], thick, color);
}

void DrawStarRow(float centerX, float y, float r, unsigned char bits, int count)
{
    float gap = r * 2.3f;
    for (int i = 0; i < 3; i++) {
        Vector2 c = { centerX + (i - 1) * gap, y };
        bool on = (count >= 0) ? (i < count) : ((bits >> i) & 1);
        if (on) {
            DrawStar((Vector2){ c.x, c.y + r * 0.12f }, r, COL_GOLD_DARK);
            DrawStar(c, r, COL_GOLD);
        } else {
            DrawStar(c, r, (Color){ 255, 255, 255, 28 });
            DrawStarOutline(c, r, 1.5f, (Color){ 255, 255, 255, 70 });
        }
    }
}

void IconRobot(Vector2 c, float s, Color color)
{
    float w = s * 0.8f, h = s * 0.62f;
    DrawLineEx((Vector2){ c.x, c.y - h / 2 }, (Vector2){ c.x, c.y - h / 2 - s * 0.2f }, s * 0.06f, color);
    DrawCircleV((Vector2){ c.x, c.y - h / 2 - s * 0.22f }, s * 0.08f, (Color){ 255, 110, 110, 255 });
    DrawRectangleRounded((Rectangle){ c.x - w / 2, c.y - h / 2, w, h }, 0.35f, 6, color);
    DrawRectangleRounded((Rectangle){ c.x - w * 0.36f, c.y - h * 0.28f, w * 0.72f, h * 0.42f }, 0.5f, 6, (Color){ 20, 30, 50, 255 });
    DrawCircleV((Vector2){ c.x - w * 0.17f, c.y - h * 0.07f }, s * 0.07f, (Color){ 120, 255, 200, 255 });
    DrawCircleV((Vector2){ c.x + w * 0.17f, c.y - h * 0.07f }, s * 0.07f, (Color){ 120, 255, 200, 255 });
    DrawRectangle((int)(c.x - w * 0.2f), (int)(c.y + h * 0.26f), (int)(w * 0.4f), (int)(s * 0.05f + 1), (Color){ 20, 30, 50, 200 });
}

void IconMask(Vector2 c, float s, Color color)
{
    // a burglar's eye mask
    DrawRectangleRounded((Rectangle){ c.x - s * 0.48f, c.y - s * 0.18f, s * 0.96f, s * 0.36f }, 0.8f, 8, color);
    DrawCircleV((Vector2){ c.x - s * 0.2f, c.y }, s * 0.11f, (Color){ 15, 15, 25, 255 });
    DrawCircleV((Vector2){ c.x + s * 0.2f, c.y }, s * 0.11f, (Color){ 15, 15, 25, 255 });
    DrawLineEx((Vector2){ c.x + s * 0.46f, c.y }, (Vector2){ c.x + s * 0.58f, c.y + s * 0.2f }, s * 0.06f, color);
    DrawLineEx((Vector2){ c.x + s * 0.46f, c.y }, (Vector2){ c.x + s * 0.62f, c.y + s * 0.06f }, s * 0.06f, color);
}

void IconGear(Vector2 c, float s, float angle, Color color)
{
    float r = s * 0.32f;
    for (int i = 0; i < 8; i++) {
        float a = angle + i * PI / 4;
        Vector2 p = Polar(c, r * 1.25f, a);
        DrawRectanglePro((Rectangle){ p.x, p.y, s * 0.18f, s * 0.18f }, (Vector2){ s * 0.09f, s * 0.09f }, a * RAD2DEG, color);
    }
    DrawCircleV(c, r, color);
    DrawCircleV(c, r * 0.42f, (Color){ 30, 32, 42, 255 });
}

void IconBug(Vector2 c, float s, float angle, Color color)
{
    Vector2 f = { cosf(angle), sinf(angle) }, side = { -f.y, f.x };
    for (int i = -1; i <= 1; i++) {
        Vector2 base = { c.x + f.x * i * s * 0.14f, c.y + f.y * i * s * 0.14f };
        for (int k = -1; k <= 1; k += 2) {
            Vector2 tip = { base.x + side.x * k * s * 0.42f + f.x * i * s * 0.08f, base.y + side.y * k * s * 0.42f + f.y * i * s * 0.08f };
            DrawLineEx(base, tip, s * 0.05f, (Color){ 30, 40, 30, 255 });
        }
    }
    DrawEllipse((int)c.x, (int)c.y, s * 0.26f, s * 0.26f, color);
    DrawCircleV((Vector2){ c.x + f.x * s * 0.27f, c.y + f.y * s * 0.27f }, s * 0.15f, (Color){ 40, 60, 40, 255 });
    DrawLineEx((Vector2){ c.x - f.x * s * 0.24f, c.y - f.y * s * 0.24f }, (Vector2){ c.x + f.x * s * 0.14f, c.y + f.y * s * 0.14f },
               s * 0.03f, (Color){ 30, 50, 30, 200 });
    DrawCircleV((Vector2){ c.x + side.x * s * 0.1f, c.y + side.y * s * 0.1f }, s * 0.045f, (Color){ 20, 30, 20, 255 });
    DrawCircleV((Vector2){ c.x - side.x * s * 0.1f - f.x * s * 0.1f, c.y - side.y * s * 0.1f - f.y * s * 0.1f }, s * 0.045f, (Color){ 20, 30, 20, 255 });
}

void IconBook(Vector2 c, float s, Color color)
{
    DrawRectangleRounded((Rectangle){ c.x - s * 0.42f, c.y - s * 0.32f, s * 0.4f, s * 0.64f }, 0.2f, 4, color);
    DrawRectangleRounded((Rectangle){ c.x + s * 0.02f, c.y - s * 0.32f, s * 0.4f, s * 0.64f }, 0.2f, 4, color);
    for (int i = 0; i < 3; i++) {
        float y = c.y - s * 0.16f + i * s * 0.14f;
        DrawLineEx((Vector2){ c.x - s * 0.34f, y }, (Vector2){ c.x - s * 0.08f, y }, s * 0.03f, (Color){ 30, 32, 42, 200 });
        DrawLineEx((Vector2){ c.x + s * 0.08f, y }, (Vector2){ c.x + s * 0.34f, y }, s * 0.03f, (Color){ 30, 32, 42, 200 });
    }
}

void IconRush(Vector2 c, float s, Color color)
{
    // falling blocks
    DrawRectangleRounded((Rectangle){ c.x - s * 0.42f, c.y + s * 0.12f, s * 0.84f, s * 0.28f }, 0.3f, 4, ColorAlpha(color, 0.55f));
    DrawRectangleRounded((Rectangle){ c.x - s * 0.22f, c.y - s * 0.42f, s * 0.44f, s * 0.36f }, 0.3f, 4, color);
    DrawLineEx((Vector2){ c.x - s * 0.36f, c.y - s * 0.36f }, (Vector2){ c.x - s * 0.36f, c.y - s * 0.12f }, s * 0.05f, color);
    DrawLineEx((Vector2){ c.x + s * 0.36f, c.y - s * 0.32f }, (Vector2){ c.x + s * 0.36f, c.y - s * 0.08f }, s * 0.05f, color);
}

void IconCrown(Vector2 c, float s, Color color)
{
    Vector2 a = { c.x - s * 0.42f, c.y + s * 0.25f }, b = { c.x + s * 0.42f, c.y + s * 0.25f };
    DrawRectangleRounded((Rectangle){ a.x, a.y - s * 0.08f, s * 0.84f, s * 0.16f }, 0.4f, 4, color);
    Vector2 p1 = { c.x - s * 0.42f, c.y - s * 0.3f }, p2 = { c.x, c.y - s * 0.42f }, p3 = { c.x + s * 0.42f, c.y - s * 0.3f };
    Tri(a, b, p1, color);
    Tri(a, b, p3, color);
    Tri((Vector2){ c.x - s * 0.25f, c.y + s * 0.2f }, (Vector2){ c.x + s * 0.25f, c.y + s * 0.2f }, p2, color);
    Tri(a, (Vector2){ c.x, c.y + s * 0.2f }, p1, color);
    Tri((Vector2){ c.x, c.y + s * 0.2f }, b, p3, color);
    DrawCircleV(p1, s * 0.07f, color);
    DrawCircleV(p2, s * 0.07f, color);
    DrawCircleV(p3, s * 0.07f, color);
    DrawCircleV((Vector2){ c.x, c.y + s * 0.02f }, s * 0.07f, (Color){ 235, 90, 90, 255 });
}

void IconFlame(Vector2 c, float s, Color color)
{
    DrawCircleV((Vector2){ c.x, c.y + s * 0.14f }, s * 0.26f, color);
    Tri((Vector2){ c.x - s * 0.26f, c.y + s * 0.1f }, (Vector2){ c.x + s * 0.26f, c.y + s * 0.1f },
                 (Vector2){ c.x + s * 0.04f, c.y - s * 0.44f }, color);
    DrawCircleV((Vector2){ c.x, c.y + s * 0.2f }, s * 0.13f, (Color){ 255, 230, 120, 255 });
}

void IconFactory(Vector2 c, float s, Color color)
{
    float x = c.x - s * 0.45f, y = c.y + s * 0.38f;
    DrawRectangle((int)(c.x + s * 0.18f), (int)(c.y - s * 0.45f), (int)(s * 0.14f), (int)(s * 0.5f), color);
    Vector2 pts[] = { { x, y }, { x, c.y - s * 0.05f }, { x + s * 0.3f, c.y - s * 0.25f }, { x + s * 0.3f, c.y - s * 0.05f },
                      { x + s * 0.6f, c.y - s * 0.25f }, { x + s * 0.6f, c.y - s * 0.05f }, { x + s * 0.9f, c.y - s * 0.25f },
                      { x + s * 0.9f, y } };
    for (int i = 1; i < 7; i++) Tri(pts[0], pts[i + 1], pts[i], color);
    DrawRectangle((int)(c.x - s * 0.1f), (int)(c.y + s * 0.1f), (int)(s * 0.2f), (int)(s * 0.28f), (Color){ 30, 32, 42, 255 });
    DrawCircleV((Vector2){ c.x + s * 0.3f, c.y - s * 0.58f }, s * 0.08f, (Color){ 200, 200, 210, 150 });
}

void IconLock(Vector2 c, float s, Color color)
{
    DrawRing((Vector2){ c.x, c.y - s * 0.1f }, s * 0.16f, s * 0.24f, 180, 360, 16, color);
    DrawRectangleRounded((Rectangle){ c.x - s * 0.3f, c.y - s * 0.1f, s * 0.6f, s * 0.46f }, 0.25f, 4, color);
    DrawCircleV((Vector2){ c.x, c.y + s * 0.1f }, s * 0.06f, (Color){ 30, 32, 42, 255 });
}

void IconMedal(Vector2 c, float s, Color color, int index)
{
    Tri((Vector2){ c.x - s * 0.3f, c.y - s * 0.5f }, (Vector2){ c.x - s * 0.05f, c.y }, (Vector2){ c.x + s * 0.05f, c.y - s * 0.5f },
                 (Color){ 86, 156, 255, 255 });
    Tri((Vector2){ c.x - s * 0.05f, c.y - s * 0.5f }, (Vector2){ c.x + s * 0.05f, c.y }, (Vector2){ c.x + s * 0.3f, c.y - s * 0.5f },
                 (Color){ 235, 90, 90, 255 });
    DrawCircleV((Vector2){ c.x, c.y + s * 0.12f }, s * 0.32f, ColorBrightness(color, -0.35f));
    DrawCircleV((Vector2){ c.x, c.y + s * 0.1f }, s * 0.29f, color);
    DrawStar((Vector2){ c.x, c.y + s * 0.11f }, s * 0.16f + (index % 3) * s * 0.01f, (Color){ 255, 255, 255, 230 });
}

void DrawModeIcon(GameMode mode, Vector2 c, float s)
{
    switch (mode) {
        case MODE_ROBOT: IconRobot(c, s, COL_ROBOT); break;
        case MODE_HEIST: IconMask(c, s, COL_HEIST); break;
        case MODE_BUGS:  IconBug(c, s, -PI / 2, COL_BUGS); break;
        default:         IconGear(c, s, (float)GetTime() * 0.6f, COL_FACTORY); break;
    }
}

// ---------------------------------------------------------------- confetti

#define MAX_PARTICLES 400

typedef struct { Vector2 p, v; float life, spin, angle; Color color; } Particle;
static Particle parts[MAX_PARTICLES];

void ParticlesBurst(float x, float y, int count, float speed)
{
    static const Color colors[] = { { 255, 205, 70, 255 }, { 120, 190, 255, 255 }, { 255, 110, 140, 255 },
                                    { 120, 215, 110, 255 }, { 190, 130, 255, 255 } };
    for (int i = 0, made = 0; i < MAX_PARTICLES && made < count; i++) {
        if (parts[i].life > 0) continue;
        float a = GetRandomValue(0, 3600) / 3600.0f * 2 * PI;
        float v = speed * (0.4f + GetRandomValue(0, 100) / 100.0f);
        parts[i] = (Particle){ { x, y }, { cosf(a) * v, sinf(a) * v - speed * 0.6f }, 1.6f + GetRandomValue(0, 100) / 100.0f,
                               (GetRandomValue(0, 1) ? 1 : -1) * 6.0f, 0, colors[GetRandomValue(0, 4)] };
        made++;
    }
}

void ParticlesDraw(void)
{
    float dt = GetFrameTime();
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &parts[i];
        if (p->life <= 0) continue;
        p->life -= dt;
        p->v.y += 900 * dt;
        p->v.x *= 0.99f;
        p->p.x += p->v.x * dt;
        p->p.y += p->v.y * dt;
        p->angle += p->spin * dt;
        float a = p->life > 0.5f ? 1.0f : p->life * 2;
        DrawRectanglePro((Rectangle){ p->p.x, p->p.y, 9, 5 }, (Vector2){ 4.5f, 2.5f }, p->angle * RAD2DEG, ColorAlpha(p->color, a));
    }
}
