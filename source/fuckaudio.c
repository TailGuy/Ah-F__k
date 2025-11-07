#include "fuckaudio.h"
#include "context.h"
#include "memory.h"
#include "raymath.h"


static AudioContext* SelfGlobal;

static const int CHANNEL_COUNT = 2;


// Static functions.
static void ApplyTheCrunchySoundFilter(float* floatBuffer, unsigned int frames)
{
    float NormalizerAmount = 3.0f;
    float FuckShitUpAmount = powf(SelfGlobal->AudioFuckShitUpAmount, NormalizerAmount);

    float MAX_MULTIPLIER = 200.0f;
    for (size_t i = 0; i < frames * CHANNEL_COUNT; i++)
    {
        float Value = floatBuffer[i];
        floatBuffer[i] = Value * (1.0f + (MAX_MULTIPLIER * FuckShitUpAmount));
    }
}

static void ClampVolume(float* floatBuffer, unsigned int frames)
{
    for (size_t i = 0; i < frames * CHANNEL_COUNT; i++)
    {
        floatBuffer[i] = Clamp(floatBuffer[i], -1.0f, 1.0f);
    }
}

static void AudioCallbackFunc(void *bufferData, unsigned int frames)
{
    float* FloatBuffer = bufferData;
    UNUSED(bufferData);
    
    for (unsigned int i = 0; i < frames * CHANNEL_COUNT; i++)
    {
        FloatBuffer[i] = 0.0f;
    }

    for (size_t SoundIndex = 0; SoundIndex < MAX_SOUNDS; SoundIndex++)
    {
        AhFuckSoundInstance* CurrentSound = &SelfGlobal->CurrentSounds[SoundIndex];
        if (!CurrentSound->IsActive)
        {
            continue;
        }

        size_t ReadSampleCount = frames * CHANNEL_COUNT;
        size_t RemainingSampleInSound = CurrentSound->Sound.SampleCount - CurrentSound->Index;
        if (!CurrentSound->IsLooped && (RemainingSampleInSound < ReadSampleCount))
        {
            ReadSampleCount = RemainingSampleInSound;
        }

        size_t CurrentSoundStartIndex = CurrentSound->Index;
        for (size_t i = 0; i < ReadSampleCount; i++)
        {
            FloatBuffer[i] += CurrentSound->Sound.Samples[(CurrentSoundStartIndex + i) % CurrentSound->Sound.SampleCount] * CurrentSound->Volume;
        }

        CurrentSound->Index += ReadSampleCount;
        if (CurrentSound->Index >= CurrentSound->Sound.SampleCount)
        {
            if (CurrentSound->IsLooped)
            {
                CurrentSound->Index %= CurrentSound->Sound.SampleCount;
            }
            else
            {
                CurrentSound->IsActive = false;
            }
        }
    }

    ApplyTheCrunchySoundFilter(FloatBuffer, frames);
    ClampVolume(FloatBuffer, frames);
}

static AhFuckSoundInstance* FindFreeSoundSpot(AudioContext* self)
{
    for (size_t i = 0; i < MAX_SOUNDS; i++)
    {
        if (self->CurrentSounds[i].IsActive)
        {
            continue;
        }
        return &self->CurrentSounds[i];
    }
    return NULL;
}

// Functions.
void Audio_Init(AudioContext* self)
{
    InitAudioDevice();

    memset(self, 0, sizeof(*self));

    SelfGlobal = self;

    self->Stream = LoadAudioStream(48000, sizeof(float), 2);
    AttachAudioStreamProcessor(self->Stream, &AudioCallbackFunc);
    PlayAudioStream(self->Stream);
}

void Audio_Update(AudioContext* self)
{
    UNUSED(self);
}

void Audio_PlaySound(AudioContext* self, AhFuckSound sound, bool isLooped, float volume)
{
    AhFuckSoundInstance* TargetSound = FindFreeSoundSpot(self);
    if (!TargetSound)
    {
        return;
    }

    TargetSound->Sound = sound;
    TargetSound->IsActive = true;
    TargetSound->Index = 0;
    TargetSound->IsLooped = isLooped;
    TargetSound->Volume = volume;
}

void Audio_Deinit(AudioContext* self)
{
    CloseAudioDevice();
    UNUSED(self);
}