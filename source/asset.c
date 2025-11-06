#include "assets.h"
#include <string.h>


// Static functions.
static void LoadOptionalAsset(Texture2D* destination, const char* path)
{
    if (FileExists(path))
    {
        *destination = LoadTexture(path);
    }
}

static void LoadSoundIntoAssets(AhFuckSound* sound ,char* path)
{
    Wave TargetWave = LoadWave(path);
    *sound = (AhFuckSound) { .Samples = LoadWaveSamples(TargetWave), TargetWave.frameCount * TargetWave.channels };
    UnloadWave(TargetWave);
}


// Functions.
void Asset_LoadAssets(AssetCollection* assets, AhFuckContext* context)
{
    memset(assets, 0, sizeof(*assets));

    const char* RootDir = GetApplicationDirectory();
    char* AssetPath = context->SharedStringBuffer;
    size_t BufferSize = sizeof(context->SharedStringBuffer);

    fprintf(context->ProgramOutStream, "Loading assets");

    // Textures.
    snprintf(AssetPath, BufferSize, "%sasset/texture/empty.png", RootDir);
    assets->EmptyTexture = LoadTexture(AssetPath);

    for (size_t i = 0; i < ROOM_ANIMATION_FRAME_COUNT; i++)
    {
        snprintf(AssetPath, BufferSize, "%sasset/texture/room/background/%zu.png", RootDir, i);
        LoadOptionalAsset(&assets->RoomAnimation[i], AssetPath);

        snprintf(AssetPath, BufferSize, "%sasset/texture/room/lights/%zu.png", RootDir, i);
        LoadOptionalAsset(&assets->RoomLightAnimation[i], AssetPath);

        snprintf(AssetPath, BufferSize, "%sasset/texture/room/shadows_day/%zu.png", RootDir, i);
        LoadOptionalAsset(&assets->ShadowDayAnimation[i], AssetPath);

        snprintf(AssetPath, BufferSize, "%sasset/texture/room/shadows_night/%zu.png", RootDir, i);
        LoadOptionalAsset(&assets->ShadownNightAnimation[i], AssetPath);
    }

    /* Sound. */
    snprintf(AssetPath, BufferSize, "%sasset/sound/music.ogg", RootDir);
    LoadSoundIntoAssets(&assets->BackgroundMusic, AssetPath);

    // Shaders.
    snprintf(AssetPath, BufferSize, "%sasset/shader/world.glsl", RootDir);
    assets->InsideWorldShader = LoadShader(NULL, AssetPath);

    snprintf(AssetPath, BufferSize, "%sasset/shader/global.glsl", RootDir);
    assets->GlobalShader = LoadShader(NULL, AssetPath);

    fprintf(context->ProgramOutStream, "Finished loading assets.");
}

void Asset_UnloadAssets(AssetCollection* assets, AhFuckContext* context)
{
    if (!context->IsInitialized)
    {
        return;
    }

    UnloadTexture(assets->TestImage);
}