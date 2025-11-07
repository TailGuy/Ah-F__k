#include <stdio.h>
#include "raylib.h"
#include "context.h"
#include <memory.h>

// Static functions.
static void InitAhFuckWindow()
{
    InitWindow(0, 0, "Ah Fuck");

    // int CurrentMonitor = GetCurrentMonitor();
    // int MonitorWidth = GetMonitorWidth(CurrentMonitor);
    // int MonitorHeight = GetMonitorHeight(CurrentMonitor);

    // SetWindowSize(MonitorWidth / 2, MonitorHeight / 2);
    SetWindowSize(1280, 720);

    SetExitKey(KEY_NULL);
}

static void InitRaylibStuff()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitAhFuckWindow();
}


// Functions.
void Context_Construct(AhFuckContext* context)
{
    context->IsInitialized = true;
    context->ProgramOutStream = stdout;
    memset(context->SharedStringBuffer, 0, sizeof(context->SharedStringBuffer));


    InitRaylibStuff();
    
    fprintf(context->ProgramOutStream, "Ah Fuck initialization begun.\n");
    fprintf(context->ProgramOutStream, "Ah Fuck initialization finalized.\n");
}

void Context_Deconstruct(AhFuckContext* context)
{
    if (context->IsInitialized)
    {
        return;
    }

    CloseWindow();
    fprintf(context->ProgramOutStream, "Deconstructing Ah Fuck context.\n");
    fprintf(context->ProgramOutStream, "Destroyed Ah Fuck, goodbye!\n");
    memset(context, 0, sizeof(*context));
}