#pragma once
#include "material.h"
#include "../texture.h"

class PhongMaterial : public Material {
public:
    PhongMaterial();
    ~PhongMaterial();

    // 光泽度设置与获取（对应 mShiness）
    void setShiness(float shiness) {
        mShiness = shiness;
    }
    const float& getShiness() const {
        return mShiness;
    }

    // Blinn-Phong模型开关设置与获取（对应 mBlinn）
    void setBlinn(bool blinn = GL_TRUE) {  // 参数名改为小写，符合命名规范
        mBlinn = blinn;
    }
    const bool& getBlinn() const {
        return mBlinn;
    }

    // 环境光反射系数（颜色向量）设置与获取（对应 mAmbientColor）
    void setAmbientColor(const glm::vec3& ambient) {  // 函数名添加 Color，与变量对应
        mAmbientColor = ambient;
    }
    const glm::vec3& getAmbientColor() const {
        return mAmbientColor;
    }

    // 漫反射基础色（颜色向量）设置与获取（对应 mDiffuseColor）
    void setDiffuseColor(const glm::vec3& diffuse) {  // 函数名添加 Color，与变量对应
        mDiffuseColor = diffuse;
    }
    const glm::vec3& getDiffuseColor() const {
        return mDiffuseColor;
    }

    // 镜面反射基础色（颜色向量）设置与获取（对应 mSpecularColor）
    void setSpecularColor(const glm::vec3& specular) {  // 函数名添加 Color，与变量对应
        mSpecularColor = specular;
    }
    const glm::vec3& getSpecularColor() const {
        return mSpecularColor;
    }

    // 漫反射纹理设置与获取（对应 public 成员 mDiffuse）
    void setDiffuseTexture(Texture* diffuseTexture) {  // 新增纹理操作函数，明确是纹理
        mDiffuse = diffuseTexture;
    }
    Texture* getDiffuseTexture() const {
        return mDiffuse;
    }

    // 高光蒙版纹理设置与获取（对应 public 成员 mSpecularMask）
    void setSpecularMask(Texture* specularMask) {  // 新增蒙版纹理操作函数
        mSpecularMask = specularMask;
    }
    Texture* getSpecularMask() const {
        return mSpecularMask;
    }

public:
    Texture* mDiffuse{ nullptr };   // 漫反射纹理：采样结果作为漫反射基础色，优先级高于 mDiffuseColor
    Texture* mSpecularMask{ nullptr };  // 高光蒙版纹理：用R通道控制局部镜面反射强度

private:
    float mShiness{ 100.0f };      // 镜面高光的锐利程度（值越大，光斑越小、越集中）
    bool mBlinn{ GL_FALSE };        // 是否使用Blinn-Phong光照模型

    // 材质属性（颜色向量）
    glm::vec3 mAmbientColor{ 1.0f };     // 材质对环境光的反射系数
    glm::vec3 mDiffuseColor{ 1.0f };     // 材质对漫反射光的基础色（无纹理时生效）
    glm::vec3 mSpecularColor{ 1.0f };    // 材质镜面反射的基础强度/颜色

};