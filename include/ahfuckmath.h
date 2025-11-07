#pragma once
#include <stdint.h>

typedef struct IntVectorStruct
{
    int32_t X;
    int32_t Y;
} IntVector;

static inline float AbsFloat(float value)
{
    if (value >= 0.0f)
    {
        return value;
    }
    return -value;
}

// Functions.
static inline float Max(float a, float b)
{
    if (a >= b)
    {
        return a;
    }
    return b;
}

static inline float Min(float a, float b)
{
    if (a <= b)
    {
        return a;
    }
    return b;
}