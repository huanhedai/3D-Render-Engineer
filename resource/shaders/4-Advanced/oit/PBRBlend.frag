#version 330 core
layout (location = 0) out vec4 FragColorWeight;  // 附件0：color × weight , 通过 location 0 写入 FBO 的 GL_COLOR_ATTACHMENT0
layout (location = 1) out vec4 FragWeightSum;    // 附件1：weight总和

// 从顶点着色器传入的变量
in vec2 UV;       
in vec3 worldPosition;
in vec3 normal;
in vec3 tangent;

// 透明度
uniform float opacity;

// 材质参数（直接用uniform，无需贴图）
uniform float metallic;
uniform float roughness;

uniform sampler2D albedoMap;     // 基础颜色贴图：存储物体固有色
uniform sampler2D normalMap;     // 法线贴图：存储表面微观凹凸信息
uniform sampler2D metallicMap;   // 金属度贴图：存储表面金属属性（0=非金属，1=纯金属）
uniform sampler2D roughnessMap;  // 粗糙度贴图：存储表面粗糙程度（0=光滑，1=粗糙）

// 新增：控制是否启用法线贴图
uniform bool useNormalMap;

// 权重计算函数（原有逻辑保留）
float computeWeight(float depth, float alpha) {
    float linearDepth = (depth + 1.0) / 2.0;
    return exp(-linearDepth * 0.1) * (1.0 - alpha);
}

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
// ----------------------------------------------------------------------------
// 从法线贴图获取世界空间法线的工具函数（使用传入的切线）
vec3 getNormalFromMap()
{
    // 1. 从法线贴图中采样切线空间的法线，并将其从 [0, 1] 范围转换到 [-1, 1] 范围
    vec3 tangentNormal = texture(normalMap, UV).xyz * 2.0 - 1.0;

    // 2. 获取从顶点着色器传入的世界空间法线和切线
    vec3 N = normalize(normal); // 确保法线是单位向量
    vec3 T = normalize(tangent); // 确保切线是单位向量

    // 3. 对切线进行格拉姆-施密特正交化，确保它与法线严格垂直
    // T' = T - (T · N) * N
    T = normalize(T - dot(T, N) * N);

    // 4. 计算副切线 B (Bitangent)
    // B = N × T (叉乘)
    vec3 B = cross(N, T);

    // 5. 构建 TBN 矩阵（Tangent, Bitangent, Normal）
    // 这个矩阵用于将切线空间的向量转换到世界空间
    mat3 TBN = mat3(T, B, N);

    // 6. 将切线空间的法线转换到世界空间
    vec3 worldNormal = TBN * tangentNormal;

    // 7. 返回归一化后的世界空间法线
    return normalize(worldNormal);
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
    vec3 N;
    if(useNormalMap){
        N = getNormalFromMap();
    }else{
        N = normalize(normal);
    }
    // 新增：背面翻转法线
    if (!gl_FrontFacing) {
        N = -N;
    }

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

    float alpha = texture(albedoMap, UV).a;

    // OIT模式：输出到2个FBO附件
    float depth = gl_FragCoord.z * 2.0 - 1.0;
    float weight = computeWeight(depth, opacity);
    //FragColorWeight = vec4(color * weight, 1.0);
    //FragWeightSum = vec4(weight, 0.0, 0.0, 1.0);

    alpha = clamp(alpha, 0.0, 1.0);
    float w = max(0.01, alpha * (1.0 - 0.5 * alpha));

    FragColorWeight = vec4(color * alpha, alpha) * w;
    FragWeightSum = vec4(alpha, 0.0, 0.0, 1.0);

}