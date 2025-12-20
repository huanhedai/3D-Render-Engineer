#include"camera.h"

Camera::Camera() {

}
Camera::~Camera() {

}

glm::mat4 Camera::getViewMatrix() {
	// lookat
	// -eye: 相机位置（使用mPosition）
	// -center: 看向世界的哪个点
	// -top: 穹顶（使用mUp替代）

	glm::vec3 front = glm::cross(mUp, mRight);		// front指向相机坐标系的负 z 轴
	glm::vec3 center = mPosition + front;			// center 代表负z轴上的某一点

	return glm::lookAt(mPosition, center, mUp);
}


glm::mat4 Camera::getProjectionMatrix() {
	return glm::identity<glm::mat4>();
}

void Camera::scale(float deltScale) {

 }


void Camera::rotateAroundAxis(
    const glm::vec3& rotationAxis,
    float speed,
    float radius,
    const glm::vec3& initialPos,
    float halfAngle
) {
    // 1. 参数合法性校验
    if (halfAngle <= 0.0f || halfAngle > glm::pi<float>()) {
        throw std::invalid_argument("半角必须在(0, π]范围内（总范围≤2π）");
    }
    glm::vec3 axis = glm::normalize(rotationAxis);
    if (glm::length(axis) < 0.0001f) {
        throw std::invalid_argument("旋转轴不能为零向量");
    }
    if (glm::length(initialPos) < 0.0001f) {
        throw std::invalid_argument("中间点不能为原点");
    }

    // 2. 处理中间点：确保在旋转平面内（垂直于旋转轴）并符合半径
    float dotMidAxis = glm::dot(initialPos, axis); // 中间点在旋转轴上的分量
    glm::vec3 midInPlane = initialPos - dotMidAxis * axis; // 投影到旋转平面（去除轴分量）
    if (glm::length(midInPlane) < 0.0001f) {
        throw std::invalid_argument("中间点不能与旋转轴共线（必须在旋转平面内）");
    }
    glm::vec3 midPosScaled = glm::normalize(midInPlane) * radius; // 缩放到指定半径

    // 3. 计算旋转角度：在[-halfAngle, +halfAngle]范围内往复
    float time = SDL_GetTicks() * 0.001f;
    float cycle = 2.0f * glm::pi<float>(); // 周期（0→2π为一个完整循环）
    float t = fmod(time * speed, cycle);   // 将时间映射到[0, 2π)

    float angle;
    if (t <= glm::pi<float>()) {
        // 前半周期（0→π）：从- halfAngle 线性增加到 + halfAngle
        angle = -halfAngle + (2.0f * halfAngle / glm::pi<float>()) * t;
    }
    else {
        // 后半周期（π→2π）：从+ halfAngle 线性减少到 - halfAngle
        angle = halfAngle - (2.0f * halfAngle / glm::pi<float>()) * (t - glm::pi<float>());
    }

    // 4. 计算当前相机位置：以中间点为基准，绕旋转轴旋转angle角
    glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), angle, axis); // 轴角旋转矩阵
    mPosition = glm::vec3(rotationMat * glm::vec4(midPosScaled, 1.0f)); // 应用旋转

    // 5. 更新前向向量（指向原点）和右向量（保持正交）
    glm::vec3 forward = glm::normalize(glm::vec3(0.0f) - mPosition);
    mRight = glm::normalize(glm::cross(forward, mUp));
}