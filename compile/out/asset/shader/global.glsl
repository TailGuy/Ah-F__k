#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 ScreenSize;
uniform float DepressionFactor;
uniform int ColorStepCount;
uniform float RandomValue;

vec3 LerpColor(vec3 colorA, vec3 colorB, float amount)
{
    return colorA + (colorB - colorA) * amount;
}

float Random(vec2 co)
{
    // https://stackoverflow.com/a/4275343
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // Pixels.
    float AspectRatio = ScreenSize.x / ScreenSize.y;
    float PixelsPerWidth = 1920.0f * AspectRatio;

    float StepsX = PixelsPerWidth;
    float StepsY = PixelsPerWidth / AspectRatio;

    vec2 ClampedCoords = vec2(floor(fragTexCoord.x * StepsX) / StepsX, floor(fragTexCoord.y * StepsY) / StepsY);

    finalColor = texture(texture0, vec2(ClampedCoords.x, ClampedCoords.y));

    // Color steps.
    finalColor.r = round(finalColor.r * ColorStepCount) / ColorStepCount;
    finalColor.g = round(finalColor.g * ColorStepCount) / ColorStepCount;
    finalColor.b = round(finalColor.b * ColorStepCount) / ColorStepCount;

    finalColor *= fragColor;

    // Noise.
    float MultiplierMin = 0.95f;
    float MultiplierMax = 1.05f;
    vec2 AlteredRandomPos = vec2(ClampedCoords.x + RandomValue, ClampedCoords.y + RandomValue);
    float MultiplierFactor = (MultiplierMin + (MultiplierMax - MultiplierMin) * Random(AlteredRandomPos));
    float NoneMultiplier = 1.0f;
    finalColor.rgb *= (NoneMultiplier + ((MultiplierFactor - NoneMultiplier) * DepressionFactor));

    // Depression factor.
    float AverageColor = (finalColor.r + finalColor.g + finalColor.b) / 3.0f;
    vec3 DistanceFromAverage = vec3(AverageColor) - finalColor.rgb;
    finalColor.r = finalColor.r + (DistanceFromAverage.r * DepressionFactor);
    finalColor.g = finalColor.g + (DistanceFromAverage.g * DepressionFactor);
    finalColor.b = finalColor.b + (DistanceFromAverage.b * DepressionFactor);
}