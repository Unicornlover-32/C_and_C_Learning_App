// The camera and the render target every scene is drawn into.
//
// A scene is drawn at up to twice the view's size into a render texture (so edges come
// out smooth), then post-processed into the view rectangle. The camera has a fixed
// angled shot that levels move between with eased camera moves, a gentle idle sway,
// and a short shake for failures.
#include "gfx.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

static RenderTexture2D target;
static int  targetW, targetH;
static bool targetReady;

static struct {
    Vector3 pos, look;  float fovy;
    Vector3 fromPos, fromLook; float fromFovy;
    Vector3 toPos, toLook;     float toFovy;
    float   t, duration;
    float   shake, time;
} cam = { .pos = { 0, 6, 8 }, .fovy = 40, .fromFovy = 40, .toFovy = 40 };

void GameInit(void)
{
    GfxInit();
    ModelsInit();
    FxInit();
}

void GameShutdown(void)
{
    if (targetReady) UnloadRenderTexture(target);
    targetReady = false;
    ModelsShutdown();
    GfxShutdown();
}

// ---------------------------------------------------------------- the camera

void CamSet(Vector3 position, Vector3 lookAt, float fovy)
{
    cam.pos = cam.toPos = position;
    cam.look = cam.toLook = lookAt;
    cam.fovy = cam.toFovy = fovy;
    cam.duration = 0;
}

void CamMove(Vector3 position, Vector3 lookAt, float fovy, float seconds)
{
    if (seconds <= 0) { CamSet(position, lookAt, fovy); return; }
    cam.fromPos = cam.pos;
    cam.fromLook = cam.look;
    cam.fromFovy = cam.fovy;
    cam.toPos = position;
    cam.toLook = lookAt;
    cam.toFovy = fovy;
    cam.t = 0;
    cam.duration = seconds;
}

void CamShake(float amount) { if (amount > cam.shake) cam.shake = amount; }

void CamUpdate(float dt)
{
    cam.time += dt;
    if (cam.duration > 0) {
        cam.t += dt;
        float k = EaseInOutCubic(cam.t / cam.duration);
        cam.pos = LerpV3(cam.fromPos, cam.toPos, k);
        cam.look = LerpV3(cam.fromLook, cam.toLook, k);
        cam.fovy = Lerpf(cam.fromFovy, cam.toFovy, k);
        if (cam.t >= cam.duration) cam.duration = 0;
    }
    cam.shake *= expf(-7.0f * dt);
    if (cam.shake < 0.002f) cam.shake = 0;
}

Camera3D CamCurrent(void)
{
    float t = cam.time;
    Vector3 sway = { sinf(t * 0.31f) * 0.05f, sinf(t * 0.23f) * 0.035f, 0 };
    Vector3 shake = { 0, 0, 0 };
    if (cam.shake > 0) {
        shake = (Vector3){ sinf(t * 61.0f) * cam.shake, sinf(t * 47.0f + 1.3f) * cam.shake * 0.7f, sinf(t * 53.0f + 2.1f) * cam.shake * 0.5f };
    }
    Camera3D c = { 0 };
    c.position = Vector3Add(Vector3Add(cam.pos, sway), shake);
    c.target = Vector3Add(cam.look, Vector3Scale(shake, 0.6f));
    c.up = (Vector3){ 0, 1, 0 };
    c.fovy = cam.fovy;
    c.projection = CAMERA_PERSPECTIVE;
    return c;
}

// ---------------------------------------------------------------- drawing a scene

void SceneBegin(Rectangle view)
{
    int w = (int)view.width, h = (int)view.height;
    if (w < 16) w = 16;
    if (h < 16) h = 16;
    // supersample small views (up to 2x) but keep big ones near 2 million pixels
    float ss = sqrtf(2.2e6f / (float)(w * h));
    if (ss > 2) ss = 2;
    if (ss < 1) ss = 1;
    int tw = (int)(w * ss), th = (int)(h * ss);
    if (!targetReady || tw != targetW || th != targetH) {
        if (targetReady) UnloadRenderTexture(target);
        target = LoadRenderTexture(tw, th);
        SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
        targetW = tw;
        targetH = th;
        targetReady = true;
    }

    BeginTextureMode(target);
    BlendOpaque();
    ClearBackground((Color){ 0, 0, 0, 0 });
    // the sky behind the stage: deep blue at the top, lighter towards the horizon
    DrawRectangleGradientV(0, 0, tw, th, (Color){ 13, 16, 28, 0 }, (Color){ 30, 34, 52, 0 });
    Camera3D c = CamCurrent();
    BeginMode3D(c);
    GfxSetCamera(c.position);
}

void SceneEnd(Rectangle view)
{
    EndMode3D();
    rlSetBlendMode(RL_BLEND_ALPHA);
    EndTextureMode();
    GfxPost(target, view, (int)view.width, (int)view.height);
    LabelsDraw(view, CamCurrent());
}
