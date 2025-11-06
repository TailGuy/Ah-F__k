#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float DayBrightness;
uniform sampler2D LightTexture;

vec3 LerpColor(vec3 colorA, vec3 colorB, float amount)
{
    return colorA + (colorB - colorA) * amount;
}

void main()
{
    // This code is so fucking bad but ight at least we get some cool visuals???

    vec2 LightTextureCoords = vec2(fragTexCoord.x, 1.0f - fragTexCoord.y);
    finalColor = texture(texture0, fragTexCoord);

    // Day brightness.
    vec3 DayTint = vec3(1.0, 1.0f, 1.0f);
    vec3 NightTint = vec3(0.8, 0.8f, 1.15f);
    vec3 EveningTint = vec3(1.05f, 0.9f, 0.85f);

    float NightTintStrength = clamp(1.0f - (DayBrightness * 2.0f), 0.0f, 1.0f);
    float EveningTintStrength = clamp(1.0f - abs(DayBrightness - 0.5f) * 2.0f, 0.0f, 1.0f);

    vec4 LightIntensityColor = texture(LightTexture, LightTextureCoords);
    bool IsLightOn = NightTintStrength >= 0.85;
    float LightStrength = ((LightIntensityColor.r + LightIntensityColor.b + LightIntensityColor.g) / 3.0) * (IsLightOn ? 1.0 : 0.0);
    bool IsLightOnInPixel = LightStrength > 0;
    float LightIntensity = max(LightStrength, DayBrightness);

    vec3 FinalTint = vec3(1.0, 1.0, 1.0);
    if (!IsLightOnInPixel)
    {
        FinalTint = LerpColor(LerpColor(DayTint, NightTint, NightTintStrength), EveningTint, EveningTintStrength);
    }
    
    finalColor.rgb *= FinalTint;

    float BrightnessMin = 0.5f;
    float BrightnessMax = IsLightOnInPixel ? 1.1 : 1.0f;
    float Brightness = BrightnessMin + ((BrightnessMax - BrightnessMin) * LightIntensity);
    finalColor.rgb *= Brightness;

    if (!IsLightOnInPixel)
    {
        float PowScaleMin = 2.5f;
        float PowScaleMax = 1.0f;
        float PowScale = PowScaleMin + ((PowScaleMax - PowScaleMin) * LightIntensity);

        float PowOffset = 2.0f;
        finalColor.r = pow(finalColor.r * PowOffset, PowScale) / PowOffset;
        finalColor.g = pow(finalColor.g * PowOffset, PowScale) / PowOffset;
        finalColor.b = pow(finalColor.b * PowOffset, PowScale) / PowOffset;
    }

    if (IsLightOn && !IsLightOnInPixel)
    {
        finalColor.rgb *= 1.5;
    }

    float GrayMin = IsLightOnInPixel ? 0.0 : 0.7;
    float GrayMan = 0.0;
    float FinalGray = clamp(GrayMin + ((GrayMan - GrayMin) * (IsLightOn ? 0.0 : DayBrightness)), 0.0f, 1.0f);

    float AverageColor = (finalColor.r + finalColor.g + finalColor.b) / 3.0f;
    vec3 DistanceFromAverage = vec3(AverageColor) - finalColor.rgb;
    finalColor.r = finalColor.r + (DistanceFromAverage.r * FinalGray);
    finalColor.g = finalColor.g + (DistanceFromAverage.g * FinalGray);
    finalColor.b = finalColor.b + (DistanceFromAverage.b * FinalGray);
}