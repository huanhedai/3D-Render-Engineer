#version 330 core
layout (location = 0) in vec3 aPos;      // 顶点位置（对应Geometry的location=0）
layout (location = 1) in vec2 aTexCoord; // UV坐标（对应Geometry的location=1）

// MVP矩阵（简化：用正交投影，避免透视带来的变形）
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoord; // 传递UV给片段着色器

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord; // 传递UV（用于片段着色器的颜色计算）
}