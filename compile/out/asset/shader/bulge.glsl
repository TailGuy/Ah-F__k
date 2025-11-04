#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;
uniform vec2 ScreenSize;

void main()
{
    vec2 Vertex2DPos = vec2(vertexPosition.x, vertexPosition.y);
    vec2 ScreenCenter = vec2(ScreenSize.x / 2.0f, ScreenSize.y / 2.0f);
    float DistanceToCenter = distance(Vertex2DPos, ScreenCenter);

    vec2 CenterToVertex = Vertex2DPos - ScreenCenter;

    float Strength = 0.00025f;
    float Multiplier = 1.0f + (DistanceToCenter * Strength);
    Multiplier = pow(Multiplier, 3.0f);

    vec3 AdjustedPosition = vec3(ScreenCenter + (CenterToVertex * Multiplier), vertexPosition.z);

    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp * vec4(AdjustedPosition, 1.0);
}