#include <stdio.h>
#include "context.h"
#include "assets.h"
#include "raylib.h"
#include "renderer.h"



// Static functions.
static void BeginAhFuckToFuckTheFuck(AhFuckContext* context, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!context->IsInitialized)
    {
        return;
    }

    while (!WindowShouldClose())
    {
        Renderer_BeginRender(renderer);

        Renderer_RenderTexture(renderer,
            assets->TestImage,
            (Vector2) { .x = 0.5, .y = 0.5 },
            (Vector2) { .x = 0.5, .y = 0.5 },
            (Vector2) { .x = 0.5, .y = 0.5 },
            0.0f,
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

    BeginAhFuckToFuckTheFuck(&Context, &Assets, &Renderer);

    Asset_UnloadAssets(&Assets, &Context);
    Renderer_Deconstruct(&Renderer, &Context);
    Context_Deconstruct(&Context);
}