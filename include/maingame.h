#pragma once
#include "context.h"
#include "renderer.h"

// Types.
typedef struct MainGameContextStruct
{
    int Placeholder;
} MainGameContext;


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext);

/* Due to renderer and user input not being their own classes, the renderer has to be passed to update loop for things like aspect ratio and window size. */
void MainGame_Update(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer);

void MainGame_Draw(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer);

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext);