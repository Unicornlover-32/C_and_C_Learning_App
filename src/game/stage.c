// Small helpers every level scene uses: cues, pop-up words, camera shots, and readers
// for the common kinds of output line.
#include "stage.h"
#include "events.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------- cues

void CuesClear(Cues *q) { memset(q, 0, sizeof(*q)); }

void CueAt(Cues *q, float when, int what, int a, int b, Vector3 v, float f)
{
    for (int i = 0; i < CUE_MAX; i++)
        if (!q->c[i].live) { q->c[i] = (Cue){ when, what, a, b, v, f, true }; return; }
}

bool CueDue(Cues *q, float now, Cue *out)
{
    int best = -1;
    for (int i = 0; i < CUE_MAX; i++)
        if (q->c[i].live && q->c[i].at <= now && (best < 0 || q->c[i].at < q->c[best].at)) best = i;
    if (best < 0) return false;
    *out = q->c[best];
    q->c[best].live = false;
    return true;
}

void CuesDrop(Cues *q, int what)
{
    for (int i = 0; i < CUE_MAX; i++) if (q->c[i].what == what) q->c[i].live = false;
}

// ---------------------------------------------------------------- pops

void PopsClear(Pops *ps) { memset(ps, 0, sizeof(*ps)); }

void PopAdd(Pops *ps, const char *text, Vector3 at, Color color)
{
    PopText *p = &ps->p[ps->next++ % POP_MAX];
    StrCopy(p->text, sizeof(p->text), text);
    p->at = at;
    p->t = 0;
    p->life = 1.3f;
    p->color = color;
}

void PopsUpdate(Pops *ps, float dt)
{
    for (int i = 0; i < POP_MAX; i++) ps->p[i].t += dt;
}

void PopsDraw(const Pops *ps)
{
    for (int i = 0; i < POP_MAX; i++) {
        const PopText *p = &ps->p[i];
        if (!p->text[0] || p->t > p->life) continue;
        Color c = p->color;
        c.a = (unsigned char)(255 * Clamp01((p->life - p->t) / 0.4f));
        Label3D((Vector3){ p->at.x, p->at.y + p->t * 0.55f, p->at.z }, p->text, 16, c,
                (Color){ 12, 14, 22, (unsigned char)(c.a * 0.75f) }, LABEL_CODE);
    }
}

// ---------------------------------------------------------------- shots and text

void ShotOrbit(Vector3 look, float dist, float pitch, float yaw, float fovy, float seconds)
{
    float p = pitch * DEG2RAD, y = yaw * DEG2RAD;
    Vector3 pos = { look.x + sinf(y) * cosf(p) * dist, look.y + sinf(p) * dist, look.z + cosf(y) * cosf(p) * dist };
    CamMove(pos, look, fovy, seconds);
}

void StrCopy(char *dst, int cap, const char *src)
{
    int n = 0;
    while (src && src[n] && n < cap - 1) { dst[n] = src[n]; n++; }
    dst[n] = '\0';
}

// ---------------------------------------------------------------- reading output lines

static bool Prefix(const char *s, int len, const char *prefix, int *i)
{
    int n = (int)strlen(prefix);
    if (!s || len < n || memcmp(s, prefix, (size_t)n) != 0) return false;
    *i = n;
    while (*i < len && (s[*i] == ' ' || s[*i] == '\t')) (*i)++;
    return true;
}

bool ScanAfterNumber(const char *s, int len, const char *prefix, double *value)
{
    int i;
    if (!Prefix(s, len, prefix, &i) || i >= len) return false;
    char buf[48];
    int n = len - i;
    if (n <= 0 || n >= (int)sizeof(buf)) return false;
    memcpy(buf, s + i, (size_t)n);
    buf[n] = '\0';
    for (int k = 0; k < n; k++)                     // digits, one point, a sign: nothing else
        if (!((buf[k] >= '0' && buf[k] <= '9') || buf[k] == '.' || (k == 0 && (buf[k] == '-' || buf[k] == '+')))) return false;
    char *end = NULL;
    double v = strtod(buf, &end);
    if (end == buf || *end != '\0' || !isfinite(v)) return false;
    *value = v;
    return true;
}

bool ScanAfterInts(const char *s, int len, const char *prefix, int *out, int max, int *count)
{
    int i;
    *count = 0;
    if (!Prefix(s, len, prefix, &i)) return false;
    return ScanIntList(s + i, len - i, out, max, count);
}

bool ScanAfterText(const char *s, int len, const char *prefix, char *out, int cap)
{
    int i;
    if (!Prefix(s, len, prefix, &i)) return false;
    int n = len - i;
    if (n > cap - 1) n = cap - 1;
    memcpy(out, s + i, (size_t)(n > 0 ? n : 0));
    out[n > 0 ? n : 0] = '\0';
    return true;
}
