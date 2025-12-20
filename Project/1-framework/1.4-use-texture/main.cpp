#include "Application/Application.h"
#include "glframework/Shader.h"
#include "glframework/Geometry.h"
#include "glframework/Texture.h"
#include "wrapper/checkError.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 全局变量
Shader* gShader = nullptr;
Geometry* gCanvas = nullptr;
Texture* gTexture = nullptr;
glm::mat4 gProjection;

// 窗口大小变化回调：同步更新投影矩阵和画布大小
void onResize(int width, int height) {
    glViewport(0, 0, width, height);    // 同步更新画布大小为窗口大小
    // 更新正交投影矩阵（适配新窗口尺寸）
    gProjection = glm::ortho(0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);

    // 销毁旧画布并创建新画布（匹配新窗口尺寸）
    if (gCanvas) {
        delete gCanvas;
    }
    gCanvas = Geometry::createCanvas(width, height);
}

// 初始化资源
bool initResources() {
    // 创建着色器
    gShader = new Shader("E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/Canvas/vertex.glsl",
        "E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/Canvas/fragment.glsl");
    if (!gShader) return false;

    // 加载纹理（替换为实际图片路径）
    gTexture = new Texture("E:/IT-Furnace/OpenGl/mindray/Framework/resource/textures/R-D.jpg", 0); // 使用纹理单元0
    if (!gTexture) return false;

    // 初始化画布和投影矩阵（使用初始窗口大小）
    auto app = Application::getInstance();
    onResize(app->getWidth(), app->getHeight());

    // 设置OpenGL状态（启用纹理和混合）
    GL_CALL(glViewport(0, 0, app->getWidth(), app->getHeight()));	// 设置OpenGL视口的大小为窗体的大小)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);   // 设置设置清屏颜色
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

// 释放资源
void releaseResources() {
    delete gTexture;
    delete gCanvas;  // 确保画布资源被释放
    delete gShader;
}

// 渲染函数
void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    gShader->begin();
    // 传递投影矩阵和纹理单元
    gShader->setMatrix4x4("projection", gProjection);
    gShader->setInt("uTexture", 0);

    // 绑定纹理并绘制画布
    gTexture->bind();
    GL_CALL(glBindVertexArray(gCanvas->getVAO()));
    GL_CALL(glDrawElements(GL_TRIANGLES, gCanvas->getIndicesCount(), GL_UNSIGNED_INT, 0));
    GL_CALL(glBindVertexArray(0));

    gShader->end();
}

int main(int argc, char* argv[]) {
    auto app = Application::getInstance();
    if (!app->init(800, 600, "Texture Display (Resizable)")) {
        return -1;
    }

    // 设置窗口大小变化回调
    app->setResizeCallback(onResize);

    if (!initResources()) {
        app->destroy();
        return -1;
    }

    // 主循环
    while (app->update()) {
        render();
    }

    releaseResources();
    app->destroy();
    return 0;
}