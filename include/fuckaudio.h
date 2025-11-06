#pragma once
#include "raylib.h"
#include <stddef.h>


// Fields.
#define MAX_SOUNDS 8


// Types.
typedef struct AhFuckSoundStruct
{
    float* Samples;
    size_t SampleCount;
} AhFuckSound;

typedef struct AhFuckSoundInstanceStruct
{
    AhFuckSound Sound;
    bool IsActive;
    size_t Index;
    bool IsLooped;
    float Volume;
} AhFuckSoundInstance;

typedef struct AudioContextStruct
{
    float AudioFuckShitUpAmount;
    AudioStream Stream;
    AhFuckSoundInstance CurrentSounds[MAX_SOUNDS];
} AudioContext;


// Functions.
void Audio_Init(AudioContext* self);

void Audio_PlaySound(AudioContext* self, AhFuckSound sound, bool isLooped, float volume);

void Audio_Deinit(AudioContext* self);