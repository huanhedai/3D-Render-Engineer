#pragma once
//#ifndef CAMERA_CONTROL_H
//#define CAMERA_CONTROL_H

#include <unordered_map>
#include "core.h"
#include "camera.h"

class CameraControl {
public:
    CameraControl();
    ~CameraControl();

    void setCamera(Camera* camera) { mCamera = camera; }
    void setSensitivity(float sensitivity) { mSensitivity = sensitivity; }

    virtual void update();

    virtual void onMouse(int button, int state, float xpos, float ypos);    // 鼠标按键事件处理
    virtual void onCursor(float xpos, float ypos);  // 鼠标移动事件处理
    virtual void onKey(int key, int state, int mods);   // 键盘事件处理
    virtual void onScroll(float yoffset);     // 鼠标滚轮事件处理

protected:
    // 1 鼠标当前位置
    float mCurrentX = 0.0f, mCurrentY = 0.0f;

    bool mLeftMouseDown = false;
    bool mRightMouseDown = false;
    bool mMiddleMouseDown = false;

    std::unordered_map<int, bool> mKeyMap;

    float mSensitivity = 0.1f;

    Camera* mCamera = nullptr;

    // 6 记录相机缩放的速度
    float mScaleSpeed = 0.2f;
};

//#endif // CAMERA_CONTROL_H
