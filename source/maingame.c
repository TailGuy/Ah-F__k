#include "maingame.h"
#include "memory.h"
#include <stddef.h>
#include "raymath.h"
#include "ahfuckmath.h"
#include "string.h"


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
const Vector2 PAPER_SIZE_DOWN = (Vector2){ .x = 0.5f, .y = 0.5f };
const Vector2 PAPER_SIZE_CHECK = (Vector2){ .x = 0.85f, .y = 0.85f };

const Vector2 DOCUMENT_POS_DOWN = (Vector2){ .x = 0.75f, .y = 0.4f };

const float PAPER_ASPECT_RATIO = 74.0f / 104.0f;

/* Day. */
static const float DAY_DURATION_SECONDS = 90.0f;
static const float SHIFT_DURATION_SECONDS = 180.0f;

/* Camera. */
static const float REQUIRED_CAMERA_OFFSET = 0.05f;
static const float CHECK_PAPER_BOUNDS = 0.15f;


// Static functions.
static float GetRandomFloat()
{  
    // The worst way to do this.
    int MaxValue = 10000;
    return (float)GetRandomValue(0, MaxValue) / (float)MaxValue;
}

static void GenRandomIndexArray(size_t* array, size_t size)
{
    if (size <= 0)
    {
        return;
    }

    for (size_t i = 0; i < size; i++)
    {
        array[i] = i;
    }

    for (size_t i = 0; i < size / 2; i++)
    {
        size_t IndexA = GetRandomValue(0, size - 1);
        size_t IndexB = GetRandomValue(0, size - 1);
        size_t ValueA = array[IndexA];
        size_t ValueB = array[IndexB];
        array[IndexA] = ValueB;
        array[IndexB] = ValueA;
    }
}

static void CopyDocumentsRandomlyIntoSource(Document* source, DocumentSource* destination, size_t documentCount)
{
    size_t IndexArray[MAX_DOCUMENTS];
    GenRandomIndexArray(IndexArray, documentCount);
    for (size_t i = 0; i < documentCount; i++)
    {
        destination->Documents[i] = source[IndexArray[i]]; // Very fucking slow considering the document struct is HUGE.
    }
    destination->Count = documentCount;
}

static void AddTempDocument(Document* documents, size_t* documentCount, DocumentType type, const char* text)
{
    if (*documentCount >= MAX_DOCUMENTS)
    {
        return;
    }

    Document* Target = &documents[*documentCount];

    strncpy(Target->Text, text, MAX_DOCUMENT_TEXT_LENGTH);
    Target->Type = type;
    Target->RotationDeg = 0.0f;
    Target->Offset = (Vector2){ .x = 0.0f, .y = 0.0f };

    *documentCount += 1;
}

static void InitDocuments(MainGameContext* self)
{
    Document* Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    size_t DocumentCount = 0;

    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 1.");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "Of course. Here is a detailed breakdown of the document types and the specific actions a player would perform on them. This structured list is designed to be easily interpreted for game development. List 1: Types of Pages/Documents These are the various 'nouns' of the game—the paper and information the player will interact with on their desk. \n\nA. Primary Forms (Project-Driving Documents) These are forms that must be processed to advance the core 'Bench Project.' F-37: Project Proposal Form: The genesis of the entire quest. Contains the initial request, project name, location, and requesting party's signature. C-11: Budgetary Request Form: Sent to the Finance department. Must detail every cost item (materials, labor) and match the total budget requested. M-03: Material Specification Sheet: Details the physical properties of the bench (e.g., wood type, metal gauge, number of bolts).");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 3.");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 4.");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 5.");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 6");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 7.");
    AddTempDocument(Documents, &DocumentCount, DocumentType_Advertisement, "This is a test document 8.");
    CopyDocumentsRandomlyIntoSource(Documents, &self->AdsDocsSource, DocumentCount);

    DocumentCount = 0;
    for (size_t i = 0; i < MAX_DOCUMENTS; i++)
    {
        AddTempDocument(Documents, &DocumentCount, DocumentType_Blank, "");
    }

    MemFree(Documents);
}

void BreakTextIntoLinesInPlace(char* text, size_t maxLineLength)
{
    size_t LineStart = 0;
    size_t LastSpace = 0;
    size_t Index = 0;

    while(text[Index])
    {
        if (text[Index] == ' ')
        {
            LastSpace = Index;
        }
        else if (text[Index] == '\n')
        {
            LineStart = Index + 1;
            Index++;
            continue;
        }

        else if(Index - LineStart >= maxLineLength && LastSpace > LineStart)
        {
            text[LastSpace] = '\n';
            LineStart = LastSpace + 1;
        }

        Index++;
    }
}

static void AddDocument(MainGameContext* self, Document* source)
{
    if (self->DocumentCount >= MAX_DOCUMENTS)
    {
        return;
    }

    Document* TargetDocument = &self->Documents[self->DocumentCount];

    TargetDocument->Type = source->Type;
    strncpy(TargetDocument->Text, source->Text, MAX_DOCUMENT_TEXT_LENGTH);
    int MAX_CHARS_PER_LINE = 60 * PAPER_ASPECT_RATIO;
    BreakTextIntoLinesInPlace(TargetDocument->Text, MAX_CHARS_PER_LINE);

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
static void PreStartUpdate(MainGameContext* self, AssetCollection* assets, float deltaTime, AhFuckRenderer* renderer, AudioContext* audio)
{
    self->PreStartState.ElapsedStateDuration += deltaTime;
    if (self->PreStartState.ElapsedStateDuration >= PRE_START_DURATION_SECONDS)
    {
        self->State = GameState_InGame;
        renderer->GlobalScreenOpacity = 1.0f;
        Audio_PlaySound(audio, assets->BackgroundMusic, true, 0.4f);
    }
    else
    {
        renderer->GlobalScreenOpacity = self->PreStartState.ElapsedStateDuration / PRE_START_DURATION_SECONDS;
    }

    UNUSED(renderer);
}

static void OnTrashPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    if (!self->IsPaperOnTable)
    {
        return;
    }

    UNUSED(renderer);
    self->ActiveDocument = NULL;
    self->IsPaperOnTable = false;
    Audio_PlaySound(audio, assets->TrashSound, false, 0.7f);
}

// static bool IsInCameraMovementBounds(AhFuckRenderer* renderer)
// {
//     float MouseY = renderer->MousePosition.y;
//     return (MouseY >= (1.0f - REQUIRED_CAMERA_OFFSET)) || (MouseY <= REQUIRED_CAMERA_OFFSET);
// }

static bool IsInPaperCheckActionBounds(AhFuckRenderer* renderer)
{
    float MouseY = renderer->MousePosition.y;
    return (MouseY >= (1.0f - CHECK_PAPER_BOUNDS)) || (MouseY <= CHECK_PAPER_BOUNDS);
}

static void OnCheckPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(renderer);
    if (!self->IsPaperOnTable)
    {
        return;
    }

    Audio_PlaySound(audio, assets->PaperSound, false, 0.25f);

    self->IsCheckingPaper = true;
}

static void OnStopCheckPaper(MainGameContext* self, AssetCollection* assets, AudioContext* audio)
{
    Audio_PlaySound(audio, assets->PaperSound, false, 0.25f);
    self->IsCheckingPaper = false;
    self->IsCameraMovementAllowed = false;
}

static void UpdatePaperCheckState(MainGameContext* self, float deltaTime, AssetCollection* assets, AhFuckRenderer* renderer, AudioContext* audio)
{
    UNUSED(assets);
    UNUSED(renderer);
    UNUSED(audio);

    float Step = self->IsCheckingPaper ? 1.0f : -1.0f;
    float CHANGE_DURATION = 0.15f;
    self->CheckPaperState = Clamp(self->CheckPaperState + (deltaTime / CHANGE_DURATION) * Step, 0.0f, 1.0f);
}

static void EnsureAnimationControls(MainGameContext* self, AhFuckRenderer* renderer)
{
    if (!IsInPaperCheckActionBounds(renderer))
    {
        self->IsCameraMovementAllowed = true;
    }

    if (self->RoomAnimationDirection || self->IsCheckingPaper || !self->IsCameraMovementAllowed)
    {
        return;
    }

    if (renderer->MousePosition.y <= REQUIRED_CAMERA_OFFSET)
    {
        self->RoomAnimationDirection = 1;        
    }
    else if (renderer->MousePosition.y >= (1.0f - REQUIRED_CAMERA_OFFSET))
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

static Rectangle GetDocumentStackBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    Rectangle PaperBounds = GetPaperBounds(self, renderer);
    Vector2 PaperSize = (Vector2){ .x = PaperBounds.width, .y = PaperBounds.height };
    Vector2 HalfSize = Vector2Divide(PaperSize, (Vector2){ .x = 2.0f, .y = 2.0f });

    Vector2 StartPos = DOCUMENT_POS_DOWN;
    StartPos.x -= HalfSize.x;
    StartPos.y += HalfSize.y;
    return (Rectangle){ .x = StartPos.x, .y = StartPos.y, .width = 1000.0f, .height = -1000.0f };
}

static Rectangle GetTrashBounds(MainGameContext* self, AhFuckRenderer* renderer)
{
    UNUSED(self);
    float TrashBoundsStart = 0.23f;
    return (Rectangle)
    {
        .x = TrashBoundsStart * (renderer->AspectRatio / WINDOW_ASPECT_RATIO),
        .y = 0.0f,
        .width = -1000.0f,
        .height = 1000.0f
    };
}

static inline bool IsPosInBounds(Vector2 pos, Rectangle bounds)
{
    float BoundsMinX = Min(bounds.x, bounds.x + bounds.width);
    float BoundsMinY = Min(bounds.y, bounds.y + bounds.height);
    float BoundsMaxX = Max(bounds.x, bounds.x + bounds.width);
    float BoundsMaxY = Max(bounds.y, bounds.y + bounds.height);

    return (pos.x >= BoundsMinX) && (pos.x <= BoundsMaxX) && (pos.y >= BoundsMinY) && (pos.y <= BoundsMaxY);
}

static void MovePaperTowards(MainGameContext* self, Vector2 pos, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(renderer);

    Vector2 PaperTargetPos = pos;
    Vector2 PaperPosToTargetPos = Vector2Subtract(PaperTargetPos, self->PaperPosition);

    const float MOVE_TIME = 0.05f;
    float MoveAmount = 1.0f / MOVE_TIME * deltaTime;
    Vector2 PaperPos = self->PaperPosition;
    PaperPos.x += PaperPosToTargetPos.x * MoveAmount;
    PaperPos.y += PaperPosToTargetPos.y * MoveAmount;

    self->PaperPosition =PaperPos;
}

static void UpdatePaperHeightTowards(MainGameContext* self, float direction, float deltaTime)
{
    float PAPER_HEIGHT_CHANGE_TIME = 0.20f;
    float PaperHeightChange = deltaTime / PAPER_HEIGHT_CHANGE_TIME;
    self->PaperHeight = Clamp(self->PaperHeight + (PaperHeightChange * direction) , 0.0f, 1.0f);
}

static void UpdatePaperData(MainGameContext* self, AssetCollection* assets, AudioContext* audio, float deltaTime, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self))
    {
        self->PaperPosition = PAPER_POS_DOWN;
        self->IsCheckingPaper = false;
        return;
    }

    
    if (self->IsCheckingPaper)
    {
        if (renderer->MousePosition.y >= (1.0f - CHECK_PAPER_BOUNDS))
        {
            OnStopCheckPaper(self, assets, audio);
        }
        MovePaperTowards(self, PAPER_POS_DOWN, deltaTime, renderer);
        UpdatePaperHeightTowards(self, -1.0f, deltaTime);
        return;
    }

    Rectangle PaperBounds = GetPaperBounds(self, renderer);
    bool IsOverPaper = IsPosInBounds(renderer->MousePosition, PaperBounds);
    bool IsPaperSelected = (IsOverPaper || self->IsPaperSelected) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);

    if (!IsPaperSelected)
    {
        if (IsPosInBounds(self->PaperPosition, GetTrashBounds(self, renderer)))
        {
            OnTrashPaper(self, assets, renderer, audio);
            return;
        }
        else if (!self->IsCheckingPaper && (self->PaperPosition.y <= CHECK_PAPER_BOUNDS))
        {
            OnCheckPaper(self, assets, renderer, audio);
        }

        MovePaperTowards(self, PAPER_POS_DOWN, deltaTime, renderer);
        UpdatePaperHeightTowards(self, -1.0f, deltaTime);
    }
    else
    {
        MovePaperTowards(self, IsPaperSelected ? renderer->MousePosition : PAPER_POS_DOWN, deltaTime, renderer);
        self->IsPaperSelected = IsPaperSelected;
        UpdatePaperHeightTowards(self, (IsPaperSelected ? 1.0f : -1.0f), deltaTime);
    }
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

static void UpdateDocumentStack(MainGameContext* self, float deltaTime, AhFuckRenderer* renderer, AssetCollection* assets, AudioContext* audio)
{
    UNUSED(deltaTime);

    if ((self->DocumentCount <= 0)
        || !IsPosInBounds(renderer->MousePosition, GetDocumentStackBounds(self, renderer))
        || self->IsPaperOnTable)
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        self->IsPaperOnTable = true;
        self->PaperPosition = renderer->MousePosition;
        self->ActiveDocument = &self->Documents[self->DocumentCount - 1];
        self->DocumentCount--;
        Audio_PlaySound(audio, assets->PaperSound, false, 0.5f);
    }
}

static void InGameUpdate(MainGameContext* self,
    AssetCollection* assets,
    AudioContext* audio,
    float deltaTime,
    AhFuckRenderer* renderer)
{
    EnsureAnimationControls(self, renderer);
    UpdateDocumentStack(self, deltaTime, renderer, assets, audio);
    UpdatePaperData(self, assets, audio, deltaTime, renderer);
    UpdateGameTtime(self, deltaTime);
    UpdatePaperCheckState(self, deltaTime, assets, renderer, audio);

    UNUSED(deltaTime);
}

/* Rendering. */
static void DrawCheckPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self) || (self->CheckPaperState <= 0.0f))
    {
        return;
    }

    Vector2 PosMax = (Vector2){ .x = 0.5f, .y =0.5f };
    Vector2 PosMin = (Vector2){.x = PosMax.x, -PAPER_POS_DOWN.y };
    Vector2 Pos = Vector2Lerp(PosMin, PosMax, self->CheckPaperState);
    Vector2 Size = PAPER_SIZE_CHECK;
    Size.x *= PAPER_ASPECT_RATIO;

    Renderer_RenderTexture(renderer, assets->PaperGeneric, Pos, Size, (Vector2){ .x = 0.5, .y = 0.5 }, 0.0f, WHITE, true, false);
}

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

static void RenderPaperText(Vector2 position,
    Vector2 paperSize,
    Document* document,
    AssetCollection* assets,
    AhFuckRenderer* renderer,
    float rotation,
    Vector2 origin)
{
    const float FONT_SIZE = 0.015f;
    float MarginAmount = 0.05f;
    Vector2 Margin = Vector2Multiply(paperSize, (Vector2){ .x = MarginAmount, .y = MarginAmount * PAPER_ASPECT_RATIO });
    Vector2 Pos = Vector2Add(Vector2Subtract(position, Vector2Divide(paperSize, (Vector2) { .x = 2.0f, .y = 2.0f} )), Margin);
    if (document)
    {
        Renderer_RenderText(renderer,
            assets->MainFont,
            FONT_SIZE,
            Pos,
            origin,
            rotation,
            false,
            BLACK,
            document->Text);
    }
}

static void DrawPaper(MainGameContext* self, AssetCollection* assets, AhFuckRenderer* renderer)
{
    if (!IsNearDesk(self) || !self->IsPaperOnTable)
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
    RenderPaperText(Position, Size, self->ActiveDocument, assets, renderer, 0.0f, (Vector2){ .x = 0.0, .y = 0.0 });
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

        // Fuck this shit can't get it to work.
        // if (i == self->DocumentCount - 1)
        // {
        //     // Vector2 TopLeft = Vector2Subtract(Position, Vector2Divide(Size, (Vector2){ .x = 2.0f, .y = 2.0f}));
        //     // Vector2 CenterToPaper = Vector2Subtract(TopLeft, Position);
        //     // Vector2 RotatedVector = Vector2Rotate(CenterToPaper, -TargetDoc->RotationDeg * RAD2DEG);
        //     // printf("tl: %f %f\n", TopLeft.x, TopLeft.y);
        //     // printf("c: %f %f\n", CenterToPaper.x, CenterToPaper.y);
        //     // printf("r: %f %f\n", RotatedVector.x, RotatedVector.y);
        //     // printf("p: %f %f\n", Position.x, Position.y);

        //     //Vector2 PaperPos = Vector2Add(Position, RotatedVector);
        //     //printf("P: %f %f\n", PaperPos.x, PaperPos.y);
        //     RenderPaperText(StartPosition, Size, TargetDoc, assets, renderer, TargetDoc->RotationDeg, Origin);
        // }
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

static Document* TakeDocumentFromSource(DocumentSource* source)
{
    Document* Target = &source->Documents[source->Count - 1];
    source->Count--;
    return Target;
}

static void BeginDay(MainGameContext* self, size_t averageDocumentCount, AudioContext* audio)
{
    if (averageDocumentCount == 0)
    {
        return;
    }

    size_t ClampedAverageDocumentCount = averageDocumentCount > MAX_DOCUMENTS ? MAX_DOCUMENTS : averageDocumentCount;

    const float MaxDocumentCountVariance = 0.1f;
    float DocumentCountVariance = ((GetRandomFloat() - 0.5f) * 2.0f);
    size_t DocumentCountMin = (size_t)MaxInt(1, (int)(ClampedAverageDocumentCount * (1.0f - MaxDocumentCountVariance)));
    size_t DocumentCountMax = (size_t)MinInt(MAX_DOCUMENTS, (int)(ClampedAverageDocumentCount * (1.0f + MaxDocumentCountVariance)));
    size_t DocumentCount = (size_t)(DocumentCountMin + ((DocumentCountMax - DocumentCountMin) * DocumentCountVariance));

    for (size_t i = 0; i < DocumentCount; i++)
    {
        if (self->AdsDocsSource.Count > 0)
        {
            AddDocument(self, TakeDocumentFromSource(&self->AdsDocsSource));
        }   
    }
    
    self->DayDocumentStartCount = self->DocumentCount;

    self->SanityFactor = 1.0f - ((float)DocumentCount / (float)MAX_DOCUMENTS);
    audio->AudioFuckShitUpAmount = 1.0f - self->SanityFactor;
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
    self->AdsDocsSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->BlankDocsSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->LegitForBenchSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->LegitButNotForBenchSource.Documents = MemAlloc(sizeof(Document) * MAX_DOCUMENTS);
    self->ActiveDocument = NULL;
    self->CheckPaperState = 0.0f;
    InitDocuments(self);
    self->IsCheckingPaper = false;
    self->IsCameraMovementAllowed = true;

    BeginDay(self, 10, audio);

    renderer->GlobalLayer.IsShaderEnabled = true;
    renderer->GlobalLayer.TargetShader = assets->GlobalShader;
    renderer->GlobalScreenOpacity = 0.0f;

    renderer->WorldLayer.IsShaderEnabled = true;
    renderer->WorldLayer.TargetShader = assets->InsideWorldShader;
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
            PreStartUpdate(self, assets, deltaTime, renderer, audio);
            break;

        case GameState_InGame:
            InGameUpdate(self, assets, audio, deltaTime, renderer);
            break;
    
        default:
            break;
    }
}

void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(programContext);
    UNUSED(deltaTime);

    UpdateShaderValues(self, assets, renderer, deltaTime);

    Renderer_BeginLayerRender(renderer, TargetRenderLayer_World);
    BeginDrawRoom(self, assets, renderer);
    DrawDocumentStack(self, assets, renderer);
    EndDrawRoom(self, assets, renderer);
    DrawPaper(self, assets, renderer);
    DrawCheckPaper(self, assets, renderer);
    Renderer_EndLayerRender(renderer);
}

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}