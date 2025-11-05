#include <stdio.h>
#include "context.h"
#include "assets.h"
#include "raylib.h"
#include "raymath.h"
#include "renderer.h"
#include "maingame.h"



// Static functions.
static void BeginAhFuckToFuckTheFuck(AhFuckContext* context, AssetCollection* assets, AhFuckRenderer* renderer, MainGameContext* mainGame)
{
    UNUSED(mainGame);

    if (!context->IsInitialized)
    {
        return;
    }

    renderer->IsGlobalScreenShaderEnabled = true;
    renderer->GlobalScreenShader = assets->GlobalShader;

    float DepFactorValue = 0.0f;
    int ColorStepCount = 4;
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DepressionFactor"), &DepFactorValue, SHADER_UNIFORM_FLOAT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ColorStepCount"), &ColorStepCount, SHADER_UNIFORM_INT);

    MainGame_Start(mainGame, context, renderer);

    while (!WindowShouldClose())
    {
        SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ScreenSize"), &renderer->WindowFloatSize, SHADER_UNIFORM_VEC2);
        
        float DeltaTime = GetFrameTime();

        MainGame_Update(mainGame, context, DeltaTime, renderer);

        Renderer_BeginRender(renderer);
        MainGame_Draw(mainGame, context, DeltaTime, renderer);
        Renderer_EndRender(renderer);
    }

    MainGame_End(mainGame, context, renderer);
}



// Functions.
int main()
{
    AhFuckContext Context;
    Context_Construct(&Context);
    AssetCollection Assets;
    Asset_LoadAssets(&Assets, &Context);
    AhFuckRenderer Renderer;
    Renderer_Construct(&Renderer, &Context);
    MainGameContext MainGame;
    MainGame_CreateContext(&MainGame, &Context);

    BeginAhFuckToFuckTheFuck(&Context, &Assets, &Renderer, &MainGame);

    Asset_UnloadAssets(&Assets, &Context);
    Renderer_Deconstruct(&Renderer, &Context);
    Context_Deconstruct(&Context);
}