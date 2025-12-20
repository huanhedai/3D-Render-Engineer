#ifndef SHADER_H
#define SHADER_H

#include<glad/glad.h>	// 这个需要在 glfw3.h 这个前面
#include <glm/glm.hpp>  // 用于矩阵/向量类型（setMatrix4x4/setVector3）
#include <string>
#include "tools.h"

// Shader类：从文件加载并管理OpenGL着色器程序
class Shader {
public:
    // 构造函数：通过顶点/片段着色器文件路径创建shader程序
    Shader(const char* vertexPath, const char* fragmentPath);

    // 新增构造函数：按源码字符串加载（直接用工具类返回的字符串）
    Shader(const char* vertexSource, const char* fragmentSource, std::vector<ShaderMacro> macros);

    // 析构函数：释放OpenGL shader程序资源
    ~Shader();

    // 激活当前shader程序（渲染前调用）
    void begin();
    // 取消激活当前shader程序（渲染后可选调用）
    void end();

    // 设置Uniform变量：float类型
    void setFloat(const std::string& name, float value);
    // 设置Uniform变量：bool类型
    void setBool(const std::string& name, bool value);
    // 设置Uniform变量：vec3类型（重载1：单独传入x/y/z）
    void setVector3(const std::string& name, float x, float y, float z);
    // 设置Uniform变量：vec3类型（重载2：传入float数组）
    void setVector3(const std::string& name, float* values);
    void setVector3(const std::string& name, glm::vec3 value);
    // 设置Uniform变量：int类型
    void setInt(const std::string& name, int value);
    // 设置Uniform变量：mat4类型（4x4矩阵，如MVP矩阵）
    void setMatrix4x4(const std::string& name, glm::mat4 value);

    void setMatrix3x3(const std::string& name, glm::mat3 value);

private:
    GLuint mProgram;  // 存储OpenGL shader程序ID

    // 私有方法：检查shader编译/链接错误
    void checkShaderErrors(GLuint target, std::string type);
};

#endif // SHADER_H