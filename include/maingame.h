#pragma once
#include "context.h"
#include "renderer.h"
#include "assets.h"
#include "fuckaudio.h"

#define MAX_DOCUMENT_TEXT_LENGTH 4096
#define DAY_COUNT = 10

static const size_t MAX_DOCUMENTS = 64;


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
    DocumentType_Advertisement,
    DocumentType_Blank
} DocumentType;

typedef struct DocumentStruct
{
    DocumentType Type;
    bool IsCorrect; // ADD THIS: Tracks if the document is valid or has errors.
    float RotationDeg;
    Vector2 Offset;
    char Text[MAX_DOCUMENT_TEXT_LENGTH];
} Document;

typedef struct DocumentSourceStruct
{
    Document* Documents;
    size_t Count;
} DocumentSource;

typedef struct MainGameContextStruct
{
    int Score; // ADD THIS: Tracks the player's score.
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
    bool IsCheckingPaper;
    float CheckPaperState;
    Document* ActiveDocument;
    size_t DayDocumentStartCount;

    size_t DocumentCount;
    Document* Documents;
    DocumentSource LegitForBenchSource;
    DocumentSource LegitButNotForBenchSource;
    DocumentSource AdsDocsSource;
    DocumentSource BlankDocsSource;

    bool IsCameraMovementAllowed;

    float SubmitIndicatorValue;
    float ReturnIndicatorValue;
    float TrashIndicatorValue;
} MainGameContext;


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext);

void MainGame_Start(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio);

void MainGame_End(MainGameContext* self,AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer, AudioContext* audio);

/* Due to renderer and user input not being their own classes, the renderer has to be passed to update loop for things like aspect ratio and window size. */
void MainGame_Update(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer, AudioContext* audio);

void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer);

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext);