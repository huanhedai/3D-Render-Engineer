#version 330 core
out vec4 FragColor;

in vec2 UV;
in vec3 normal;

uniform sampler2D sampler;
uniform vec3 lightColor;
uniform vec3 lightDirection;

void main()
{
    // 获取物体当前像素的颜色
    vec3 objectColor = texture(sampler, UV).xyz;

    // 准备漫反射相关数据
    vec3 normalN = normalize(normal);
    vec3 lightDirN = normalize(lightDirection);

    float diffuse = clamp(dot(-lightDirN, normalN), 0.0, 1.0);

    vec3 finalColor = lightColor * diffuse * objectColor;

    FragColor = vec4(finalColor, 1.0);
}
