#include "Application.h"
#include "Geometry.h"
#include "Shader.h"
#include "checkError.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// 全局变量：渲染相关（便于回调访问，实际项目可封装为Render类）
Shader* g_shader = nullptr;
Geometry* g_triangle = nullptr;
glm::mat4 g_projection; // 投影矩阵（窗口大小变化时更新）

// 窗口大小变化回调：更新投影矩阵，避免三角形拉伸
void onWindowResized(int width, int height) {
    glViewport(0, 0, width, height);    // 设置opengl执行渲染的区域大小
    //std::cout << "当前的窗体大小：" << width << ", " << height << std::endl;
}

// 键盘事件回调：ESC键退出程序
void onKeyEvent(int key, int scancode, int action, int mods) {
    if (key == SDLK_ESCAPE && action == 1) {
        Application::getInstance()->quit();
    }
    std::cout << "cao zuo jian pan" << std::endl;
}

// 渲染函数：绘制三角形
void render() {
    // 1. 清屏（颜色缓冲+深度缓冲，避免残留）
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 深灰背景
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); // 启用深度测试（虽三角形简单，规范起见添加）

    // 2. 激活Shader并设置Uniform（MVP矩阵）
    g_shader->begin();
    // 模型矩阵：单位矩阵（三角形不旋转、不缩放、不平移）
    glm::mat4 model = glm::mat4(1.0f);
    // 视图矩阵：相机在(0,0,5)，看向原点（Z轴负方向）
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    // 传递MVP矩阵到Shader
    g_shader->setMatrix4x4("model", model);
    g_shader->setMatrix4x4("view", view);
    g_shader->setMatrix4x4("projection", g_projection);

    // 3. 绑定三角形VAO并绘制（用glDrawArrays，因无EBO）
    glBindVertexArray(g_triangle->getVAO());
    // 注意：原Geometry类用glDrawElements，此处三角形无EBO，改用glDrawArrays
    glDrawArrays(
        GL_TRIANGLES,  // 绘制类型：三角形
        0,             // 起始顶点索引
        3              // 顶点总数（三角形3个顶点）
    );

    // 4. 解绑资源（避免后续污染）
    glBindVertexArray(0);
    g_shader->end();
}

int main(int argc, char* argv[]) {
    // 1. 初始化SDL Application（创建窗口+OpenGL上下文）
    Application* app = Application::getInstance();
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    if (!app->init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        std::cerr << "SDL Application初始化失败！" << std::endl;
        return -1;
    }

    // 2. 设置回调函数（窗口大小变化+键盘退出）
    app->setResizeCallback(onWindowResized);
    app->setKeyBoardCallback(onKeyEvent);


    // 3. 创建Shader（需在SDL上下文创建后！）
    g_shader = new Shader("E:\\IT-Furnace\\OpenGl\\mindray\\Framework\\resource\\shaders\\triangle\\vertex.glsl", "E:\\IT-Furnace\\OpenGl\\mindray\\Framework\\resource\\shaders\\triangle\\fragment.glsl");
    // 4. 创建三角形几何体
    g_triangle = Geometry::createTriangle();

    // 5. 初始化投影矩阵（初始窗口大小）
    // 正交投影矩阵（适合2D/简单3D，避免透视变形，三角形比例一致）
    g_projection = glm::ortho(
        -1.0f, (float)app->getWidth() / app->getHeight(),  // 左、右（适配窗口宽高比）
        -1.0f, 1.0f,                 // 下、上
        0.1f, 100.0f                 // 近、远裁面
    );

    // 6. 主循环（SDL事件处理+渲染）
    std::cout << "程序运行中，按ESC键退出..." << std::endl;
    while (app->update()) {
        render(); // 每帧绘制三角形
    }

    // 7. 释放资源（顺序：Shader→Geometry→Application，避免内存泄漏）
    delete g_shader;
    delete g_triangle;
    app->destroy();

    return 0;
}