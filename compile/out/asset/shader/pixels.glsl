#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 ScreenSize;
uniform vec2 MousePos;
uniform float DepressionFactor;
uniform int ColorStepCount;
uniform float DayBrightness;
uniform sampler2D LightTexture;

vec3 LerpColor(vec3 colorA, vec3 colorB, float amount)
{
    return colorA + (colorB - colorA) * amount;
}

void main()
{
    // Pixels.
    float AspectRatio = ScreenSize.x / ScreenSize.y;
    float PixelsPerWidth = 480.0f * AspectRatio;

    float StepsX = PixelsPerWidth;
    float StepsY = PixelsPerWidth / AspectRatio;

    vec2 ClampedCoords = vec2(floor(fragTexCoord.x * StepsX) / StepsX, floor(fragTexCoord.y * StepsY) / StepsY);

    finalColor = texture(texture0, vec2(ClampedCoords.x, ClampedCoords.y));


    // Day brightness.
    vec3 DayTint = vec3(1.0, 1.0f, 1.0f);
    vec3 NightTint = vec3(0.8, 0.8f, 1.15f);
    vec3 EveningTint = vec3(1.05f, 0.9f, 0.85f);

    float NightTintStrength = clamp(1.0f - (DayBrightness * 2.0f), 0.0f, 1.0f);
    float EveningTintStrength = clamp(1.0f - abs(DayBrightness - 0.5f) * 2.0f, 0.0f, 1.0f);

    vec4 LightIntensityColor = texture(LightTexture, ClampedCoords);
    float LightStrength = LightIntensityColor.a;

    vec3 FinalTint = LerpColor(LerpColor(DayTint, NightTint, NightTintStrength), EveningTint, EveningTintStrength);

    finalColor.rgb *= FinalTint;

    float BrightnessMin = 0.5f;
    float BrightnessMax = 1.0f;
    float Brightness = BrightnessMin + ((BrightnessMax - BrightnessMin) * (DayBrightness + LightStrength));
    finalColor.rgb *= Brightness;

    float PowScaleMin = 2.5f;
    float PowScaleMax = 1.0f;
    float PowScale = PowScaleMin + ((PowScaleMax - PowScaleMin) * DayBrightness + LightStrength);

    float PowOffset = 2.0f;
    finalColor.r = pow(finalColor.r * PowOffset, PowScale) / PowOffset;
    finalColor.g = pow(finalColor.g * PowOffset, PowScale) / PowOffset;
    finalColor.b = pow(finalColor.b * PowOffset, PowScale) / PowOffset;

    float DayDepressionMin = 0.7;
    float DayDepressionMax = 0.0;
    float FinalDepressionFactor = DayDepressionMin + ((DayDepressionMax - DayDepressionMin) * DayBrightness);

    // Depression factor.
    float AverageColor = (finalColor.r + finalColor.g + finalColor.b) / 3.0f;
    vec3 DistanceFromAverage = vec3(AverageColor) - finalColor.rgb;
    finalColor.r = finalColor.r + (DistanceFromAverage.r * FinalDepressionFactor);
    finalColor.g = finalColor.g + (DistanceFromAverage.g * FinalDepressionFactor);
    finalColor.b = finalColor.b + (DistanceFromAverage.b * FinalDepressionFactor);


    // Color steps.
    finalColor.r = round(finalColor.r * ColorStepCount) / ColorStepCount;
    finalColor.g = round(finalColor.g * ColorStepCount) / ColorStepCount;
    finalColor.b = round(finalColor.b * ColorStepCount) / ColorStepCount;

    finalColor *= fragColor;
}