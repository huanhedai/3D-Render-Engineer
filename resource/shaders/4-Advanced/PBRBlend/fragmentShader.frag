#version 330 core
out vec4 FragColor;

// 从顶点着色器传入的变量
in vec2 UV;       
in vec3 worldPosition;
in vec3 Normal;
in vec3 tangent;

// 透明度控制
uniform float opacity;

// 材质参数
uniform float metallic;
uniform float roughness;
uniform sampler2D albedoMap;     // 基础颜色+alpha贴图
uniform sampler2D normalMap;     // 切线空间法线贴图
uniform sampler2D metallicMap;   // 金属度贴图
uniform sampler2D roughnessMap;  // 粗糙度贴图
uniform bool useNormalMap;       // 法线贴图开关

// 光源参数
struct PointLight{
    vec3 position;
    vec3 color;
    float specularIntensity;
    float k2; // 二次衰减
    float k1; // 一次衰减
    float kc; // 常数衰减
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirectionLight{
    vec3 direction;    // 已归一化
    vec3 color;
    float specularIntensity;
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

// 从法线贴图获取世界空间法线（使用顶点切线）
vec3 getNormalFromMap()
{
    // 1. 采样切线空间法线并转换到[-1,1]范围
    vec3 tangentNormal = texture(normalMap, UV).xyz * 2.0 - 1.0;

    // 2. 获取世界空间法线和切线（确保单位向量）
    vec3 N = normalize(Normal);
    // 背面翻转法线（修复双面渲染）
    if (!gl_FrontFacing) {
        N = -N;
    }
    vec3 T = normalize(tangent);

    // 3. 格拉姆-施密特正交化（确保T与N垂直）
    T = normalize(T - dot(T, N) * N);
    // 4. 计算副切线
    vec3 B = cross(N, T);
    // 5. 构建TBN矩阵（切线→世界空间）
    mat3 TBN = mat3(T, B, N);

    // 6. 转换法线并归一化
    return normalize(TBN * tangentNormal);
}

// GGX法线分布函数
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Schlick-GGX几何函数（单项）
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness;
    float k = (r * r) / 2.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Smith几何函数（组合项）
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Schlick菲涅尔近似
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    vec3 edgeColor = vec3(1.0);
    return F0 + (edgeColor - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// VTK PBR色调映射（HDR压缩）
vec3 VTKPBRToneMap(vec3 color)
{
    const float maxLuminance = 1.8;
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    vec3 mapped = color * (1.0 + color / (maxLuminance * maxLuminance)) / (1.0 + color);
    return clamp(mapped, 0.0, 1.0);
}

void main(){
    // 1. 采样材质属性（缓存albedo的rgb和alpha，避免重复采样）
    vec4 albedoSample = texture(albedoMap, UV);
    vec3 albedo = albedoSample.rgb;
    float albedoAlpha = albedoSample.a;

    // 2. 金属度/粗糙度（贴图+uniform混合，灵活调整）
    float metallicTex = texture(metallicMap, UV).r;
    float roughnessTex = texture(roughnessMap, UV).r;
    float Metallic = mix(metallicTex, metallic, 0.5); // 权重可自定义
    float Roughness = mix(roughnessTex, roughness, 0.5);
    Roughness = clamp(Roughness, 0.01, 0.99); // 避免极端值导致光照异常

    // 3. 计算世界空间法线（支持开关法线贴图）
    vec3 N;
    if(useNormalMap){
        N = getNormalFromMap();
    }else{
        N = normalize(Normal);
        if (!gl_FrontFacing) { // 背面翻转
            N = -N;
        }
    }

    // 4. 视角方向（世界空间）
    vec3 V = normalize(cameraPosition - worldPosition);

    // 5. PBR基础参数：F0（基础反射率）
    vec3 F0 = vec3(0.04); // 非金属默认反射率
    F0 = mix(F0, albedo, Metallic); // 金属材质用albedo替代F0

    // 6. 反射方程：累加所有光源贡献
    vec3 Lo = vec3(0.0);

    // 6.1 点光源贡献（4个）
    for(int i = 0; i < 4; ++i)
    {
        vec3 L = normalize(pointLights[i].position - worldPosition);
        vec3 H = normalize(V + L); // 半程向量
        float distance = length(pointLights[i].position - worldPosition);
        // 衰减计算
        float attenuation = 1.0 / (pointLights[i].kc + pointLights[i].k1 * distance + pointLights[i].k2 * distance * distance);
        vec3 radiance = pointLights[i].color * attenuation;

        // PBR三大核心函数
        float NDF = DistributionGGX(N, H, Roughness);
        float G = GeometrySmith(N, V, L, Roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        // 镜面反射与漫反射分离
        vec3 kS = F; // 镜面反射系数（菲涅尔结果）
        vec3 kD = vec3(1.0) - kS; // 漫反射系数
        kD *= 1.0 - Metallic; // 金属无漫反射

        // 最终光源贡献
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // 6.2 方向光贡献
    {
        vec3 L = normalize(-directionLight.direction);
        vec3 H = normalize(V + L);
        vec3 radiance = directionLight.color;

        float NDF = DistributionGGX(N, H, Roughness);
        float G = GeometrySmith(N, V, L, Roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - Metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // 7. 环境光贡献
    vec3 ambient = ambientLight.Intensity * ambientLight.color * albedo * (1.0 - Metallic);

    // 8. 最终颜色计算（色调映射+Gamma校正）
    vec3 color = ambient + Lo;
    color = VTKPBRToneMap(color); // HDR动态范围压缩
    color = pow(color, vec3(1.0/2.2)); // Gamma校正（sRGB标准）

    // 9. 透明度混合（opacity * 贴图alpha）
    float finalAlpha = opacity * albedoAlpha;

    FragColor = vec4(color, finalAlpha);
}