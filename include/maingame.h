#pragma once
#include "context.h"
#include "renderer.h"
#include "assets.h"

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

typedef struct MainGameContextStruct
{
    GameState State;
    float SanityFactor;
    float DayTime;
    size_t AnimationIndex;

    GamePreStartStateData PreStartState;
} MainGameContext;


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext);

void MainGame_Start(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer);

void MainGame_End(MainGameContext* self,AssetCollection* assets, AhFuckContext* programContext, AhFuckRenderer* renderer);

/* Due to renderer and user input not being their own classes, the renderer has to be passed to update loop for things like aspect ratio and window size. */
void MainGame_Update(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer);

void MainGame_Draw(MainGameContext* self, AssetCollection* assets, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer);

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext);