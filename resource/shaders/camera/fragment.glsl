#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// 简单示例：使用固定颜色（蓝色）
void main()
{
    FragColor = vec4(0.2f, 0.5f, 0.9f, 1.0f); // 蓝色立方体
}

// 如需使用纹理，可取消以下注释并确保Texture类正确加载纹理
/*
uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
}
*/