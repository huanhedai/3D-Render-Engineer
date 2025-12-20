#pragma once
#include "light.h"
#include "../object.h"
#include "../../../include/imgui/imgui.h"

class PointLight : public Light, public Object {
public:
    PointLight();
    ~PointLight();

    // 环境光的设置与获取
    void setAmbient(const glm::vec3& amb) { mAmbient = amb; }
    glm::vec3& getAmbient() { return mAmbient; }

    // 漫反射光的设置与获取
    void setDiffuse(const glm::vec3& diff) { mDiffuse = diff; }
    glm::vec3& getDiffuse() { return mDiffuse; }

    // 镜面反射光的设置与获取
    void setSpecular(const glm::vec3& spec) { mSpecular = spec; }
    glm::vec3& getSpecular() { return mSpecular; }

    // 衰减系数的设置与获取（常数衰减）
    void setKc(const float& kc) { mKc = kc; }
    float& getKc() { return mKc; }

    // 衰减系数的设置与获取（线性衰减）
    void setK1(const float& k1) { mK1 = k1; }
    float& getK1() { return mK1; }

    // 衰减系数的设置与获取（二次衰减）
    void setK2(const float& k2) { mK2 = k2; }
    float& getK2() { return mK2; }


public:
    // 光属性成员变量
    glm::vec3 mAmbient{ 1.0f };     // 环境光颜色
    glm::vec3 mDiffuse{ 1.0f };     // 漫反射光颜色
    glm::vec3 mSpecular{ 1.0f };    // 镜面反射光颜色

    // 衰减系数（原有成员）
    float mKc{ 1.0f };  // 常数衰减
    float mK1{ 0.09f };   // 线性衰减
    float mK2{ 0.032f }; // 二次衰减

};