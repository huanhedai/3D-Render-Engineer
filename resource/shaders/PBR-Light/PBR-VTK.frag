#version 330 core
out vec4 FragColor;

// optional color passed in from the vertex shader, vertexColor
uniform float ambientIntensity;         // the material ambient
uniform float diffuseIntensity;         // the material diffuse
uniform float specularIntensity;        // the material specular intensity
uniform float specularPowerUniform;
uniform float opacityUniform;		  // the fragment opacity       !!!特殊，一般是给材质定义透明度啊！
uniform vec3 ambientColorUniform;	 // the ambient color
uniform vec3 diffuseColorUniform;	 // the diffuse color
uniform vec3 specularColorUniform;	 // the specular color

// optional surface normal declaration      
uniform float normalScaleUniform;
in vec3 tangent;    // 法线贴图需要用的切线
uniform mat3 normalMatrix;      // 顶点着色器才会使用，片段着色器不需要
in vec3 Normal;     // the surface normal

// extra lighting paraments
# define CLEAR_COAT // 是否启用涂层，油漆层

const float PI = 3.14159265359;
const float recPI = 0.31830988618; // 1/PI
uniform float metallicUniform;	// the material metallic
uniform float roughnessUniform;	// the material roughness
uniform vec3 emissiveFactorUniform; // the material emissive factor     发射因子，描述自发光的
uniform float aoStrengthUniform; // the material ambient occlusion strength 环境光遮蔽强度
uniform float baseF0Uniform; // the material base reflectivity at normal incidence 基础反射率    -- 菲涅尔现象
uniform vec3 edgeTintUniform; // the material edge tint 边缘色调                                 -- 菲涅尔现象

#ifdef ANISOTRORY      // 是否启用各向异性
uniform float anisotropyUniform;    // the material anisotropy 各向异性强度
#endif

#ifdef CLEAR_COAT
uniform float coatF0Uniform;    // the coat layer reflectivity at normal incidence 涂层反射率
uniform float coatRoughnessUniform; // the coat layer roughness 涂层粗糙度
uniform float coatStrengthUniform;  // the coat layer strength 涂层强度
uniform vec3 coatColorUniform;  // the coat layer color 涂层颜色
#endif

float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0;
    return a2 / (PI * d * d);
}

float V_SmithCorrelated(float NdotV, float NdotL, float roughness)
{
    float a2 = roughness * roughness;
    float ggxV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float ggxL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    return 0.5 / (ggxV + ggxL);
}

vec3 F_Schlick(vec3 F0,vec3 F90,float HdL)
{
    return F0 + (F90 - F0) * pow(1.0 - HdL, 5.0);
}

vec3 DiffuseLambert(vec3 albedo)
{
    return albedo * recPI;
}

vec3 SpecularIsotropic(float NdotH, float NdotV, float NdL, float HdL, float roughness,
    vec3 F0, vec3 F90, out vec3 F)  // F会传出去
{
    float D = D_GGX(NdotH, roughness);
    float V = V_SmithCorrelated(NdotV, NdL, roughness);
    F = F_Schlick(F0, F90, HdL);
    return (D * V) * F;
}

#ifdef ANISOTRORY
// ... 各向异性相关函数待补充
#endif



// PBR 贴图
uniform sampler2D albedoMap;     // 基础颜色贴图：存储物体固有色
uniform sampler2D normalMap;     // 法线贴图：存储表面微观凹凸信息
uniform sampler2D metallicMap;   // 金属度贴图：存储表面金属属性（0=非金属，1=纯金属）
uniform sampler2D roughnessMap;  // 粗糙度贴图：存储表面粗糙程度（0=光滑，1=粗糙）
uniform sampler2D aoMap;         // 环境光遮蔽贴图：存储环境光遮蔽信息

// Fragment shader inputs from vertex shader
in vec2 UV;
in vec3 worldPosition;

// Camera prop
uniform vec3 cameraPosition;


// Light struct definitions
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

// 聚光灯光源参数
struct SpotLight{
    vec3 position;
    vec3 targetDirection;
    vec3 color;
    float innerLine;
    float outerLine;
    float specularIntensity;
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
    float Intensity;
    vec3 color;
};

#ifdef MAX_POINT_LIGHTS
uniform PointLight pointLights[MAX_POINT_LIGHTS];
#endif

#ifdef MAX_DIRECTION_LIGHTS
uniform DirectionLight directionLights[MAX_DIRECTION_LIGHTS];
#endif

#ifdef MAX_SPOT_LIGHTS
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
#endif
uniform AmbientLight ambientLight;

void main()
{
    // value Pass
    vec3 specularColor = specularIntensity * specularColorUniform;
    float specularPower = specularPowerUniform;
    vec3 ambientColor = ambientIntensity * ambientColorUniform;
    vec3 diffuseColor = diffuseIntensity * diffuseColorUniform;
    float opacity = opacityUniform;
    
    
    // Generate the normal if we are not passed in one  没有法线则生成法线
    vec3 Normal = normalize(Normal);
    if(gl_FrontFacing == false)
        Normal = -Normal;   // 背面法线取反
    vec3 coatNormal = Normal;   // 涂层法线初始化为基础法线
    vec3 tangentVC = tangent;
    tangentVC = normalize(tangentVC - dot(tangentVC, Normal) * Normal);
    vec3 bitangentVC = cross(Normal, tangentVC);
    mat3 TBN = mat3(tangentVC, bitangentVC, Normal);
    vec3 NormalTS = texture(normalMap, UV).xyz * 2.0 - 1.0;
    NormalTS = normalize(NormalTS * vec3(normalScaleUniform,normalScaleUniform,1.0));       // NormalTS 在x和y方向上进行缩放
    Normal = normalize(TBN * NormalTS);

    // PBR Impl
    vec4 albedoSample = texture(albedoMap, UV);
    vec3 albedo = albedoSample.rgb * diffuseColor;
    opacity = opacity * albedoSample.a;
    float metallic = texture(metallicMap, UV).r * metallicUniform;
    float roughness = texture(roughnessMap, UV).r * roughnessUniform;
    float ao = texture(aoMap, UV).r;

    vec3 emissiveColor = vec3(0.0);
    vec3 N = Normal;
    vec3 V = normalize(cameraPosition - worldPosition); // 视线方向
    float NdV = clamp(dot(N, V), 1e-5, 1.0);    // clamp: 若 dot(N, V) < 1e-5 则取 1e-5，若 >1 则取 1

    vec3 coatN = coatNormal;
    float coatRoughness = coatRoughnessUniform;
    float coatStrength = coatStrengthUniform;
    float coatNdV = clamp(dot(coatN, V), 1e-5, 1.0);

    vec3 irradiance = vec3(0.0);    // 环境光照
    vec3 prefilteredSpecularColor = vec3(0.0); // 预过滤环境光照
    vec2 brdf = vec2(0.0); // BRDF 折射率 n1, n2
    vec3 prefilteredSpecularCoatColor = vec3(0.0); // 涂层预过滤环境光照
    vec2 coatBrdf = vec2(0.0); // 涂层 BRDF 折射率 n1, n2

    vec3 Lo = vec3(0.0); // 直接光照累加
    vec3 F0 = mix(vec3(baseF0Uniform), albedo, metallic); // 基础反射率
    float f90 = clamp(dot(F0,vec3(50.0 * 0.33)),0.0,1.0); // 边缘反射率强度
    vec3 F90 = mix(vec3(f90), edgeTintUniform, metallic); // 边缘反射率颜色

    vec3 L,H,radiance,F,specular,diffuse;
    float NdL,NdH,HdL,ditanceVC,attenuation,D,Vis;

    vec3 coatF0 = vec3(coatF0Uniform);
    vec3 coatF90 = vec3(1.0);
    vec3 coatLayer, Fc;
    float coatNdL,coatNdH;
    vec3 coatColorFactor = mix(vec3(1.0), coatColorUniform, coatStrength);

    // 平行光累加
    #ifdef MAX_DIRECTION_LIGHTS
    for(int i = 0; i < MAX_DIRECTION_LIGHTS;i++){
        vec3 L = normalize(-directionLights[i].direction);
        H = normalize(V + L);
        HdL = clamp(dot(H, L), 1e-5, 1.0);
        NdL = clamp(dot(N, L), 1e-5, 1.0);
        NdH = clamp(dot(N, H), 1e-5, 1.0);
        radiance = directionLights[i].color; // 光照颜色
        // 计算各向同性镜面反射
        specular = SpecularIsotropic(NdH, NdV, NdL, HdL, roughness, F0, F90, F);
        diffuse = (1.0 - metallic) * (1.0 - F) * DiffuseLambert(albedo);
        coatNdL = clamp(dot(coatN, L), 1e-5, 1.0);
        coatNdH = clamp(dot(coatN, H), 1e-5, 1.0);
        coatLayer = SpecularIsotropic(coatNdH, coatNdV, coatNdL, HdL, coatRoughness, coatF0, coatF90, Fc) * radiance * coatNdL * coatStrength;
        // Energy compensation depending on how much light is reflected by the coat layer
        Fc *= coatStrength;
        specular *= (1.0 - Fc) * (1.0 - Fc);
        diffuse *= (1.0 - Fc);
        radiance *= coatColorFactor;
        Lo += coatLayer;
        Lo += radiance * (specular + diffuse) * NdL;
    }
    #endif

    // 点光源累加
    #ifdef MAX_POINT_LIGHTS
    for(int i = 0; i < MAX_POINT_LIGHTS;i++){
        vec3 L = normalize(pointLights[i].position - worldPosition);
        H = normalize(V + L);
        HdL = clamp(dot(H, L), 1e-5, 1.0);
        NdL = clamp(dot(N, L), 1e-5, 1.0);
        NdH = clamp(dot(N, H), 1e-5, 1.0);

        ditanceVC = length(pointLights[i].position - worldPosition);
        attenuation = 1.0 / (pointLights[i].kc + pointLights[i].k1 * ditanceVC + pointLights[i].k2 * ditanceVC * ditanceVC);
        radiance = pointLights[i].color * attenuation;
        
        // 计算各向同性镜面反射
        specular = SpecularIsotropic(NdH, NdV, NdL, HdL, roughness, F0, F90, F);
        diffuse = (1.0 - metallic) * (1.0 - F) * DiffuseLambert(albedo);
        coatNdL = clamp(dot(coatN, L), 1e-5, 1.0);
        coatNdH = clamp(dot(coatN, H), 1e-5, 1.0);
        coatLayer = SpecularIsotropic(coatNdH, coatNdV, coatNdL, HdL, coatRoughness, coatF0, coatF90, Fc) * radiance * coatNdL * coatStrength;
        // Energy compensation depending on how much light is reflected by the coat layer
        Fc *= coatStrength;
        specular *= (1.0 - Fc) * (1.0 - Fc);
        diffuse *= (1.0 - Fc);
        radiance *= coatColorFactor;
        Lo += coatLayer;
        Lo += radiance * (specular + diffuse) * NdL;
    }
    #endif

    // 聚光灯累加
    #ifdef MAX_SPOT_LIGHTS
    for(int i = 0; i < MAX_SPOT_LIGHTS;i++){
        vec3 L = normalize(spotLights[i].position - worldPosition);
        vec3 targetDirN = normalize(spotLights[i].targetDirection);

        H = normalize(V + L);
        HdL = clamp(dot(H, L), 1e-5, 1.0);
        NdL = clamp(dot(N, L), 1e-5, 1.0);
        NdH = clamp(dot(N, H), 1e-5, 1.0);

        ditanceVC = length(spotLights[i].position - worldPosition);
        attenuation = 1.0 / (spotLights[i].kc + spotLights[i].k1 * ditanceVC + spotLights[i].k2 * ditanceVC * ditanceVC);
        if(spotLights[i].innerLine < 90.0){
            float coneDot = dot(-L, targetDirN);
            // if inside the cone(锥)
            if(coneDot >= cos(radians(spotLights[i].outerLine))
            {
                float cGamma = dot(-L,targetDirN);
                float intensity = clamp((cGamma - light.outerLine) / (light.innerLine - light.outerLine),0.0, 1.0);
                attenuation = attenuation * intensity;
            }
            else
            {
                attenuation = 0.0;  // 不在锥体内设为0
            }
        }
        radiance = spotLights[i].color * attenuation;
        
        // 计算各向同性镜面反射
        specular = SpecularIsotropic(NdH, NdV, NdL, HdL, roughness, F0, F90, F);
        diffuse = (1.0 - metallic) * (1.0 - F) * DiffuseLambert(albedo);
        coatNdL = clamp(dot(coatN, L), 1e-5, 1.0);
        coatNdH = clamp(dot(coatN, H), 1e-5, 1.0);
        coatLayer = SpecularIsotropic(coatNdH, coatNdV, coatNdL, HdL, coatRoughness, coatF0, coatF90, Fc) * radiance * coatNdL * coatStrength;
        // Energy compensation depending on how much light is reflected by the coat layer
        Fc *= coatStrength;
        specular *= (1.0 - Fc) * (1.0 - Fc);
        diffuse *= (1.0 - Fc);
        radiance *= coatColorFactor;
        Lo += coatLayer;
        Lo += radiance * (specular + diffuse) * NdL;
    }
    #endif

    // In IBL, we assume that v = n, so the amount of light reflected by the reflectance F0
    vec3 specularBrdf = F0 * brdf.r + F90 * brdf.g;
    vec3 iblSpecular = prefilteredSpecularColor * specularBrdf;
    vec3 iblDiffuse = (1.0 - F0) * (1.0 - metallic) * irradiance * albedo;
    vec3 color = iblDiffuse + iblSpecular;

    // Clear coat attenuation
    Fc = F_Schlick(coatF0, coatF90, coatNdV) * coatStrength;
    iblSpecular *= (1.0 - Fc);
    iblDiffuse *= (1.0 - Fc) * (1.0 - Fc);
    // Clear coat specular
    vec3 iblSpecularClearCoat = prefilteredSpecularCoatColor * (coatF0 * coatBrdf.r + coatBrdf.g) * Fc;
    color *= coatColorFactor;
    color += iblSpecularClearCoat;

    color += Lo;
    color = mix(color, color * ao, aoStrengthUniform);
    color += emissiveColor;
    color = pow(color, vec3(1.0/2.2));
    FragColor = vec4(color, opacity);
}