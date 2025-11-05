#pragma once
#include "context.h"
#include <stddef.h>
#include <raylib.h>

// Types.
typedef struct AssetCollectionStruct
{
    Texture2D TestImage;
    Texture2D Shadows0;
    Texture2D NightShadows0;
    Texture2D Lights0;
    Texture2D PaperGeneric;
    Shader GlobalShader;
} AssetCollection;


// Functions.
void Asset_LoadAssets(AssetCollection* assets, AhFuckContext* context);

void Asset_UnloadAssets(AssetCollection* assets, AhFuckContext* context);