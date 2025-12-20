#pragma once

#include"../glframework/core.h"


class Camera {
public:
	Camera();
	~Camera();

	glm::mat4 getViewMatrix();
	virtual glm::mat4 getProjectionMatrix();		// 子类会重写虚函数
	virtual void scale(float deltScale);

	// 核心函数：从中间点-halfAngle → 中间点+halfAngle → 往复循环
		// 参数说明：
		// - rotationAxis：旋转轴（如(0,1,0)）
		// - speed：旋转速度（值越大越快，周期 = 2π / speed）
		// - radius：旋转半径（相机到原点的距离）
		// - initialPos：旋转区域的中间点（基准位置）
		// - halfAngle：半角（弧度，总旋转范围为2×halfAngle，即从- halfAngle到+halfAngle）
	void rotateAroundAxis(
		const glm::vec3& rotationAxis,
		float speed,
		float radius,
		const glm::vec3& initialPos,
		float halfAngle
	);
public:
	/*
	mPosition：相机位置

	mUp和mRight的位置必须垂直
	*/
	glm::vec3 mPosition{ 0.0f,0.0f,5.0f };
	glm::vec3 mUp{ 0.0f,1.0f,0.0f };	// 相机的顶部朝向
	glm::vec3 mRight{ 1.0f,0.0f,0.0f };	// 相机镜头的右边，注意它必须与mUp相互垂直

	float mNear = 0.0f;
	float mFar = 0.0f;
};