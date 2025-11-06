#pragma once
#include "context.h"
#include <stddef.h>
#include <raylib.h>


// Fields.
#define ROOM_ANIMATION_FRAME_COUNT 10


// Types.
typedef struct AssetCollectionStruct
{
    Texture2D TestImage;

    Texture2D EmptyTexture;

    Texture2D RoomAnimation[ROOM_ANIMATION_FRAME_COUNT];
    Texture2D ShadowDayAnimation[ROOM_ANIMATION_FRAME_COUNT];
    Texture2D ShadownNightAnimation[ROOM_ANIMATION_FRAME_COUNT];
    Texture2D RoomLightAnimation[ROOM_ANIMATION_FRAME_COUNT];

    Texture2D PaperGeneric;
    
    Shader InsideWorldShader;
    Shader GlobalShader;
} AssetCollection;


// Functions.
void Asset_LoadAssets(AssetCollection* assets, AhFuckContext* context);

void Asset_UnloadAssets(AssetCollection* assets, AhFuckContext* context);