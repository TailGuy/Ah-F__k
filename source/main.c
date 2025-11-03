#include <stdio.h>
#include "raylib.h"

// Static functions.
static void InitAhFuckWindow()
{
    int CurrentMonitor = GetCurrentMonitor();
    InitWindow(GetMonitorWidth(CurrentMonitor) / 2, GetMonitorHeight(CurrentMonitor) / 2, "Test");

    // Placeholder loop for now, should be moved out.
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLUE);
        EndDrawing();
    }
}

static void InitAhFuckProgram()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitAhFuckWindow();
}


// Functions.
int main()
{
    InitAhFuckProgram();
}