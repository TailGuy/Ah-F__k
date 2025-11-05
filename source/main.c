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

    float RotationRad = 0.0f;

    renderer->IsGlobalScreenShaderEnabled = false;
    renderer->GlobalScreenShader = assets->GlobalShader;
    SetTextureFilter(renderer->ScreenRenderTarget.texture, TEXTURE_FILTER_POINT); // TODO: ( FilterPoint

    while (!WindowShouldClose())
    {
        RotationRad += GetFrameTime() * 4.0f;

        SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ScreenSize"), &renderer->WindowFloatSize, SHADER_UNIFORM_VEC2);

        Renderer_BeginRender(renderer);

        Vector2 Position = renderer->MousePosition;

        Renderer_RenderTexture(renderer,
            assets->TestImage,
            Position,
            (Vector2) { .x = 1.0f, .y = 1.0f },
            (Vector2) { .x = 0.5, .y = 0.5 },
            RAD2DEG * RotationRad,
            WHITE,
            false,
            false);

        Renderer_EndRender(renderer);
    }
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