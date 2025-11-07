#include "renderer.h"
#include "raymath.h"
#include <memory.h>
#include "limits.h"
#include <math.h> // <-- ADD THIS INCLUDE

// Define the virtual resolution for a 16:9 aspect ratio.
static const int VIRTUAL_WIDTH = 1280;
static const int VIRTUAL_HEIGHT = 720;

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
    // This function no longer resizes the render textures.
    // It only updates the stored windowed size for the fullscreen toggle logic.
    if (!IsWindowFullscreen())
    {
        self->WindowedSize = (IntVector){ .X = GetScreenWidth(), .Y = GetScreenHeight() };
    }
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

    // Set the renderer's internal dimensions to the VIRTUAL resolution.
    // All rendering calculations will be based on this fixed 16:9 space.
    self->WindowIntSize = (IntVector){ .X = VIRTUAL_WIDTH, .Y = VIRTUAL_HEIGHT };
    self->WindowFloatSize = (Vector2){ .x = VIRTUAL_WIDTH, .y = VIRTUAL_HEIGHT };
    self->AspectRatio = (float)VIRTUAL_WIDTH / (float)VIRTUAL_HEIGHT;

    // Create the render layers at the fixed VIRTUAL resolution.
    ProcessLayerDataOnScreenChange(self, &self->WorldLayer);
    ProcessLayerDataOnScreenChange(self, &self->UILayer);
    ProcessLayerDataOnScreenChange(self, &self->GlobalLayer);

    // Store the initial real window size for the fullscreen toggle.
    self->WindowedSize = (IntVector){ .X = GetScreenWidth(), .Y = GetScreenHeight() };

    SetTargetFPS(240);
}

void Renderer_UpdateState(AhFuckRenderer* self)
{
    EnsureHotkeys(self);

    if (IsWindowResized())
    {
        OnWindowSizeChange(self);
    }

    // Mouse position needs to be mapped from window coordinates to virtual render coordinates.
    Vector2 mousePosition = GetMousePosition();
    float realScreenWidth = (float)GetScreenWidth();
    float realScreenHeight = (float)GetScreenHeight();

    // Calculate the scale and position of the letterboxed virtual screen
    float scale = fminf(realScreenWidth / VIRTUAL_WIDTH, realScreenHeight / VIRTUAL_HEIGHT);
    float offsetX = (realScreenWidth - (VIRTUAL_WIDTH * scale)) * 0.5f;
    float offsetY = (realScreenHeight - (VIRTUAL_HEIGHT * scale)) * 0.5f;

    // Translate mouse coordinates into the virtual screen space
    Vector2 virtualMouse = { 0 };
    virtualMouse.x = (mousePosition.x - offsetX) / scale;
    virtualMouse.y = (mousePosition.y - offsetY) / scale;

    // Clamp to ensure the mouse is within the virtual screen bounds
    virtualMouse.x = Clamp(virtualMouse.x, 0.0f, (float)VIRTUAL_WIDTH);
    virtualMouse.y = Clamp(virtualMouse.y, 0.0f, (float)VIRTUAL_HEIGHT);

    // Normalize the virtual coordinates for game logic (0.0 to 1.0)
    self->MousePosition = WindowToNormalizedVector(self, virtualMouse);
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
    ClearBackground(BLACK); // Clear with black for letterboxing

    RenderLayer GlobalLayer = self->GlobalLayer;

    if (GlobalLayer.IsShaderEnabled)
    {
        BeginShaderMode(GlobalLayer.TargetShader);
    }

    // The source is the entire GlobalLayer texture, which is VIRTUAL_WIDTH x VIRTUAL_HEIGHT.
    // We flip the Y-axis because render textures are upside down in OpenGL.
    Rectangle Source = { 0.0f, 0.0f, (float)GlobalLayer.Texture.texture.width, (float)GlobalLayer.Texture.texture.height };

    float realScreenWidth = (float)GetScreenWidth();
    float realScreenHeight = (float)GetScreenHeight();

    // Calculate the scale factor to fit the virtual screen within the real window while maintaining aspect ratio.
    float scale = fminf(realScreenWidth / VIRTUAL_WIDTH, realScreenHeight / VIRTUAL_HEIGHT);

    // Calculate the destination rectangle to center the virtual screen.
    Rectangle Destination = {
        (realScreenWidth - (VIRTUAL_WIDTH * scale)) * 0.5f,
        (realScreenHeight - (VIRTUAL_HEIGHT * scale)) * 0.5f,
        VIRTUAL_WIDTH * scale,
        VIRTUAL_HEIGHT * scale
    };

    // This color mask is used for the fade-in effect.
    uint8_t ChannelValue = (uint8_t)(UINT8_MAX * self->GlobalScreenOpacity);
    Color DrawColor =
    {
        .r = ChannelValue,
        .g = ChannelValue,
        .b = ChannelValue,
        .a = UINT8_MAX,
    };

    DrawTexturePro(GlobalLayer.Texture.texture, Source, Destination, (Vector2){ .x = 0, .y = 0 }, 0, DrawColor);

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