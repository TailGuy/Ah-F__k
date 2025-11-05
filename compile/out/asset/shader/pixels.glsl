#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 ScreenSize;
uniform float DepressionFactor;
uniform int ColorStepCount;

void main()
{
    // Pixels.
    float AspectRatio = ScreenSize.x / ScreenSize.y;
    float PixelsPerWidth = 128.0f * AspectRatio;

    float StepsX = PixelsPerWidth;
    float StepsY = PixelsPerWidth / AspectRatio;

    vec2 ClampedCoords = vec2(floor(fragTexCoord.x * StepsX) / StepsX, floor(fragTexCoord.y * StepsY) / StepsY);

    finalColor = texture(texture0, vec2(ClampedCoords.x, ClampedCoords.y));


    // Depression factor.
    float AverageColor = (finalColor.x + finalColor.y + finalColor.z) / 3.0f;
    vec3 DistanceFromAverage = vec3(AverageColor) - finalColor.rgb;
    finalColor.r = finalColor.r + (DistanceFromAverage.r * DepressionFactor);
    finalColor.g = finalColor.g + (DistanceFromAverage.g * DepressionFactor);
    finalColor.b = finalColor.b + (DistanceFromAverage.b * DepressionFactor);


    // Color steps.
    finalColor.r = round(finalColor.r * ColorStepCount) / ColorStepCount;
    finalColor.g = round(finalColor.g * ColorStepCount) / ColorStepCount;
    finalColor.b = round(finalColor.b * ColorStepCount) / ColorStepCount;

    finalColor.rgb *= colDiffuse.rgb;
}