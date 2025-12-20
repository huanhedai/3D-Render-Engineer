#include "Application.h"
#include "glad/glad.h"
#include <iostream>

// 简单的渲染函数，绘制一个彩色背景
void render() {
    // 清屏颜色随时间变化
    static float time = 0.0f;
    time += 0.01f;

    float r = (sin(time) + 1.0f) / 2.0f;
    float g = (sin(time + 2.0f) + 1.0f) / 2.0f;
    float b = (sin(time + 4.0f) + 1.0f) / 2.0f;

    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// 窗口大小改变回调
void onWindowResized(int width, int height) {
    glViewport(0, 0, width, height);
}

// 键盘事件回调
void onKeyEvent(int key, int scancode, int action, int mods) {
    // 按下ESC键退出程序
    if (key == SDLK_ESCAPE && action == SDL_KEYDOWN) {
        Application::getInstance()->destroy();
    }
}

int main(int argc, char* argv[]) {
    // 获取Application单例
    Application* app = Application::getInstance();

    // 初始化应用程序，创建800x600的窗口
    if (!app->init(800, 600)) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }

    // 设置回调函数
    app->setResizeCallback(onWindowResized);
    app->setKeyBoardCallback(onKeyEvent);

    // 设置视口
    glViewport(0, 0, 800, 600);

    // 主循环
    std::cout << "Application started. Press ESC to exit." << std::endl;
    while (app->update()) {
        // 渲染
        render();
    }

    // 清理资源
    app->destroy();
    return 0;
}
