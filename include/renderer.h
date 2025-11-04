#pragma once
#include "context.h"
#include <ahfuckmath.h>
#include <raylib.h>

// Types.
typedef struct AhFuckRendererStruct
{
    IntVector WindowIntSize;
    Vector2 WindowFloatSize;
    float AspectRatio;
    bool IsScreenCleared;
    Color ScreenClearColor;
    Vector2 MousePosition;
    IntVector WindowedSize;
} AhFuckRenderer;



// Functions.
void Renderer_Construct(AhFuckRenderer* self, AhFuckContext* context);

void Renderer_BeginRender(AhFuckRenderer* self);

void Renderer_EndRender(AhFuckRenderer* self);

void Renderer_RenderTexture(AhFuckRenderer* self,
    Texture2D texture,
    Vector2 position,
    Vector2 size,
    Vector2 origin,
    float rotation,
    Color mask,
    bool isSizeAdjusted,
    bool isPosAdjusted);

void Renderer_Deconstruct(AhFuckRenderer* self, AhFuckContext* context);