#pragma once

#include"camera.h"

class perspectiveCamera :public Camera {
public:
    perspectiveCamera(float mFovy = 0.0f,
        float mAspect = 0.0f,
        float mNear = 0.0f,
        float mFar = 0.0f);
	~perspectiveCamera();

    glm::mat4 getProjectionMatrix() override;
    void scale(float deltScale) override;
    void setAspect(float Aspect) {
        mAspect = Aspect;
    }

private:    // 一次性赋值之后，就不需要再更改了
    float mFovy = 0.0f;
    float mAspect = 0.0f;
};