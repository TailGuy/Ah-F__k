#pragma once
#include <stdio.h>


#define UNUSED(x) (void)(x)


typedef struct AhFuckContextStruct
{
    bool IsInitialized;
    FILE* ProgramOutStream;
    char SharedStringBuffer[16384];
} AhFuckContext;


// Functions.
void Context_Construct(AhFuckContext* context);

void Context_Deconstruct(AhFuckContext* context);