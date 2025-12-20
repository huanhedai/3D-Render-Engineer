# 收集主程序源文件
file(GLOB_RECURSE SOURCES "*.cpp")

# 创建可执行文件
add_executable(1.7-model-framwork ${SOURCES} "${PROJECT_SOURCE_DIR}/Project/glad.c")

# 在外面的CMakeLists中已经添加了全局的include路径和lib路径
# 链接自定义库
target_link_libraries(1.7-model-framwork PRIVATE MyLibrary)

# 链接第三方库
target_link_libraries(1.7-model-framwork PRIVATE
    SDL2
    SDL2main
    SDL2test
    SDL2_image
)

# 复制 DLL 到输出目录
add_custom_command(
    TARGET 1.7-model-framwork
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/SDL2.dll"
        "$<TARGET_FILE_DIR:1.7-model-framwork>"
)
add_custom_command(
    TARGET 1.7-model-framwork
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/SDL2_image.dll"
        "$<TARGET_FILE_DIR:1.7-model-framwork>"
)
