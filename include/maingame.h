#pragma once
#include "context.h"
#include "renderer.h"
#include "assets.h"
#include "fuckaudio.h"

#define MAX_DOCUMENT_TEXT_LENGTH 4096
static const size_t MAX_DOCUMENTS = 128;


// Types.
typedef enum GameStateEnum
{
    GameState_PreStart,
    GameState_InGame,
} GameState;

typedef struct GamePreStartStateDataStruct
{
    float ElapsedStateDuration;
} GamePreStartStateData;

typedef enum DocumentTypeEnum
{
    DocumentType_LegitForBench,
    DocumentType_LegitButNotForBench,
    DocumentType_Scam,
    DocumentType_Advertisement,
    DocumentType_Blank,
    DocumentType_Garbage
} DocumentType;

typedef struct DocumentStruct
{
    DocumentType Type;
    float RotationDeg;
    Vector2 Offset;
    char Text[MAX_DOCUMENT_TEXT_LENGTH];
} Document;

typedef struct MainGameContextStruct
{
    GameState State;
    float SanityFactor;
    float GameTime;
    float DayTime;
    int32_t AnimationIndex;

    float TimeSinceShaderRandomUpdate;

    float TimeSinceRoomAnimationUpdate;
    int32_t RoomAnimationDirection;

    GamePreStartStateData PreStartState;

    Vector2 PaperPosition;
    bool IsPaperSelected;
    bool IsPaperOnTable;
    float PaperHeight;
    Document* ActiveDocument;

    size_t DocumentCount;
    Document* Documents;
} MainGameContext;


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext);

void MainGame_Start(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio);

void MainGame_End(MainGameContext* self,AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio);

/* Due to renderer and user input not being their own classes, the renderer has to be passed to update loop for things like aspect ratio and window size. */
void MainGame_Update(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer, AudioContext* audio);

void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer);

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext);