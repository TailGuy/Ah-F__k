#include "maingame.h"
#include "memory.h"
#include <stddef.h>
#include "raymath.h"
#include "ahfuckmath.h"


/* The WHOLE game is dumped in this 1 file, so good luck scrolling and figuring out what's going on. */


// Fields.

/* Idk. */
const float WINDOW_ASPECT_RATIO = 16.0f / 9.0f;

/* Pre start stage. */
const float PRE_START_DURATION_SECONDS = 2.0f;

/* Room animation. */
const float ROOM_ANIMATION_DURATION_SECONDS = 0.5f;


/* Shadows. */
const float SHADOWN_BRIGHTNESS_MAX = 0.1f;
const float SHADOWN_BRIGHTNESS_MIN = 0.0125f;
const uint8_t SHADOW_BRIGHTNESS_NIGHT = (uint8_t)(UINT8_MAX * 0.20f);
const float NIGHT_SHADOW_ENABLE_TIME = 0.5 * 0.15f; // Check the fucking shader for context.

/* Colors. */
const Color COLOR_DAY = (Color){ .r = 157, .g = 198, .b = 225, .a = 255 };
const Color COLOR_EVENING = (Color){ .r = 229, .g = 143, .b = 84, .a = 255 };
const Color COLOR_NIGHT = (Color){ .r = 5, .g = 13, .b = 23, .a = 255 };

/* Shader. */
const int COLOR_STEP_COUNT_MAX = 255;
const int COLOR_STEP_COUNT_MIN = 16;
const float SHADER_RANDOM_UPDATE_FREQUENCY = 10.0f;


/* Paper. */
const Vector2 PAPER_POS_DOWN = (Vector2){ .x = 0.53f, .y = 0.65f };
const Vector2 PAPER_SIZE_DOWN = (Vector2){ .x = 0.45f, .y = 0.45f };

const Vector2 DOCUMENT_POS_DOWN = (Vector2){ .x = 0.75f, .y = 0.4f };

const float PAPER_ASPECT_RATIO = 74.0f / 104.0f;

/* Day. */
static const float DAY_DURATION_SECONDS = 90.0f;
static const float SHIFT_DURATION_SECONDS = 180.0f;


// Static functions.
static float GetRandomFloat()
{  
    // The worst way to do this.
    int MaxValue = 10000;
    return (float)GetRandomValue(0, MaxValue) / (float)MaxValue;
}

static void AddDocument(MainGameContext* self, DocumentType type, const char* text)
{
    if (self->DocumentCount >= MAX_DOCUMENTS)
    {
        return;
    }

    Document* TargetDocument = &self->Documents[self->DocumentCount];

    TargetDocument->Type = type;
    strncpy(TargetDocument->Text, text, MAX_DOCUMENT_TEXT_LENGTH);

    const float MAX_ROTATION_DEG = 3.0f;
    const float MAX_OFFSET = 0.005f;

    TargetDocument->RotationDeg = ((GetRandomFloat() - 0.5f )* 2.0f) * MAX_ROTATION_DEG;
    TargetDocument->Offset = (Vector2)
    {
        .x = ((GetRandomFloat() - 0.5f )* 2.0f) * MAX_OFFSET,
        .y = ((GetRandomFloat() - 0.5f )* 2.0f) * MAX_OFFSET
    };

    self->DocumentCount++;
}


/* States. */
static void PreStartUpdate(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    self->PreStartState.ElapsedStateDuration += deltaTime;
    if (self->PreStartState.ElapsedStateDuration >= PRE_START_DURATION_SECONDS)
    {
        self->State = GameState_InGame;
        renderer->GlobalScreenOpacity = 1.0f;
    }
    else
    {
        renderer->GlobalScreenOpacity = self->PreStartState.ElapsedStateDuration / PRE_START_DURATION_SECONDS;
    }

    UNUSED(programContext);
    UNUSED(renderer);
}

static void EnsureAnimationControls(MainGameContext* self, AhFuckRenderer* renderer)
{
    if (self->RoomAnimationDirection)
    {
        return;
    }

    const float REQUIRED_OFFSET = 0.025f;

    if (renderer->MousePosition.y <= REQUIRED_OFFSET)
    {
        self->RoomAnimationDirection = 1;        
    }
    else if (renderer->MousePosition.y >= (1.0f - REQUIRED_OFFSET))
    {
        self->RoomAnimationDirection = -1;        
    }
}

static inline bool IsNearDesk(MainGameContext* self)
{
    return self->AnimationIndex >= (ROOM_ANIMATION_FRAME_COUNT - 1);
}

static Rectangle GetItemBounds(AhFuckRenderer* renderer, Vector2 pos, Vector2 size, float textureAspectRatio)
{
    Vector2 Size = (Vector2){ 1.0 * textureAspectRatio, .y = 1.0 };
    Size = Vector2Multiply(Size, size);
    Size = Renderer_AdjustVector(renderer, Size);
    Vector2 HalfSize = Vector2Divide(Size, (Vector2){ .x = 2.0f, .y = 2.0f });
    Vector2 Min = Vector2Subtract(pos, HalfSize);
    return (Rectangle){ .x = Min.x, .y = Min.y, .width = Size.x, .height = Size.y };
}

static Rectangle GetPaperBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    return GetItemBounds(renderer, self->PaperPosition, PAPER_SIZE_DOWN, PAPER_ASPECT_RATIO);
}

static inline bool IsPosInBounds(Vector2 pos, Rectangle bounds)
{
    return (pos.x >= bounds.x) && (pos.x <= (bounds.x + bounds.width)) && (pos.y >= bounds.y) && (pos.y <= (bounds.y + bounds.height));
}

static void UpdatePaperData(MainGameContext* self, float deltaTime, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self))
    {
        self->PaperPosition = PAPER_POS_DOWN;
        return;
    }

    Rectangle PaperBounds = GetPaperBounds(self, renderer);
    bool IsOverPaper = IsPosInBounds(renderer->MousePosition, PaperBounds);
    bool IsPaperSelected = (IsOverPaper || self->IsPaperSelected) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    Vector2 PaperTargetPos = IsPaperSelected ? renderer->MousePosition : PAPER_POS_DOWN;
    Vector2 PaperPosToTargetPos = Vector2Subtract(PaperTargetPos, self->PaperPosition);

    const float MOVE_TIME = 0.05f;
    float MoveAmount = 1.0f / MOVE_TIME * deltaTime;
    Vector2 PaperPos = self->PaperPosition;
    PaperPos.x += PaperPosToTargetPos.x * MoveAmount;
    PaperPos.y += PaperPosToTargetPos.y * MoveAmount;

    self->PaperPosition = PaperPos;
    self->IsPaperSelected = IsPaperSelected;

    float PAPER_HEIGHT_CHANGE_TIME = 0.20f;
    float PaperHeightChange = deltaTime / PAPER_HEIGHT_CHANGE_TIME;
    self->PaperHeight = Clamp(self->PaperHeight + (IsPaperSelected ? PaperHeightChange : -PaperHeightChange), 0.0f, 1.0f);
}

static void UpdateGameTtime(MainGameContext* self, float deltaTime)
{
    self->GameTime += deltaTime;

    // This assumes that shift duration >= day duration (like everything else in this code).
    float DurationWithoutDayAdvance = (SHIFT_DURATION_SECONDS - DAY_DURATION_SECONDS);
    float DurationWithoutDayAdvancePerPart = DurationWithoutDayAdvance / 2.0f;

    if (self->GameTime <= DurationWithoutDayAdvancePerPart)
    {
        self->DayTime = 1.0f;
    }
    else if (self->GameTime >= (SHIFT_DURATION_SECONDS - DurationWithoutDayAdvancePerPart))
    {
        self->DayTime = 0.0f;
    }
    else
    {
        self->DayTime = (SHIFT_DURATION_SECONDS - self->GameTime - DurationWithoutDayAdvancePerPart) / DAY_DURATION_SECONDS;
    }
}

// static void UpdateDocumentStack(MainGameContext* self, float deltaTime, AhFuckRenderer* renderer)
// {

// }

static void InGameUpdate(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    EnsureAnimationControls(self, renderer);
    UpdatePaperData(self, deltaTime, renderer);
    UpdateGameTtime(self, deltaTime);

    UNUSED(deltaTime);
    UNUSED(programContext);
}

/* Rendering. */
static void UpdateRoomAnimation(MainGameContext* self, float deltaTime)
{
    if (!self->RoomAnimationDirection)
    {
        return;
    }

    int32_t NormalizedDirection = self->RoomAnimationDirection > 0 ? 1 : -1;
    self->TimeSinceRoomAnimationUpdate += deltaTime;
    float TimePerFrame = ROOM_ANIMATION_DURATION_SECONDS / ROOM_ANIMATION_FRAME_COUNT;

    if (self->TimeSinceRoomAnimationUpdate <= TimePerFrame)
    {
        return;
    }

    const int32_t MinIndex = 0;
    const int32_t MaxIndex = ROOM_ANIMATION_FRAME_COUNT - 1;

    self->TimeSinceRoomAnimationUpdate -= TimePerFrame;
    int32_t NewIndex = self->AnimationIndex + NormalizedDirection;
    if (NewIndex > MaxIndex)
    {
        self->RoomAnimationDirection = 0;
        NewIndex = MaxIndex;
    }
    else if (NewIndex < MinIndex)
    {
        self->RoomAnimationDirection = 0;
        NewIndex = MinIndex;
    }
    self->AnimationIndex = NewIndex;
}

static Texture2D TextureOrEmpty(Texture2D texture, AssetCollection* assets)
{
    if (texture.id)
    {
        return texture;
    }
    return assets->EmptyTexture;
}

static void BeginDrawRoom(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    Texture2D RoomTexture = TextureOrEmpty(assets->RoomAnimation[self->AnimationIndex], assets);
    Texture2D LightsTexture = TextureOrEmpty(assets->RoomLightAnimation[self->AnimationIndex], assets);

    Vector2 Position = (Vector2){.x = 0.5, .y = 0.5 };
    Vector2 Size = (Vector2){ .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    SetShaderValueTexture(assets->InsideWorldShader, GetShaderLocation(assets->InsideWorldShader, "LightTexture"), LightsTexture);

    Renderer_RenderTexture(renderer, RoomTexture, Position, Size, Origin, 0.0f, WHITE, true, false);
}

static void EndDrawRoom(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    Vector2 Position = (Vector2){.x = 0.5, .y = 0.5 };
    Vector2 Size = (Vector2){ .x = 1.0 * WINDOW_ASPECT_RATIO, .y = 1.0 };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    Texture2D DayShadowTexture = TextureOrEmpty(assets->ShadowDayAnimation[self->AnimationIndex], assets);
    Texture2D NightShadowTexture = TextureOrEmpty(assets->ShadownNightAnimation[self->AnimationIndex], assets);

    float ShadowStrength = Clamp((self->DayTime * 2.0f), 0.0f, 1.0f);
    Color ShadowBrightness = (Color) 
    {
        .r = UINT8_MAX,
        .g = UINT8_MAX,
        .b = UINT8_MAX,
        .a = (uint8_t)(UINT8_MAX * (SHADOWN_BRIGHTNESS_MIN + (SHADOWN_BRIGHTNESS_MAX - SHADOWN_BRIGHTNESS_MIN) * ShadowStrength))
    };

    Texture2D ShadowTexture;
    if (self->DayTime <= NIGHT_SHADOW_ENABLE_TIME)
    {
        ShadowTexture = NightShadowTexture;
        ShadowBrightness.a = SHADOW_BRIGHTNESS_NIGHT;
    }
    else
    {    
        ShadowTexture = DayShadowTexture;
    }

    Renderer_RenderTexture(renderer, ShadowTexture, Position, Size, Origin, 0.0f, ShadowBrightness, true, false);
}

static void DrawPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self))
    {
        return;
    }

    Vector2 Position = self->PaperPosition;
    Vector2 ShadowPosition = Position;
    Rectangle PaperBounds = GetPaperBounds(self, renderer);
    Vector2 Size = (Vector2){ .x = PaperBounds.width, .y = PaperBounds.height };
    Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

    const float PaperOffset = 0.015f;
    if (self->IsPaperSelected)
    {
        Position.x -= PaperOffset * Size.x * self->PaperHeight; 
        Position.y -= PaperOffset * Size.y * self->PaperHeight; 
    }

    Color ShadowColor = (Color){ .r = 0, .g = 0, .b = 0, .a = 200 };

    Renderer_RenderTexture(renderer, assets->PaperGeneric, ShadowPosition, Size, Origin, 0.0f, ShadowColor, false, false);
    Renderer_RenderTexture(renderer, assets->PaperGeneric, Position, Size, Origin, 0.0f, WHITE, false, false);
}

static void DrawDocumentStack(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self))
    {
        return;
    }

    Vector2 ChangePerPaper = Renderer_AdjustVector(renderer, (Vector2){ .x = 0.005, .y = -0.010 });
    
    Vector2 Size = PAPER_SIZE_DOWN;
    Size.x *= PAPER_ASPECT_RATIO;

    Vector2 StartPosition = DOCUMENT_POS_DOWN;

    for (size_t i = 0; i < self->DocumentCount; i++)
    {
        Document* TargetDoc = &self->Documents[i];

        Vector2 PaperOffset = Vector2Multiply(ChangePerPaper, (Vector2){ .x = i, .y = i });
        Vector2 Position = Vector2Add(StartPosition, PaperOffset);
        Position = Vector2Add(Position, Renderer_AdjustVector(renderer, TargetDoc->Offset));
        Vector2 ShadowPosition = Position;

        Vector2 Origin = (Vector2){ .x = 0.5, .y = 0.5 };

        if (i > 0)
        {
            const float ShadowOffset = 0.006f;
            ShadowPosition.x -= ShadowOffset * Size.x; 
            ShadowPosition.y += ShadowOffset * Size.y; 
        }

        Color ShadowColor = (Color){ .r = 0, .g = 0, .b = 0, .a = 160 };

        
        float Rotation = TargetDoc->RotationDeg;

        Renderer_RenderTexture(renderer, assets->PaperGeneric, ShadowPosition, Size, Origin, Rotation, ShadowColor, true, false);
        Renderer_RenderTexture(renderer, assets->PaperGeneric, Position, Size, Origin, Rotation, WHITE, true, false);
    }
}

static void UpdateShaderValues(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, float deltaTime)
{
    float Sanity = self->SanityFactor;

    int ColorStepCount = (int)roundf(COLOR_STEP_COUNT_MIN + ((COLOR_STEP_COUNT_MAX - COLOR_STEP_COUNT_MIN) * Sanity));
    float DepressionFactor = -Sanity + 1.0f;
    float DayBrightness = self->DayTime;
    Vector2 ScreenSize = renderer->WindowFloatSize;

    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ScreenSize"), &ScreenSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "DepressionFactor"), &DepressionFactor, SHADER_UNIFORM_FLOAT);
    SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "ColorStepCount"), &ColorStepCount, SHADER_UNIFORM_INT);


    self->TimeSinceShaderRandomUpdate += deltaTime;
    if (self->TimeSinceShaderRandomUpdate >= (1.0f / SHADER_RANDOM_UPDATE_FREQUENCY))
    {
        self->TimeSinceShaderRandomUpdate = 0.0f;
        float RandomValue = GetRandomFloat();
        SetShaderValue(assets->GlobalShader, GetShaderLocation(assets->GlobalShader, "RandomValue"), &RandomValue, SHADER_UNIFORM_FLOAT);
    }

    SetShaderValue(assets->InsideWorldShader, GetShaderLocation(assets->InsideWorldShader, "DayBrightness"), &DayBrightness, SHADER_UNIFORM_FLOAT);
}

static void UpdateBackgroundColor(MainGameContext* self, AhFuckRenderer* renderer)
{
    float NightTimeStrength = Clamp(1.0f - (self->DayTime * 2.0f), 0.0f, 1.0f);
    float EveningTimeStrength = Clamp(AbsFloat(1.0f - self->DayTime) * 2.0f, 0.0f, 1.0f);

    renderer->ScreenClearColor = ColorLerp(ColorLerp(COLOR_DAY, COLOR_EVENING, EveningTimeStrength), COLOR_NIGHT, NightTimeStrength);
}


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}

void MainGame_Start(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(programContext);
    UNUSED(renderer);

    self->State = GameState_PreStart;
    self->SanityFactor = 1.0f;
    self->PreStartState.ElapsedStateDuration = 0.0f;
    self->DayTime = 1.0f;
    self->AnimationIndex = 0;
    self->TimeSinceShaderRandomUpdate = 0.0;
    self->TimeSinceRoomAnimationUpdate = 0.0f;
    self->IsPaperSelected = false;
    self->PaperPosition = PAPER_POS_DOWN;
    self->GameTime = 0.0f;
    self->IsPaperOnTable = false;
    self->Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);

    for (size_t i = 0; i < 50; i++)
    {
        AddDocument(self, DocumentType_Blank, "Garbage");
    }

    renderer->GlobalLayer.IsShaderEnabled = true;
    renderer->GlobalLayer.TargetShader = assets->GlobalShader;
    renderer->GlobalScreenOpacity = 0.0f;

    renderer->WorldLayer.IsShaderEnabled = true;
    renderer->WorldLayer.TargetShader = assets->InsideWorldShader;

    audio->AudioFuckShitUpAmount = 0.0f;

    Audio_PlaySound(audio, assets->BackgroundMusic, true, 0.4f);
}

void MainGame_End(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(renderer);
    UNUSED(assets);
    UNUSED(audio);
    MemFree(self->Documents);
}

void MainGame_Update(MainGameContext* self,
    AssetCollection* assets,
    AhFuckContext* programContext,
    float deltaTime,
    AhFuckRenderer* renderer,
    AudioContext* audio)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);
    UNUSED(assets);

    UpdateBackgroundColor(self, renderer);
    UpdateRoomAnimation(self, deltaTime);

    switch (self->State)
    {
        case GameState_PreStart:
            PreStartUpdate(self, programContext, deltaTime, renderer);
            break;

        case GameState_InGame:
            InGameUpdate(self, programContext, deltaTime, renderer);
            break;
    
        default:
            break;
    }

    audio->AudioFuckShitUpAmount = 1.0f - self->SanityFactor;
}

void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(programContext);
    UNUSED(deltaTime);

    UpdateShaderValues(self, assets, renderer, deltaTime);

    Renderer_BeginLayerRender(renderer, TargetRenderLayer_World);
    BeginDrawRoom(self, assets, renderer);
    DrawDocumentStack(self, assets, renderer);
    DrawPaper(self, assets, renderer);
    EndDrawRoom(self, assets, renderer);
    Renderer_EndLayerRender(renderer);
}

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}