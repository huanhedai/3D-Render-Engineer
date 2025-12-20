#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 UV;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);       // 直接将二维屏幕坐标传入齐次坐标NDC
    UV = aUV;
}