// Little things that fly: sparks when something breaks, confetti when a level is won,
// dust when things land and sparkle on coins. Drawn as tiny cubes in one batch.
#include "gfx.h"
#include "rlgl.h"
#include <math.h>
#include <stddef.h>

#define FX_MAX 1600

enum { FX_SPARK, FX_CONFETTI, FX_DUST, FX_SPARKLE };

typedef struct {
    Vector3 p, v;
    float   life, max, size, rot, spin;
    Color   color;
    int     kind;
} Particle;

static Particle parts[FX_MAX];
static int      count;
static bool     enabled = true;
static unsigned seed = 12345;

static float Rnd(void)
{
    seed = seed * 1664525u + 1013904223u;
    return (float)((seed >> 8) & 0xFFFF) / 65535.0f;
}

static float Rnd2(void) { return Rnd() * 2 - 1; }

void FxInit(void) { count = 0; }
void FxClear(void) { count = 0; }
void FxEnable(bool on) { enabled = on; }

static Particle *New(int kind, Vector3 at, float life)
{
    if (!enabled || count >= FX_MAX) return NULL;
    Particle *q = &parts[count++];
    q->kind = kind;
    q->p = at;
    q->life = q->max = life * (0.7f + 0.6f * Rnd());
    q->rot = Rnd() * 360;
    q->spin = Rnd2() * 600;
    return q;
}

void FxSparks(Vector3 at, int n, Color color)
{
    for (int i = 0; i < n; i++) {
        Particle *q = New(FX_SPARK, at, 0.55f);
        if (!q) return;
        q->v = (Vector3){ Rnd2() * 3.2f, 1.5f + Rnd() * 3.5f, Rnd2() * 3.2f };
        q->size = 0.035f + Rnd() * 0.03f;
        q->color = i % 3 == 0 ? (Color){ 255, 250, 220, 255 } : color;
    }
}

void FxConfetti(Vector3 at, int n, float spread)
{
    static const Color cols[] = { { 255, 205, 70, 255 }, { 86, 156, 255, 255 }, { 80, 200, 120, 255 },
                                  { 255, 110, 140, 255 }, { 190, 130, 255, 255 }, { 255, 255, 255, 255 } };
    for (int i = 0; i < n; i++) {
        Particle *q = New(FX_CONFETTI, (Vector3){ at.x + Rnd2() * spread, at.y, at.z + Rnd2() * spread }, 2.6f);
        if (!q) return;
        q->v = (Vector3){ Rnd2() * 2.4f, 3.5f + Rnd() * 4.0f, Rnd2() * 2.4f };
        q->size = 0.07f + Rnd() * 0.04f;
        q->color = cols[i % 6];
    }
}

void FxDust(Vector3 at, int n)
{
    for (int i = 0; i < n; i++) {
        Particle *q = New(FX_DUST, (Vector3){ at.x + Rnd2() * 0.2f, at.y + 0.05f, at.z + Rnd2() * 0.2f }, 0.7f);
        if (!q) return;
        float a = Rnd() * 2 * PI;
        q->v = (Vector3){ cosf(a) * 1.1f, 0.3f + Rnd() * 0.4f, sinf(a) * 1.1f };
        q->size = 0.06f + Rnd() * 0.05f;
        q->color = (Color){ 120, 126, 146, 255 };
    }
}

void FxSparkle(Vector3 at, int n, Color color)
{
    for (int i = 0; i < n; i++) {
        Particle *q = New(FX_SPARKLE, (Vector3){ at.x + Rnd2() * 0.3f, at.y + Rnd2() * 0.2f, at.z + Rnd2() * 0.3f }, 0.8f);
        if (!q) return;
        q->v = (Vector3){ Rnd2() * 0.4f, 0.6f + Rnd() * 0.8f, Rnd2() * 0.4f };
        q->size = 0.03f + Rnd() * 0.03f;
        q->color = color;
    }
}

void FxUpdate(float dt)
{
    for (int i = 0; i < count; i++) {
        Particle *q = &parts[i];
        q->life -= dt;
        if (q->life <= 0) { parts[i--] = parts[--count]; continue; }
        float gravity = 9.0f, drag = 0.0f;
        switch (q->kind) {
            case FX_SPARK:    gravity = 9.5f; drag = 0.6f; break;
            case FX_CONFETTI: gravity = 3.2f; drag = 2.4f; break;
            case FX_DUST:     gravity = -0.4f; drag = 3.0f; break;
            case FX_SPARKLE:  gravity = 0.0f; drag = 1.0f; break;
        }
        q->v.y -= gravity * dt;
        float k = expf(-drag * dt);
        q->v.x *= k;
        q->v.z *= k;
        if (q->kind == FX_CONFETTI) {        // flutter side to side as it falls
            q->v.y *= k;
            q->v.x += sinf(q->life * 9 + q->rot) * 0.9f * dt;
        }
        q->p.x += q->v.x * dt;
        q->p.y += q->v.y * dt;
        q->p.z += q->v.z * dt;
        if (q->p.y < 0.01f && q->v.y < 0) {   // the floor
            q->p.y = 0.01f;
            if (q->kind == FX_SPARK) { q->v.y *= -0.35f; q->v.x *= 0.6f; q->v.z *= 0.6f; }
            else { q->v = (Vector3){ 0, 0, 0 }; q->spin = 0; }
        }
        q->rot += q->spin * dt;
    }
}

void FxDraw(void)
{
    for (int i = 0; i < count; i++) {
        const Particle *q = &parts[i];
        float t = q->life / q->max;                 // 1 when new, 0 when gone
        float s = q->size;
        unsigned char glow = 255;
        switch (q->kind) {
            case FX_SPARK:    s *= 0.4f + 0.6f * t; break;
            case FX_CONFETTI: s *= t < 0.2f ? t / 0.2f : 1; glow = 40; break;
            case FX_DUST:     s *= 0.6f + (1 - t) * 1.2f; s *= t < 0.3f ? t / 0.3f : 1; glow = 0; break;
            case FX_SPARKLE:  s *= (0.5f + 0.5f * sinf(q->life * 30)) * t; break;
        }
        if (s <= 0.002f) continue;
        Color c = q->color;
        c.a = glow;                                  // alpha is the bloom mask
        rlPushMatrix();
        rlTranslatef(q->p.x, q->p.y, q->p.z);
        rlRotatef(q->rot, 0.3f, 1, 0.5f);
        if (q->kind == FX_CONFETTI) DrawCube((Vector3){ 0, 0, 0 }, s, s * 0.12f, s * 0.7f, c);
        else DrawCube((Vector3){ 0, 0, 0 }, s, s, s, c);
        rlPopMatrix();
    }
}
