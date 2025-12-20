#version 330 core
out vec4 FragColor;

in vec3 normal; // 像素法向
in vec3 worldPosition;

uniform vec3 ambientColor; // 环境光颜色
uniform vec3 cameraPosition;

uniform float shiness;  // 镜面高光指数（建议50~200）
uniform bool blinn;     // 是否启用Blinn-Phong

// 平行灯光源
struct DirectionLight{
    vec3 direction;    // 光照方向（归一化）
    vec3 color;        // 光照颜色
    float specularIntensity; // 镜面强度
};

// 点光源
struct PointLight{
    vec3 position;     // 光源位置
    vec3 color;        // 光照颜色
    float specularIntensity; // 镜面强度
    
    float k2; // 二次衰减系数
    float k1; // 一次衰减系数
    float kc; // 常数衰减系数
};

uniform DirectionLight directionLight;
#define POINT_LIGHT_NUM 4       
uniform PointLight pointLights[4];

// 计算漫反射
vec3 calculateDiffuse(vec3 lightColor, vec3 lightDirN, vec3 normalN, vec3 objectColor){
    float diffuse = clamp(dot(lightDirN, normalN), 0.0, 1.0);
    return lightColor * diffuse * objectColor;
};

// 计算镜面反射
vec3 calculateSpecular(vec3 lightColor, vec3 lightDirN, vec3 normalN, vec3 viewDirN, float intensity){
    float specular = 0.0;
    if(blinn){
        vec3 halfwayDir = normalize(lightDirN + viewDirN); // 半程向量（Blinn-Phong核心）
        specular = pow(max(dot(normalN, halfwayDir), 0.0), shiness);
    } else {
        vec3 reflectDir = reflect(-lightDirN, normalN); // 反射向量（Phong模型）
        specular = pow(max(dot(reflectDir, viewDirN), 0.0), shiness);
    }
    return lightColor * specular * intensity;
};

// 点光源光照计算
vec3 calculatePointLight(PointLight light, vec3 normalN, vec3 viewDirN){
    vec3 objectColor = vec3(0.5, 0.5, 0.5); // 目标灰色（可调整RGB值改变深浅）
    vec3 lightDirN = normalize(light.position - worldPosition); // 光源→物体方向

    // 衰减计算
    float dist = length(light.position - worldPosition);
    float attenuation = 1.0 / (light.kc + light.k1 * dist + light.k2 * dist * dist);

    // 漫反射 + 镜面反射
    vec3 diffuse = calculateDiffuse(light.color, lightDirN, normalN, objectColor);
    vec3 specular = calculateSpecular(light.color, lightDirN, normalN, viewDirN, light.specularIntensity);

    return (diffuse + specular) * attenuation;
};

// 平行光光照计算
vec3 calculateDirectionalLight(DirectionLight light, vec3 normalN, vec3 viewDirN){
    vec3 objectColor = vec3(0.5, 0.5, 0.5); // 目标灰色
    vec3 lightDirN = normalize(light.direction); // 平行光方向（已归一化）

    // 漫反射 + 镜面反射
    vec3 diffuse = calculateDiffuse(light.color, lightDirN, normalN, objectColor);
    vec3 specular = calculateSpecular(light.color, lightDirN, normalN, viewDirN, light.specularIntensity);

    return diffuse + specular;
};

vec3 calcPointLight(PointLight pointlight,vec3 fragPos,vec3 cameraPos,vec3 objectCol){
    vec3 lightDir = normalize(pointlight.position - fragPos);
    vec3 viewDir = normalize(cameraPos - fragPos);
    float diff = max(dot(normal,lightDir),0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal,halfwayDir),0.0),shiness);
    float dist = length(pointlight.position - fragPos);
    float attenuation = 1.0 / (pointlight.kc + pointlight.k1 * dist + pointlight.k2 * dist * dist);
    vec3 ambient = ambientColor;
    vec3 diffuse = pointlight.color * objectCol * diff;
    vec3 specular = pointlight.color * spec;

    return (ambient + diffuse + specular) * attenuation;
}

void main()
{
    vec3 result = vec3(0.0);
    vec3 normalN = normalize(normal);
    vec3 viewDirN = normalize(cameraPosition - worldPosition); // 相机→物体方向

    // 平行光贡献
    result += calculateDirectionalLight(directionLight, normalN, viewDirN);
    // 点光源贡献
    for(int i = 0; i < POINT_LIGHT_NUM; i++){
        //result += calculatePointLight(pointLights[i], normalN, viewDirN);
        result += calcPointLight(pointLights[i],worldPosition,cameraPosition,vec3(0.5, 0.5, 0.5));
    }

    // 环境光贡献（避免全黑区域）
    //vec3 ambient = vec3(0.5, 0.5, 0.5) * ambientColor; // 灰色 * 环境光颜色

    //vec3 finalColor = result + ambient;

    FragColor = vec4(result, 1.0);
}