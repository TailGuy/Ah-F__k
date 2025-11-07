#include "renderer.h"
#include "raymath.h"
#include <memory.h>
#include "limits.h"


// Static functions.
static inline Vector2 NormalizedToWindowVector(AhFuckRenderer* self, Vector2 normalizedVector)
{
    return Vector2Multiply(normalizedVector, self->WindowFloatSize);
}

static inline Vector2 WindowToNormalizedVector(AhFuckRenderer* self, Vector2 windowVector)
{
    return Vector2Divide(windowVector, self->WindowFloatSize);
}

static void ProcessLayerDataOnScreenChange(AhFuckRenderer* self, RenderLayer* layer)
{
    if (layer->Texture.texture.id)
    {
        UnloadRenderTexture(layer->Texture);
    }

    layer->Texture = LoadRenderTexture(self->WindowIntSize.X, self->WindowIntSize.Y);
    SetTextureFilter(layer->Texture.texture, TEXTURE_FILTER_POINT);
    SetTextureWrap(layer->Texture.texture, TEXTURE_WRAP_REPEAT);
}

static void OnWindowSizeChange(AhFuckRenderer* self)
{
    int WindowWidth = GetScreenWidth();
    int WindowHeight = GetScreenHeight();

    self->WindowIntSize = (IntVector) { .X = WindowWidth, .Y = WindowHeight };
    self->WindowFloatSize = (Vector2) { .x = WindowWidth, .y = WindowHeight };
    self->AspectRatio = self->WindowFloatSize.x / self->WindowFloatSize.y;

    if (!IsWindowFullscreen())
    {
        self->WindowedSize = (IntVector){ .X = self->WindowIntSize.X, .Y = self->WindowIntSize.Y };
    }

    ProcessLayerDataOnScreenChange(self, &self->WorldLayer);
    ProcessLayerDataOnScreenChange(self, &self->UILayer);
    ProcessLayerDataOnScreenChange(self, &self->GlobalLayer);
}

static void EnsureHotkeys(AhFuckRenderer* self)
{
    if (IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)))
    {
        IntVector FixedSize;
        if (!IsWindowFullscreen())
        {
            int Monitor = GetCurrentMonitor();
            int MonitorWidth = GetMonitorWidth(Monitor);
            int MonitorHeight = GetMonitorHeight(Monitor);
            FixedSize = (IntVector){.X = MonitorWidth, .Y = MonitorHeight };
        }
        else
        {
            FixedSize = (IntVector){.X = self->WindowedSize.X, .Y = self->WindowedSize.Y };
        }

        SetWindowSize(FixedSize.X, FixedSize.Y);
        
        ToggleFullscreen();
    }
}

static void DrawOntoGlobalLayer(AhFuckRenderer* self, RenderLayer layer)
{
    BeginTextureMode(self->GlobalLayer.Texture);
    if (layer.IsShaderEnabled)
    {
        BeginShaderMode(layer.TargetShader);
    }

    Rectangle Source = (Rectangle) {.x = 0, .y = 0, .width = self->WindowFloatSize.x, .height = self->WindowFloatSize.y };
    Rectangle Destination = (Rectangle) {.x = 0, .y = 0, .width = self->WindowFloatSize.x, .height = self->WindowFloatSize.y };

    Color DrawColor = { .r = UINT8_MAX, .g = UINT8_MAX, .b = UINT8_MAX, .a = (uint8_t)(UINT8_MAX * self->GlobalScreenOpacity), };
 
    DrawTexturePro(layer.Texture.texture,Source, Destination, (Vector2) { .x = 0, .y = 0 }, 0, DrawColor);

    EndShaderMode();
    EndTextureMode();
}


// Functions.
void Renderer_Construct(AhFuckRenderer* self, AhFuckContext* context)
{
    UNUSED(context);

    memset(self, 0, sizeof(*self));
    self->IsScreenCleared = true;
    self->ScreenClearColor = BLACK;
    self->GlobalScreenOpacity = 1.0f;

    OnWindowSizeChange(self);
    SetTargetFPS(240);
}

void Renderer_UpdateState(AhFuckRenderer* self)
{
    EnsureHotkeys(self);

    if (IsWindowResized())
    {
        OnWindowSizeChange(self);
    }

    self->MousePosition = Renderer_WindowToNormalizedPosition(self, GetMousePosition(), false);
    self->MousePosition.y =self->MousePosition.y;
}

void Renderer_BeginRender(AhFuckRenderer* self)
{
    BeginTextureMode(self->GlobalLayer.Texture);

    if (self->IsScreenCleared)
    {
        ClearBackground(self->ScreenClearColor);
    }
    
    EndTextureMode();
}

void Renderer_BeginLayerRender(AhFuckRenderer* self, TargetRenderLayer layer)
{
    switch (layer)
    {
        case TargetRenderLayer_World:
            self->_activeLayer = &self->WorldLayer;
            break;
        case TargetRenderLayer_UI:
            self->_activeLayer = &self->UILayer;   
            break;
        default:
            self->_activeLayer = &self->WorldLayer; // Idk what to do here tbh.
            break;
    }

    BeginTextureMode(self->_activeLayer->Texture);
    ClearBackground(BLANK);
}

void Renderer_EndLayerRender(AhFuckRenderer* self)
{
    EndTextureMode();
    DrawOntoGlobalLayer(self, *self->_activeLayer);
}

void Renderer_EnableDrawShader(AhFuckRenderer* self, Shader shader)
{
    UNUSED(self);
    EndShaderMode();
    BeginShaderMode(shader);
}

void Renderer_DisableDrawShader(AhFuckRenderer* self)
{
    UNUSED(self);
    EndShaderMode();
}

void Renderer_EndRender(AhFuckRenderer* self)
{
    BeginDrawing();

    RenderLayer GlobalLayer = self->GlobalLayer;

    if (GlobalLayer.IsShaderEnabled)
    {
        BeginShaderMode(GlobalLayer.TargetShader);
    }

    Rectangle Source = (Rectangle) {.x = 0, .y = 0, .width = self->WindowFloatSize.x, .height = self->WindowFloatSize.y };
    Rectangle Destination = (Rectangle) {.x = 0, .y = 0, .width = self->WindowFloatSize.x, .height = self->WindowFloatSize.y };

    uint8_t ChannelValue = (uint8_t)(UINT8_MAX * self->GlobalScreenOpacity);
    Color DrawColor = 
    {
        .r = ChannelValue, 
        .g = ChannelValue,
        .b = ChannelValue, 
        .a = UINT8_MAX, 
    };
 
    DrawTexturePro(GlobalLayer.Texture.texture, Source, Destination, (Vector2) { .x = 0, .y = 0 }, 0, DrawColor);

    EndShaderMode();
    EndDrawing();
}

Vector2 Renderer_WindowToNormalizedPosition(AhFuckRenderer* self, Vector2 position, bool isAdjusted)
{
    Vector2 NormalizedVector = WindowToNormalizedVector(self, position);
    if (isAdjusted)
    {
        NormalizedVector = Renderer_AdjustVector(self, NormalizedVector);
    }
    return NormalizedVector;
}

Vector2 Renderer_NormalizedToWindowPosition(AhFuckRenderer* self, Vector2 position, bool isAdjusted)
{
    Vector2 WindowVector = position;
    if (isAdjusted)
    {
        WindowVector = Renderer_UnadjustVector(self, WindowVector);
    }
    WindowVector = NormalizedToWindowVector(self, WindowVector);
    return WindowVector;
}

Vector2 Renderer_WindowToNormalizedSize(AhFuckRenderer* self, Vector2 size, bool isAdjusted)
{
    Vector2 NormalizedVector = WindowToNormalizedVector(self, size);
    if (isAdjusted)
    {
        NormalizedVector = Renderer_UnadjustVector(self, NormalizedVector);
    }
    return NormalizedVector;
}

Vector2 Renderer_NormalizedToWindowSize(AhFuckRenderer* self, Vector2 size, bool isAdjusted)
{
    Vector2 WindowVector = size;
    if (isAdjusted)
    {
        WindowVector = Renderer_AdjustVector(self, WindowVector);
    }
    WindowVector = NormalizedToWindowVector(self, WindowVector);
    return WindowVector;
}

Vector2 Renderer_AdjustVector(AhFuckRenderer* self, Vector2 vector)
{
    float AspectRatio = self->AspectRatio;
    Vector2 ModifiedVector = vector;
    if (AspectRatio >= 1.0f)
    {
        ModifiedVector.x /= AspectRatio;
    }
    else
    {
        ModifiedVector.y *= AspectRatio;
    }
    return ModifiedVector;
}

Vector2 Renderer_UnadjustVector(AhFuckRenderer* self, Vector2 vector)
{
        float AspectRatio = self->AspectRatio;
    Vector2 ModifiedVector = vector;
    if (AspectRatio >= 1.0f)
    {
        ModifiedVector.x *= AspectRatio;
    }
    else
    {
        ModifiedVector.y /= AspectRatio;
    }
    return ModifiedVector;
}

void Renderer_RenderTexture(AhFuckRenderer* self,
    Texture2D texture,
    Vector2 position,
    Vector2 size,
    Vector2 origin,
    float rotation,
    Color mask,
    bool isSizeAdjusted,
    bool isPosAdjusted)
{
    Rectangle SourceRect =
    {
        .x = 0,
        .y = 0,
        .width = texture.width,
        .height = texture.height
    };

    Vector2 DestCoords = Renderer_NormalizedToWindowPosition(self, position, isPosAdjusted);
    Vector2 AdjustedSize = Renderer_NormalizedToWindowSize(self, size, isSizeAdjusted);
    Rectangle DestinationRect =
    {
        .x = DestCoords.x,
        .y = DestCoords.y,
        .width = AdjustedSize.x,
        .height = AdjustedSize.y
    };

    Vector2 Origin = (Vector2) { .x = origin.x * DestinationRect.width, .y = origin.y * DestinationRect.height };

    DrawTexturePro(texture, SourceRect, DestinationRect, Origin, rotation, mask);
}

void Renderer_RenderText(AhFuckRenderer* self,
    Font font,
    float fontSize,
    Vector2 pos,
    Vector2 origin,
    float rotation,
    bool isPosAdjusted,
    Color color,
    const char* text)
{
    const float SPACING = 0.0f;
    float Size = self->WindowFloatSize.y * fontSize;
    Vector2 Pos = Renderer_NormalizedToWindowPosition(self, pos, isPosAdjusted);
    Vector2 TextAdvance = MeasureTextEx(font, text, Size, SPACING);
    DrawTextPro(font, text, Pos, Vector2Multiply(origin, TextAdvance), rotation, Size, SPACING, color);
}

void Renderer_Deconstruct(AhFuckRenderer* self, AhFuckContext* context)
{
    UNUSED(context);

    memset(self, 0, sizeof(*self));
}