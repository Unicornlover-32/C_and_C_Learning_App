#ifndef GAME_GFX_H
#define GAME_GFX_H

// The 3D engine every level scene is built on:
//   tween.c     easing curves and springs (squash and stretch)
//   shaders.c   the lighting shader (flat shading, sun + sky + rim light) and the
//               post-processing (bloom, vignette), with a plain fallback
//   models.c    low-poly parts made from raylib's mesh generators, and the robot
//   particles.c sparks, confetti, dust and sparkle
//   labels.c    text pinned to points in the 3D world
//   scene.c     the camera, the render targets, and drawing a scene into a rectangle
#include "raylib.h"
#include <stdbool.h>

// ---------------------------------------------------------------- tween.c
float   Clamp01(float x);
float   Lerpf(float a, float b, float t);
Vector3 LerpV3(Vector3 a, Vector3 b, float t);
float   EaseInQuad(float t);
float   EaseOutQuad(float t);
float   EaseInOutQuad(float t);
float   EaseOutCubic(float t);
float   EaseInOutCubic(float t);
float   EaseOutBack(float t);
float   EaseOutElastic(float t);
float   EaseOutBounce(float t);
float   Smooth(float current, float target, float rate, float dt);   // eases towards target, frame-rate independent
float   Hash01(int n);                                               // a repeatable "random" number 0..1
float   Pulse(float time, float speed);                               // 0..1..0, smooth

// A spring that rests at 1: kick it below 1 to squash, above 1 to stretch
typedef struct { float value, velocity; } Spring;
void SpringReset(Spring *s);
void SpringKick(Spring *s, float amount);
void SpringUpdate(Spring *s, float dt);

// ---------------------------------------------------------------- shaders.c
// Opaque surfaces write how much they glow into the alpha channel; the bloom pass
// spreads exactly that glow, so only things meant to glow do.
bool   GfxInit(void);                       // false: shaders failed, everything is drawn plain
void   GfxShutdown(void);
bool   GfxReady(void);
Shader GfxLitShader(void);
void   GfxSetCamera(Vector3 position);
void   GfxSetSurface(Color emissive, float glow, bool transparent);
// Draws the finished scene texture into dest with bloom and a vignette
void   GfxPost(RenderTexture2D scene, Rectangle dest, int viewW, int viewH);

// ---------------------------------------------------------------- models.c
typedef enum { MESH_CUBE, MESH_SPHERE, MESH_CYLINDER, MESH_CONE, MESH_PYRAMID, MESH_COUNT } MeshKind;

void ModelsInit(void);
void ModelsShutdown(void);
// One part of a model: a unit mesh scaled to `size` and centred on `center`, inside the
// current rlgl matrix (use rlPushMatrix / rlTranslatef / rlRotatef to build hierarchies).
// glow 0..1 makes it light up (and bloom).
void Part(MeshKind kind, Vector3 center, Vector3 size, Color color, float glow);
void PartRot(MeshKind kind, Vector3 center, Vector3 size, Vector3 rotationDeg, Color color, float glow);
void PartGlass(MeshKind kind, Vector3 center, Vector3 size, Color color);   // see-through (color.a)
void BlobShadow(Vector3 at, float radius, float strength);                 // a soft dark spot on the floor
void BlendOpaque(void);
void BlendGlass(void);

typedef struct {
    Vector3 pos;
    float   yaw;          // degrees; 0 faces +z (towards the camera)
    float   bob;          // lifted off the ground
    float   squash;       // 1 = normal, <1 squashed, >1 stretched
    float   lean;         // forward lean, degrees
    float   headTurn, headTilt;
    float   armL, armR;   // degrees; 0 hangs down, 180 points up
    float   wheel;        // wheel spin, degrees
    float   blink;        // 0 open, 1 closed
    float   power;        // 1 on, 0 off (eyes go dark)
    Color   eyes;
    Color   body, accent;
    float   antenna;      // antenna light 0..1
    float   scale;        // 1 = full size (about 1.3 tall)
} RobotPose;
RobotPose RobotPoseDefault(Vector3 pos);
void      DrawRobot(const RobotPose *p);

// ---------------------------------------------------------------- particles.c
void FxInit(void);
void FxClear(void);
void FxEnable(bool on);                        // off while a run is being fast-forwarded
void FxUpdate(float dt);
void FxDraw(void);
void FxSparks(Vector3 at, int count, Color color);
void FxConfetti(Vector3 at, int count, float spread);
void FxDust(Vector3 at, int count);
void FxSparkle(Vector3 at, int count, Color color);

// ---------------------------------------------------------------- labels.c
typedef enum { LABEL_CODE = 1, LABEL_BIG = 2, LABEL_PLAIN = 4, LABEL_LEFT = 8 } LabelFlags;   // LEFT: `at` is the left end
void LabelsClear(void);
// Queued while drawing the 3D scene, drawn on top of it afterwards. bg.a == 0: no box.
void Label3D(Vector3 at, const char *text, float size, Color fg, Color bg, int flags);
void LabelsDraw(Rectangle view, Camera3D camera);

// ---------------------------------------------------------------- scene.c
void     GameInit(void);                 // after InitWindow
void     GameShutdown(void);             // before CloseWindow
void     CamSet(Vector3 position, Vector3 target, float fovy);
void     CamMove(Vector3 position, Vector3 target, float fovy, float seconds);
void     CamShake(float amount);
void     CamUpdate(float dt);
Camera3D CamCurrent(void);
void     SceneBegin(Rectangle view);     // starts drawing the 3D world for this rectangle
void     SceneEnd(Rectangle view);       // finishes it: post-processing, then the labels

#endif
