#include "maingame.h"
#include "memory.h"
#include <stddef.h>
#include "raymath.h"


// Fields.
const float PRE_START_DURATION_SECONDS = 2.0f;
const float WINDOW_ASPECT_RATIO = 16.0f / 9.0f;
const float SHADOWN_BRIGHTNESS_MAX = 0.1f;
const float SHADOWN_BRIGHTNESS_MIN = 0.0125f;



// Static functions.
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

    renderer->IsGlobalScreenShaderEnabled = true;
    renderer->GlobalScreenShader = assets->GlobalShader;
    renderer->GlobalScreenOpacity = 0.0f;

    float DepFactorValue = 0.0f;
    int ColorStepCount = 1024;
    float DayBrightness = 1.0;
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DepressionFactor"), &DepFactorValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ColorStepCount"), &ColorStepCount, SHADER_UNIFORM_INT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DayBrightness"), &DayBrightness, SHADER_UNIFORM_FLOAT);
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
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DayBrightness"), &self->DayTime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "MousePos"), &renderer->MousePosition, SHADER_UNIFORM_VEC2);

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
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);

    SetShaderValueTexture(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "LightTexture"), assets->Lights0);

    Renderer_RenderTexture(renderer,
        assets->TestImage,
        (Vector2) { .x = 0.5, .y = 0.5 },
        (Vector2) { .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 },
        (Vector2) { .x = 0.5, .y = 0.5 },
        0.0f,
        WHITE,
        true,
        false);
    

    float ShadowStrength = Clamp((self->DayTime * 2.0f), 0.0f, 1.0f);
    Color ShadowBrightness = (Color) 
    {
        .r = UINT8_MAX,
        .g = UINT8_MAX,
        .b = UINT8_MAX,
        .a = (uint8_t)(UINT8_MAX * (SHADOWN_BRIGHTNESS_MIN + (SHADOWN_BRIGHTNESS_MAX - SHADOWN_BRIGHTNESS_MIN) * ShadowStrength))
    };

    Renderer_RenderTexture(renderer,
        assets->Shadows0,
        (Vector2) { .x = 0.5, .y = 0.5 },
        (Vector2) { .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 },
        (Vector2) { .x = 0.5, .y = 0.5 },
        0.0f,
        ShadowBrightness,
        true,
        false);
}

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}