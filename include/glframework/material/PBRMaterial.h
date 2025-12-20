#pragma once
#include "material.h"
#include "../texture.h"

class PBRMaterial : public Material {
public:
    PBRMaterial();
    ~PBRMaterial();

    void setAo(float ao) {
        mAo = ao;
    }
    void setRoughness(float roughness) {
        mRoughness = roughness;
    }
    void setMetallic(float metallic) {
        mMetallic = metallic;
    }

    // 获取环境光遮蔽值
    float getAo() const {
        return mAo;
    }

    // 获取粗糙度值
    float getRoughness() const {
        return mRoughness;
    }

    // 获取金属度值
    float getMetallic() const {
        return mMetallic;
    }

public:
    Texture* mAlbedoMap{ nullptr };     // 基础颜色贴图：存储物体固有色
    Texture* mNormalMap{ nullptr };     // 法线贴图：存储表面微观凹凸信息
    Texture* mMetallicMap{ nullptr };   // 金属度贴图：存储表面金属属性（0=非金属，1=纯金属）
    Texture* mRoughnessMap{ nullptr };  // 粗糙度贴图：存储表面粗糙程度（0=光滑，1=粗糙）
    Texture* mAoMap{ nullptr };         // 环境光遮蔽贴图：存储凹陷区域的环境光遮挡程度

private:
    float mAo = { 1.0f };    // 默认环境光遮蔽
    float mRoughness = { 1.0f };
    float mMetallic = { 0.0f };

public:
    float n1 = { 1.5 };   // 折射率计算F0
    /* coat PBR涂层 */
    float coatn1 = { 1.5 };
    float coatRoughness = { 0.2 };
    float coatStrength = { 1.0 };
    glm::vec3 coatColor = glm::vec3(1.0,1.0,1.0);
};