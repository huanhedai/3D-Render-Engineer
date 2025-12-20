#include "shader.h"
#include "checkError.h"  // 依赖的OpenGL错误检查（需适配SDL2，见下文）

#include<glad/glad.h>	// 这个需要在 glfw3.h 这个前面
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// 构造函数：从文件加载并初始化着色器
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // 【SDL2关键约束】：此构造函数必须在「SDL_GL_CreateContext()之后调用」
    // 原因：OpenGL函数（如glCreateShader）需要有效上下文才能执行，否则触发GL_INVALID_OPERATION
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // 配置文件流：允许抛出异常（处理文件打开/读取失败）
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // 打开着色器文件（路径需与SDL程序运行目录匹配）
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        // 读取文件内容到字符串流
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();  // 读取顶点着色器缓冲区
        fShaderStream << fShaderFile.rdbuf();  // 读取片段着色器缓冲区

        // 关闭文件（读取完成后释放资源）
        vShaderFile.close();
        fShaderFile.close();

        // 转换为字符串格式的着色器源码
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR[Shader]: 着色器文件操作失败 - " << e.what() << std::endl;
        std::cerr << "提示：检查着色器文件路径是否与SDL程序运行目录一致" << std::endl;
    }

    // 转换为C风格字符串（OpenGL接口要求）
    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // 1. 编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GL_CALL(glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr));  // 绑定源码
    GL_CALL(glCompileShader(vertexShader));                                  // 执行编译
    checkShaderErrors(vertexShader, "COMPILE");  // 检查编译错误

    // 2. 编译片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GL_CALL(glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr));  // 绑定源码
    GL_CALL(glCompileShader(fragmentShader));                                    // 执行编译
    checkShaderErrors(fragmentShader, "COMPILE");  // 检查编译错误

    // 3. 链接着色器程序（合并为可执行的OpenGL程序）
    mProgram = glCreateProgram();
    GL_CALL(glAttachShader(mProgram, vertexShader));  // 附加顶点着色器
    GL_CALL(glAttachShader(mProgram, fragmentShader));// 附加片段着色器
    GL_CALL(glLinkProgram(mProgram));                 // 执行链接
    checkShaderErrors(mProgram, "LINK");              // 检查链接错误

    // 4. 清理中间资源（链接完成后，单独的着色器对象可删除）
    GL_CALL(glDeleteShader(vertexShader));
    GL_CALL(glDeleteShader(fragmentShader));
}


// 新增构造函数：按源码字符串加载（直接用工具类返回的字符串）
Shader::Shader(const char* vertexPath, const char* fragmentPath, std::vector<ShaderMacro> macros) {
    std::string VertSource = Tools::readShaderSourceWithMacros(vertexPath, macros);
    std::string FragSource = Tools::readShaderSourceWithMacros(fragmentPath, macros);

    // 转换为C风格字符串（OpenGL接口要求）
    const char* vertexShaderSource = VertSource.c_str();
    const char* fragmentShaderSource = FragSource.c_str();

    // 1. 编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GL_CALL(glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr));  // 绑定源码
    GL_CALL(glCompileShader(vertexShader));                                  // 执行编译
    checkShaderErrors(vertexShader, "COMPILE");  // 检查编译错误

    // 2. 编译片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    GL_CALL(glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr));  // 绑定源码
    GL_CALL(glCompileShader(fragmentShader));                                    // 执行编译
    checkShaderErrors(fragmentShader, "COMPILE");  // 检查编译错误

    // 3. 链接着色器程序（合并为可执行的OpenGL程序）
    mProgram = glCreateProgram();
    GL_CALL(glAttachShader(mProgram, vertexShader));  // 附加顶点着色器
    GL_CALL(glAttachShader(mProgram, fragmentShader));// 附加片段着色器
    GL_CALL(glLinkProgram(mProgram));                 // 执行链接
    checkShaderErrors(mProgram, "LINK");              // 检查链接错误

    // 4. 清理中间资源（链接完成后，单独的着色器对象可删除）
    GL_CALL(glDeleteShader(vertexShader));
    GL_CALL(glDeleteShader(fragmentShader));
}

// 析构函数：释放OpenGL着色器程序
Shader::~Shader() {
    // 【SDL2关键约束】：此析构函数必须在「SDL_GL_DeleteContext()之前调用」
    // 原因：上下文销毁后，OpenGL对象（如mProgram）无法再访问，会触发无效操作
    if (mProgram != 0) {
        GL_CALL(glDeleteProgram(mProgram));
        mProgram = 0;
    }
}

// 激活当前着色器程序（后续OpenGL渲染将使用此程序）
void Shader::begin() {
    GL_CALL(glUseProgram(mProgram));
}

// 取消激活着色器程序（避免后续操作意外污染）
void Shader::end() {
    GL_CALL(glUseProgram(0));
}

// 设置Uniform变量：float类型（如光照强度、透明度）
void Shader::setFloat(const std::string& name, float value) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    GL_CALL(glUniform1f(location, value));
}

// 设置Uniform变量：bool类型
void Shader::setBool(const std::string& name, bool value) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    GL_CALL(glUniform1i(location, value));
}

// 设置Uniform变量：int类型（常用于纹理单元索引，如glActiveTexture的参数）
void Shader::setInt(const std::string& name, int value) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    GL_CALL(glUniform1i(location, value));
}

// 设置Uniform变量：vec3类型（重载1：单独传入x/y/z，如颜色、坐标）
void Shader::setVector3(const std::string& name, float x, float y, float z) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    if (location != -1) {
        GL_CALL(glUniform3f(location, x, y, z));
    }
    else {
        std::cerr << "WARNING[Shader]: Uniform变量 '" << name << "' 未在着色器中定义" << std::endl;
    }
}

// 设置Uniform变量：vec3类型（重载2：传入float数组，如批量设置坐标）
void Shader::setVector3(const std::string& name, float* values) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    GL_CALL(glUniform3fv(location, 1, values));  // v=vector，表示数组输入
}

void Shader::setVector3(const std::string& name, glm::vec3 value) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    GL_CALL(glUniform3fv(location, 1, glm::value_ptr(value)));// 使用glm::value_ptr(value)来获取指针
}

// 设置Uniform变量：mat4类型（4x4矩阵，如MVP矩阵、模型变换矩阵）
void Shader::setMatrix4x4(const std::string& name, glm::mat4 value) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    if (location != -1) {
        // GL_FALSE：不转置（GLM与OpenGL均采用列优先矩阵存储，无需转置）
        GL_CALL(glUniformMatrix4fv(
            location, 1, GL_FALSE, glm::value_ptr(value)
        ));
    }
    else {
        std::cerr << "WARNING[Shader]: Uniform变量 '" << name << "' 未在着色器中定义" << std::endl;
    }
}

void Shader::setMatrix3x3(const std::string& name, glm::mat3 value) {
    GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
    // GL_FALSE：不转置（GLM与OpenGL均采用列优先矩阵存储，无需转置）
    GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value)));
}

// 私有方法：检查着色器编译/链接错误
void Shader::checkShaderErrors(GLuint target, std::string type) {
    GLint success = 0;
    char infoLog[1024] = { 0 };  // 存储错误日志的缓冲区

    if (type == "COMPILE") {
        // 检查着色器编译错误（target为着色器对象ID）
        GL_CALL(glGetShaderiv(target, GL_COMPILE_STATUS, &success));
        if (!success) {
            GL_CALL(glGetShaderInfoLog(target, 1024, nullptr, infoLog));
            std::cerr << "===================== Shader 编译错误 =====================" << std::endl;
            std::cerr << "错误日志: " << infoLog << std::endl;
            std::cerr << "提示：检查着色器语法（如版本号是否与OpenGL 4.6匹配）" << std::endl;
            std::cerr << "================================================================" << std::endl;
        }
    }
    else if (type == "LINK") {
        // 检查着色器程序链接错误（target为程序对象ID）
        GL_CALL(glGetProgramiv(target, GL_LINK_STATUS, &success));
        if (!success) {
            GL_CALL(glGetProgramInfoLog(target, 1024, nullptr, infoLog));
            std::cerr << "===================== Shader 链接错误 =======================" << std::endl;
            std::cerr << "错误日志: " << infoLog << std::endl;
            std::cerr << "提示：检查顶点/片段着色器的接口是否匹配（如out/in变量名）" << std::endl;
            std::cerr << "================================================================" << std::endl;
        }
    }
    else {
        std::cerr << "ERROR[Shader]: 错误检查类型无效！仅支持 'COMPILE' 或 'LINK'" << std::endl;
    }
}