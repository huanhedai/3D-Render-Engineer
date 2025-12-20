#version 330 core
out vec4 FragColor;

in vec2 UV;       
in vec3 normal; // 像素法向
in vec3 worldPosition;

uniform vec3 cameraPosition;
uniform sampler2D sampler;

// 透明度
uniform float opacity;

struct Material{
    vec3 ambient;    // 环境光反射色（需手动设，如vec3(0.1)）
    vec3 diffuse;    // 漫反射色（需手动设，如vec3(0.5)）
    vec3 specular;   // 高光反射色（需手动设，如vec3(1.0)）
    float shiness;   // 光泽度（经验值，如32、64，无物理对应）
};
struct AmbientLight{
    float intensity;
    vec3 color;
};

// 平行灯光源
struct DirectionLight{
    vec3 direction;    // 光照方向（归一化）
    float specularIntensity; // 镜面强度

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// 点光源
struct PointLight{
    vec3 position;     // 光源位置
    vec3 color;        // 光照颜色
    float specularIntensity; // 镜面强度
    
    float k2; // 二次衰减系数
    float k1; // 一次衰减系数
    float kc; // 常数衰减系数

    // 这三个参数是光源在求解对应三种反射光时与之相乘的，这是一种高度抽象，无实际物理意义，胜在效果不错
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirectionLight directionLight;
#define POINT_LIGHT_NUM 4       
uniform PointLight pointLights[POINT_LIGHT_NUM];
uniform int numPointLights;
uniform Material material;
uniform AmbientLight ambientLight;


// 点光源光照计算
vec3 calculatePointLight(PointLight light, vec3 normalN, vec3 fragPos,vec3 viewDirN, vec3 objectColor){
    vec3 lightDirN = normalize(light.position - fragPos); // 光源→物体方向
    // 计算漫反射
    float diff = clamp(dot(lightDirN, normalN), 0.0, 1.0); 
    vec3 diffuse = light.diffuse * objectColor * diff;
    
    // 计算镜面反射
    vec3 halfwayDir = normalize(lightDirN + viewDirN); // 半程向量（Blinn-Phong核心）
    float spec = pow(max(dot(normalN, halfwayDir), 0.0), material.shiness);
    vec3 specular = light.specular * material.specular * spec * light.specularIntensity;

    // 计算环境光照
    vec3 ambient = light.ambient * material.ambient;

    // 衰减计算
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / (light.kc + light.k1 * dist + light.k2 * dist * dist);

    return (diffuse + specular + ambient) * attenuation;
};

// 平行光光照计算
vec3 calculateDirectionalLight(DirectionLight light, vec3 normalN, vec3 fragPos,vec3 viewDirN, vec3 objectColor){
    vec3 lightDirN = normalize(-light.direction); // 平行光方向（已归一化）

    // 计算漫反射
    float diff = clamp(dot(lightDirN, normalN), 0.0, 1.0); 
    vec3 diffuse = light.diffuse * objectColor * diff;
    
    // 计算镜面反射
    vec3 halfwayDir = normalize(lightDirN + viewDirN); // 半程向量（Blinn-Phong核心）
    float spec = pow(max(dot(normalN, halfwayDir), 0.0), material.shiness);
    vec3 specular = light.specular * material.specular * spec * light.specularIntensity;

    // 计算环境光照
    vec3 ambient = light.ambient * material.ambient;

    // 平行光无衰减，故无衰减计算

    return diffuse + specular + ambient;
};

vec3 calculateAmbientLight(AmbientLight light, vec3 normalN, vec3 fragPos,vec3 viewDirN, vec3 objectColor){
    return light.intensity * light.color * objectColor;
}

void main()
{
    //vec3 objectColor = vec3(0.5, 0.5, 0.5); // 目标灰色
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 result = vec3(0.0);
    vec3 normalN = normalize(normal);
    vec3 viewDirN = normalize(cameraPosition - worldPosition); // 相机→物体方向
    // 平行光贡献
    result += calculateDirectionalLight(directionLight, normalN, worldPosition, viewDirN, objectColor);
    // 点光源贡献
    for(int i = 0; i < POINT_LIGHT_NUM; i++){
        result += calculatePointLight(pointLights[i], normalN, worldPosition, viewDirN, objectColor);
    }

    // 环境光贡献（避免全黑区域）
    result += calculateAmbientLight(ambientLight, normalN, worldPosition, viewDirN, objectColor);

    vec3 finalColor = result;

    float alpha = texture(sampler, UV).a;
    FragColor = vec4(finalColor, opacity * alpha);
}