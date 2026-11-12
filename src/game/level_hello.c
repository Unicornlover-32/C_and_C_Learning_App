// Hello, World! -> "Boot Up"
//
// The robot stands powered off. When the program prints, it boots: its eyes light up and
// a holographic screen opens from its chest and types exactly what printf printed, letter
// by letter. A typo glitches the screen red from the first wrong letter on, with the line
// it should have printed underneath. Printing nothing leaves the screen blank.
#include "levels.h"
#include "stage.h"
#include "../ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { HB_ROBOT = 1, HB_TYPE, HB_LINE };
enum { HBF_TYPO = 1, HBF_BLANK, HBF_EXTRA };

// ---------------------------------------------------------------- the parser

static void ParseHello(const LevelRun *run, EventList *ev)
{
    Parse p;
    ParseBegin(&p, run, ev);
    EvSpawn(&p, HB_ROBOT, 0, 0, 0);
    if (ParsePreview(&p)) return;

    char note[120];
    for (int e = 0; e < p.exp.count && !p.stopped; e++) {
        int el, al;
        const char *E = LineAt(&p.exp, e, &el);
        const char *A = ParsePeek(&p, &al);
        if (ParseTake(&p, e)) {
            GameEvent *g = EvAct(&p, HB_TYPE, e, el, 0);
            if (g) { StrCopy(g->text, EV_TEXT, E); if (el < EV_TEXT) g->text[el] = '\0'; }
            EvAct(&p, HB_LINE, e, 0, 0);
            continue;
        }
        if (!A) { ParseFail(&p, FAIL_MISSING, HBF_BLANK, e, 0, 0, 0, "Nothing was printed, so the screen stayed blank"); break; }
        // type the part that is right, then glitch from the first wrong letter
        ParseShow(&p);
        int k = 0;
        while (k < el && k < al && E[k] == A[k]) k++;
        GameEvent *g = EvAct(&p, HB_TYPE, e, k, 0);
        if (g) { int n = k < EV_TEXT - 1 ? k : EV_TEXT - 1; memcpy(g->text, A, (size_t)n); g->text[n] = '\0'; }
        if (k < el && k < al && (E[k] | 32) == (A[k] | 32) && ((E[k] | 32) >= 'a' && (E[k] | 32) <= 'z'))
            snprintf(note, sizeof(note), "Capital letters matter: character %d should be '%c', not '%c'", k + 1, E[k], A[k]);
        else if (k >= al)
            snprintf(note, sizeof(note), "Something is missing at the end: '%c' should come next", E[k]);
        else if (k >= el)
            snprintf(note, sizeof(note), "There's something extra at the end: '%c'", A[k]);
        else
            snprintf(note, sizeof(note), "Character %d should be '%c', not '%c'", k + 1, E[k], A[k] == ' ' ? '_' : A[k]);
        ParseFail(&p, FAIL_WRONG, HBF_TYPO, e, k, al, 0, note);
        break;
    }
    ParseEnd(&p, HBF_EXTRA);
}

// ---------------------------------------------------------------- the scene

enum { DO_BOOT, DO_CHEER, DO_GLITCH };

static struct {
    const EventList *ev;
    Bot   bot;
    Cues  cues;
    Pops  pops;
    float time;
    float screen;           // 0 closed .. 1 open
    float boot;             // power coming on
    char  text[EV_TEXT];    // what has been typed (the right part)
    int   typed, target;    // letters shown / letters to type
    float typeT;
    bool  glitch, blank, done;
    char  bad[FAIL_TEXT];   // the wrong part, shown in red
    char  want[FAIL_TEXT];  // what it should have printed
    int   lines;            // new lines printed
} H;

static Vector3 ScreenPos(void) { return V3(0, 1.08f, 0.9f); }

static void HelloReset(const EventList *ev)
{
    memset(&H, 0, sizeof(H));
    H.ev = ev;
    BotInit(&H.bot, V3(0, 0.32f, -0.5f), 0);
    H.bot.scale = 1.25f;
    H.bot.off = true;                  // waits, powered down, for the program
    H.bot.power = 0;
    ShotOrbit(V3(0, 1.0f, 0.2f), 6.2f, 16, 0, 38, 0);
}

static float HelloEvent(const GameEvent *e)
{
    switch (e->type) {
        case EV_SPAWN: return 0;
        case EV_PRINT: return 0.05f;
        case EV_ACT:
            if (e->code == HB_TYPE) {
                if (H.bot.off) CueAt(&H.cues, H.time, DO_BOOT, 0, 0, V3(0, 0, 0), 0);
                StrCopy(H.text, sizeof(H.text), e->text);
                H.target = (int)strlen(H.text);
                H.typed = 0;
                return 0.9f + H.target * 0.075f;
            }
            if (e->code == HB_LINE) {
                H.lines++;
                PopAdd(&H.pops, "\\n", V3(1.45f, 1.0f, 0.95f), (Color){ 160, 200, 255, 255 });
                return 0.35f;
            }
            return 0;
        case EV_FAIL:
            H.done = true;
            if (e->code == HBF_BLANK) {
                if (H.bot.off) CueAt(&H.cues, H.time, DO_BOOT, 0, 0, V3(0, 0, 0), 0);
                H.blank = true;
                BotLookAround(&H.bot);
                return 2.4f;
            }
            StrCopy(H.want, sizeof(H.want), H.ev->expected);
            {   // the wrong part: from the first wrong letter of what was printed
                int k = e->a, n = (int)strlen(H.ev->actual);
                StrCopy(H.bad, sizeof(H.bad), k < n ? H.ev->actual + k : "");
            }
            CueAt(&H.cues, H.time + 0.2f, DO_GLITCH, 0, 0, V3(0, 0, 0), 0);
            return 2.6f;
        case EV_CRASH:
            H.done = true;
            BotPowerDown(&H.bot);
            PlaybackShake(0.2f);
            return 2.4f;
        case EV_WIN:
            H.done = true;
            CueAt(&H.cues, H.time + 0.3f, DO_CHEER, 0, 0, V3(0, 0, 0), 0);
            return 2.2f;
    }
    return 0;
}

static void HelloUpdate(float dt)
{
    H.time += dt;
    BotUpdate(&H.bot, dt);
    PopsUpdate(&H.pops, dt);
    Cue c;
    while (CueDue(&H.cues, H.time, &c)) {
        switch (c.what) {
            case DO_BOOT:
                H.bot.off = false;
                H.boot = 1;
                FxSparkle(BotHead(&H.bot), 24, (Color){ 120, 235, 255, 255 });
                break;
            case DO_GLITCH:
                H.glitch = true;
                FxSparks(ScreenPos(), 40, (Color){ 255, 90, 90, 255 });
                BotMood(&H.bot, MOOD_SAD);
                BotShakeHead(&H.bot);
                PlaybackShake(0.22f);
                break;
            case DO_CHEER:
                BotCheer(&H.bot);
                FxConfetti(V3(0, 1.6f, 0.4f), PlaybackFinale() ? 150 : 70, 0.8f);
                if (PlaybackFinale()) PlaybackSlowMo(0.3f, 1.3f);
                break;
        }
    }
    bool on = !H.bot.off && (H.target > 0 || H.blank || H.glitch);
    H.screen = Smooth(H.screen, on ? 1.0f : 0.0f, 7, dt);
    H.boot = Smooth(H.boot, 0, 2, dt);
    if (H.screen > 0.8f && H.typed < H.target) {
        H.typeT += dt;
        while (H.typeT > 0.075f && H.typed < H.target) {
            H.typeT -= 0.075f;
            H.typed++;
            if (H.text[H.typed - 1] != ' ')
                FxSparkle(V3(-1.1f + 2.2f * H.typed / (float)(H.target > 13 ? H.target : 13), 1.08f, 0.95f), 2, (Color){ 140, 230, 255, 255 });
        }
    }
}

static void HelloDraw(void)
{
    KitStage(V3(0, 0, 0.3f), 3.6f, 2.4f);
    // the pedestal the robot stands on
    Part(MESH_CYLINDER, V3(0, 0.16f, -0.5f), V3(1.4f, 0.32f, 1.4f), (Color){ 60, 66, 90, 255 }, 0);
    Part(MESH_CYLINDER, V3(0, 0.325f, -0.5f), V3(1.25f, 0.02f, 1.25f), (Color){ 86, 156, 255, 255 }, 0.35f + 0.4f * H.boot);
    BotDraw(&H.bot);

    // the holographic screen: a beam from the chest and a glowing pane
    float s = EaseOutBack(Clamp01(H.screen));
    if (s > 0.02f) {
        Vector3 c = ScreenPos();
        Color edge = H.glitch ? (Color){ 255, 90, 90, 255 } : (Color){ 110, 210, 255, 255 };
        float w = 2.9f * s, h = 0.85f * s;
        PartGlass(MESH_CUBE, c, V3(w, h, 0.02f), (Color){ 40, 120, 200, 70 });
        Part(MESH_CUBE, V3(c.x, c.y + h / 2, c.z), V3(w, 0.025f, 0.03f), edge, 0.9f);
        Part(MESH_CUBE, V3(c.x, c.y - h / 2, c.z), V3(w, 0.025f, 0.03f), edge, 0.9f);
        Part(MESH_CUBE, V3(c.x - w / 2, c.y, c.z), V3(0.025f, h, 0.03f), edge, 0.9f);
        Part(MESH_CUBE, V3(c.x + w / 2, c.y, c.z), V3(0.025f, h, 0.03f), edge, 0.9f);
        PartGlass(MESH_CONE, V3(0, 0.98f, 0.3f), V3(0.5f * s, 0.6f, 0.5f * s), (Color){ 90, 190, 255, 40 });
    }
    PopsDraw(&H.pops);
    if (H.ev && H.ev->preview)
        Label3D(V3(0, 2.35f, -0.5f), "waiting for printf...", 15, LBL_SOFT, LBL_DIM, LABEL_CODE);
}

// The screen's text is drawn flat on top, so it can be two colours and glitch
static void HelloOverlay(Rectangle view)
{
    if (H.screen < 0.85f) return;
    Camera3D cam = CamCurrent();
    Vector3 c = ScreenPos();
    Vector2 l = GetWorldToScreenEx(V3(c.x - 1.3f, c.y, c.z), cam, (int)view.width, (int)view.height);
    Vector2 r = GetWorldToScreenEx(V3(c.x + 1.3f, c.y, c.z), cam, (int)view.width, (int)view.height);
    Vector2 m = GetWorldToScreenEx(c, cam, (int)view.width, (int)view.height);
    float px = r.x - l.x;
    int good = H.typed, bad = H.glitch ? (int)strlen(H.bad) : 0;
    int chars = good + bad + 1;
    if (chars < 13) chars = 13;
    float size = px / (chars * 0.62f);
    if (size > view.height * 0.09f) size = view.height * 0.09f;
    float sp = UiCodeSpacing(size);
    float cw = MeasureTextEx(gCodeFont, "M", size, sp).x + sp;
    float x = view.x + m.x - cw * (good + bad) / 2, y = view.y + m.y - size / 2;

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
    if (H.blank) {
        const char *t = "(nothing)";
        float w = MeasureTextEx(gCodeFont, t, size * 0.8f, sp).x;
        DrawTextEx(gCodeFont, t, (Vector2){ view.x + m.x - w / 2, y }, size * 0.8f, sp, (Color){ 255, 130, 130, 255 });
    } else {
        char buf[EV_TEXT];
        StrCopy(buf, sizeof(buf), H.text);
        buf[good < (int)sizeof(buf) ? good : (int)sizeof(buf) - 1] = '\0';
        DrawTextEx(gCodeFont, buf, (Vector2){ x, y }, size, sp, (Color){ 200, 245, 255, 255 });
        for (int i = 0; i < bad; i++) {       // the glitch: each wrong letter jitters
            char one[2] = { H.bad[i], 0 };
            float jx = sinf(H.time * 47 + i * 3.1f) * size * 0.08f, jy = cosf(H.time * 39 + i * 1.7f) * size * 0.1f;
            DrawTextEx(gCodeFont, one, (Vector2){ x + cw * (good + i) + jx, y + jy }, size, sp, (Color){ 255, 90, 90, 255 });
        }
        bool blink = fmodf(H.time, 0.8f) < 0.45f && !H.done;
        if (blink) DrawRectangle((int)(x + cw * (good + bad)), (int)(y + size * 0.85f), (int)(cw * 0.8f), (int)(size * 0.1f) + 1,
                                 (Color){ 200, 245, 255, 255 });
        if (H.glitch && H.want[0]) {
            float s2 = size * 0.42f;
            const char *t = TextFormat("should be: %s", H.want);
            float w = MeasureTextEx(gCodeFont, t, s2, UiCodeSpacing(s2)).x;
            DrawRectangleRounded((Rectangle){ view.x + m.x - w / 2 - 8, y + size * 1.25f - 3, w + 16, s2 + 6 }, 0.4f, 6, (Color){ 12, 14, 22, 220 });
            DrawTextEx(gCodeFont, t, (Vector2){ view.x + m.x - w / 2, y + size * 1.25f }, s2, UiCodeSpacing(s2), (Color){ 140, 255, 180, 255 });
        }
    }
    EndScissorMode();
}

const LevelDef LEVEL_HELLO = {
    "c-hello", "Boot Up", "Your printf is the first thing the robot ever says - letter by letter.",
    { 120, 210, 255, 255 },
    ParseHello, HelloReset, HelloEvent, HelloUpdate, HelloDraw, HelloOverlay,
};
