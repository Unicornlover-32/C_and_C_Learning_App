// Every model is built in code from a handful of unit meshes (made once with raylib's
// GenMesh functions), so the app needs no model files. A part is one of those meshes,
// scaled, turned and placed inside the current rlgl matrix; models are lists of parts.
#include "gfx.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

static Mesh     meshes[MESH_COUNT];
static Material material;
static bool     loaded;

void ModelsInit(void)
{
    meshes[MESH_CUBE] = GenMeshCube(1, 1, 1);
    meshes[MESH_SPHERE] = GenMeshSphere(0.5f, 7, 10);       // few rings: the facets show
    meshes[MESH_CYLINDER] = GenMeshCylinder(0.5f, 1, 14);
    meshes[MESH_CONE] = GenMeshCone(0.5f, 1, 12);
    meshes[MESH_PYRAMID] = GenMeshCone(0.5f, 1, 4);       // four sides: roofs
    material = LoadMaterialDefault();
    if (GfxReady()) material.shader = GfxLitShader();
    loaded = true;
}

void ModelsShutdown(void)
{
    if (!loaded) return;
    for (int i = 0; i < MESH_COUNT; i++) UnloadMesh(meshes[i]);
    MemFree(material.maps);                     // the shader belongs to shaders.c
    loaded = false;
}

// Opaque things write their glow into alpha (the bloom mask). See-through things blend
// their colour and leave the mask alone.
void BlendOpaque(void)
{
    rlSetBlendFactorsSeparate(RL_ONE, RL_ZERO, RL_ONE, RL_ZERO, RL_FUNC_ADD, RL_FUNC_ADD);
    rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);
}

void BlendGlass(void)
{
    rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_ZERO, RL_ONE, RL_FUNC_ADD, RL_FUNC_ADD);
    rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);
}

static void DrawPart(MeshKind kind, Vector3 c, Vector3 size, Vector3 rot, Color color, float glow, bool glass)
{
    if (!loaded || (int)kind < 0 || (int)kind >= MESH_COUNT) return;
    Matrix m = MatrixScale(size.x, size.y, size.z);
    if (kind == MESH_CYLINDER || kind == MESH_CONE)          // those are made standing on y = 0
        m = MatrixMultiply(MatrixTranslate(0, -0.5f, 0), m);
    if (rot.x != 0 || rot.y != 0 || rot.z != 0)
        m = MatrixMultiply(m, MatrixRotateXYZ((Vector3){ rot.x * DEG2RAD, rot.y * DEG2RAD, rot.z * DEG2RAD }));
    m = MatrixMultiply(m, MatrixTranslate(c.x, c.y, c.z));
    GfxSetSurface(color, glow, glass);
    material.maps[MATERIAL_MAP_DIFFUSE].color = color;
    DrawMesh(meshes[kind], material, m);
}

void Part(MeshKind kind, Vector3 center, Vector3 size, Color color, float glow)
{
    DrawPart(kind, center, size, (Vector3){ 0, 0, 0 }, color, glow, false);
}

void PartRot(MeshKind kind, Vector3 center, Vector3 size, Vector3 rotationDeg, Color color, float glow)
{
    DrawPart(kind, center, size, rotationDeg, color, glow, false);
}

void PartGlass(MeshKind kind, Vector3 center, Vector3 size, Color color)
{
    BlendGlass();
    DrawPart(kind, center, size, (Vector3){ 0, 0, 0 }, color, 0, true);
    BlendOpaque();
}

void BlobShadow(Vector3 at, float radius, float strength)
{
    float s = strength < 0 ? 0 : strength > 1 ? 1 : strength;
    BlendGlass();
    DrawPart(MESH_CYLINDER, (Vector3){ at.x, at.y + 0.006f, at.z }, (Vector3){ radius * 2.2f, 0.004f, radius * 2.2f },
             (Vector3){ 0, 0, 0 }, (Color){ 0, 0, 0, (unsigned char)(70 * s) }, 0, true);
    DrawPart(MESH_CYLINDER, (Vector3){ at.x, at.y + 0.008f, at.z }, (Vector3){ radius * 1.5f, 0.004f, radius * 1.5f },
             (Vector3){ 0, 0, 0 }, (Color){ 0, 0, 0, (unsigned char)(90 * s) }, 0, true);
    BlendOpaque();
}

// ---------------------------------------------------------------- the robot

RobotPose RobotPoseDefault(Vector3 pos)
{
    RobotPose p = { 0 };
    p.pos = pos;
    p.squash = 1;
    p.power = 1;
    p.eyes = (Color){ 120, 235, 255, 255 };
    p.body = (Color){ 214, 222, 236, 255 };
    p.accent = (Color){ 96, 170, 255, 255 };
    p.antenna = 0.6f;
    p.scale = 1;
    return p;
}

static Color Dim(Color c, float k)
{
    return (Color){ (unsigned char)(c.r * k), (unsigned char)(c.g * k), (unsigned char)(c.b * k), c.a };
}

static void Arm(float side, float angle, const RobotPose *p, Color joint)
{
    rlPushMatrix();
    rlTranslatef(side * 0.335f, 0.64f, 0);
    rlRotatef(-angle, 1, 0, 0);
    rlRotatef(side * 6, 0, 0, 1);
    Part(MESH_SPHERE, (Vector3){ 0, 0, 0 }, (Vector3){ 0.12f, 0.12f, 0.12f }, joint, 0);
    Part(MESH_CUBE, (Vector3){ 0, -0.14f, 0 }, (Vector3){ 0.085f, 0.24f, 0.1f }, p->body, 0);
    Part(MESH_SPHERE, (Vector3){ 0, -0.29f, 0 }, (Vector3){ 0.13f, 0.12f, 0.13f }, p->accent, 0);
    rlPopMatrix();
}

void DrawRobot(const RobotPose *p)
{
    Color dark = { 40, 44, 58, 255 }, mid = { 88, 96, 120, 255 };
    float on = p->power < 0 ? 0 : p->power > 1 ? 1 : p->power;
    Color eyes = (Color){ (unsigned char)Lerp(50, p->eyes.r, on), (unsigned char)Lerp(56, p->eyes.g, on),
                          (unsigned char)Lerp(70, p->eyes.b, on), 255 };
    float sq = p->squash < 0.4f ? 0.4f : p->squash;
    float wide = 1.0f / sqrtf(sq);

    rlPushMatrix();
    float size = p->scale > 0 ? p->scale : 1;
    rlTranslatef(p->pos.x, p->pos.y + p->bob, p->pos.z);
    rlRotatef(p->yaw, 0, 1, 0);
    rlScalef(wide * size, sq * size, wide * size);
    rlRotatef(p->lean, 1, 0, 0);

    // wheels, with a hub that shows them turning
    for (int s = -1; s <= 1; s += 2) {
        rlPushMatrix();
        rlTranslatef(s * 0.275f, 0.16f, 0);
        rlRotatef(p->wheel, 1, 0, 0);
        PartRot(MESH_CYLINDER, (Vector3){ 0, 0, 0 }, (Vector3){ 0.32f, 0.11f, 0.32f }, (Vector3){ 0, 0, 90 }, dark, 0);
        Part(MESH_CUBE, (Vector3){ s * 0.06f, 0.075f, 0 }, (Vector3){ 0.03f, 0.07f, 0.05f }, p->accent, 0.2f * on);
        Part(MESH_CUBE, (Vector3){ s * 0.06f, -0.075f, 0 }, (Vector3){ 0.03f, 0.07f, 0.05f }, mid, 0);
        rlPopMatrix();
    }
    Part(MESH_CUBE, (Vector3){ 0, 0.22f, 0 }, (Vector3){ 0.44f, 0.1f, 0.34f }, mid, 0);

    // body, with a light on the chest
    Part(MESH_CUBE, (Vector3){ 0, 0.5f, 0 }, (Vector3){ 0.56f, 0.44f, 0.42f }, p->body, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.3f, 0 }, (Vector3){ 0.58f, 0.05f, 0.44f }, p->accent, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.52f, 0.212f }, (Vector3){ 0.3f, 0.17f, 0.02f }, dark, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.52f, 0.224f }, (Vector3){ 0.21f, 0.08f, 0.012f }, Dim(p->accent, 0.6f + 0.4f * on), 0.55f * on);

    Arm(-1, p->armL, p, mid);
    Arm(1, p->armR, p, mid);

    // head: face plate, eyes, ear bolts and the antenna
    Part(MESH_CYLINDER, (Vector3){ 0, 0.76f, 0 }, (Vector3){ 0.12f, 0.07f, 0.12f }, mid, 0);
    rlPushMatrix();
    rlTranslatef(0, 0.79f, 0);
    rlRotatef(p->headTurn, 0, 1, 0);
    rlRotatef(p->headTilt, 0, 0, 1);
    Part(MESH_CUBE, (Vector3){ 0, 0.17f, 0 }, (Vector3){ 0.5f, 0.33f, 0.38f }, p->body, 0);
    Part(MESH_CUBE, (Vector3){ 0, 0.16f, 0.191f }, (Vector3){ 0.42f, 0.23f, 0.02f }, dark, 0);
    float eyeH = 0.1f * (1 - Clamp01(p->blink) * 0.85f);
    for (int s = -1; s <= 1; s += 2) {
        Part(MESH_CUBE, (Vector3){ s * 0.1f, 0.17f, 0.205f }, (Vector3){ 0.085f, eyeH, 0.02f }, eyes, on);
        PartRot(MESH_CYLINDER, (Vector3){ s * 0.265f, 0.17f, 0 }, (Vector3){ 0.13f, 0.04f, 0.13f }, (Vector3){ 0, 0, 90 }, p->accent, 0);
    }
    Part(MESH_CYLINDER, (Vector3){ 0, 0.4f, 0 }, (Vector3){ 0.025f, 0.13f, 0.025f }, mid, 0);
    Part(MESH_SPHERE, (Vector3){ 0, 0.49f, 0 }, (Vector3){ 0.09f, 0.09f, 0.09f }, p->accent, Clamp01(p->antenna) * on);
    rlPopMatrix();

    rlPopMatrix();
}
