#pragma once

#include <SDL2/SDL.h>
#include<glad/glad.h>	// 这个需要在 glfw3.h 这个前面

//GLM
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL		// 想要使用下面这个头文件就得先打开这个宏
#include<glm/gtx/string_cast.hpp>		// 用于输出矩阵
#include<glm/gtx/matrix_decompose.hpp>	// 用于解构矩阵
#include<glm/gtx/euler_angles.hpp>		// 用于解构欧拉角
#include<glm/gtx/quaternion.hpp>		// 四元数，用来表示旋转变化

#include <vector>
#include <map>
#include <iostream>