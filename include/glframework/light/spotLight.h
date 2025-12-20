#pragma once

#include "light.h"
#include "../object.h"	// 聚光灯也有位置信息，需要用Object承载

class SpotLight :public Light, public Object {
public:
	SpotLight();
	~SpotLight();
	glm::vec3 getTargetDirection() { return mTargetDirection; }
	void setTargetDirection(glm::vec3& TargetDirection) {mTargetDirection = TargetDirection;}

	float getInnerAngle() { return mInnerAngle; }
	void setInnerAngle(float VisibleAngle) { mInnerAngle = VisibleAngle; }

	float getOuterAngle() { return mOuterAngle; }
	void setOuterAngle(float VisibleAngle) { mOuterAngle = VisibleAngle; }
private:
	glm::vec3 mTargetDirection{ -1.0f };	// 聚光灯中心朝向
	float mInnerAngle{ 30.0f };
	float mOuterAngle{ 50.0f };

public:
	float mKc{ 1.0f };  // 常数衰减
	float mK1{ 0.09f };   // 线性衰减
	float mK2{ 0.032f }; // 二次衰减
};