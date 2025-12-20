//#define SDL_MAIN_HANDLED  // 使用SDL2，Windows会重新定义入口函数，或者使用 int main(int argc, char* argv[]) 这样的main函数头
#include "core.h" 
#include "Application.h"
#include "checkError.h"
#include "geometry.h"
#include "Texture.h"
#include "shader.h"

#include "../../include/camera/cameraType/othographicCamera.h"
#include "../../include/camera/cameraType/perspectiveCamera.h"
#include "../../include/camera/cameraControl/gameCameraControl.h"
#include "../../include/camera/cameraControl/trackBallCameraControl.h"
#include "../../include/camera/cameraControl.h"

#include <SDL2/SDL_main.h>
#include <iostream>
/*
* 实验相机系统
*
* 将添加事件回调函数，创建顶点数据，shader program，相机矩阵等操作封装
*
*/


// 平行光：参数（方向，光强）
glm::vec3 lightDirection = glm::vec3(-1.0f, 0.0f, -1.0f);
glm::vec3 lightColor = glm::vec3(0.9f, 0.85f, 0.75f);

// specular
float specularIntensity = 0.5f;

// ambient 环境光 --- 照亮死黑
glm::vec3 ambientColor = glm::vec3(0.2f);

// 在类中或全局范围定义变量（用于记录上一帧时间）
Uint32 lastTime = SDL_GetTicks();
const float rotationSpeed = glm::radians(45.0f); // 匀速旋转速度（每秒旋转45度，转为弧度）

Geometry* geometry = nullptr;
Geometry* geometry2 = nullptr;
Texture* texture = nullptr;
Shader* shader = nullptr;
glm::mat4 ModelMatrix(1.0f);

//orthographicCamera* camera = nullptr;
perspectiveCamera* camera = nullptr;

//CameraControl* cameraControl = nullptr;
//GameCameraControl* cameraControl = nullptr;
TrackBallCameraControl* cameraControl = nullptr;

void prepareCamera() {
    float size = 6.0f; // 视图盒子大小
    //camera = new orthographicCamera(-size, size, -size, size, size, -size);

    camera = new perspectiveCamera(
        60.0f,
        (float)App->getWidth() / (float)App->getHeight(),
        0.1f,
        1000.0f);

    //cameraControl = new GameCameraControl();
    cameraControl = new TrackBallCameraControl();

    cameraControl->setCamera(camera);
    cameraControl->setSensitivity(0.4f);
    //cameraControl->setMoveSpeed(0.05f);
}
void onKeyboard(int scancode, int sym, int state, int mod) {
    /*
    在Application中四个参数的传入如下

    event.key.keysym.scancode, // 扫描码   SDL physical key code   WASD移动一般使用物理键码
    event.key.keysym.sym,    // 键码  SDL virtual key code    虚拟键码适合有文本输入的场景, Esc也适合
    event.key.state,         // 状态(按下/释放)
    event.key.keysym.mod     // 修饰键
    */
    /*
    退出窗口是一个 “功能操作”，而非 “物理位置操作”。
    虚拟键码能统一标识按键的语义功能，不受键盘布局或物理位置影响。例如：
    SDLK_ESCAPE 始终代表 “退出 / 取消” 功能，无论 Esc 键在键盘的左上角还是右上角；
    */
    if (state == SDL_PRESSED) {
        if (sym == SDLK_ESCAPE)
        {
            Application::getInstance()->quit();
        }
        else if (scancode == SDL_SCANCODE_W) {
            std::cout << "按下了W键" << std::endl;
        }
        else if (scancode == SDL_SCANCODE_A) {
            std::cout << "按下了A键" << std::endl;
        }
        else if (scancode == SDL_SCANCODE_S) {
            std::cout << "按下了S键" << std::endl;
        }
        else if (scancode == SDL_SCANCODE_D) {
            std::cout << "按下了D键" << std::endl;
        }
        else if (sym == SDLK_ESCAPE) {
            std::cout << "按下了Esc键，即将退出" << std::endl;
            Application::getInstance()->quit();
        }
    }
    cameraControl->onKey(scancode, state, mod);
}


// 鼠标按下/抬起
void OnMouseButton(int button, int state, int x, int y, int mod) {
    App->getCursorPosition(&x, &y);
    cameraControl->onMouse(button, state, x, y);
}

// 鼠标光标移动
void OnMouseMotion(int xpos, int ypos, int dx, int dy, int buttonState) {
    cameraControl->onCursor(xpos, ypos);
}

// 鼠标滚轮
void OnMouseWheel(int scrollX, int scrollY) {
    cameraControl->onScroll(scrollY);
}

void onResize(int width, int height) {
    /*
    在Application两个参数的传入如下

    mWidth = event.window.data1;
    mHeight = event.window.data2;
    */

    // 按窗口当前的大小设置画布尺寸
    glViewport(0, 0, width, height);
}

void prepareEventCallback() {
    App->setKeyBoardCallback(onKeyboard);
    App->setResizeCallback(onResize);
    App->setMouseButtonCallback(OnMouseButton);
    App->setMouseMotionCallback(OnMouseMotion);
    App->setMouseWheelCallback(OnMouseWheel);
}

void prepareShader() {
    std::string vertexshaderPath = std::string(SHADER_DIR) + "/3.1-set-light/3_vertex_specular.glsl";
    std::string fragmentshaderPath = std::string(SHADER_DIR) + "/3.1-set-light/4_fragment_ambient.glsl";
    shader = new Shader(vertexshaderPath.c_str(), fragmentshaderPath.c_str());
}

void prepareVAO() {
     //geometry2 = Geometry::createBox(1.0f);// 这里 createBox 是静态成员函数，不能用new
    //geometry = Geometry::createPlane(2.0f,1.0f);
    //geometry = Geometry::createSphere(3.0f); 
    
    try {
        std::string objPath = std::string(OBJ_DIR) +"/backpack//backpack.obj";
        geometry = Geometry::createFromOBJ(objPath.c_str());
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl; // 打印具体错误原因
    }
}
void prepareTexture() {
    std::string texturePath = std::string(TEXTURE_DIR) + "/world.png";
    //std::string texturePath = std::string(TEXTURE_DIR) + "/R-D.jpg";
    //std::string texturePath = std::string(OBJ_DIR) + "/rock/diffuso.tif";

    texture = new Texture(texturePath.c_str(), 0);   // 绑定0号纹理单元
}


void prepareState() {
    // 开启深度缓存
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void render() {
    // 1. 计算当前帧与上一帧的时间差（deltaTime，单位：秒）
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f; // 转换为秒
    lastTime = currentTime;

    // 2. 计算当前帧应旋转的角度（速度 × 时间差 = 匀速角度）
    // 无论帧率高低，单位时间内旋转的角度相同（例如每秒45度）
    float angle = rotationSpeed * deltaTime;

    // 3. 应用旋转（旋转轴保持不变，角度匀速变化）
    // 注意：旋转轴建议归一化（可选，GLM会自动处理，但显式归一化更规范）
    glm::vec3 rotationAxis = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    ModelMatrix = glm::rotate(ModelMatrix, 0.0f, rotationAxis);


    // 执行opengl画布清理操作
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));	// 每一帧都清理 GL_DEPTH_BUFFER_BIT深度缓冲区
    // 渲染操作

    // 1 绑定当前的program
    shader->begin();
    shader->setInt("sampler", 0);	// 绑定采样器到纹理单元0，因为前面激活了0号

    // 光源参数的uniform参数
    shader->setVector3("lightDirection", lightDirection);
    shader->setVector3("lightColor", lightColor);
    shader->setVector3("cameraPosition", camera->mPosition);
    shader->setFloat("specularIntensity", specularIntensity);
    shader->setVector3("ambientColor", ambientColor);

    shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
    shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
    shader->setMatrix4x4("ModelMatrix", ModelMatrix);	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
    // 计算normalMatrix
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));
    shader->setMatrix3x3("normalMatrix", normalMatrix);

    // 2 绑定当前的vao
    glBindVertexArray(geometry->getVAO());
    // 3 发出绘制指令
    glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);// 需要绘制geometry->getIndicesCount()个索引


    // 2 绑定当前的vao
    //glBindVertexArray(geometry2->getVAO());
    // 3 发出绘制指令
    //glDrawElements(GL_TRIANGLES, geometry2->getIndicesCount(), GL_UNSIGNED_INT, 0);// 需要绘制geometry->getIndicesCount()个索引

    glBindVertexArray(0);

    shader->end();
}

int main(int argc, char* argv[]) {
    if (!App->init(800, 600)) {	// 初始化Application类，创建窗体
        std::cout << "Application init failed!" << std::endl;
        return -1;
    }

    // 设置opengl视口以及清理颜色
    GL_CALL(glViewport(0, 0, App->getWidth(), App->getHeight()));	// 设置OpenGL视口的大小为窗体的大小)
    GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));	// 设置清屏颜色，RGBA四个分量，范围是[0.0, 1.0]，每个分量的值越大，颜色越亮

    prepareEventCallback();
    prepareShader();
    prepareVAO();
    prepareTexture();
    prepareCamera();
    prepareState();

    while (App->update()) {
        cameraControl->update();
        render();
    }

    delete geometry;
    App->destroy();
    return 0;
}