// Easing curves and springs: how things start, stop and land.
#include "gfx.h"
#include <math.h>

float Clamp01(float x) { return x < 0 ? 0 : x > 1 ? 1 : x; }
float Lerpf(float a, float b, float t) { return a + (b - a) * t; }

Vector3 LerpV3(Vector3 a, Vector3 b, float t)
{
    return (Vector3){ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
}

float EaseInQuad(float t)    { t = Clamp01(t); return t * t; }
float EaseOutQuad(float t)   { t = Clamp01(t); return 1 - (1 - t) * (1 - t); }
float EaseInOutQuad(float t) { t = Clamp01(t); return t < 0.5f ? 2 * t * t : 1 - powf(-2 * t + 2, 2) / 2; }
float EaseOutCubic(float t)  { t = Clamp01(t); return 1 - powf(1 - t, 3); }
float EaseInOutCubic(float t){ t = Clamp01(t); return t < 0.5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2; }

float EaseOutBack(float t)
{
    t = Clamp01(t);
    const float c1 = 1.70158f, c3 = c1 + 1;
    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}

float EaseOutElastic(float t)
{
    t = Clamp01(t);
    if (t == 0 || t == 1) return t;
    return powf(2, -10 * t) * sinf((t * 10 - 0.75f) * (2 * PI / 3)) + 1;
}

float EaseOutBounce(float t)
{
    t = Clamp01(t);
    const float n1 = 7.5625f, d1 = 2.75f;
    if (t < 1 / d1) return n1 * t * t;
    if (t < 2 / d1) { t -= 1.5f / d1; return n1 * t * t + 0.75f; }
    if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + 0.9375f; }
    t -= 2.625f / d1;
    return n1 * t * t + 0.984375f;
}

float Smooth(float current, float target, float rate, float dt)
{
    return target + (current - target) * expf(-rate * dt);
}

float Hash01(int n)
{
    unsigned int x = (unsigned int)n * 747796405u + 2891336453u;
    x = ((x >> ((x >> 28) + 4)) ^ x) * 277803737u;
    x = (x >> 22) ^ x;
    return (float)(x & 0xFFFFFF) / (float)0xFFFFFF;
}

float Pulse(float time, float speed) { return 0.5f - 0.5f * cosf(time * speed); }

void SpringReset(Spring *s) { s->value = 1; s->velocity = 0; }
void SpringKick(Spring *s, float amount) { s->value = amount; s->velocity = 0; }

void SpringUpdate(Spring *s, float dt)
{
    // a stiff, slightly bouncy spring towards 1
    const float k = 260, damping = 14;
    float steps = dt > 0.05f ? 0.05f : dt;
    float accel = -k * (s->value - 1) - damping * s->velocity;
    s->velocity += accel * steps;
    s->value += s->velocity * steps;
    if (fabsf(s->value - 1) < 0.0005f && fabsf(s->velocity) < 0.001f) { s->value = 1; s->velocity = 0; }
}
