#include <stdio.h>
#include "context.h"
#include "assets.h"
#include "raylib.h"



// Functions.
int main()
{
    AhFuckContext Context;
    Context_Construct(&Context);

    AssetCollection Assets;
    Asset_LoadAssets(&Assets, &Context);

    Context_Start(&Context);

    Asset_UnloadAssets(&Assets, &Context);

    Context_Deconstruct(&Context);
}