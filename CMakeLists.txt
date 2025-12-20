# 设置 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(OpenGLProject LANGUAGES CXX C)


include_directories(${PROJECT_BINARY_DIR}) # 让代码能包含config.h

# 往项目中添加一个全局的预编译宏
# add_definitions(-DDEBUG)

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



# 设置全局包含路径（供所有子模块使用）# 添加自定义文件的 include 路径（供所有子模块使用）
# 首先添加根目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 递归查找 include 目录下的所有子目录并添加到包含路径
file(GLOB_RECURSE SUB_INCLUDE_DIRS 
     LIST_DIRECTORIES true 
     "${PROJECT_SOURCE_DIR}/include/*"
)

# 遍历所有找到的路径，只添加目录类型的路径
foreach(DIR ${SUB_INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

###############################################################################################

# 添加第三方库路径（头文件 + lib库等）
include_directories(${PROJECT_SOURCE_DIR}/thirdParty/include)

# target_include_directories(myapp PRIVATE ${PROJECT_SOURCE_DIR}/thirdParty/include) 仅为myapp这个可执行文件设置include包含路径, target_link_directories同理

link_directories(${PROJECT_SOURCE_DIR}/thirdParty/lib)


###############################################################################################
# 添加子目录
# add_subdirectory 的主要作用是将一个子目录添加到构建系统中，并执行该目录下的 CMakeLists.txt 文件。
# 编译用户自定义库
add_subdirectory(src)

# 框架相关子目录（公共路径提取）
set(FRAMEWORK_DIR "Project/1-framework")
set(FRAMEWORK_SUBDIRS
    "1.1-get-start"
    "1.2-get-Application"
    "1.3-use-shader"
    "1.4-use-texture"
    "1.5-use-camera"
    "1.6-event-callback"
    "1.7-model-framwork"
    "frame_test"
)
foreach(subdir IN LISTS FRAMEWORK_SUBDIRS)
    add_subdirectory("${FRAMEWORK_DIR}/${subdir}")
endforeach()

# OBJ文件读取相关
add_subdirectory("Project/2-read-objfile/obj_test")

# 光照相关子目录（公共路径提取）
set(LIGHT_DIR "Project/3-light")
set(LIGHT_SUBDIRS
    "3.1-set-light"
    "3.2-duophy"
    "3.3-Materials"
    "3.4-Demo"
    "3.5-SpecularMask"
    "3.6-dotLight"
    "3.7-spotLight"
    "3.8-MultiLight"
    "3.9-LightArray"
    "3.10-Scene-IMGUI"
    "3.11-PBR"
    "3.12-PBR-Heart"
    "3.13-Blinn-Phong-Heart"
)
foreach(subdir IN LISTS LIGHT_SUBDIRS)
    add_subdirectory("${LIGHT_DIR}/${subdir}")
endforeach()

# 高级功能相关子目录（公共路径提取）
set(LIGHT_DIR "Project/4-Advanced")
set(LIGHT_SUBDIRS
    "4.1-Depth-test"
    "4.2-Stencil-test"
    "4.3-Color-Blending"
    "4.4-PBR-Blend"
    "4.5-Phong-Blend"
    "4.6-FBO"
    "4.7-OIT"
)
foreach(subdir IN LISTS LIGHT_SUBDIRS)
    add_subdirectory("${LIGHT_DIR}/${subdir}")
endforeach()