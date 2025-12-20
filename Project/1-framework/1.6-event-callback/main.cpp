#include "Application.h"
#include "glad/glad.h"
#include <iostream>


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
    }
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

void OnMouseButton(int button, int state, int x, int y, int mod){
    /*
    event.button.button,       // 按钮类型
    event.button.state,        // 按下/释放
    event.button.x, event.button.y,  // 位置
    SDL_GetModState()          // 修饰键
    */
    std::cout << "x: " << x << ", y: " << y << std::endl;
    if (state == SDL_PRESSED) {
        if (button == SDL_BUTTON_LEFT) {
            std::cout << "按下了鼠标左键" << std::endl;
        }
        else if (button == SDL_BUTTON_RIGHT) {
            std::cout << "按下了鼠标右键" << std::endl;
        }
    }
}

void OnMouseMotion(int x, int y, int dx, int dy, int buttonState) {
    /*
    参数对应关系：
    event.motion.x      -> x：鼠标当前绝对位置X（窗口左上角为原点）
    event.motion.y      -> y：鼠标当前绝对位置Y
    event.motion.xrel   -> dx：相对于上一帧的X偏移量（右为正）
    event.motion.yrel   -> dy：相对于上一帧的Y偏移量（下为正）
    event.motion.state  -> buttonState：移动时按住的鼠标按键掩码
    */

    // 打印基本位置和偏移信息
    std::cout << "鼠标移动 - 绝对位置: (" << x << ", " << y
        << "), 相对偏移: (" << dx << ", " << -dy << "), 按住的按键: ";

    // 解析按键状态掩码（判断移动时哪些按键被按住）
    bool isLeftDown = (buttonState & SDL_BUTTON_LMASK) != 0;    // 左键
    bool isRightDown = (buttonState & SDL_BUTTON_RMASK) != 0;   // 右键
    bool isMiddleDown = (buttonState & SDL_BUTTON_MMASK) != 0;  // 中键

    // 输出按住的按键信息
    if (isLeftDown) std::cout << "左键 ";
    if (isRightDown) std::cout << "右键 ";
    if (isMiddleDown) std::cout << "中键 ";
    if (!isLeftDown && !isRightDown && !isMiddleDown) std::cout << "无";

    std::cout << std::endl;  // 换行分隔每次移动事件
}

void OnMouseWheel(int scrollX, int scrollY) {
    /*
    参数对应关系：
    event.wheel.x  -> scrollX：水平滚动值（SDL默认规则：向右滚动为正，向左滚动为负）
    event.wheel.y  -> scrollY：垂直滚动值（SDL默认规则：向上滚动为正，向下滚动为负）
    注意：部分鼠标仅支持垂直滚动，此时scrollX恒为0
    */

    // 1. 打印原始滚动数值（方便调试原始数据）
    std::cout << "鼠标滚轮事件 - 原始值：水平scrollX=" << scrollX << "，垂直scrollY=" << scrollY;

    // 2. 解析垂直滚动方向（最常用场景：上/下滚）
    std::string verticalDir = "无垂直滚动";
    if (scrollY > 0) {
        verticalDir = "向上滚动（常见：页面上翻/放大）";
    }
    else if (scrollY < 0) {
        verticalDir = "向下滚动（常见：页面下翻/缩小）";
    }

    // 3. 解析水平滚动方向（部分鼠标支持：左/右滚）
    std::string horizontalDir = "无水平滚动";
    if (scrollX > 0) {
        horizontalDir = "向右滚动（常见：横向内容右移）";
    }
    else if (scrollX < 0) {
        horizontalDir = "向左滚动（常见：横向内容左移）";
    }

    // 4. 输出最终解析结果（换行分隔每次滚轮事件）
    std::cout << " | 解析：" << verticalDir << "，" << horizontalDir << std::endl;
}



int main(int argc, char* argv[]) {  // 将 main 函数的参数修改为 SDL 要求的标准格式（即使不使用参数，也需要保留参数列表）
    Application* app = Application::getInstance();
    app->init(800, 600, "Event Callback Display");

    // 设置键盘回调函数
    app->setKeyBoardCallback(onKeyboard);
    app->setResizeCallback(onResize);
    app->setMouseButtonCallback(OnMouseButton);
    app->setMouseMotionCallback(OnMouseMotion);
    app->setMouseWheelCallback(OnMouseWheel);

    // 设置画布尺寸，初始化画布尺寸
    glViewport(0, 0, app->getWidth(), app->getHeight());

    // 主循环
    while (app->update()) {
        // 设置画布颜色并执行opengl画布清理操作
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    app->destroy();
    return 0;
}