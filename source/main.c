#include <stdio.h>
#include "context.h"
#include "assets.h"
#include "raylib.h"
#include "raymath.h"
#include "renderer.h"



// Static functions.
static void BeginAhFuckToFuckTheFuck(AhFuckContext* context, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!context->IsInitialized)
    {
        return;
    }

    float RotationRad = 0.0f;

    while (!WindowShouldClose())
    {
        SetShaderValueV(assets->PixelsShader, GetShaderLocation(assets->PixelsShader, "ScreenSize"), &renderer->WindowFloatSize, SHADER_UNIFORM_VEC2, 1);
        BeginShaderMode(assets->PixelsShader);

        Renderer_BeginRender(renderer);

        Vector2 Position = renderer->MousePosition;

        Renderer_RenderTexture(renderer,
            assets->TestImage,
            Position,
            (Vector2) { .x = 0.3, .y = 0.3 },
            (Vector2) { .x = 0.5, .y = 0.5 },
            RAD2DEG * RotationRad,
            WHITE,
            true,
            false);

        Renderer_EndRender(renderer);

        EndShaderMode();
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

    BeginAhFuckToFuckTheFuck(&Context, &Assets, &Renderer);

    Asset_UnloadAssets(&Assets, &Context);
    Renderer_Deconstruct(&Renderer, &Context);
    Context_Deconstruct(&Context);
}