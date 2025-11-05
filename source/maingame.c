#include "maingame.h"
#include "memory.h"
#include <stddef.h>


// Fields.
const float PRE_START_DURATION_SECONDS = 2.0f;



// Static functions.
static void PreStartUpdate(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    self->PreStartState.ElapsedStateDuration += deltaTime;
    if (self->PreStartState.ElapsedStateDuration >= PRE_START_DURATION_SECONDS)
    {
        self->State = GameState_InGame;
    }

    UNUSED(programContext);
    UNUSED(renderer);
}


// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}

void MainGame_Start(MainGameContext* self, AhFuckContext* programContext, AhFuckRenderer* renderer)
{
    UNUSED(programContext);
    UNUSED(renderer);

    self->State = GameState_PreStart;
    self->SanityFactor = 1.0f;

    self->PreStartState.ElapsedStateDuration = 0.0f;
}

void MainGame_End(MainGameContext* self, AhFuckContext* programContext, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(renderer);
}

void MainGame_Update(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);

    switch (self->State)
    {
        case GameState_PreStart:
            PreStartUpdate(self, programContext, deltaTime, renderer);
            break;
    
        default:
            break;
    }
}

void MainGame_Draw(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);
}

void MainGame_DestroyContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}