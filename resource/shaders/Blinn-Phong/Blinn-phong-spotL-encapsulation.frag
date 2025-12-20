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

// 计算漫反射光照
vec3 caculateDiffuse(vec3 lightColor, vec3 lightDirN, vec3 normalN, vec3 objectColor){
    float diffuse = clamp(dot(-lightDirN, normalN), 0.0, 1.0);
    vec3 diffuseColor = lightColor * diffuse * objectColor;
    return diffuseColor;
};

// 计算镜面反射光照
vec3 caculateSpecular(vec3 lightColor, vec3 lightDirN, vec3 normalN, vec3 viewDir, float intensity){
    // 补充: 防止背面光效果
    float dotResult = dot(-lightDirN, normalN);
    float flag = step(0.0, dotResult);

    float specular = 0.0f;

    if(blinn){
        // 半程向量【片元->相机 向量与 片元->光源 向量的和】与法线向量夹角的cos值控制specular
        vec3 halfwayDir = normalize(-lightDirN - viewDir);
        specular = max(dot(normalN, halfwayDir), 0.0);
        // 控制光斑大小
        specular = pow(specular,shiness);    // shiness越大，光斑越小，(cos x)^shiness
    }else{
        // 视线向量与法线向量夹角的cos值控制specular
        vec3 lightReflect = normalize(reflect(lightDirN, normalN)); // 求反射向量
        specular = max(dot(lightReflect, -viewDir), 0.0);
        // 控制光斑大小
        specular = pow(specular,shiness);    // shiness越大，光斑越小，(cos x)^shiness
    }

    // 读取高光贴图
    float specularMap = texture(specularMaskSampler, UV).r; // 只取红色通道
    vec3 specularColor = spotLight.color * specular * flag * spotLight.specularIntensity; 

    return specularColor;
};

void main()
{
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    // 计算光照的通用数据
    vec3 objectColor = texture(sampler, UV).xyz;
    vec3 normalN = normalize(normal);
    vec3 lightDirN = normalize(worldPosition - spotLight.position);
    vec3 viewDir = normalize(worldPosition - cameraPosition);
    vec3 targetDirN = normalize(spotLight.targetDirection);

    // 计算spotlight的照射范围
    float cGamma = dot(lightDirN,targetDirN);
    float spotIntensity = clamp((cGamma - spotLight.outerLine) / (spotLight.innerLine - spotLight.outerLine),0.0, 1.0);

    // 计算衰减
    float dist = length(worldPosition - spotLight.position);
    float attenuation = 1.0 / (spotLight.kc + spotLight.k1 * dist + spotLight.k2 * dist * dist);

    // 计算漫反射
    result += caculateDiffuse(spotLight.color, lightDirN, normalN, objectColor);

    // 计算specular
    result += caculateSpecular(spotLight.color, lightDirN, normalN, viewDir, spotLight.specularIntensity);
    result *=  (attenuation * spotIntensity);

    // 环境光计算
    result += objectColor * ambientColor;

    FragColor = vec4(result, 1.0);

}
