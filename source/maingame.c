#include "maingame.h"
#include "memory.h"
#include <stddef.h>
#include "raymath.h"
#include "ahfuckmath.h"


/* The WHOLE game is dumped in this 1 file, so good luck scrolling and figuring out what's going on. */


// Fields.

/* Idk. */
const float WINDOW_ASPECT_RATIO = 16.0f / 9.0f;

/* Pre start stage. */
const float PRE_START_DURATION_SECONDS = 2.0f;

/* Shadows. */
const float SHADOWN_BRIGHTNESS_MAX = 0.1f;
const float SHADOWN_BRIGHTNESS_MIN = 0.0125f;
const uint8_t SHADOW_BRIGHTNESS_NIGHT = (uint8_t)(UINT8_MAX * 0.20f);
const float NIGHT_SHADOW_ENABLE_TIME = 0.5 * 0.15f; // Check the fucking shader for context.

/* Colors. */
const Color COLOR_DAY = (Color){ .r = 157, .g = 198, .b = 225, .a = 255 };
const Color COLOR_EVENING = (Color){ .r = 229, .g = 143, .b = 84, .a = 255 };
const Color COLOR_NIGHT = (Color){ .r = 20, .g = 50, .b = 120, .a = 255 };

/* Shader. */
const int COLOR_STEP_COUNT_MAX = 255;
const int COLOR_STEP_COUNT_MIN = 16;



// Static functions.
static float GetRandomFloat()
{  
    // The worst way to do this.
    int MaxValue = 10000;
    return (float)GetRandomValue(0, MaxValue) / (float)MaxValue;
}

static void PreStartUpdate(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    self->PreStartState.ElapsedStateDuration += deltaTime;
    if (self->PreStartState.ElapsedStateDuration >= PRE_START_DURATION_SECONDS)
    {
        self->State = GameState_InGame;
        renderer->GlobalScreenOpacity = 1.0f;
    }
    else
    {
        renderer->GlobalScreenOpacity = self->PreStartState.ElapsedStateDuration / PRE_START_DURATION_SECONDS;
    }

    UNUSED(programContext);
    UNUSED(renderer);
}

static Texture2D TextureOrEmpty(Texture2D texture, AssetCollection* assets)
{
    if (texture.id)
    {
        return texture;
    }
    return assets->EmptyTexture;
}

static void DrawRoom(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    Texture2D RoomTexture = TextureOrEmpty(assets->RoomAnimation[self->AnimationIndex], assets);
    Texture2D LightsTexture = TextureOrEmpty(assets->RoomLightAnimation[self->AnimationIndex], assets);
    Texture2D DayShadowTexture = TextureOrEmpty(assets->ShadowDayAnimation[self->AnimationIndex], assets);
    Texture2D NightShadowTexture = TextureOrEmpty(assets->ShadownNightAnimation[self->AnimationIndex], assets);

    Vector2 Position = (Vector2){.x = 0.5, .y = 0.5 };
    Vector2 Size = (Vector2){ .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    SetShaderValueTexture(assets->InsideWorldShader, GetShaderLocation(assets->InsideWorldShader, "LightTexture"), LightsTexture);

    Renderer_RenderTexture(renderer, RoomTexture, Position, Size, Origin, 0.0f, WHITE, true, false);

    float ShadowStrength = Clamp((self->DayTime * 2.0f), 0.0f, 1.0f);
    Color ShadowBrightness = (Color) 
    {
        .r = UINT8_MAX,
        .g = UINT8_MAX,
        .b = UINT8_MAX,
        .a = (uint8_t)(UINT8_MAX * (SHADOWN_BRIGHTNESS_MIN + (SHADOWN_BRIGHTNESS_MAX - SHADOWN_BRIGHTNESS_MIN) * ShadowStrength))
    };

    Texture2D ShadowTexture;
    if (self->DayTime <= NIGHT_SHADOW_ENABLE_TIME)
    {
        ShadowTexture = NightShadowTexture;
        ShadowBrightness.a = SHADOW_BRIGHTNESS_NIGHT;
    }
    else
    {    
        ShadowTexture = DayShadowTexture;
    }

    Renderer_RenderTexture(renderer, ShadowTexture, Position, Size, Origin, 0.0f, ShadowBrightness, true, false);
}

static void UpdateShaderValues(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    float Sanity = self->SanityFactor;

    int ColorStepCount = (int)roundf(COLOR_STEP_COUNT_MIN + ((COLOR_STEP_COUNT_MAX - COLOR_STEP_COUNT_MIN) * Sanity));
    float DepressionFactor = -Sanity + 1.0f;
    float DayBrightness = self->DayTime;
    Vector2 ScreenSize = renderer->WindowFloatSize;
    float RandomValue = GetRandomFloat();

    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ScreenSize"), &ScreenSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DepressionFactor"), &DepressionFactor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ColorStepCount"), &ColorStepCount, SHADER_UNIFORM_INT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "RandomValue"), &RandomValue, SHADER_UNIFORM_FLOAT);

    SetShaderValue(assets->InsideWorldShader, GetShaderLocation(assets->InsideWorldShader, "DayBrightness"), &DayBrightness, SHADER_UNIFORM_FLOAT);
}

static void UpdateBackgroundColor(MainGameContext* self, AhFuckRenderer* renderer)
{
    float NightTimeStrength = Clamp(1.0f - (self->DayTime * 2.0f), 0.0f, 1.0f);
    float EveningTimeStrength = Clamp(AbsFloat(1.0f - self->DayTime) * 2.0f, 0.0f, 1.0f);

    renderer->ScreenClearColor = ColorLerp(ColorLerp(COLOR_DAY, COLOR_EVENING, EveningTimeStrength), COLOR_NIGHT, NightTimeStrength);
}


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}

void MainGame_Start(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer)
{
    UNUSED(programContext);
    UNUSED(renderer);

    self->State = GameState_PreStart;
    self->SanityFactor = 1.0f;
    self->PreStartState.ElapsedStateDuration = 0.0f;
    self->DayTime = 1.0f;
    self->AnimationIndex = 0;

    renderer->GlobalLayer.IsShaderEnabled = true;
    renderer->GlobalLayer.TargetShader = assets->GlobalShader;
    renderer->GlobalScreenOpacity = 0.0f;

    renderer->WorldLayer.IsShaderEnabled = true;
    renderer->WorldLayer.TargetShader = assets->InsideWorldShader;
}

void MainGame_End(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(renderer);
    UNUSED(assets);
}

void MainGame_Update(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);
    UNUSED(assets);

    self->DayTime = renderer->MousePosition.x;
    self->SanityFactor = renderer->MousePosition.y;

    UpdateBackgroundColor(self, renderer);

    switch (self->State)
    {
        case GameState_PreStart:
            PreStartUpdate(self, programContext, deltaTime, renderer);
            break;
    
        default:
            break;
    }
}

void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(programContext);
    UNUSED(deltaTime);

    UpdateShaderValues(self, assets, renderer);

    Renderer_BeginLayerRender(renderer, TargetRenderLayer_World);
    DrawRoom(self, assets, renderer);
    Renderer_EndLayerRender(renderer);
}

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}