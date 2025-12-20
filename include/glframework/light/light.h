#pragma once
#include "../core.h"

class Light {
public:
    Light();
    ~Light();
	void setColor(const glm::vec3& color) { mColor = color; }
	void setColor(float r, float g, float b) { mColor = glm::vec3(r, g, b); }
	void setSpecularIntensity(float intensity) { mSpecularIntensity = intensity; }
	void setIntensity(float intensity) { mIntensity = intensity; }

	glm::vec3 getColor() const { return mColor; }
	float getIntensity() const { return mIntensity; }
	float getSpecularIntensity() const { return mSpecularIntensity; }

protected:
    glm::vec3    mColor{ 1.0f };        // 光的颜色
	float        mIntensity{ 1.0f };    // 光的强度
    float        mSpecularIntensity{ 1.0f };    // 镜面反射光的强度
};