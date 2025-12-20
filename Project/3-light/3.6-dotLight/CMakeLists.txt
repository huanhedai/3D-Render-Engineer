set(SOURCES
    "main.cpp"
    # 显式列出所有源文件
    "${PROJECT_SOURCE_DIR}/Project/glad.c"
)
add_executable(3.6-dotLight ${SOURCES})


# 允许链接不在当前目录构建的目标   MyLibrary    
cmake_policy(SET CMP0079 NEW)

# 在外面的CMakeLists中已经添加了全局的include路径和lib路径
# 链接第三方库
target_link_libraries(3.6-dotLight PRIVATE
    MyLibrary   # 自己创建的也放进来
    SDL2
    SDL2main
    SDL2test
    SDL2_image
    OPENGL32
)

# 复制 DLL 到输出目录
add_custom_command(TARGET 3.6-dotLight POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/SDL2.dll"
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/SDL2_image.dll"
        "${PROJECT_SOURCE_DIR}/thirdParty/dll/libtiff-5.dll"
        "$<TARGET_FILE_DIR:3.6-dotLight>"
)
