#pragma once
#ifndef GAME_CAMERA_CONTROL_H
#define GAME_CAMERA_CONTROL_H

#include "cameraControl.h"
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_keyboard.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include "camera.h" 

class GameCameraControl : public CameraControl {
public:
    GameCameraControl();
    ~GameCameraControl();

    void onCursor(float xpos, float ypos) override;

    void update() override;

    void setSpeed(float speed) { mSpeed = speed; }

private:
    void pitch(float angle);

    void yaw(float angle);

private:
    float mSensitivity;

    float mSpeed;

    float mPitch;
};

#endif // GAME_CAMERA_CONTROL_H

