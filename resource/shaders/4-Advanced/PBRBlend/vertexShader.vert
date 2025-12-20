#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out vec2 UV;
out vec3 Normal;
out vec3 tangent;
out vec3 worldPosition;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    vec4 transformPosition = ModelMatrix * vec4(aPos, 1.0);
    worldPosition = transformPosition.xyz;
    gl_Position = ProjectionMatrix * ViewMatrix * transformPosition;
    
    UV = aUV;
    Normal = normalize(normalMatrix * aNormal); // 归一化确保单位向量
    tangent = normalize(normalMatrix * aTangent); // 切线同步变换+归一化
}