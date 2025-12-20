// composite.frag（OIT核心复合逻辑）
#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

// 采样OIT的两个附件
uniform sampler2D colorWeightTexture;  // color × weight
uniform sampler2D weightSumTexture;    // weight总和

const float EPS = 1e-6;  // 避免除零

void main() {
    // 采样两个附件
    vec4 colorWeight = texture(colorWeightTexture, TexCoords);
    float weightSum = texture(weightSumTexture, TexCoords).r;

    // 计算最终透明颜色：(sum(color×weight)) / (sum(weight) + ε)
    //vec3 finalColor = colorWeight.rgb / (weightSum + EPS);
    //float alpha = 1.0 - exp(-weightSum);  // 整体透明度（可选，控制全局透明强度）

    //FragColor = vec4(finalColor, alpha);

    float alpha = clamp(1.0 - weightSum,0.0,1.0);
    if(colorWeight.a <= 1e-5 || alpha <= 1e-5){
        FragColor = vec4(0.0);
        return;
    }
    vec3 color = colorWeight.rgb / colorWeight.a;
    FragColor = vec4(colorWeight.rgb / colorWeight.a, alpha);
    

}