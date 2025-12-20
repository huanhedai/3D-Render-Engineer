#include "Application.h"
#include <iostream>
#include "../../../include//imgui/imgui_impl_sdl2.h"

// 初始化静态成员
Application* Application::minstance = nullptr;

Application::Application()
    : mWindow(nullptr), mGLContext(nullptr),
    mWidth(0), mHeight(0), mRunning(true) {
}

Application::~Application() {
    destroy();
    if (minstance != nullptr) {
        minstance = nullptr;
    }
}

Application* Application::getInstance() {
    if (minstance == nullptr) {
        minstance = new Application();
    }
    return minstance;
}

bool Application::init(const int& width, const int& height, const char* title) {
    mWidth = width;
    mHeight = height;

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cout << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 设置OpenGL版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 开启双缓冲
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // 创建窗口
    mWindow = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        mWidth, mHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );


    if (!mWindow) {
        std::cout << "窗口创建失败: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // 创建OpenGL上下文
    mGLContext = SDL_GL_CreateContext(mWindow);
    if (!mGLContext) {
        std::cout << "OpenGL上下文创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
        return false;
    }

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        SDL_GL_DeleteContext(mGLContext);
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
        return false;
    }

    // 启用垂直同步
    SDL_GL_SetSwapInterval(1);

    return true;
}

void Application::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // 关键：将SDL事件传递给ImGui，使其能响应交互
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            mRunning = false;
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                mWidth = event.window.data1;
                mHeight = event.window.data2;
                if (mResizeCallback) {
                    mResizeCallback(mWidth, mHeight);
                }
                //std::cout << "Window resized to: " << mWidth << "x" << mHeight << std::endl;
            }
            break;

        case SDL_KEYUP:
        case SDL_KEYDOWN:
            if (mKeyBoardCallback) {
                mKeyBoardCallback(
                    event.key.keysym.scancode, // 扫描码
                    event.key.keysym.sym,    // 键码
                    event.key.state,         // 状态(按下/释放)
                    event.key.keysym.mod     // 修饰键
                );
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (mMouseButtonCallback) {
                mMouseButtonCallback(
                    event.button.button,       // 按钮类型
                    event.button.state,        // 按下/释放
                    event.button.x, event.button.y,  // 位置
                    SDL_GetModState()          // 修饰键
                );
                /*
                    这里的x和y是按下时鼠标的绝对位置坐标，与后面的相对移动要区分开
                */
            }
            break;

            // 鼠标移动事件
        case SDL_MOUSEMOTION:
            if (mMouseMotionCallback) {
                mMouseMotionCallback(
                    event.motion.x, event.motion.y,  // 绝对位置
                    event.motion.xrel, event.motion.yrel,  // 相对移动
                    event.motion.state  // 移动时按住的按键
                );
            }
            break;

            // 鼠标滚轮事件
        case SDL_MOUSEWHEEL:
            if (mMouseWheelCallback) {
                mMouseWheelCallback(
                    event.wheel.x,  // 水平滚动
                    event.wheel.y   // 垂直滚动
                );
            }
            break;
        }
    }
}

bool Application::update() {
    if (!mRunning) {
        return false;
    }

    // 处理事件
    processEvents();

    // 交换缓冲区
    SDL_GL_SwapWindow(mWindow);

    return true;
}

void Application::destroy() {
    if (mGLContext) {
        SDL_GL_DeleteContext(mGLContext);
        mGLContext = nullptr;
    }

    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    SDL_Quit();
}

void Application::getCursorPosition(int* x, int* y) {
    if (x && y) {
        SDL_GetMouseState(x, y);
    }
}
