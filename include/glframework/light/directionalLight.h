#pragma once
#include "light.h"

class DirectionalLight : public Light {
public:
    DirectionalLight();
    ~DirectionalLight();

    // 方向向量的设置与获取（已有）
    void setDirection(const glm::vec3& dir) { mDirection = glm::normalize(dir); }
    const glm::vec3& getDirection() const{ return mDirection; }

    // 环境光的设置与获取
    void setAmbient(const glm::vec3& amb) { mAmbient = amb; }  // 完善原有空函数
    glm::vec3& getAmbient() { return mAmbient; }

    // 漫反射光的设置与获取
    void setDiffuse(const glm::vec3& diff) { mDiffuse = diff; }
    glm::vec3& getDiffuse() { return mDiffuse; }

    // 镜面反射光的设置与获取
    void setSpecular(const glm::vec3& spec) { mSpecular = spec; }
    glm::vec3& getSpecular() { return mSpecular; }

public:
    glm::vec3 mDirection{ -1.0f };  // 平行光方向（世界坐标）
    glm::vec3 mAmbient{ 1.0f };     // 环境光颜色
    glm::vec3 mDiffuse{ 1.0f };     // 漫反射光颜色
    glm::vec3 mSpecular{ 1.0f };    // 镜面反射光颜色
};