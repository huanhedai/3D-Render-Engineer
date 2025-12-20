#include "tools.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <algorithm>

// 辅助函数：转义正则表达式中的特殊字符
static std::string escapeRegexSpecialChars(const std::string& input) {
    const std::string specialChars = R"(\^$.|?*+()[]{})";
    std::string result;
    for (char c : input) {
        if (specialChars.find(c) != std::string::npos) {
            result += '\\'; // 对特殊字符添加转义符
        }
        result += c;
    }
    return result;
}

void Tools::decompose(
	glm::mat4 matrix, 
	glm::vec3& position, 
	glm::vec3& eularAngle, 
	glm::vec3& scale)
{	
	// 四元数，用来表示旋转变化
	glm::quat quaternion;
	glm::vec3 skew;
	glm::vec4 perspective;

	glm::decompose(matrix, scale, quaternion, position, skew, perspective);

	// 将四元数-欧拉角
	glm::mat4 rotation = glm::toMat4(quaternion);
	glm::extractEulerAngleXYX(rotation, eularAngle.x, eularAngle.y, eularAngle.z);

	// 解构出来的是弧度，要转为角度
	eularAngle.x = glm::degrees(eularAngle.x);
	eularAngle.y = glm::degrees(eularAngle.y);
	eularAngle.z = glm::degrees(eularAngle.z);
}

// 辅助函数：获取文件后缀（忽略大小写）
static std::string getFileExtension(const std::string& filePath) {
    auto dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    std::string ext = filePath.substr(dotPos);
    // 转为小写，兼容 .VERT/.FRAG 等大写后缀
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

// 辅助函数：根据文件路径判断 Shader 类型
ShaderTarget Tools::getShaderTypeFromPath(const std::string& filePath) {
    std::string ext = getFileExtension(filePath);
    if (ext == ".vert") {
        return ShaderTarget::VERTEX;
    }
    else if (ext == ".frag") {
        return ShaderTarget::FRAGMENT;
    }
    else {
        // 未知后缀，默认按 ALL 处理（不过滤）
        std::cerr << "[Tools Warning] Unknown shader file extension: " << filePath << ", treat as ALL target." << std::endl;
        return ShaderTarget::ALL;
    }
}

// 辅助函数：检查宏是否已存在（替换原正则版本，无兼容性问题）
bool Tools::isMacroExists(const std::string& source, const std::string& macroName) {
    // 构造目标字符串："#define 宏名"（注意后面要跟空格或行尾）
    std::string targetPrefix = "#define " + macroName;
    size_t targetLen = targetPrefix.length();

    // 按行读取源码
    std::istringstream sourceStream(source);
    std::string line;
    while (std::getline(sourceStream, line)) {
        // 步骤1：去掉行开头的所有空白符（空格、Tab）
        size_t firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace == std::string::npos) {
            continue; // 空行，跳过
        }
        std::string trimmedLine = line.substr(firstNonSpace);

        // 步骤2：检查行是否以 "#define 宏名" 开头
        if (trimmedLine.size() < targetLen) {
            continue;
        }
        if (trimmedLine.substr(0, targetLen) != targetPrefix) {
            continue;
        }

        // 步骤3：检查宏名后是否是空白符或行尾（避免匹配 "宏名_XXX" 等情况）
        if (trimmedLine.size() == targetLen || isspace(trimmedLine[targetLen])) {
            return true; // 找到已存在的宏
        }
    }

    return false; // 未找到
}

// 辅助函数：ShaderMacro 转 GLSL 字符串
std::string Tools::macroToString(const ShaderMacro& macro) {
    std::stringstream ss;
    ss << "#define " << macro.name;
    if (!macro.value.empty()) {
        ss << " " << macro.value;  // 带值宏：#define MAX_LIGHTS 8
    }
    if (!macro.comment.empty()) {
        ss << " // " << macro.comment;  // 带注释
    }
    ss << "\n";  // 统一换行，兼容所有编译器
    return ss.str();
}

// 重载1：处理 ShaderMacro 列表（支持类型过滤）
std::string Tools::readShaderSourceWithMacros(
    const std::string& filePath,
    const std::vector<ShaderMacro>& macros,
    bool skipExisting
) {
    // 1. 读取 Shader 文件内容
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Tools Error] Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << file.rdbuf();
    file.close();
    std::string shaderSource = shaderStream.str();

    // 2. 获取当前 Shader 类型（.vert/.frag）
    ShaderTarget currentShaderType = getShaderTypeFromPath(filePath);

    // 3. 生成要插入的宏字符串（过滤类型 + 去重）
    std::stringstream macrosStream;
    for (const auto& macro : macros) {
        // 类型过滤：仅保留 目标类型为 ALL 或 与当前 Shader 类型匹配 的宏
        bool isTargetMatch = (macro.target == ShaderTarget::ALL) || (macro.target == currentShaderType);
        if (!isTargetMatch) {
            // 调试信息：可选开启，查看被过滤的宏
            // std::cout << "[Tools Info] Macro '" << macro.name << "' skipped (target mismatch) for " << filePath << std::endl;
            continue;
        }

        // 去重过滤：如果已存在且开启去重，跳过
        if (skipExisting && isMacroExists(shaderSource, macro.name)) {
            std::cout << "[Tools Info] Macro '" << macro.name << "' already exists in " << filePath << ", skipped." << std::endl;
            continue;
        }

        // 符合条件，添加到宏字符串
        macrosStream << macroToString(macro);
    }
    std::string macrosStr = macrosStream.str();
    if (macrosStr.empty()) {
        return shaderSource;  // 无符合条件的宏，返回原源码
    }

    // 4. 确定插入位置：#version 之后（GLSL 强制要求 #version 是第一行有效代码）
    size_t versionPos = shaderSource.find("#version");
    if (versionPos != std::string::npos) {
        // 兼容 Windows(\r\n) 和 Linux(\n) 换行符
        size_t newlinePos = shaderSource.find("\n", versionPos);
        if (newlinePos != std::string::npos) {
            // 插入到 #version 下一行
            shaderSource.insert(newlinePos + 1, macrosStr);
        }
        else {
            // #version 是最后一行，追加到末尾（加换行避免拼接错误）
            shaderSource += "\n" + macrosStr;
        }
    }
    else {
        // 无 #version 声明，插入到文件开头
        shaderSource = macrosStr + shaderSource;
    }

    return shaderSource;
}

// 重载2：处理原始宏字符串（无类型过滤，保持原有灵活度）
std::string Tools::readShaderSourceWithMacros(
    const std::string& filePath,
    const std::vector<std::string>& rawMacros,
    bool skipExisting
) {
    // 1. 读取文件内容
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[Tools Error] Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream shaderStream;
    shaderStream << file.rdbuf();
    file.close();
    std::string shaderSource = shaderStream.str();

    // 2. 生成宏字符串（仅对 #define 做去重）
    std::stringstream macrosStream;
    std::regex definePattern(R"(^\s*#define\s+(\w+))", std::regex_constants::grep);
    for (const auto& rawMacro : rawMacros) {
        // 仅对 #define 类型宏做去重检查
        std::smatch match;
        if (skipExisting && std::regex_search(rawMacro, match, definePattern) && match.size() > 1) {
            std::string macroName = match[1].str();
            if (isMacroExists(shaderSource, macroName)) {
                std::cout << "[Tools Info] Macro '" << macroName << "' already exists in " << filePath << ", skipped." << std::endl;
                continue;
            }
        }
        macrosStream << rawMacro << "\n";
    }
    std::string macrosStr = macrosStream.str();
    if (macrosStr.empty()) {
        return shaderSource;
    }

    // 3. 插入位置逻辑（和重载1一致）
    size_t versionPos = shaderSource.find("#version");
    if (versionPos != std::string::npos) {
        size_t newlinePos = shaderSource.find("\n", versionPos);
        if (newlinePos != std::string::npos) {
            shaderSource.insert(newlinePos + 1, macrosStr);
        }
        else {
            shaderSource += "\n" + macrosStr;
        }
    }
    else {
        shaderSource = macrosStr + shaderSource;
    }

    return shaderSource;
}