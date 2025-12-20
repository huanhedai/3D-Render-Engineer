#include"othographicCamera.h"

orthographicCamera::orthographicCamera(float l, float r, float y, float b, float n, float f) {
     mLeft = l;
     mRight = r;
     mTop = y;
     mBottom = b;
     mNear = n;
     mFar = f;
}

orthographicCamera::~orthographicCamera() {

}

glm::mat4 orthographicCamera::getProjectionMatrix() {
    float scale = std::pow(2.0f, mScale);
    return glm::ortho(mLeft * scale, mRight * scale, mTop * scale, mBottom * scale, mNear, mFar);   // 不需要修改near和far
}

void orthographicCamera::scale(float deltScale) {
    mScale += deltScale;
}