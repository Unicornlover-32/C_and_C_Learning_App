// The look of every scene: one lighting shader and a small post-processing chain.
//
// Lighting: flat shading (each triangle gets one normal, worked out from screen-space
// derivatives, so every low-poly facet shows), a warm sun, a cool sky / dark ground
// ambient, a rim light on the edges and an emissive colour for things that glow.
// Opaque surfaces write their glow amount into alpha, which is the bloom mask.
//
// Post-processing: the scene's glow is cut out at half size, blurred at half and quarter
// size, then added back on top of the scene with a soft vignette.
// If any shader fails to compile, GfxReady() is false and scenes are drawn plain.
#include "gfx.h"
#include "rlgl.h"
#include <stddef.h>

static const char *LIT_VS =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n"
    "out vec3 fragPos;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));\n"
    "    fragColor = vertexColor;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *LIT_FS =
    "#version 330\n"
    "in vec3 fragPos;\n"
    "in vec4 fragColor;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform vec3 viewPos;\n"
    "uniform vec3 lightDir;\n"       // from the surface towards the sun
    "uniform vec3 lightColor;\n"
    "uniform vec3 skyColor;\n"
    "uniform vec3 groundColor;\n"
    "uniform vec3 rimColor;\n"
    "uniform vec4 emissive;\n"        // rgb: light it gives off, a: how much it blooms
    "uniform vec3 fogColor;\n"
    "uniform vec2 fogRange;\n"
    "uniform float transparent;\n"   // 1: alpha is opacity, 0: alpha is the glow mask
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec3 n = normalize(cross(dFdx(fragPos), dFdy(fragPos)));\n"
    "    vec3 v = normalize(viewPos - fragPos);\n"
    "    if (dot(n, v) < 0.0) n = -n;\n"
    "    vec3 base = colDiffuse.rgb * fragColor.rgb;\n"
    "    float sun = max(dot(n, lightDir), 0.0);\n"
    "    vec3 amb = mix(groundColor, skyColor, n.y * 0.5 + 0.5);\n"
    "    float rim = pow(1.0 - max(dot(n, v), 0.0), 3.0);\n"
    "    vec3 col = base * (amb + lightColor * sun) + rimColor * rim + emissive.rgb;\n"
    "    float d = length(viewPos - fragPos);\n"
    "    float f = clamp((d - fogRange.x) / max(fogRange.y - fogRange.x, 0.001), 0.0, 1.0);\n"
    "    col = mix(col, fogColor, f * 0.85);\n"
    "    float a = colDiffuse.a * fragColor.a;\n"
    "    finalColor = vec4(col, mix(emissive.a * (1.0 - f), a, transparent));\n"
    "}\n";

// Cuts the glow out of the (bigger) scene texture: four bilinear taps cover a 4x4 block
static const char *BRIGHT_FS =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 texel;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec3 c = vec3(0.0);\n"
    "    vec4 t;\n"
    "    t = texture(texture0, fragTexCoord + texel * vec2(-1.0, -1.0)); c += t.rgb * t.a;\n"
    "    t = texture(texture0, fragTexCoord + texel * vec2( 1.0, -1.0)); c += t.rgb * t.a;\n"
    "    t = texture(texture0, fragTexCoord + texel * vec2(-1.0,  1.0)); c += t.rgb * t.a;\n"
    "    t = texture(texture0, fragTexCoord + texel * vec2( 1.0,  1.0)); c += t.rgb * t.a;\n"
    "    finalColor = vec4(c * 0.25, 1.0);\n"
    "}\n";

// A 9-tap gaussian in one direction, done with 5 bilinear taps
static const char *BLUR_FS =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 dir;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec3 c = texture(texture0, fragTexCoord).rgb * 0.2270270270;\n"
    "    c += texture(texture0, fragTexCoord + dir * 1.3846153846).rgb * 0.3162162162;\n"
    "    c += texture(texture0, fragTexCoord - dir * 1.3846153846).rgb * 0.3162162162;\n"
    "    c += texture(texture0, fragTexCoord + dir * 3.2307692308).rgb * 0.0702702703;\n"
    "    c += texture(texture0, fragTexCoord - dir * 3.2307692308).rgb * 0.0702702703;\n"
    "    finalColor = vec4(c, 1.0);\n"
    "}\n";

static const char *COMPOSITE_FS =
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "uniform sampler2D texture0;\n"
    "uniform sampler2D bloomNear;\n"
    "uniform sampler2D bloomFar;\n"
    "uniform float bloom;\n"
    "uniform float vignette;\n"
    "out vec4 finalColor;\n"
    "void main() {\n"
    "    vec3 c = texture(texture0, fragTexCoord).rgb;\n"
    "    vec3 b = texture(bloomNear, fragTexCoord).rgb * 1.1 + texture(bloomFar, fragTexCoord).rgb * 0.9;\n"
    "    c += b * bloom;\n"
    "    vec2 q = (fragTexCoord - 0.5) * vec2(1.0, 0.8);\n"
    "    float v = smoothstep(0.78, 0.18, length(q));\n"
    "    c *= mix(1.0 - vignette, 1.0, v);\n"
    "    c = c / (1.0 + max(c - 1.0, 0.0));\n"       // soft clip where bloom stacks up
    "    finalColor = vec4(c, 1.0);\n"
    "}\n";

static struct {
    bool   ready;
    Shader lit, bright, blur, composite;
    int    viewPos, lightDir, lightColor, skyColor, groundColor, rimColor, emissive, fogColor, fogRange, transparent;
    int    brightTexel, blurDir, compNear, compFar, compBloom, compVignette;
    RenderTexture2D a, b, c, d;     // half size (a, b) and quarter size (c, d)
    int    w, h;
} gfx;

static void SetVec3(Shader s, int loc, float x, float y, float z)
{
    float v[3] = { x, y, z };
    SetShaderValue(s, loc, v, SHADER_UNIFORM_VEC3);
}

bool GfxInit(void)
{
    gfx.lit = LoadShaderFromMemory(LIT_VS, LIT_FS);
    gfx.bright = LoadShaderFromMemory(NULL, BRIGHT_FS);
    gfx.blur = LoadShaderFromMemory(NULL, BLUR_FS);
    gfx.composite = LoadShaderFromMemory(NULL, COMPOSITE_FS);
    // a shader that fails to compile comes back as raylib's default one
    unsigned int def = rlGetShaderIdDefault();
    gfx.ready = IsShaderValid(gfx.lit) && IsShaderValid(gfx.bright) && IsShaderValid(gfx.blur) && IsShaderValid(gfx.composite)
                && gfx.lit.id != def && gfx.bright.id != def && gfx.blur.id != def && gfx.composite.id != def;
    if (!gfx.ready) {
        TraceLog(LOG_WARNING, "GAME: shaders unavailable, drawing scenes without lighting");
        UnloadShader(gfx.lit);
        UnloadShader(gfx.bright);
        UnloadShader(gfx.blur);
        UnloadShader(gfx.composite);
        return false;
    }

    Shader s = gfx.lit;
    gfx.viewPos     = GetShaderLocation(s, "viewPos");
    gfx.lightDir    = GetShaderLocation(s, "lightDir");
    gfx.lightColor  = GetShaderLocation(s, "lightColor");
    gfx.skyColor    = GetShaderLocation(s, "skyColor");
    gfx.groundColor = GetShaderLocation(s, "groundColor");
    gfx.rimColor    = GetShaderLocation(s, "rimColor");
    gfx.emissive    = GetShaderLocation(s, "emissive");
    gfx.fogColor    = GetShaderLocation(s, "fogColor");
    gfx.fogRange    = GetShaderLocation(s, "fogRange");
    gfx.transparent = GetShaderLocation(s, "transparent");

    // the light rig: a warm sun from the upper left, a cool sky, a dim ground
    SetVec3(s, gfx.lightDir, -0.42f, 0.82f, 0.39f);
    SetVec3(s, gfx.lightColor, 0.95f, 0.90f, 0.80f);
    SetVec3(s, gfx.skyColor, 0.42f, 0.46f, 0.60f);
    SetVec3(s, gfx.groundColor, 0.20f, 0.18f, 0.22f);
    SetVec3(s, gfx.rimColor, 0.16f, 0.22f, 0.38f);
    SetVec3(s, gfx.fogColor, 26 / 255.0f, 30 / 255.0f, 46 / 255.0f);
    float fog[2] = { 20.0f, 55.0f };
    SetShaderValue(s, gfx.fogRange, fog, SHADER_UNIFORM_VEC2);
    GfxSetSurface(BLACK, 0, false);

    gfx.brightTexel = GetShaderLocation(gfx.bright, "texel");
    gfx.blurDir = GetShaderLocation(gfx.blur, "dir");
    gfx.compNear = GetShaderLocation(gfx.composite, "bloomNear");
    gfx.compFar = GetShaderLocation(gfx.composite, "bloomFar");
    gfx.compBloom = GetShaderLocation(gfx.composite, "bloom");
    gfx.compVignette = GetShaderLocation(gfx.composite, "vignette");
    return true;
}

static void FreeTargets(void)
{
    if (gfx.w == 0) return;
    UnloadRenderTexture(gfx.a);
    UnloadRenderTexture(gfx.b);
    UnloadRenderTexture(gfx.c);
    UnloadRenderTexture(gfx.d);
    gfx.w = gfx.h = 0;
}

void GfxShutdown(void)
{
    FreeTargets();
    if (gfx.ready) {
        UnloadShader(gfx.lit);
        UnloadShader(gfx.bright);
        UnloadShader(gfx.blur);
        UnloadShader(gfx.composite);
    }
    gfx.ready = false;
}

bool   GfxReady(void) { return gfx.ready; }
Shader GfxLitShader(void) { return gfx.lit; }

void GfxSetCamera(Vector3 position)
{
    if (gfx.ready) SetVec3(gfx.lit, gfx.viewPos, position.x, position.y, position.z);
}

void GfxSetSurface(Color emissive, float glow, bool transparent)
{
    if (!gfx.ready) return;
    float g = glow < 0 ? 0 : glow > 1 ? 1 : glow;
    float e[4] = { emissive.r / 255.0f * g, emissive.g / 255.0f * g, emissive.b / 255.0f * g, g };
    float t = transparent ? 1.0f : 0.0f;
    SetShaderValue(gfx.lit, gfx.emissive, e, SHADER_UNIFORM_VEC4);
    SetShaderValue(gfx.lit, gfx.transparent, &t, SHADER_UNIFORM_FLOAT);
}

static RenderTexture2D Target(int w, int h)
{
    RenderTexture2D rt = LoadRenderTexture(w < 1 ? 1 : w, h < 1 ? 1 : h);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(rt.texture, TEXTURE_WRAP_CLAMP);
    return rt;
}

// Draws src over the whole of dst with a shader. Render textures are stored upside
// down; a negative source height keeps every target the same way up.
static void Pass(RenderTexture2D dst, Texture2D src, Shader shader)
{
    BeginTextureMode(dst);
    ClearBackground(BLACK);
    BeginShaderMode(shader);
    DrawTexturePro(src, (Rectangle){ 0, 0, (float)src.width, -(float)src.height },
                   (Rectangle){ 0, 0, (float)dst.texture.width, (float)dst.texture.height }, (Vector2){ 0, 0 }, 0, WHITE);
    EndShaderMode();
    EndTextureMode();
}

static void Blur(RenderTexture2D *ping, RenderTexture2D *pong)
{
    float w = (float)ping->texture.width, h = (float)ping->texture.height;
    float horiz[2] = { 1.0f / w, 0 }, vert[2] = { 0, 1.0f / h };
    SetShaderValue(gfx.blur, gfx.blurDir, horiz, SHADER_UNIFORM_VEC2);
    Pass(*pong, ping->texture, gfx.blur);
    SetShaderValue(gfx.blur, gfx.blurDir, vert, SHADER_UNIFORM_VEC2);
    Pass(*ping, pong->texture, gfx.blur);
}

void GfxPost(RenderTexture2D scene, Rectangle dest, int viewW, int viewH)
{
    Rectangle src = { 0, 0, (float)scene.texture.width, -(float)scene.texture.height };
    if (!gfx.ready) {
        DrawTexturePro(scene.texture, src, dest, (Vector2){ 0, 0 }, 0, WHITE);
        return;
    }
    int hw = viewW / 2 > 1 ? viewW / 2 : 1, hh = viewH / 2 > 1 ? viewH / 2 : 1;
    if (gfx.w != hw || gfx.h != hh) {
        FreeTargets();
        gfx.a = Target(hw, hh);
        gfx.b = Target(hw, hh);
        gfx.c = Target(hw / 2, hh / 2);
        gfx.d = Target(hw / 2, hh / 2);
        gfx.w = hw;
        gfx.h = hh;
    }

    // 1. the glow, at half size
    float texel[2] = { 1.0f / scene.texture.width, 1.0f / scene.texture.height };
    SetShaderValue(gfx.bright, gfx.brightTexel, texel, SHADER_UNIFORM_VEC2);
    Pass(gfx.a, scene.texture, gfx.bright);
    // 2. blurred at half and at quarter size
    Blur(&gfx.a, &gfx.b);
    Pass(gfx.c, gfx.a.texture, gfx.blur);   // (a cheap extra blur on the way down)
    Blur(&gfx.c, &gfx.d);

    // 3. back on top of the scene, with the vignette
    float bloom = 1.25f, vignette = 0.42f;
    BeginShaderMode(gfx.composite);
    SetShaderValue(gfx.composite, gfx.compBloom, &bloom, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gfx.composite, gfx.compVignette, &vignette, SHADER_UNIFORM_FLOAT);
    SetShaderValueTexture(gfx.composite, gfx.compNear, gfx.a.texture);
    SetShaderValueTexture(gfx.composite, gfx.compFar, gfx.c.texture);
    DrawTexturePro(scene.texture, src, dest, (Vector2){ 0, 0 }, 0, WHITE);
    EndShaderMode();
}
