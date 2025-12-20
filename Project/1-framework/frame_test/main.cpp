#include <SDL2/SDL.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow(
        "SDL键盘事件示例",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    int running = 1;
    SDL_Event event;

    // 主循环：主动从事件队列提取事件
    while (running) {
        // 轮询事件队列
        while (SDL_PollEvent(&event) != 0) {
            // 处理窗口关闭事件
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // 处理键盘按下事件
            else if (event.type == SDL_KEYDOWN) {
                // ESC键退出
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                    printf("ESC键按下，准备退出...\n");
                }
                // WASD键处理
                switch (event.key.keysym.sym) {
                case SDLK_w: printf("W键按下\n"); break;
                case SDLK_a: printf("A键按下\n"); break;
                case SDLK_s: printf("S键按下\n"); break;
                case SDLK_d: printf("D键按下\n"); break;
                }
            }
            // 处理键盘释放事件
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_w: printf("W键释放\n"); break;
                case SDLK_a: printf("A键释放\n"); break;
                case SDLK_s: printf("S键释放\n"); break;
                case SDLK_d: printf("D键释放\n"); break;
                }
            }
        }

        // 渲染逻辑（此处省略）
    }

    // 清理资源
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}