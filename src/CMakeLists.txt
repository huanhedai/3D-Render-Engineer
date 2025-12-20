# 收集所有 .cpp 和 .c 源文件（递归）
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.CPP")

#创建静态库
add_library(MyLibrary STATIC ${SOURCES})

# 全局宏定义（对当前目录及所有子目录的所有目标生效）
# 1. 先转换路径为跨平台格式（统一使用 / 分隔符）
file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/resource/shaders/" GLOBAL_SHADERS_DIR)
file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/resource/Textures/" GLOBAL_TEXTURES_DIR)  # 修正拼写
file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/resource/objs/" GLOBAL_OBJS_DIR)
file(TO_CMAKE_PATH "${PROJECT_SOURCE_DIR}/resource/fbxs/" GLOBAL_FBXS_DIR)

# 2. 定义全局宏（使用 add_compile_definitions 而非 compile_definitions）
add_compile_definitions(
    SHADER_DIR="${GLOBAL_SHADERS_DIR}"
    TEXTURE_DIR="${GLOBAL_TEXTURES_DIR}"
    OBJ_DIR="${GLOBAL_OBJS_DIR}"
    FBX_DIR="${GLOBAL_FBXS_DIR}"
)