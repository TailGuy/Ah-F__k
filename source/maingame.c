#include "maingame.h"
#include "memory.h"



// Functions.
void MainGame_CreateContext(MainGameContext* self, AhFuckContext* programContext)
{
    memset(self, 0, sizeof(*self));
    UNUSED(programContext);
}

void MainGame_Update(MainGameContext* self, AhFuckContext* programContext, float deltaTime, AhFuckRenderer* renderer)
{
    UNUSED(self);
    UNUSED(programContext);
    UNUSED(deltaTime);
    UNUSED(renderer);
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