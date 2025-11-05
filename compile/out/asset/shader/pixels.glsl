#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec2 ScreenSize;

void main()
{
    float AspectRatio = ScreenSize.x / ScreenSize.y;
    float PixelsPerWidth = 960.0f * AspectRatio;

    float StepsX = PixelsPerWidth;
    float StepsY = PixelsPerWidth / AspectRatio;

    vec2 ClampedCoords = vec2(floor(fragTexCoord.x * StepsX) / StepsX, floor(fragTexCoord.y * StepsY) / StepsY);
    finalColor = texture(texture0, ClampedCoords);
}