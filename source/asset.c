#include "assets.h"
#include <string.h>


// Functions.
void Asset_LoadAssets(AssetCollection* assets, AhFuckContext* context)
{
    const char* RootDir = GetApplicationDirectory();
    char* AssetPath = context->SharedStringBuffer;
    size_t BufferSize = sizeof(context->SharedStringBuffer);

    fprintf(context->ProgramOutStream, "Loading textures");

    snprintf(AssetPath, BufferSize, "%sasset/texture/test.png", RootDir);
    assets->TestImage = LoadTexture(AssetPath);
}

void Asset_UnloadAssets(AssetCollection* assets, AhFuckContext* context)
{
    if (!context->IsInitialized)
    {
        return;
    }

    UnloadTexture(assets->TestImage);
}