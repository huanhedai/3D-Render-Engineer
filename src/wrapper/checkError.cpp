#include <string>
#include <iostream>
#include <glad/glad.h>	// 这个需要在 glfw3.h 这个前面
#include <SDL2/SDL.h>   // SDL库头文件
#include <cassert>      // 断言库
#include "checkError.h"

void checkError() {
    GLenum error = glGetError();  // 获取OpenGL错误代码（与窗口库无关）

    if (error != GL_NO_ERROR) {   // 检测到错误时处理
        std::string errorString;

        // 转换错误代码为可读字符串
        switch (error) {
        case GL_INVALID_ENUM:
            errorString = "INVALID_ENUM（枚举值无效）";
            break;
        case GL_INVALID_VALUE:
            errorString = "INVALID_VALUE（参数值无效）";
            break;
        case GL_INVALID_OPERATION:
            errorString = "INVALID_OPERATION（操作无效，如上下文未激活）";
            break;
        case GL_OUT_OF_MEMORY:
            errorString = "OUT_OF_MEMORY（内存不足）";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorString = "INVALID_FRAMEBUFFER_OPERATION（帧缓冲操作无效）";
            break;
        default:
            errorString = "UNKNOWN_ERROR（未知错误）";
            break;
        }

        // 输出错误信息（包含SDL相关上下文提示）
        std::cerr << "OpenGL错误：" << std::endl;
        std::cerr << "  错误代码：0x" << std::hex << error << std::dec << std::endl;
        std::cerr << "  错误描述：" << errorString << std::endl;

        // 断言终止程序（调试阶段快速定位问题）
        assert(false && "OpenGL错误已触发断言");
    }
}
