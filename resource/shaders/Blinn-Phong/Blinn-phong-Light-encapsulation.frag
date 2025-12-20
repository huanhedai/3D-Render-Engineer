#version 330 core
out vec4 FragColor;

in vec2 UV;
in vec3 normal; // 像素法向
in vec3 worldPosition;

uniform sampler2D sampler;
uniform sampler2D specularMaskSampler;

uniform vec3 ambientColor;
// 相机位置
uniform vec3 cameraPosition;

uniform float shiness;  // 控制光斑大小   光斑大小与该值成反比  属于material的参数
uniform bool blinn;     // 控制是否开启Blinn 属于material的参数

// 平行灯光源参数
struct DirectionLight{
    vec3 direction;
    vec3 color;
    float specularIntensity;
};

// 点光源参数
struct PointLight{
    vec3 position;
    vec3 color;
    float specularIntensity;
    
    float k2;
    float k1;
    float kc;
};

// 聚光灯光源参数
struct SpotLight{
    vec3 position;
    vec3 targetDirection;
    vec3 color;
    float innerLine;
    float outerLine;
    float specularIntensity;

    float k2;
    float k1;
    float kc;
};

uniform SpotLight spotLight;
uniform DirectionLight directionLight;
uniform PointLight pointLight;

// 计算漫反射光照
vec3 caculateDiffuse(vec3 lightColor, vec3 lightDirN, vec3 normalN, vec3 objectColor){
    float diffuse = clamp(dot(-lightDirN, normalN), 0.0, 1.0);
    vec3 diffuseColor = lightColor * diffuse * objectColor;
    return diffuseColor;
};

// 计算镜面反射光照
vec3 caculateSpecular(vec3 lightColor, vec3 lightDirN, vec3 normalN, vec3 viewDirN, float intensity){
    // 补充: 防止背面光效果
    float dotResult = dot(-lightDirN, normalN);
    float flag = step(0.0, dotResult);

    float specular = 0.0f;

    if(blinn){
        vec3 halfwayDir = normalize(-lightDirN - viewDirN);
        specular = max(dot(normalN, halfwayDir), 0.0);
        // 控制光斑大小
        specular = pow(specular,shiness);    // shiness越大，光斑越小，(cos x)^shiness
    }else{
        vec3 lightReflect = normalize(reflect(lightDirN, normalN)); // 求反射向量
        specular = max(dot(lightReflect, -viewDirN), 0.0);
        // 控制光斑大小
        specular = pow(specular,shiness);  
    }

    // 读取高光贴图
    float specularMap = texture(specularMaskSampler, UV).r; // 只取红色通道
    vec3 specularColor = lightColor * specular * flag * intensity * specularMap; 

    return specularColor;
};

// 计算聚光灯光照的函数
vec3 caculateSpotLight(SpotLight light,vec3 normalN, vec3 viewDirN){
    // 计算光照的通用数据
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 lightDirN = normalize(worldPosition - light.position);
    vec3 targetDirN = normalize(light.targetDirection);

    // 计算spotlight的照射范围
    float cGamma = dot(lightDirN,targetDirN);
    float intensity = clamp((cGamma - light.outerLine) / (light.innerLine - light.outerLine),0.0, 1.0);

    // 计算衰减
    float dist = length(worldPosition - light.position);
    float attenuation = 1.0 / (light.kc + light.k1 * dist + light.k2 * dist * dist);

    // 计算漫反射
    vec3 diffuseColor = caculateDiffuse(light.color, lightDirN, normalN, objectColor);

    // 计算specular
    vec3 specularColor = caculateSpecular(light.color, lightDirN, normalN, viewDirN, light.specularIntensity);

    return (diffuseColor + specularColor) * (attenuation * intensity);

};

// 计算平行光光照的函数
vec3 caculateDirectionalLight(DirectionLight light, vec3 normalN, vec3 viewDirN){
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 lightDirN = normalize(light.direction);

    // 1. 计算diffuse
    vec3 diffuseColor = caculateDiffuse(light.color, lightDirN, normalN, objectColor);

    // 2. 计算specular
    vec3 specularColor = caculateSpecular(light.color, lightDirN, normalN, viewDirN, light.specularIntensity);

    return diffuseColor + specularColor;
};

// 计算点光源光照的函数
vec3 caculatePointLight(PointLight light, vec3 normalN, vec3 viewDirN){
    // 计算光照的通用数据
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 lightDirN = normalize(worldPosition - light.position);

    // 计算衰减
    float dist = length(worldPosition - light.position);
    float attenuation = 1.0 / (light.kc + light.k1 * dist + light.k2 * dist * dist);

    // 计算漫反射
    vec3 diffuseColor = caculateDiffuse(light.color, lightDirN, normalN, objectColor);

    // 计算specular
    vec3 specularColor = caculateSpecular(light.color, lightDirN, normalN, viewDirN, light.specularIntensity);

    return (diffuseColor + specularColor) * attenuation;
};

void main()
{
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    // 计算光照的通用数据
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 normalN = normalize(normal);
    vec3 viewDirN = normalize(worldPosition - cameraPosition);

    result += caculateSpotLight(spotLight, normalN, viewDirN);
    result += caculateDirectionalLight(directionLight,normalN, viewDirN);
    result += caculatePointLight(pointLight, normalN, viewDirN);

    // 环境光计算
    vec3 ambientColor = objectColor * ambientColor;

    vec3 finalColor = result + ambientColor;     // 注意环境光不衰减

    FragColor = vec4(finalColor, 1.0);
}
