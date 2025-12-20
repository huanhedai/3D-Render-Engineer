#version 330 core
in vec2 TexCoord;  // 接收顶点着色器的UV
out vec4 FragColor; // 输出像素颜色

void main() {
    // 用UV生成渐变色：左下（黑）→ 右下（红）→ 顶部（黄）
    FragColor = vec4(TexCoord.x, TexCoord.y, 0.0f, 1.0f);
}