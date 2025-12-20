#include"perspectiveCamera.h"

perspectiveCamera::perspectiveCamera(float fovy,
    float aspect,
    float near,
    float far) {
    mFovy = fovy;
    mAspect = aspect;
    mNear = near;
    mFar = far;
}

perspectiveCamera::~perspectiveCamera() {

}

glm::mat4 perspectiveCamera::getProjectionMatrix() {
    return glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar);
}

void perspectiveCamera::scale(float deltScale) {
    auto front = glm::cross(mUp, mRight);
    mPosition += (front * deltScale);
}

