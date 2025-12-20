//#define SDL_MAIN_HANDLED  // 使用SDL2，Windows会重新定义入口函数，或者使用 int main(int argc, char* argv[]) 这样的main函数头
#include "core.h" 
#include "Application.h"
#include "checkError.h"
#include "geometry.h"
#include "Texture.h"
#include "shader.h"

#include "../../../include/camera/cameraType/othographicCamera.h"
#include "../../../include/camera/cameraType/perspectiveCamera.h"
#include "../../../include/camera/cameraControl/gameCameraControl.h"
#include "../../../include/camera/cameraControl/trackBallCameraControl.h"
#include "../../../include/camera/cameraControl.h"

#include <SDL2/SDL_main.h>
#include <iostream>

#include "../../../include/glframework/material/phongMaterial.h"
#include "../../../include/glframework/material/whiteMaterial.h"
#include "../../../include/glframework/mesh.h"
#include "../../../include/glframework/renderer/renderer.h"
#include "../../../include/glframework/light/spotLight.h"

#include "../../../include/imgui/imgui.h"
#include "../../../include//imgui/imgui_impl_sdl2.h"
#include "../../../include/imgui/imgui_impl_opengl3.h"


void onKeyboard(int scancode, int sym, int state, int mod);
void OnMouseButton(int button, int state, int x, int y, int mod);// 鼠标按下/抬起
void OnMouseMotion(int xpos, int ypos, int dx, int dy, int buttonState);// 鼠标光标移动
void OnMouseWheel(int scrollX, int scrollY);// 鼠标滚轮
void onResize(int width, int height);

void prepareEventCallback() {
    App->setKeyBoardCallback(onKeyboard);
    App->setResizeCallback(onResize);
    App->setMouseButtonCallback(OnMouseButton);
    App->setMouseMotionCallback(OnMouseMotion);
    App->setMouseWheelCallback(OnMouseWheel);
}

void lightTransform();
void lightCameraTransform();
void PrintVec3(glm::vec3& vec);


Renderer* renderer = nullptr;
std::vector<Mesh*> meshes{};
Mesh* meshWhite = nullptr;
SpotLight* spotLight = nullptr;
AmbientLight* ambLight = nullptr;

Camera* camera = nullptr;
CameraControl* cameraControl = nullptr;

glm::vec3 clearColor{};
void initIMGUI() {
    ImGui::CreateContext();// 创建imgui上下文
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark(); // 暗黑系

    // 设置ImGui与SDL和OpenGL绑定
    ImGui_ImplSDL2_InitForOpenGL(App->getWindow(), App->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 460");
}

void renderIMGUI() {        // 每一帧除了要渲染三维模型外，还要渲染一个IMGUI的UI层
    //1 开启当前的IMGUI渲染
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    //2 决定当前的GUI上面有哪些控件，从上到下
    ImGui::Begin("Hello, world!");
    ImGui::Text("ChangeColor Demo");
    ImGui::Button("Test Button", ImVec2(40, 20));
    ImGui::ColorEdit3("Clear Color", (float*)&clearColor);    //(float*) 将clearColor修改成一个float类型的数组
    ImGui::End();

    // 3 执行UI渲染
    ImGui::Render();
    // 获取当前窗体的宽高
    int display_w, display_h;
    SDL_GL_GetDrawableSize(App->getWindow(), &display_w, &display_h);

    // 重置OpenGL状态（关键：避免3D渲染影响UI）
    glViewport(0, 0, display_w, display_h); // 重置视口大小
    //glClear(GL_COLOR_BUFFER_BIT); // 可选：清除背景

    glDisable(GL_DEPTH_TEST); // 禁用深度测试（确保UI在3D上层）
    glEnable(GL_BLEND);       // 启用混合（确保UI半透明正常）
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 渲染ImGui绘制指令
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 恢复3D渲染状态（若后续要画3D）
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void prepareCamera() {
    float size = 6.0f; // 视图盒子大小
    //camera = new orthographicCamera(-size, size, -size, size, size, -size);

    camera = new perspectiveCamera(
        60.0f,
        (float)App->getWidth() / (float)App->getHeight(),
        0.1f,
        1000.0f);
    //camera->mPosition = glm::vec3(10.0f, 10.0f, 3.0f);
    //camera->mUp = glm::vec3(0.0f, 0.0f, 1.0f);
    //camera->mRight = glm::vec3(-1.0f, 1.0f, 0.0f);
    //cameraControl = new GameCameraControl();
    cameraControl = new TrackBallCameraControl();

    cameraControl->setCamera(camera);
    cameraControl->setSensitivity(0.4f);
    //cameraControl->setMoveSpeed(0.05f);
}

void prepare() {
    std::string vertexshaderPath = "E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/Blinn-Phong/Blinn-phong.vert";
    std::string fragmentshaderPath = "E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/Blinn-Phong/Blinn-phong-spotL-encapsulation.frag";

    renderer = new Renderer(vertexshaderPath, fragmentshaderPath);

    // 1 创建geometry
    auto geometry = Geometry::createBox(3.0f);
    //auto geometry = Geometry::createSphere(2.0f);
    //auto geometry = Geometry::createPlane(50.0f, 50.0f);

    // 2 创建一个material并且配置参数
    std::string texturePath = std::string(TEXTURE_DIR) + "/container2.png";
    auto material = new PhongMaterial();
    material->mShiness = 1.0f;
    material->mDiffuse = new Texture(texturePath.c_str(), 0);
    material->mBlinn = GL_TRUE;
    //material->mBlinn = GL_FALSE;
    texturePath = std::string(TEXTURE_DIR) + "/container2_specular.png";
    material->mSpecularMask = new Texture(texturePath.c_str(), 1);  // 与区分开来

    // 3 生成mesh
    auto mesh = new Mesh(geometry, material);
    meshes.push_back(mesh);

	// 创建一个纯白色材质的mesh
	auto geometryWhite = Geometry::createSphere(0.1f);
	auto materialWhite = new WhiteMaterial();
	meshWhite = new Mesh(geometryWhite, materialWhite);
    meshWhite->setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
	meshes.push_back(meshWhite);

    // 4 配置光照
    // 聚光灯
    spotLight = new SpotLight();
    spotLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    spotLight->setPosition(meshWhite->getPosition());  // 将点光源放在白色物体的位置
    spotLight->setTargetDirection(glm::vec3(0.0f, 0.0f, -1.0f));
    spotLight->setInnerAngle(30.0);
    spotLight->setOuterAngle(50.0);
    spotLight->mK2 = 0.017;
    spotLight->mK1 = 0.07;
    spotLight->mKc = 1.0;
    spotLight->setSpecularIntensity(1.0f);

    // 环境光
    ambLight = new AmbientLight();
    ambLight->setColor(glm::vec3{ 1.0f });
}

int main(int argc, char* argv[]) {
    if (!App->init(800, 600)) {	// 初始化Application类，创建窗体
        std::cout << "Application init failed!" << std::endl;
        return -1;
    }

    // 设置opengl视口以及清理颜色
    GL_CALL(glViewport(0, 0, App->getWidth(), App->getHeight()));	// 设置OpenGL视口的大小为窗体的大小)
    GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));	// 设置清屏颜色，RGBA四个分量，范围是[0.0, 1.0]，每个分量的值越大，颜色越亮

    prepareEventCallback();
    prepareCamera();
    prepare();
    initIMGUI();

    while (App->update()) {
        cameraControl->update();
        //lightTransform();
        //lightCameraTransform();     // 随相机位置移动的点光源
        renderer->render(meshes,camera, spotLight,ambLight);
    }

    App->destroy();
    return 0;
}

void lightTransform() {
    float zPos = glm::sin(SDL_GetTicks() * 0.001f) + 3.0f;
    meshWhite->setPosition(glm::vec3(0.0f, 0.0f, zPos));
    spotLight->setPosition(meshWhite->getPosition());
}

void lightCameraTransform() {
    meshWhite->setPosition(camera->mPosition);
    spotLight->setPosition(meshWhite->getPosition());
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

void PrintVec3(glm::vec3& vec) {
    std::cout <<"（"
        << vec.x << ", "  // 方式1：通过成员变量x/y/z
        << vec.y << ", "
        << vec.z << ")" << std::endl;
}