#include "Application.h"
#include "camera.h"
#include "othographicCamera.h"
#include "perspectiveCamera.h"
#include "shader.h"
#include "geometry.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 相机类型枚举
enum class CameraType {
    Perspective,
    Orthographic
};

int main(int argc, char* argv[]) {
    // 初始化应用程序
    Application* app = Application::getInstance();
    if (!app->init(1280, 720, "Projection Camera with Cube Example")) {
        return -1;
    }

    // 设置窗口大小调整回调
    int windowWidth = app->getWidth();
    int windowHeight = app->getHeight();
    float aspectRatio = static_cast<float>(windowWidth) / windowHeight;

    // 创建两种投影相机
    perspectiveCamera perspCam(60.0f, aspectRatio, 0.1f, 1000.0f);
    orthographicCamera orthoCam(-5.0f, 5.0f, 3.75f, -3.75f, 0.1f, 1000.0f);
    Camera* activeCamera = &perspCam;
    CameraType currentType = CameraType::Perspective;

    // 创建着色器和立方体几何体
    Shader shader("E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/camera/vertex.glsl",
                "E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/camera/fragment.glsl");
    Geometry* cube = Geometry::createBox(1.0f);  // 创建边长为1的立方体

    // 设置键盘回调
    app->setKeyBoardCallback([&](int key, int scancode, int action, int mods) {
        if (key == SDLK_c && action == 1) {
            currentType = (currentType == CameraType::Perspective) ?
                CameraType::Orthographic : CameraType::Perspective;
            activeCamera = (currentType == CameraType::Perspective) ?
                static_cast<Camera*>(&perspCam) : static_cast<Camera*>(&orthoCam);
        }
        // 相机缩放控制
        if (key == SDLK_UP && action == 1) {
            activeCamera->scale(1.1f);  // 放大
        }
        if (key == SDLK_DOWN && action == 1) {
            activeCamera->scale(0.9f);  // 缩小
        }
        if (key == SDLK_ESCAPE && action == 1) {
            app->quit();
        }
        });

    // 设置窗口大小调整回调
    app->setResizeCallback([&](int newWidth, int newHeight) {
        glViewport(0, 0, newWidth, newHeight);
        float newAspect = static_cast<float>(newWidth) / newHeight;
        perspCam.setAspect(newAspect);
        float orthoSize = 5.0f * newAspect;
        orthoCam = orthographicCamera(-orthoSize, orthoSize, newAspect * 3.75f, -newAspect * 3.75f, 0.1f, 1000.0f);
        });

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 渲染循环
    while (app->update()) {
        // 清屏（颜色缓冲和深度缓冲）
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        // 使用着色器并设置相机矩阵
        shader.begin();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)(SDL_GetTicks() * 0.0001f), glm::vec3(0.5f, 1.0f, 0.0f));  // 立方体旋转动画
        shader.setMatrix4x4("model", model);
        shader.setMatrix4x4("view", activeCamera->getViewMatrix());
        shader.setMatrix4x4("projection", activeCamera->getProjectionMatrix());

        // 绘制立方体
        glBindVertexArray(cube->getVAO());
        glDrawElements(GL_TRIANGLES, cube->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        shader.end();
    }

    // 释放资源
    delete cube;  // 释放几何体资源
    app->destroy();
    return 0;
}