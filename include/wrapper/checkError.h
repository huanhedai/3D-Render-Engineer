#ifndef CHECK_ERROR_H
#define CHECK_ERROR_H

// 1. 先声明 checkError() 函数（必须在宏定义之前，因为宏会调用它）
void checkError();

// 2. 宏定义：跨多行必须加续行符 \，确保编译器解析为一个整体
#ifdef DEBUG
#define GL_CALL(function) \
	do { \
		function; \
		checkError(); \
	} while (0)
#else
// 非DEBUG模式：直接执行函数，不检查错误（同样确保语法正确）
#define GL_CALL(function) function
#endif // DEBUG

#endif // CHECK_ERROR_H