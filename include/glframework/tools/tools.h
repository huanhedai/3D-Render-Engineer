#pragma once
#include "../core.h"
#include <vector>

// 定义 Shader 类型枚举（用于宏过滤）
enum class ShaderTarget {
    VERTEX,    // 仅顶点着色器（.vert）
    FRAGMENT,  // 仅片段着色器（.frag）
    ALL        // 所有着色器（默认）
};

// 增强版 Shader 宏结构体（支持类型过滤、带值/无值/注释）
struct ShaderMacro {
    std::string name;        // 宏名称（必填）
    std::string value;       // 宏数值（可选，空=无值宏）
    std::string comment;     // 注释（可选）
    ShaderTarget target;     // 作用范围（默认：所有着色器）

    // 构造函数（兼容原有使用方式，新增 target 参数默认值）
    // 1. 无值宏 + 默认作用范围（ALL）
    ShaderMacro(const std::string& _name)
        : name(_name), target(ShaderTarget::ALL) {
    }

    // 2. 无值宏 + 自定义作用范围
    ShaderMacro(const std::string& _name, ShaderTarget _target)
        : name(_name), target(_target) {
    }

    // 3. 无值宏 + 注释 + 默认作用范围
    ShaderMacro(const std::string& _name, const std::string& _comment)
        : name(_name), comment(_comment), target(ShaderTarget::ALL) {
    }

    // 4. 无值宏 + 注释 + 自定义作用范围
    ShaderMacro(const std::string& _name, const std::string& _comment, ShaderTarget _target)
        : name(_name), comment(_comment), target(_target) {
    }

    // 5. 带值宏 + 默认作用范围
    ShaderMacro(const std::string& _name, const std::string& _value, const std::string& _comment = "")
        : name(_name), value(_value), comment(_comment), target(ShaderTarget::ALL) {
    }

    // 6. 带值宏 + 自定义作用范围
    ShaderMacro(const std::string& _name, const std::string& _value, ShaderTarget _target, const std::string& _comment = "")
        : name(_name), value(_value), comment(_comment), target(_target) {
    }
};

class Tools {
public:
    // 传入一个矩阵，解构其中的位置信息，旋转信息，缩放信息
    static void decompose(glm::mat4 matrix, glm::vec3& position, glm::vec3& eularAngle, glm::vec3& scale);

    // 禁止拷贝构造和赋值（可选，进一步强化单例性）
    Tools(const Tools&) = delete;
    Tools& operator=(const Tools&) = delete;

    // 读取 Shader 文件内容，并插入 MAX_DIRECTION_LIGHTS 宏
    std::string readShaderSourceWithMacro(const std::string& filePath, int maxDirLights);

private:
    // 私有构造函数：禁止外部实例化，这个类是工具类，只能通过静态方法使用，无需关注实例状态。
    Tools() = default;


public:
    // 重载1：传入 ShaderMacro 结构体列表（推荐，支持类型过滤）
    static std::string readShaderSourceWithMacros(
        const std::string& filePath,
        const std::vector<ShaderMacro>& macros,
        bool skipExisting = true  // 是否跳过已存在的宏（避免重复）
    );

    // 重载2：传入原始宏字符串列表（无类型过滤，兼容灵活场景）
    static std::string readShaderSourceWithMacros(
        const std::string& filePath,
        const std::vector<std::string>& rawMacros,
        bool skipExisting = true
    );

private:
    // 辅助函数：根据文件路径判断 Shader 类型（.vert→VERTEX，.frag→FRAGMENT）
    static ShaderTarget getShaderTypeFromPath(const std::string& filePath);

    // 辅助函数：检查宏是否已存在于源码中
    static bool isMacroExists(const std::string& source, const std::string& macroName);

    // 辅助函数：将 ShaderMacro 转换为 GLSL 字符串
    static std::string macroToString(const ShaderMacro& macro);
};
