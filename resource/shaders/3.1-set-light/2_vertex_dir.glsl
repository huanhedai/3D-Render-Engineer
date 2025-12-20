#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

out vec2 UV;
out vec3 normal;
out vec3 worldPosition;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
    // 将输入的顶点位置，转化为齐次坐标（3维-4维）
    vec4 transformPosition = vec4(aPos, 1.0);

    transformPosition = ModelMatrix * transformPosition;
    
    // 计算当前顶点的 worldPosition，并且向后传输给fragmentShader
    worldPosition = transformPosition.xyz;
    // 运用前面计算过的结果，防止重复计算
    gl_Position = ProjectionMatrix * ViewMatrix * transformPosition;
    UV = aUV;
    normal = aNormal;
}