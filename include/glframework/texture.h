#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <SDL2/SDL.h>
#include <glad/glad.h>  // 解决GLuint未定义问题
#include <map>
#include <windows.h> // Ensure this header is included for APIENTRY definition
#include <GL/gl.h> // Include OpenGL header for GLuint

class Texture {
private:
    unsigned int mTexture;  // OpenGL纹理ID
    unsigned int mUnit;     // 纹理单元
    int mWidth;             // 纹理宽度
    int mHeight;            // 纹理高度

    //注意：静态！！属于类的不属于某个对象
    static std::map<std::string, Texture*> mTextureCache;
    // 上述结构用来缓存已经加载的纹理，避免重复加载同一张纹理

private:
    // 辅助函数：垂直翻转SDL表面
    void flipSurfaceVertically(SDL_Surface* surface);

    // 初始化SDL_image支持的图像格式（包括TIF）
    static bool initImageFormats();
    // 静态成员：确保SDL_image只初始化一次
    static bool sInitialized;

public:
    // 静态工厂方法
    // 从硬盘读取文件创建纹理，使用纹理缓存避免重复加载
    static Texture* createTexture(const std::string& path, unsigned int unit);
    // 从内存数据创建纹理
    static Texture* createTextureFromMemory(
        const std::string& path,
        unsigned int unit,
        unsigned char* dataIn,
        uint32_t widthIn,
        uint32_t heightIn
    );

    // 构造函数：从文件路径创建纹理，从硬盘中读取数据创建纹理
    Texture(const std::string& path, unsigned int unit);

    // 构造函数：从内存中读取数据创建纹理
    Texture(
        unsigned int unit,
        unsigned char* dataIn,
        uint32_t widthIn,
        uint32_t heightIn
    );

    // 引进FBO时用的构造函数
    Texture(unsigned int width, unsigned int height, unsigned int unit);

    GLuint getID() const { return mTexture; } // Ensure GLuint is recognized

    // 析构函数：释放纹理资源
    ~Texture();

    // 绑定纹理到指定单元
    void bind();

    // 设置纹理单元
    void setUnit(unsigned int unit) { mUnit = unit; }

    // 获取纹理宽度
    int getWidth() const { return mWidth; }

    // 获取纹理高度
    int getHeight() const { return mHeight; }

    // 获取纹理单元
    unsigned int getUnit() const { return mUnit; }
};

#endif // TEXTURE_H