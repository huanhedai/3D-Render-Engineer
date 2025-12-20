#version 330 core
// 输出最终像素颜色
out vec4 FragColor;

// 从顶点着色器传入的变量
in vec2 UV;       // 纹理坐标
in vec3 worldPosition;        // 世界空间位置
in vec3 normal;          // 原始法线向量

// 材质参数贴图（PBR核心属性）
uniform sampler2D albedoMap;     // 基础颜色贴图：存储物体固有色
uniform sampler2D normalMap;     // 法线贴图：存储表面微观凹凸信息
uniform sampler2D metallicMap;   // 金属度贴图：存储表面金属属性（0=非金属，1=纯金属）
uniform sampler2D roughnessMap;  // 粗糙度贴图：存储表面粗糙程度（0=光滑，1=粗糙）
uniform sampler2D aoMap;         // 环境光遮蔽贴图：存储凹陷区域的环境光遮挡程度

// 光源参数
struct PointLight{
    vec3 position;
    vec3 color;
};
uniform PointLight pointLights[4];

// 相机（观察者）位置
uniform vec3 cameraPosition;

// 数学常量π
const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// 从法线贴图获取世界空间法线的工具函数
// 作用：将切线空间的法线转换到世界空间，无需额外传入切线向量
vec3 getNormalFromMap()
{
    // 从法线贴图采样并转换范围（贴图存储范围是[0,1]，转换为[-1,1]）
    vec3 tangentNormal = texture(normalMap, UV).xyz * 2.0 - 1.0;

    // 计算用于构建TBN矩阵的辅助向量（通过位置和纹理坐标的偏导数）
    vec3 Q1  = dFdx(worldPosition);    // 世界位置对x的偏导数
    vec3 Q2  = dFdy(worldPosition);    // 世界位置对y的偏导数
    vec2 st1 = dFdx(UV);   // 纹理坐标对x的偏导数
    vec2 st2 = dFdy(UV);   // 纹理坐标对y的偏导数

    // 构建TBN矩阵（切线-副切线-法线矩阵）
    vec3 N   = normalize(normal);                  // 世界空间法线
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);      // 切线向量
    vec3 B  = -normalize(cross(N, T));             // 副切线向量（与切线、法线垂直）
    mat3 TBN = mat3(T, B, N);                      // 切线空间到世界空间的转换矩阵

    // 将切线空间法线转换到世界空间并返回
    return normalize(TBN * tangentNormal);
}

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

// ----------------------------------------------------------------------------
void main()
{		
    // 1. 采样材质属性（从贴图获取当前像素的材质参数）
    vec3 albedo     = pow(texture(albedoMap, UV).rgb, vec3(2.2));  // 基础颜色（伽马校正）
    float metallic  = texture(metallicMap, UV).r;                  // 金属度（取红色通道）
    float roughness = texture(roughnessMap, UV).r;                 // 粗糙度（取红色通道）
    float ao        = texture(aoMap, UV).r;                        // 环境光遮蔽（取红色通道）

    // 2. 计算关键向量
    vec3 N = getNormalFromMap();  // 从法线贴图获取世界空间法线
    vec3 V = normalize(cameraPosition - worldPosition);  // 视角方向向量（从表面指向相机）

    // 3. 计算基础反射率F0
    // 非金属默认F0为0.04，金属使用自身颜色作为F0（金属工作流特性）
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);  // 根据金属度混合F0

    // 4. 计算反射方程（累加所有光源的贡献）
    vec3 Lo = vec3(0.0);  // 出射辐射度（初始化为0）
    for(int i = 0; i < 4; ++i)  // 遍历所有4个光源
    {
        // 4.1 计算光源相关向量和辐射度
        vec3 L = normalize(pointLights[i].position - worldPosition);  // 光源方向向量（从表面指向光源）
        vec3 H = normalize(V + L);                         // 半程向量（视角与光源方向的中间向量）
        float distance = length(pointLights[i].position - worldPosition);  // 表面到光源的距离
        float attenuation = 1.0 / (distance * distance);         // 点光源衰减（平方反比定律）
        vec3 radiance = pointLights[i].color * attenuation;            // 光源辐射度（颜色×衰减）

        // 4.2 计算Cook-Torrance BRDF组件
        float NDF = DistributionGGX(N, H, roughness);          // 法线分布函数（高光形状）
        float G   = GeometrySmith(N, V, L, roughness);         // 几何函数（高光遮蔽）
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);   // 菲涅尔方程（反射占比）
           
        // 4.3 计算高光项（镜面反射）
        vec3 numerator    = NDF * G * F; 
        // 分母（+0.0001避免除零错误）
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        
        // 4.4 能量守恒：漫反射占比 = 1 - 镜面反射占比
        vec3 kS = F;  // 镜面反射占比（等于菲涅尔值）
        vec3 kD = vec3(1.0) - kS;  // 漫反射占比
        kD *= 1.0 - metallic;  // 金属无漫反射（金属度越高，漫反射占比越低）

        // 4.5 计算光源对当前像素的贡献（根据法线与光源的夹角衰减）
        float NdotL = max(dot(N, L), 0.0);        
        // 累加出射辐射度（漫反射 + 高光）× 光源辐射度 × 角度衰减
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   
    
    // 5. 环境光（简化版，实际项目中常用IBL环境贴图）
    vec3 ambient = vec3(0.03) * albedo * ao;  // 环境光 × 基础颜色 × 环境遮蔽
    
    ambient = vec3(0.03) * albedo * ao;  // 环境光 × 基础颜色 × 环境遮蔽

    //vec3 ambient = vec3(1.0) * albedo;
    
    // 简化环境光（无IBL时，用固定值替代）
    ambient = vec3(0.03) * albedo * (1.0 - metallic); // 弱化环境光，避免能量溢出

    // 6. 最终颜色计算
    vec3 color = ambient + Lo;  // 环境光 + 所有光源的直接光照

    // 7. HDR色调映射（将高动态范围颜色映射到[0,1]范围，避免过曝）
    color = color / (color + vec3(1.0));
    // 8. 伽马校正（将线性颜色空间转换为显示设备的非线性空间）
    color = pow(color, vec3(1.0/2.2)); 

    // 输出最终颜色
    FragColor = vec4(color, 1.0);
}
