#version 330 core
out vec4 FragColor;

in vec2 UV;
in vec3 normal;
in vec3 worldPosition;

uniform sampler2D sampler;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;

void main()
{
    // 获取物体的当前像素值
    vec3 objectColor = texture(sampler, UV).xyz;

    // 准备漫反射相关数据
    vec3 normalN = normalize(normal);
    vec3 lightDirN = normalize(lightDirection);

    float diffuse = clamp(dot(-lightDirN, normalN), 0.0, 1.0);

    vec3 finalColor = lightColor * diffuse * objectColor;


    vec3 viewDir = normalize(worldPosition - cameraPosition);
    viewDir = clamp(-viewDir,0.0f, 1.0f);
    FragColor = vec4(viewDir, 1.0);
}
