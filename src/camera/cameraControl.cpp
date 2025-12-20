#include "camera/cameraControl.h"
#include <iostream>


CameraControl::CameraControl() {
}

CameraControl::~CameraControl() {
}

// 实现update()函数
void CameraControl::update() {}

void CameraControl::onMouse(int button, int state, float xpos, float ypos) {
   
    bool pressed = (state == SDL_PRESSED);

    if (pressed) {
        mCurrentX = xpos;
        mCurrentY = ypos;
    }

     switch (button) {
     case SDL_BUTTON_LEFT:
         mLeftMouseDown = pressed;
         break;
     case SDL_BUTTON_RIGHT:
         mRightMouseDown = pressed;
         break;
     case SDL_BUTTON_MIDDLE:
         mMiddleMouseDown = pressed;
         break;
     }
}

void CameraControl::onCursor(float xpos, float ypos) {
    // 鼠标移动处理逻辑
}

void CameraControl::onKey(int key, int state, int mods) {
    bool pressed = (state == SDL_PRESSED);
    mKeyMap[key] = pressed;
}

void CameraControl::onScroll(float yoffset) {
    // 鼠标滚轮处理逻辑
}
