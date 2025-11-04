#include "renderer.h"
#include <memory.h>


// Static functions.
static inline Vector2 NormalizedToWindowVector(AhFuckRenderer* self, Vector2 normalizedVector)
{
    return (Vector2) 
    {
        .x = normalizedVector.x * self->WindowFloatSize.x,
        .y = normalizedVector.y * self->WindowFloatSize.y
    };
}

static inline void AdjustVector(Vector2* vector, float aspectRatio)
{
    if (aspectRatio >= 1.0f)
    {
        vector->x /= aspectRatio;
    }
    else
    {
        vector->y *= aspectRatio;
    }
}

static inline Vector2 NormalizedToWindowPosition(AhFuckRenderer* self, Vector2 normalizedPos, bool isAdjusted)
{
    Vector2 Position = NormalizedToWindowVector(self, normalizedPos);
    if (isAdjusted)
    {
        AdjustVector(&Position, self->AspectRatio);
    }
    return Position;
}

static inline Vector2 NormalizedToWindowSize(AhFuckRenderer* self, Vector2 normalizedSize, bool isAdjusted)
{
    Vector2 Size = NormalizedToWindowVector(self, normalizedSize);
    if (isAdjusted)
    {
        AdjustVector(&Size, self->AspectRatio);
    }
    return Size;
}

static inline Vector2 WindowToNormalizedPosition(AhFuckRenderer* self, Vector2 position)
{
    return (Vector2) 
    {
        .x = position.x / self->WindowFloatSize.x,
        .y = position.y / self->WindowFloatSize.y
    };
}


static void UpdateWindowSize(AhFuckRenderer* self)
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
        UpdateWindowSize(self);
    }
}


// Functions.
void Renderer_Construct(AhFuckRenderer* self, AhFuckContext* context)
{
    UNUSED(context);

    memset(self, 0, sizeof(*self));
    self->IsScreenCleared = true;
    self->ScreenClearColor = BLACK;

    UpdateWindowSize(self);
    SetTargetFPS(240);
}

void Renderer_BeginRender(AhFuckRenderer* self)
{
    if (self->IsScreenCleared)
    {
        ClearBackground(self->ScreenClearColor);
    }

    EnsureHotkeys(self);

    if (IsWindowResized())
    {
        UpdateWindowSize(self);
    }

    self->MousePosition = WindowToNormalizedPosition(self, GetMousePosition());

    BeginDrawing();
}

void Renderer_EndRender(AhFuckRenderer* self)
{
    UNUSED(self);

    EndDrawing();
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

    Vector2 DestCoords = NormalizedToWindowPosition(self, position, isPosAdjusted);
    Vector2 AdjustedSize = NormalizedToWindowSize(self, size, isSizeAdjusted);
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