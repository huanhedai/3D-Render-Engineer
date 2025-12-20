#ifndef APPLICATION_H
#define APPLICATION_H

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <functional>

#define App Application::getInstance()  // 定义一个宏，方便外界调用

class Application {
public:
    // 回调函数类型定义
    using ResizeCallback = std::function<void(int, int)>;
    using KeyBoardCallback = std::function<void(int , int , int , int )>;
    using MouseButtonCallback = std::function<void(int button, int state, int x, int y, int mod)>; 
    using MouseMotionCallback = std::function<void(int x, int y, int dx, int dy, int state)>;
    using MouseWheelCallback = std::function<void(int x, int y)>;

    // 单例模式
    static Application* getInstance();

    // 初始化
    bool init(const int& width, const int& height, const char* title = "SDL2 OpenGL Study");

    // 事件处理与更新
    void processEvents();
    bool update();

    // 资源释放
    void destroy();

    // 退出应用
    void quit() { mRunning = false; }

    // 获取鼠标位置
    void getCursorPosition(int* x, int* y);

    // 回调函数设置方法
    void setResizeCallback(ResizeCallback callback) { mResizeCallback = callback; }
    void setKeyBoardCallback(KeyBoardCallback callback) { mKeyBoardCallback = callback; }
    void setMouseButtonCallback(MouseButtonCallback callback) { mMouseButtonCallback = callback; }
    void setMouseMotionCallback(MouseMotionCallback callback) { mMouseMotionCallback = callback; }
    void setMouseWheelCallback(MouseWheelCallback callback) { mMouseWheelCallback = callback; }

    // 获取窗口和上下文
    SDL_Window* getWindow() const { return mWindow; }
    SDL_GLContext getGLContext() const { return mGLContext; }

    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }

private:
    // 单例模式私有构造函数
    Application();
    ~Application();

    // 禁止拷贝和赋值
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    static Application* minstance;

    SDL_Window* mWindow;
    SDL_GLContext mGLContext;
    int mWidth;
    int mHeight;
    bool mRunning;

    // 回调函数成员
    ResizeCallback mResizeCallback;
    KeyBoardCallback mKeyBoardCallback;
    MouseButtonCallback mMouseButtonCallback;
    MouseMotionCallback mMouseMotionCallback;
    MouseWheelCallback mMouseWheelCallback;
};

#endif // APPLICATION_H
