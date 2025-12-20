#version 330 core
out vec4 FragColor;

in vec2 UV;
in vec3 normal; // 像素法向
in vec3 worldPosition;

uniform sampler2D sampler;

// 光源参数
uniform vec3 lightColor;
uniform vec3 lightDirection;

uniform vec3 ambientColor;
// 相机位置
uniform vec3 cameraPosition;

// 控制光斑强弱
uniform float specularIntensity;

void main()
{
    // 计算光照的通用数据
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 normalN = normalize(normal);
    vec3 lightDirN = normalize(lightDirection);
    vec3 viewDir = normalize(worldPosition - cameraPosition);

    // 准备diffuse漫反射相关的各类数据
    float diffuse = clamp(dot(-lightDirN, normalN), 0.0, 1.0);
    vec3 diffuseColor = lightColor * diffuse * objectColor;

    // 计算specular
    // 补充: 防止背面光效果
    float dotResult = dot(-lightDirN, normalN);
    float flag = step(0.0, dotResult);

    vec3 lightReflect = normalize(reflect(lightDirN, normalN)); // 求反射向量
    float specular = max(dot(lightReflect, -viewDir), 0.0);
    // 控制光斑大小
    specular = pow(specular,12);    // N越大，光斑越小，(cos x)^N
    vec3 specularColor = lightColor * specular * flag * specularIntensity; // 不需要乘 objectColor，因为是直接反射，不考虑吸收

    // 环境光计算
    vec3 ambientColor = objectColor * ambientColor;

    vec3 finalColor = specularColor + diffuseColor + ambientColor;
    FragColor = vec4(finalColor, 1.0);

    /*
    float step(float edge, float x);

    如果x比edge大，返回1，反之返回0.
    */
}
