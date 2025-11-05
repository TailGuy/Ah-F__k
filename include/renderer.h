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
    RenderTexture2D ScreenRenderTarget;
    Shader GlobalScreenShader;
    bool IsGlobalScreenShaderEnabled;
    float GlobalScreenOpacity;
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

void Renderer_EnableDrawShader(AhFuckRenderer* self, Shader shader);

void Renderer_DisableDrawShader(AhFuckRenderer* self);

Vector2 Renderer_WindowToNormalizedPosition(AhFuckRenderer* self, Vector2 position, bool isAdjusted);

Vector2 Renderer_NormalizedToWindowPosition(AhFuckRenderer* self, Vector2 position, bool isAdjusted);

Vector2 Renderer_NormalizedToWindowSize(AhFuckRenderer* self, Vector2 size, bool isAdjusted);

Vector2 Renderer_WindowToNormalizedSize(AhFuckRenderer* self, Vector2 size, bool isAdjusted);

Vector2 Renderer_AdjustVector(AhFuckRenderer* self, Vector2 vector);

Vector2 Renderer_UnadjustVector(AhFuckRenderer* self, Vector2 vector);

void Renderer_Deconstruct(AhFuckRenderer* self, AhFuckContext* context);