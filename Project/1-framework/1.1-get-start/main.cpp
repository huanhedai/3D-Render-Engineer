#include <iostream>
#define SDL_MAIN_HANDLED  // 使用SDL2，Windows会重新定义入口函数
#include <SDL2/SDL.h>

// 在包含 glad.h 之前定义 GLAD_GLAPI_EXPORT
#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>
#include <windows.h>

// 窗口基础参数
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "简单窗口";

int main() {
	// 加载动态库
	HMODULE hlib = LoadLibraryA("SDL2.dll");
	if (hlib == NULL) {
		std::cerr << "加载库失败！错误码: " << GetLastError() << std::endl;
		return 1;
	}

	// 获取函数地址，设函数原型为: void Func(GLint x, GLint y, GLsizei width, GLsizei height)
	using Myfunptr = void(*)(GLint x, GLint y, GLsizei width, GLsizei height);
	Myfunptr myfun = (Myfunptr)GetProcAddress(hlib, "MyFunction");

	if (myfun == NULL) {
		// std::cerr << "获取函数失败！错误码: " << GetLastError() << std::endl;
		// return 1;
	}

	// 1.初始化SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL初始化失败" << SDL_GetError() << std::endl;
		return 1;
	}

	// 2. 配置OpenGL核心属性
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // 双缓冲避免画面闪烁

	// 3. 创建SDL窗口（居中显示，支持调节大小）
	SDL_Window* window = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);
	if (!window) {
		std::cerr << "窗口创建失败" << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// 4. 创建OpenGL上下文（关联窗口和OpenGL环境）
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (!gl_context) {
		std::cerr << "OpenGL初始化失败" << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// ✅ 在 OpenGL 上下文创建后调用 gladLoadGL()
	gladLoadGL();  // 直接加载OpenGL的函数地址，不依赖外部加载器，一般推荐使用gladLoadGLLoader
	const GLubyte* version = glGetString(GL_VERSION);
	if (!version) {
		std::cerr << "无法获取OpenGL版本信息" << std::endl;
	}

	std::cout << version << std::endl;
	// 6. 初始化窗口
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	float bg_r = 0.2f, bg_g = 0.3f, bg_b = 0.5f;
	bool is_running = true;
	SDL_Event event;

	while (is_running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				is_running = false;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					is_running = false;
				}
				else if (event.key.keysym.sym == SDLK_r) {
					bg_r = (bg_r == 0.2f) ? 0.8f : 0.2f;
				}
			}
			else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			glViewport(0, 0, event.window.data1, event.window.data2);
			}
		}

	glClearColor(bg_r, bg_g, bg_b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// 交换双缓冲
	SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}