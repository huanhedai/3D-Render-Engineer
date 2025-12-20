#pragma once

#include"camera.h"

class orthographicCamera :public Camera {
public:
    orthographicCamera(float l = 0.0f,
        float r = 0.0f,
        float y = 0.0f,
        float b = 0.0f,
        float n = 0.0f,
        float f = 0.0f);
    ~orthographicCamera();
    glm::mat4 getProjectionMatrix() override;
    void scale(float deltScale) override;

private:    // 一次性赋值之后，就不需要再更改了
    float mLeft = 0.0f;
    float mRight = 0.0f;
    float mTop = 0.0f;
    float mBottom = 0.0f;

    float mScale{ 0.0f };   // 记录累加/累减的结果
};