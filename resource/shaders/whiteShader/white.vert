#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

out vec2 UV;
out vec3 normal;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    vec4 transformPosition = vec4(aPos, 1.0);

    transformPosition = ModelMatrix * transformPosition;
    
    gl_Position = ProjectionMatrix * ViewMatrix * transformPosition;

    UV = aUV;
    normal = aNormal;
}