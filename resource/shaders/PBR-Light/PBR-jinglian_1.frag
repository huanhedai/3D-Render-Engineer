#version 330 core
out vec4 FragColor;

// 从顶点着色器传入的变量
in vec2 UV;       
in vec3 worldPosition;
in vec3 normal;

// 材质参数（直接用uniform，无需贴图）
uniform float metallic;
uniform float roughness;

uniform sampler2D albedoMap;     // 基础颜色贴图：存储物体固有色
uniform sampler2D normalMap;     // 法线贴图：存储表面微观凹凸信息
uniform sampler2D metallicMap;   // 金属度贴图：存储表面金属属性（0=非金属，1=纯金属）
uniform sampler2D roughnessMap;  // 粗糙度贴图：存储表面粗糙程度（0=光滑，1=粗糙）

// 光源参数
struct PointLight{
    vec3 position;     // 光源位置
    vec3 color;        // 光照颜色
    float specularIntensity; // 镜面强度
    
    float k2; // 二次衰减系数
    float k1; // 一次衰减系数
    float kc; // 常数衰减系数

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirectionLight{
    vec3 direction;    // 光照方向（归一化）
    vec3 color;        // 光照颜色
    float specularIntensity; // 镜面强度

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct AmbientLight{
    vec3 color;
    float Intensity;
};

uniform PointLight pointLights[4];
uniform AmbientLight ambientLight;
uniform DirectionLight directionLight;
uniform vec3 cameraPosition;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// 从法线贴图获取世界空间法线的工具函数
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, UV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(worldPosition);
    vec3 Q2  = dFdy(worldPosition);
    vec2 st1 = dFdx(UV);
    vec2 st2 = dFdy(UV);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
// GGX法线分布函数（NDF）
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// Schlick-GGX几何函数（单项）
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 0.0);
    float k = (r * r) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// Smith几何函数（组合项）
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
// Schlick菲涅尔近似
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    vec3 edgeColor = vec3(1.0, 1.0, 1.0);
    return F0 + (edgeColor - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------------------------
// VTK PBR 颜色映射（基于VTK官方PBR实现的Reinhard色调映射）
// 参考VTK的vtkPBRLightingHelper中的色调映射逻辑：
// 核心是带最大亮度控制的Reinhard映射，兼顾HDR动态范围压缩与自然视觉效果
vec3 VTKPBRToneMap(vec3 color)
{
    // VTK PBR默认参数：最大参考亮度（可根据需求调整，VTK默认1.5~2.0）
    const float maxLuminance = 1.8;
    
    // 计算线性亮度（BT.709标准，VTK统一采用）
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // VTK风格Reinhard变体：保留高光细节，避免过曝
    vec3 mapped = color * (1.0 + color / (maxLuminance * maxLuminance)) / (1.0 + color);
    
    // 钳位到[0,1]范围（VTK PBR强制颜色范围约束）
    return clamp(mapped, 0.0, 1.0);
}

// Blender 使用了更接近物理的色调映射（如 ACES）
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// 启用Gamma校正，使用VTK PBR颜色映射
void main(){
    // PBR部分属性初始化
    vec3 albedo = texture(albedoMap, UV).rgb;
    float Metallic = texture(metallicMap, UV).r;
    float Roughness = texture(roughnessMap, UV).r;

    Metallic = metallic;
    Roughness = roughness;

    // 法线向量与视角方向向量计算
    vec3 N = normalize(normal);
    N = getNormalFromMap();
    vec3 V = normalize(cameraPosition - worldPosition);

    // 计算基础反射率F0
    vec3  F0 = vec3(0.04); 
    F0 = mix(F0, albedo, Metallic);  

    // 计算反射方程（累加所有点光源的贡献）
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        vec3 L = normalize(pointLights[i].position - worldPosition);
        vec3 H = normalize(V + L);
        float distance = length(pointLights[i].position - worldPosition);
        float attenuation = 1.0 / (pointLights[i].kc + pointLights[i].k1 * distance + pointLights[i].k2 * distance * distance);
        vec3 radiance = pointLights[i].color * attenuation;

        float NDF = DistributionGGX(N, H, Roughness);
        float G   = GeometrySmith(N, V, L, Roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - Metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   

    // 计算平行光贡献
    {
        vec3 L = normalize(-directionLight.direction);
        vec3 H = normalize(V + L);
        vec3 radiance = directionLight.color;

        float NDF = DistributionGGX(N, H, Roughness);
        float G   = GeometrySmith(N, V, L, Roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - Metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   
    
    // 计算环境光
    vec3 ambient = ambientLight.Intensity * ambientLight.color * albedo * (1.0 - Metallic);

    // 最终颜色计算
    vec3 color = ambient + Lo;

    // 替换ACES为VTK PBR颜色映射
    //color = ACESFilm(color);

    // Gamma校正（VTK PBR默认采用sRGB标准Gamma=2.2）
    //color = pow(color, vec3(1.0/2.2)); 
    if(texture(normalMap, UV).x == 1.0){
        color.x = 1.0;
    }
    FragColor = vec4(color, 1.0);
}