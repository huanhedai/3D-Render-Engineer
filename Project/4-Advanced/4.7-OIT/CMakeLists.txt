# 定义目标名称变量（后续修改只需改这里）
set(TARGET_NAME "4.7-OIT")

set(SOURCES
    "main.cpp"
    # 显式列出所有源文件
    "${PROJECT_SOURCE_DIR}/Project/glad.c"
)

# 创建可执行目标
add_executable(${TARGET_NAME} ${SOURCES})

##################################################################################

# 允许链接非当前目录构建的目标（添加注释说明用途）
# CMP0079: 允许target_link_libraries()链接不在当前目录的目标
cmake_policy(SET CMP0079 NEW)

##################################################################################
# 整理第三方库和自定义库到变量（方便统一管理）
set(LIBS_TO_LINK
    MyLibrary       # 自定义库
    SDL2            # SDL2核心库
    SDL2main        # SDL2主程序支持
    SDL2test        # SDL2测试库
    SDL2_image      # SDL2图像库
    OPENGL32        # OpenGL库
)

# 链接库（使用变量简化命令）
target_link_libraries(${TARGET_NAME} PRIVATE ${LIBS_TO_LINK})

##################################################################################

# 复制 DLL 到输出目录
add_custom_command(TARGET 4.7-OIT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/SDL2.dll"
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/SDL2_image.dll"
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/libtiff-5.dll"
        "$<TARGET_FILE_DIR:4.7-OIT>"
)
