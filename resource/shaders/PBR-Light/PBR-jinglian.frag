#version 330 core
out vec4 FragColor;

// 从顶点着色器传入的变量
in vec2 UV;       
in vec3 worldPosition;
in vec3 normal;

// 材质参数（直接用uniform，无需贴图）
uniform float metallic;
uniform float roughness;

const vec3 objectcolor = vec3(0.5, 0.5, 0.5);   // 仅一个固有色（albedo）

// 光源参数
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


struct AmbientLight{
    float Intensity;
    vec3 color;
};

uniform PointLight pointLights[4];
uniform AmbientLight ambientLight;
uniform vec3 cameraPosition;

const float PI = 3.14159265359;


// ----------------------------------------------------------------------------
// GGX法线分布函数（NDF）
// 作用：计算朝向半程向量H的微表面占比，决定高光的集中/分散程度
// 参数：N=法线，H=半程向量，roughness=粗糙度
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;    // 粗糙度的平方（更符合物理的近似）
    float a2 = a * a;                   // 粗糙度的四次方
    float NdotH = max(dot(N, H), 0.0);  // 法线与半程向量的点积（取正值）
    float NdotH2 = NdotH * NdotH;       // 点积的平方

    // GGX分布公式计算
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// Schlick-GGX几何函数（单项）
// 作用：计算单个方向（视角或光源）的几何遮蔽/阴影（微表面相互遮挡）
// 参数：NdotV=法线与视角方向的点积，roughness=粗糙度
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;  // 几何函数的粗糙度修正因子

    // 几何遮蔽公式
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// Smith几何函数（组合项）
// 作用：综合考虑视角和光源方向的几何遮蔽（双向遮蔽）
// 参数：N=法线，V=视角方向，L=光源方向，roughness=粗糙度
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);  // 法线与视角方向的点积
    float NdotL = max(dot(N, L), 0.0);  // 法线与光源方向的点积
    
    // 分别计算视角和光源方向的几何遮蔽，再相乘
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
// Schlick菲涅尔近似
// 作用：计算不同视角下的镜面反射占比（菲涅尔效应）
// 参数：cosTheta=视角方向与半程向量的夹角余弦，F0=基础反射率
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    // 菲涅尔公式近似：视角越倾斜，反射越强
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 启用Gamma校正，加HDR色调映射
// 没有法线贴图，没有任何纹理贴图
void main(){
    // PBR部分属性初始化
    vec3 albedo = objectcolor;

    // 法线向量与视角方向向量计算
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - worldPosition);  // 视角方向向量（从表面指向相机）

    // 计算基础反射率F0
    vec3  F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);  

    // 计算反射方程（累加所有光源的贡献）
    vec3 Lo = vec3(0.0);  // 出射辐射度（初始化为0）
    for(int i = 0; i < 4; ++i)  // 遍历所有4个光源
    {
        // 计算光源相关向量和辐射度
        vec3 L = normalize(pointLights[i].position - worldPosition);  
        vec3 H = normalize(V + L);                        
        float distance = length(pointLights[i].position - worldPosition); 
        float attenuation = 1.0 / (pointLights[i].kc + pointLights[i].k1 * distance + pointLights[i].k2 * distance * distance);      
        vec3 radiance = pointLights[i].color * attenuation;          

        // 计算Cook-Torrance BRDF组件
        float NDF = DistributionGGX(N, H, roughness);          // 法线分布函数（高光形状）
        float G   = GeometrySmith(N, V, L, roughness);         // 几何函数（高光遮蔽）
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);   // 菲涅尔方程（反射占比）
           
        // 计算高光项（镜面反射）
        vec3 numerator    = NDF * G * F; 
        // 分母（+0.0001避免除零错误）
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        // 能量守恒：漫反射占比 = 1 - 镜面反射占比
        vec3 kS = F;  
        vec3 kD = vec3(1.0) - kS; 
        kD *= 1.0 - metallic; 

        // 计算光源对当前像素的贡献（根据法线与光源的夹角衰减）
        float NdotL = max(dot(N, L), 0.0);        
        // 累加出射辐射度（漫反射 + 高光）× 光源辐射度 × 角度衰减
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   

    // 计算平行光

    
    // 计算环境光
    vec3 ambient = ambientLight.color * ambientLight.Intensity * albedo * (1.0 - metallic);

    // 最终颜色计算
    vec3 color = ambient + Lo;  // 环境光 + 所有光源的直接光照

    // HDR 颜色映射
    color = color / (color + vec3(1.0));
    // gamma 校正
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}