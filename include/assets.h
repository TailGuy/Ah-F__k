#pragma once
#include "context.h"
#include <stddef.h>
#include <raylib.h>

// Types.
typedef struct AssetCollectionStruct
{
    Texture2D TestImage;
} AssetCollection;


// Functions.
void Asset_LoadAssets(AssetCollection* assets, AhFuckContext* context);

void Asset_UnloadAssets(AssetCollection* assets, AhFuckContext* context);