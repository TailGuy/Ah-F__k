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

static inline Vector2 NormalizedToWindowPosition(AhFuckRenderer* self, Vector2 normalizedPos)
{
    return NormalizedToWindowVector(self, normalizedPos);
}

static inline Vector2 NormalizedToWindowSize(AhFuckRenderer* self, Vector2 normalizedSize)
{
    return NormalizedToWindowVector(self, normalizedSize);
}


static void UpdateWindowSize(AhFuckRenderer* self)
{
    int WindowWidth = GetScreenWidth();
    int WindowHeight = GetScreenHeight();

    self->WindowIntSize = (IntVector) { .X = WindowWidth, .Y = WindowHeight };
    self->WindowFloatSize = (Vector2) { .x = WindowWidth, .y = WindowHeight };
    self->AspectRatio = self->WindowFloatSize.x / self->WindowFloatSize.y;
}

static void EnsureHotkeys()
{
    if (IsKeyPressed(KEY_F11) || (IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)))
    {
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

    UpdateWindowSize(self);
    SetTargetFPS(240);
}

void Renderer_BeginRender(AhFuckRenderer* self)
{
    if (self->IsScreenCleared)
    {
        ClearBackground(self->ScreenClearColor);
    }

    EnsureHotkeys();

    if (IsWindowResized())
    {
        UpdateWindowSize(self);
    }

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
    UNUSED(isSizeAdjusted);
    UNUSED(isPosAdjusted);

    Rectangle SourceRect =
    {
        .x = 0,
        .y = 0,
        .width = texture.width,
        .height = texture.height
    };

    Vector2 DestCoords = NormalizedToWindowPosition(self, position);
    Vector2 AdjustedSize = NormalizedToWindowSize(self, size);
    Rectangle DestinationRect =
    {
        .x = DestCoords.x,
        .y = DestCoords.y,
        .width = AdjustedSize.x,
        .height = AdjustedSize.y
    };

    Vector2 Origin = (Vector2) { .x = origin.x * DestinationRect.width, .y = origin.y * DestinationRect.y };

    DrawTexturePro(texture, SourceRect, DestinationRect, Origin, rotation, mask);
}

void Renderer_Deconstruct(AhFuckRenderer* self, AhFuckContext* context)
{
    UNUSED(context);

    memset(self, 0, sizeof(*self));
}