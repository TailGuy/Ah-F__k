#include "renderer.h"
#include "raymath.h"
#include <memory.h>


// Static functions.
static inline Vector2 NormalizedToWindowVector(AhFuckRenderer* self, Vector2 normalizedVector)
{
    return Vector2Multiply(normalizedVector, self->WindowFloatSize);
}

static inline Vector2 WindowToNormalizedVector(AhFuckRenderer* self, Vector2 windowVector)
{
    return Vector2Divide(windowVector, self->WindowFloatSize);
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

    if (self->ScreenRenderTarget.texture.id)
    {
        UnloadRenderTexture(self->ScreenRenderTarget);
    }
    self->ScreenRenderTarget = LoadRenderTexture(WindowWidth, WindowHeight);
}

static void EnsureHotkeys(AhFuckRenderer* self)
{
    if (IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)))
    {
        if (!IsWindowFullscreen())
        {
            int Monitor = GetCurrentMonitor();
            int MonitorWidth = GetMonitorWidth(Monitor);
            int MonitorHeight = GetMonitorHeight(Monitor);
            SetWindowSize(MonitorWidth, MonitorHeight);
        }
        else
        {
            SetWindowSize(self->WindowedSize.X, self->WindowedSize.Y);
        }
        
        ToggleFullscreen();
    }
}


// Functions.
void Renderer_Construct(AhFuckRenderer* self, AhFuckContext* context)
{
    UNUSED(context);

    memset(self, 0, sizeof(*self));
    self->IsScreenCleared = true;
    self->ScreenClearColor = BLACK;

    OnWindowSizeChange(self);
    SetTargetFPS(240);
}

void Renderer_BeginRender(AhFuckRenderer* self)
{
    EnsureHotkeys(self);

    if (IsWindowResized())
    {
        OnWindowSizeChange(self);
    }

    self->MousePosition = Renderer_WindowToNormalizedPosition(self, GetMousePosition(), false);

    BeginTextureMode(self->ScreenRenderTarget);

    if (self->IsScreenCleared)
    {
        ClearBackground(self->ScreenClearColor);
    }
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
    EndTextureMode();

    if (self->IsGlobalScreenShaderEnabled)
    {
        BeginShaderMode(self->GlobalScreenShader);
    }

    BeginDrawing();

    Rectangle Source = (Rectangle) {.x = 0, .y = self->WindowFloatSize.y, .width = self->WindowFloatSize.x, .height = -self->WindowFloatSize.y };
    Rectangle Destination = (Rectangle) {.x = 0, .y = 0, .width = self->WindowFloatSize.x, .height = self->WindowFloatSize.y };
    DrawTexturePro(self->ScreenRenderTarget.texture,Source, Destination, (Vector2) { .x = 0, .y = 0 }, 0, WHITE);

    EndDrawing();
    EndShaderMode();
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

void Renderer_Deconstruct(AhFuckRenderer* self, AhFuckContext* context)
{
    UNUSED(context);

    memset(self, 0, sizeof(*self));
}