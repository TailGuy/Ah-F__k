#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

void main()
{
    float StepCount = 32;
    vec2 ClampedCoords = vec2(floor(fragTexCoord.x * StepCount) / StepCount, floor(fragTexCoord.y * StepCount) / StepCount);
    finalColor = texture(texture0, ClampedCoords);
}