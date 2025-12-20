#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
// 如需使用纹理，可取消以下注释并确保Texture类正确加载纹理
uniform sampler2D albedoMap;

// 透明度
uniform float opacity;

void main()
{
    float alpha = texture(albedoMap, TexCoord).a;
    FragColor = vec4(texture(albedoMap, TexCoord).xyz, alpha * opacity);
}
