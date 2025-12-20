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
#include <vector>

#include "../../../include/glframework/material/phongMaterial.h"
#include "../../../include/glframework/material/whiteMaterial.h"
#include "../../../include/glframework/material/PBRMaterial.h"

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

void applyImGuiSettings_config();
void UpdateCameraViewParams();
void UpdateLightAndWhite();


void lightTransform();
void lightCameraTransform();
void PrintVec3(glm::vec3& vec);


Renderer* renderer = nullptr;
std::vector<Mesh*> meshes{};
Mesh* meshWhite = nullptr;


// 材质类
auto pbrmaterial = new PBRMaterial();
auto phongmaterial = new PhongMaterial();

// 灯光们
DirectionalLight* dirLight = nullptr;
SpotLight* spotLight = nullptr;
std::vector <PointLight*> pointLights;

AmbientLight* ambLight = nullptr;


Camera* camera = nullptr;
CameraControl* cameraControl = nullptr;


// 自定义可调节参数
struct MaterialStruct {
    glm::vec3 ambient = glm::vec3(0.08f);
    glm::vec3 diffuse = glm::vec3(0.375f);
    glm::vec3 specular = glm::vec3(0.402f);
    float shiness = 10.0f;
};

struct AmbientLightStruct {
    float intensity = 0.0f;
    glm::vec3 color = glm::vec3(0.0f);
};  // 环境光只配置 光强度

// 平行灯光源
struct DirectionLightStruct {
    glm::vec3 direction = glm::vec3(1.0f);    // 光照方向（归一化）
    glm::vec3 color = glm::vec3(0.0f);        // 光照颜色
    float specularIntensity = 1.0f; // 镜面强度

    glm::vec3 ambient = glm::vec3(0.0f);
    glm::vec3 diffuse = glm::vec3(0.0f);
    glm::vec3 specular = glm::vec3(0.0f);
};

// 点光源
struct PointLightStruct {
    glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f);;     // 光源位置
    glm::vec3 color = glm::vec3(0.0f);        // 光照颜色
    float specularIntensity = 1.0f; // 镜面强度，光源强度默认 1.0（中等强度，可通过调大增强光照，调小减弱）

    float k2{ 0.032f }; // 二次衰减系数
    float k1{ 0.09f }; // 一次衰减系数
    float kc{ 1.0f }; // 常数衰减系数
    
    glm::vec3 ambient = glm::vec3(0.0f);    // 环境光颜色默认全黑（默认不主动提供环境光，避免场景过亮）
    glm::vec3 diffuse = glm::vec3(0.0f);    // 漫反射光颜色默认白色（白光照射下，物体颜色接近其自身漫反射色
    glm::vec3 specular = glm::vec3(0.0f);   // 高光颜色默认白色（与漫反射光同色，符合自然光源的高光特性）
};

// 结构体
// 平行光配置
DirectionLightStruct directionLightConfig;
// 点光源配置数组
PointLightStruct pointLightConfigs[4];
// 材质配置
MaterialStruct materialConfig;
// 环境光配置
AmbientLightStruct ambientLightConfig;

// 窗口颜色
glm::vec3 clearColor{};

// 帧率计算变量
float lastFrameTime = 0.0f;
int frameCount = 0;
float fps = 0.0f;


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

void initIMGUI() {
    ImGui::CreateContext();// 创建imgui上下文
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark(); // 暗黑系

    // 设置ImGui与SDL和OpenGL绑定
    ImGui_ImplSDL2_InitForOpenGL(App->getWindow(),App->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 460");
}


void renderIMGUI_config() {
    // 1. 帧率计算（保持逻辑不变）
    float currentTime = (float)SDL_GetTicks() / 1000.0f;
    static int frameCount = 0;
    static float lastFrameTime = 0.0f;
    static float fps = 0.0f;
    frameCount++;
    if (currentTime - lastFrameTime >= 1.0f) {
        fps = frameCount / (currentTime - lastFrameTime);
        frameCount = 0;
        lastFrameTime = currentTime;
    }

    // 2. ImGui 初始化（保持接口不变）
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 3. 主调节窗口：材质+光源配置（变量全部替换为新命名）
    ImGui::Begin("Light & Material Parameter Adjustment");

    // -------------------------- 材质配置（绑定 materialConfig） --------------------------
    if (ImGui::TreeNode("Material Settings")) {
        ImGui::Text("Ambient Color (Material)");
        ImGui::ColorEdit3("Mat Ambient vec", (float*)&materialConfig.ambient);

        ImGui::Text("Diffuse Color (Material)");
        ImGui::ColorEdit3("Mat Diffuse vec", (float*)&materialConfig.diffuse);

        ImGui::Text("Specular Color (Material)");
        ImGui::ColorEdit3("Mat Specular vec", (float*)&materialConfig.specular);

        ImGui::Text("Shininess (Gloss Level)");
        ImGui::SliderFloat("Mat Shininess", &materialConfig.shiness, 0.0f, 200.0f); // 合理范围
        ImGui::TreePop();
    }
    ImGui::Separator();

    // -------------------------- 环境光配置（绑定 ambientLightConfig） --------------------------
    if (ImGui::TreeNode("Ambient Light Settings")) {
        ImGui::Text("Ambient Intensity");
        ImGui::SliderFloat("Amb Intensity", &ambientLightConfig.intensity, 0.0f, 100.0f);

        ImGui::Text("Ambient Color");
        ImGui::ColorEdit3("Amb Color", (float*)&ambientLightConfig.color);
        ImGui::TreePop();
    }
    ImGui::Separator();

    // -------------------------- 平行光配置（绑定 directionLightConfig） --------------------------
    if (ImGui::TreeNode("Directional Light Settings")) {
        ImGui::Text("Light Direction (Normalized)");
        ImGui::InputFloat3("Dir Vector", (float*)&directionLightConfig.direction);
        directionLightConfig.direction = glm::normalize(directionLightConfig.direction); // 强制归一化

        ImGui::Text("Light Color");
        ImGui::ColorEdit3("Dir Light Color", (float*)&directionLightConfig.color);

        ImGui::Text("Specular Intensity");
        ImGui::SliderFloat("Dir Spec Intensity", &directionLightConfig.specularIntensity, 0.0f, 100.0f);

        ImGui::Text("Light Source Ambient");
        ImGui::ColorEdit3("Dir Ambient", (float*)&directionLightConfig.ambient);

        ImGui::Text("Light Source Diffuse");
        ImGui::ColorEdit3("Dir Diffuse", (float*)&directionLightConfig.diffuse);

        ImGui::Text("Light Source Specular");
        ImGui::ColorEdit3("Dir Specular", (float*)&directionLightConfig.specular);
        ImGui::TreePop();
    }
    ImGui::Separator();

    // -------------------------- 点光源配置（绑定 pointLightConfigs[4]） --------------------------
    if (ImGui::TreeNode("Point Lights Settings (4 Lights)")) {
        for (int i = 0; i < 4; i++) {
            // 每个点光源独立折叠节点，标签唯一（避免ImGui控件冲突）
            std::string nodeName = "Point Light " + std::to_string(i + 1);
            if (ImGui::TreeNode(nodeName.c_str())) {
                // 位置
                std::string posLabel = "Pos##PL" + std::to_string(i);
                ImGui::Text("Position");
                ImGui::InputFloat3(posLabel.c_str(), (float*)&pointLightConfigs[i].position);

                // 颜色
                std::string colorLabel = "Color##PL" + std::to_string(i);
                ImGui::Text("Light Color");
                ImGui::ColorEdit3(colorLabel.c_str(), (float*)&pointLightConfigs[i].color);

                // 镜面强度
                std::string specLabel = "Spec Int##PL" + std::to_string(i);
                ImGui::Text("Specular Intensity");
                ImGui::SliderFloat(specLabel.c_str(), &pointLightConfigs[i].specularIntensity, 0.0f, 500.0f);

                // 衰减系数
                std::string k2Label = "k2 (Quadratic)##PL" + std::to_string(i);
                std::string k1Label = "k1 (Linear)##PL" + std::to_string(i);
                std::string kcLabel = "kc (Constant)##PL" + std::to_string(i);
                ImGui::Text("Attenuation Coefficients");
                ImGui::SliderFloat(k2Label.c_str(), &pointLightConfigs[i].k2, 0.001f, 0.1f);
                ImGui::SliderFloat(k1Label.c_str(), &pointLightConfigs[i].k1, 0.01f, 0.5f);
                ImGui::SliderFloat(kcLabel.c_str(), &pointLightConfigs[i].kc, 0.5f, 2.0f);

                // 光源自身环境/漫反射/镜面光
                std::string ambLabel = "Ambient##PL" + std::to_string(i);
                std::string diffLabel = "Diffuse##PL" + std::to_string(i);
                std::string specLightLabel = "Specular##PL" + std::to_string(i);
                ImGui::Text("Light Source Properties");
                ImGui::ColorEdit3(ambLabel.c_str(), (float*)&pointLightConfigs[i].ambient);
                ImGui::ColorEdit3(diffLabel.c_str(), (float*)&pointLightConfigs[i].diffuse);
                ImGui::ColorEdit3(specLightLabel.c_str(), (float*)&pointLightConfigs[i].specular);

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    ImGui::End(); // 结束主调节窗口

    // 4. 帧率监控窗口（保持不变）
    ImGui::Begin("FPS Monitor");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::End();

    // 5. ImGui 渲染（保持接口不变）
    ImGui::Render();
    int display_w, display_h;
    SDL_GL_GetDrawableSize(App->getWindow(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void prepare() {
    std::string pbrVertexPath = std::string(SHADER_DIR) +  "/PBR-Light/PBR.vert";
    std::string pbrFragmentPath = std::string(SHADER_DIR) + "/PBR-Light/PBR-simplify.frag";

    std::string phongVertexPath = std::string(SHADER_DIR) + "/PBR-Light/PBR.vert";
    std::string phongFragmentPath = std::string(SHADER_DIR) + "/PBR-Light/Blinn-phong-point-direct.frag";

    std::string whiteVertexPath = std::string(SHADER_DIR) + "/whiteShader/white.vert";
    std::string whiteFragmentPath = std::string(SHADER_DIR) + "/whiteShader/white.frag";

    
    // Ensure paths are valid before passing to Renderer constructor  
    if (phongVertexPath.empty() || phongFragmentPath.empty() || pbrVertexPath.empty() || pbrFragmentPath.empty()) {
        std::cerr << "Error: Shader paths cannot be empty!" << std::endl;
        return;
    }
    renderer = new Renderer(phongVertexPath, phongFragmentPath, whiteVertexPath, whiteFragmentPath, pbrVertexPath, pbrFragmentPath);
    
    // 1 创建geometry  
    //auto PBRgeometry = Geometry::createFromSTL("E:/IT-Furnace/OpenGl/mindray/Framework/resource/objs/humanHeart/anatomical-heart.stl",true);
    auto PBRgeometry = Geometry::createFromOBJ("E:/IT-Furnace/OpenGl/mindray/Framework/resource/objs/pbrheart/pbrheart.obj");

    //auto PBRgeometry = Geometry::createBox(5.0f);
    //auto PhongGeometry = Geometry::createSphere(1.0f);
    //auto PBRgeometry = Geometry::createPlane(50.0f, 50.0f);

    // 2.1 创建一个pbrmaterial并且配置参数
    std::string TalbedoPath = std::string(TEXTURE_DIR) + "/rusted_iron/albedo.png";
    std::string TnormalPath = std::string(TEXTURE_DIR) + "/rusted_iron/normal.png";
    std::string TaoPath = std::string(TEXTURE_DIR) + "/rusted_iron/ao.png";
    std::string TmetallicPath = std::string(TEXTURE_DIR) + "/rusted_iron/metallic.png";
    std::string TroughnessPath = std::string(TEXTURE_DIR) + "/rusted_iron/roughness.png";

    //auto pbrmaterial = new PBRMaterial();
    pbrmaterial->mAlbedoMap = new Texture(TalbedoPath, 0);
    pbrmaterial->mNormalMap = new Texture(TnormalPath, 1);
    pbrmaterial->mMetallicMap = new Texture(TmetallicPath, 2);
    pbrmaterial->mRoughnessMap = new Texture(TroughnessPath, 3);
    pbrmaterial->mAoMap = new Texture(TaoPath, 4);


    // 2.1 创建一个Phongmaterial并且配置参数
    std::string texturePath = std::string(TEXTURE_DIR) + "/pbrheart/Albedo.png";

    //auto phongmaterial = new PhongMaterial();
    phongmaterial->mDiffuse = new Texture(texturePath, 5);
    phongmaterial->setShiness(1.0f);
    phongmaterial->setBlinn(GL_TRUE);
    //phongmaterial->mBlinn = GL_FALSE;
    texturePath = std::string(TEXTURE_DIR) + "/container2_specular.png";
    phongmaterial->mSpecularMask = new Texture(texturePath, 6);  // 与区分开来


    // 3 生成mesh
    auto PBRmesh = new Mesh(PBRgeometry, pbrmaterial);
    auto PhongMesh = new Mesh(PBRgeometry, phongmaterial);
    PBRmesh->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    PBRmesh->setAngleY(180.0f);
    PBRmesh->setAngleX(90.0f);
    PBRmesh->setScale(glm::vec3(0.1f));
    
    PhongMesh->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    PhongMesh->setAngleY(180.0f);
    PhongMesh->setAngleX(90.0f);
    PhongMesh->setScale(glm::vec3(0.1f));

    // 创建父子关系
    //PBRmesh->addChild(spMesh);
    //meshes.push_back(PBRmesh);
    meshes.push_back(PhongMesh);

	// 创建一个纯白色材质的mesh
	auto geometryWhite = Geometry::createSphere(0.1f);
	auto materialWhite = new WhiteMaterial();
	meshWhite = new Mesh(geometryWhite, materialWhite);
    meshWhite->setPosition(glm::vec3(4.0f, 0.0f, 8.0f));
	//meshes.push_back(meshWhite);

    auto meshWhite2 = new Mesh(geometryWhite, materialWhite);
    meshWhite2->setPosition(glm::vec3(4.0f, 0.0f, 8.0f));
    //meshes.push_back(meshWhite2);

    // 4 配置光照
    // 聚光灯
    spotLight = new SpotLight();
	spotLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));   // 全都设为0，关闭聚光灯
    spotLight->setPosition(meshWhite->getPosition());  // 将灯放在白色物体的位置
    spotLight->setTargetDirection(glm::vec3(0.0f, 0.0f, -1.0f));
    spotLight->setInnerAngle(30.0);
    spotLight->setOuterAngle(50.0);
    spotLight->mK2 = 0.017;
    spotLight->mK1 = 0.07;
    spotLight->mKc = 1.0;
    spotLight->setSpecularIntensity(1.0f);

    // 平行光
    dirLight = new DirectionalLight();
	dirLight->setColor(glm::vec3(1.0f));    // 全都设为0，关闭平行光
    dirLight->setDirection(glm::vec3(-1.0f));
    dirLight->setSpecularIntensity(30.0f);

    // 点光源
    auto pointLight1 = new PointLight();
    pointLight1->setPosition(meshWhite->getPosition());
    pointLight1->setColor(glm::vec3(0.0f));
    pointLight1->setSpecularIntensity(500.0f);
    pointLight1->setK2(0.017);
    pointLight1->setK1(0.07);
    pointLight1->setKc(1.0);
    pointLights.push_back(pointLight1);

    auto pointLight2 = new PointLight();
    pointLight2->setPosition(meshWhite->getPosition());
    pointLight2->setColor(glm::vec3(1.0f));
    pointLight2->setSpecularIntensity(0.1f);
    pointLight2->setK2(0.032);
    pointLight2->setK1(0.09);
    pointLight2->setKc(1.0);
    pointLights.push_back(pointLight2);

    auto pointLight3 = new PointLight();
    pointLight3->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    pointLight3->setColor(glm::vec3(0.0f));
    pointLight3->setSpecularIntensity(10.0f);
    pointLight3->setK2(0.017);
    pointLight3->setK1(0.07);
    pointLight3->setKc(1.0);
    pointLights.push_back(pointLight3);

    auto pointLight4 = new PointLight();
    pointLight4->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    pointLight4->setColor(glm::vec3(0.0f));
    pointLight4->setSpecularIntensity(10.0f);
    pointLight4->setK2(0.017);
    pointLight4->setK1(0.07);
    pointLight4->setKc(1.0);
    pointLights.push_back(pointLight4);

    // 环境光
    ambLight = new AmbientLight();
    ambLight->setColor(glm::vec3{ 0.0f });
}

int main(int argc, char* argv[]) {
    if (!App->init(800, 600,"Blinn-Phong")) {	// 初始化Application类，创建窗体
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

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    // 可选：设置深度比较规则（默认GL_LESS）
    glDepthFunc(GL_LESS);

    // 启用混合（透明度所需）
    glEnable(GL_BLEND);
    // 设置混合函数：最终颜色 = 源颜色 * 源alpha + 目标颜色 * (1 - 源alpha)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLboolean blendEnabled;
    glGetBooleanv(GL_BLEND, &blendEnabled);
    if (!blendEnabled) {
        std::cout << "混合功能未启用！" << std::endl;
    }

    while (App->update()) {
        // 在渲染循环（render loop）的开头调用
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 同时清除颜色和深度缓冲

        //meshes[0]->rotateZ(1.0f);
        //meshes[1]->rotateY(1.0f);

        cameraControl->update();
        //camera->rotateAroundAxis(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 8.0f, pointLights[1]->getPosition() + 6.0f,glm::pi<float>()/2.0f);
        renderer->setClearColor(clearColor);

        applyImGuiSettings_config();
        //UpdateCameraViewParams();
        renderer->render(meshes, camera, dirLight, pointLights, spotLight, ambLight);
        renderIMGUI_config();      // ImGui UI 应在3D 场景之后渲染，确保 UI 显示在最上层：
    }

    App->destroy();
    // 清理资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    return 0;
}

void UpdateLightAndWhite() {
    pointLights[1]->setPosition(pointLightConfigs[1].position);
    meshWhite->setPosition(pointLights[1]->getPosition());
}

void applyImGuiSettings_config() {
    // 光照：从 ImGui 配置同步到实际光源对象
    // -------------------------- 1. 点光源（对应 pointLightConfigs 配置） --------------------------
    for (int i = 0; i < 4; i++) {
        pointLights[i]->setSpecularIntensity(pointLightConfigs[i].specularIntensity);
        pointLights[i]->setColor(pointLightConfigs[i].color);
        pointLights[i]->setAmbient(pointLightConfigs[i].ambient);
        pointLights[i]->setDiffuse(pointLightConfigs[i].diffuse);
        pointLights[i]->setSpecular(pointLightConfigs[i].specular);
        pointLights[i]->setPosition(pointLightConfigs[i].position);
    }

    // -------------------------- 2. 平行光（对应 directionLightConfig 配置） --------------------------
    dirLight->setSpecularIntensity(directionLightConfig.specularIntensity);
    dirLight->setDirection(directionLightConfig.direction);
    dirLight->setColor(directionLightConfig.color);
    dirLight->setAmbient(directionLightConfig.ambient);
    dirLight->setDiffuse(directionLightConfig.diffuse);
    dirLight->setSpecular(directionLightConfig.specular);

    // -------------------------- 3. 材质（对应 materialConfig 配置） --------------------------
    phongmaterial->setShiness(materialConfig.shiness);
    phongmaterial->setAmbientColor(materialConfig.ambient);
    phongmaterial->setDiffuseColor(materialConfig.diffuse);
    phongmaterial->setSpecularColor(materialConfig.specular);

    // -------------------------- 4. 环境光（对应 ambientLightConfig 配置） --------------------------
    ambLight->setIntensity(ambientLightConfig.intensity);
    ambLight->setColor(ambientLightConfig.color);
}

void UpdateCameraViewParams() {
    //lightTransform();
    //lightCameraTransform();     
    // 随相机位置移动的点光源
    meshWhite->setPosition(camera->mPosition);
    pointLights[1]->setPosition(camera->mPosition);
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