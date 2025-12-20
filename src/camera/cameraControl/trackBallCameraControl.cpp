#include"trackBallCameraControl.h"

TrackBallCameraControl::TrackBallCameraControl() {

}
TrackBallCameraControl::~TrackBallCameraControl() {

}

// 父类当中的接口函数是否需要重写呢？
void TrackBallCameraControl::onCursor(float xpos, float ypos) {
	if (mLeftMouseDown) {
		float deltaX = (xpos - mCurrentX) * mSensitivity;
		float deltaY = (ypos - mCurrentY) * mSensitivity;

		// 2 分开pitch 跟 yaw 各自计算
		pitch(-deltaY);
		yaw(-deltaX);
	}
	else if (mMiddleMouseDown) {
		float deltaX = (xpos - mCurrentX) * mMoveSpeed;
		float deltaY = (ypos - mCurrentY) * mMoveSpeed;

		mCamera->mPosition -= mCamera->mRight * deltaX;
		mCamera->mPosition += mCamera->mUp * deltaY;
	}
	//更新鼠标位置
	mCurrentX = xpos;
	mCurrentY = ypos;
}

void TrackBallCameraControl::pitch(float angle) {
	// 绕着相机的mRight旋转
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);
	// 影响当前相机的up向量和位置position
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);	// vec4给到vec3，给了xyz
	mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
	// 上诉是一种增量的变化，每次都保存最新的up和position
}

void TrackBallCameraControl::yaw(float angle) {
	// 绕着世界坐标系的y旋转
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f,1.0f,0.0f));
	// 影响当前相机的up, right向量和位置position
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);	// vec4给到vec3，给了xyz
	mCamera->mRight = mat * glm::vec4(mCamera->mRight, 0.0f);
	mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
}

void TrackBallCameraControl::setMoveSpeed(float speed) {
	mMoveSpeed = speed;
}

void TrackBallCameraControl::onScroll(float offset) {
	mCamera->scale(mScaleSpeed * offset);
}