#include "gameCameraControl.h"
#include <glm/gtc/matrix_transform.hpp>

GameCameraControl::GameCameraControl() 
    : mSensitivity(0.1f), mSpeed(0.05f), mPitch(0.0f) {}

GameCameraControl::~GameCameraControl() {}

void GameCameraControl::onCursor(float xpos, float ypos) {
    float deltaX = (xpos - mCurrentX) * mSensitivity;
    float deltaY = (ypos - mCurrentY) * mSensitivity;

    if (mRightMouseDown && mCamera) {
        pitch(deltaY);
        yaw(deltaX);
    }
    
    // 更新鼠标位置
    mCurrentX = xpos;
    mCurrentY = ypos;
}


void GameCameraControl::update() {
    if (!mCamera) return;

    glm::vec3 direction(0.0f); 

    auto front = glm::cross(mCamera->mUp, mCamera->mRight);
    auto right = mCamera->mRight;

    if (mKeyMap[SDL_SCANCODE_W]) {
        direction += front;
    }

    if (mKeyMap[SDL_SCANCODE_S]) {
        direction -= front;
    }

    if (mKeyMap[SDL_SCANCODE_A]) {
        direction -= right;
    }

    if (mKeyMap[SDL_SCANCODE_D]) {
        direction += right;
    }

    if (glm::length(direction) > 0.0001f) {
        direction = glm::normalize(direction);
        mCamera->mPosition += direction * mSpeed;
    }
}

void GameCameraControl::pitch(float angle) {
    if (!mCamera) return;
    
    mPitch += angle;
    if (mPitch > 89.0f) {
        mPitch = -89.0f;
        return;
    }
    if (mPitch < -89.0f) {
        mPitch = +89.0f;
        return;
    }

    auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);
    mCamera->mUp = glm::vec3(mat * glm::vec4(mCamera->mUp, 0.0f));
}

void GameCameraControl::yaw(float angle) {
    if (!mCamera) return;
    
    // 应用旋转到相机上方向量和右方向量
    auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    mCamera->mUp = glm::vec3(mat * glm::vec4(mCamera->mUp, 0.0f));
    mCamera->mRight = glm::vec3(mat * glm::vec4(mCamera->mRight, 0.0f));
}
    