#pragma once

#include"cameraControl.h"

class TrackBallCameraControl :public CameraControl {
public:
	TrackBallCameraControl();
	~TrackBallCameraControl();

	// 父类当中的接口函数是否需要重写呢？
	void onCursor(float xpos, float ypos) override;
	void setMoveSpeed(float speed);
	void onScroll(float offset) override;
private:
	void pitch(float angle);
	void yaw(float angle);

private:
	float mMoveSpeed = 0.01f;
};