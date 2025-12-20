#include "texture.h"
#include <SDL2/SDL_image.h>
#include <glad/glad.h>
#include <stdexcept>
#include <cstring>
#include<iostream>

// 必须在cpp中初始化静态成员
bool Texture::sInitialized = false;
std::map<std::string, Texture*> Texture::mTextureCache{};

Texture* Texture::createTexture(const std::string& path, unsigned int unit) {
    // 检查缓存中是否已有该纹理
    auto it = mTextureCache.find(path);
    if (it != mTextureCache.end()) {
        return it->second; // 返回已缓存的纹理
    }
    // 缓存中没有，创建新纹理并缓存
    Texture* newTexture = new Texture(path, unit);
    mTextureCache[path] = newTexture;
    return newTexture;
}

Texture* Texture::createTextureFromMemory(
    const std::string& path,
    unsigned int unit,
    unsigned char* dataIn,
    uint32_t widthIn,
    uint32_t heightIn
) {
    // 检查缓存中是否已有该纹理
    auto it = mTextureCache.find(path);
    if (it != mTextureCache.end()) {
        return it->second; // 返回已缓存的纹理
    }

    // 缓存中没有，创建新纹理并缓存
    Texture* newTexture = new Texture(unit, dataIn, widthIn, heightIn);
    mTextureCache[path] = newTexture;
    return newTexture;
}

Texture::Texture(
    unsigned int unit,
    unsigned char* dataIn,
    uint32_t widthIn,
    uint32_t heightIn
) : mUnit(unit) { // 初始化纹理单元成员变量
    // 计算图片数据大小（保留原有逻辑）
    uint32_t dataInSize = 0;
    if (!heightIn) {
        dataInSize = widthIn; // 压缩格式：widthIn代表图片大小
    }
    else {
        dataInSize = widthIn * heightIn * 4; // 非压缩格式：RGBA每个像素4字节
    }

    // 1. 从内存数据创建SDL_RWops（用于SDL_image加载内存中的图像）
    SDL_RWops* rwOps = SDL_RWFromMem(dataIn, dataInSize);
    if (!rwOps) {
        throw std::runtime_error("Failed to create RWops from memory: " + std::string(SDL_GetError()));
    }

    // 2. 使用SDL_image从内存加载图像数据
    SDL_Surface* surface = IMG_Load_RW(rwOps, 1); // 第二个参数1表示自动释放rwOps
    if (!surface) {
        throw std::runtime_error("Failed to load texture from memory: " + std::string(IMG_GetError()));
    }

    // 3. 转换为RGBA32格式（与OpenGL兼容）
    SDL_Surface* rgbaSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface); // 释放原始表面
    if (!rgbaSurface) {
        throw std::runtime_error("Failed to convert surface format: " + std::string(SDL_GetError()));
    }

    // 保存纹理尺寸
    mWidth = rgbaSurface->w;
    mHeight = rgbaSurface->h;

    // 4. 翻转Y轴（SDL原点在左上角，OpenGL在左下角）
    flipSurfaceVertically(rgbaSurface);

    // 5. 生成并配置纹理
    glGenTextures(1, &mTexture);
    glActiveTexture(GL_TEXTURE0 + mUnit);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // 6. 设置像素对齐方式
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 7. 传输纹理数据到GPU
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        mWidth,
        mHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgbaSurface->pixels
    );

    // 8. 释放CPU内存
    SDL_FreeSurface(rgbaSurface);

    // 9. 设置纹理过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // 10. 设置纹理包裹方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(const std::string& path, unsigned int unit)
	: mUnit(unit), mTexture(0), mWidth(0), mHeight(0) {    
    // mTexture初始设为 0 是规范的 “未初始化” 状态，不会导致两个纹理共用同一个 ID。
    // 0 是「默认纹理」的 ID（绑定 ID=0 等价于 “解绑当前纹理”），不是用户生成的有效纹理 ID；
    // mTexture 初始赋值为 0 只是「未初始化标记」，最终会被 glGenTextures 覆盖为「非 0 的有效纹理 ID」 

    if(path.substr(path.size() - 4,path.size()) == ".tif") {
        // 确保SDL_image正确初始化并支持TIF格式
        if (!initImageFormats()) {
            throw std::runtime_error("SDL_image初始化失败，不支持TIF格式");
        }
    }

    // 1. 使用SDL_image加载图片
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        throw std::runtime_error("Failed to load texture: " + std::string(IMG_GetError()) +
            " (Path: " + path + ")");
    }

    // 2. 转换为RGBA32格式（与OpenGL兼容）
    SDL_Surface* rgbaSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface); // 释放原始表面
    if (!rgbaSurface) {
        throw std::runtime_error("Failed to convert surface format: " + std::string(SDL_GetError()));
    }

    // 保存纹理尺寸
    mWidth = rgbaSurface->w;
    mHeight = rgbaSurface->h;

    // 3. 翻转Y轴（SDL原点在左上角，OpenGL在左下角）
    flipSurfaceVertically(rgbaSurface);

    // 4. 生成并配置纹理
    glGenTextures(1, &mTexture);
    glActiveTexture(GL_TEXTURE0 + mUnit);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // 5. 设置像素对齐方式
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 6. 传输纹理数据到GPU
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        mWidth,
        mHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        rgbaSurface->pixels
    );

    // 7. 释放CPU内存
    SDL_FreeSurface(rgbaSurface);

    // 8. 设置纹理过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // 9. 设置纹理包裹方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Texture::Texture(unsigned int width, unsigned int height, unsigned int unit)
	: mUnit(unit), mTexture(0), mWidth(width), mHeight(height) {    
    // 为FBO创建空纹理，生成ID，并且将该纹理绑定到指定单元

	// 1. 生成并配置纹理
	glGenTextures(1, &mTexture);
    glActiveTexture(GL_TEXTURE0 + mUnit);
	glBindTexture(GL_TEXTURE_2D, mTexture); // 直接绑定，无需激活单元

	// 开辟空的纹理内存（不初始化数据）
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        mWidth,
        mHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        NULL
	);      // glTexImage2D：创建一个空的纹理对象，数据为NULL

	// 2. 设置纹理过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// 3. 设置纹理包裹方式，一般不用，因为作为画布我们也不会超过0-1采样
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // 4. 解绑纹理（避免后续误操作）
    glBindTexture(GL_TEXTURE_2D, 0);
}


Texture::~Texture() {
    if (mTexture != 0) {
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }
}

void Texture::bind() {
    glActiveTexture(GL_TEXTURE0 + mUnit);
    glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture::flipSurfaceVertically(SDL_Surface* surface) {
    if (!surface) return;

    int pitch = surface->pitch;
    unsigned char* pixels = static_cast<unsigned char*>(surface->pixels);
    unsigned char* temp = new unsigned char[pitch];
    int halfHeight = surface->h / 2;

    for (int y = 0; y < halfHeight; ++y) {
        unsigned char* row1 = pixels + y * pitch;
        unsigned char* row2 = pixels + (surface->h - 1 - y) * pitch;

        // 交换两行像素数据
        std::memcpy(temp, row1, pitch);
        std::memcpy(row1, row2, pitch);
        std::memcpy(row2, temp, pitch);
    }

    delete[] temp;
}

bool Texture::initImageFormats() {
    if (sInitialized) return true;

    // 在初始化前添加，查看SDL_image支持的格式
    std::cout << "SDL_image支持的格式: ";
    if (IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG) std::cout << "JPG ";
    if (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) std::cout << "PNG ";
    if (IMG_Init(IMG_INIT_TIF) & IMG_INIT_TIF) std::cout << "TIF ";
    std::cout << std::endl;

    // 显式初始化支持的格式：JPG, PNG, TIF
    int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF; // 按位或，都为 1 结果位才为 1，如果都能加载那结果是0111
    if ((IMG_Init(flags) & flags) != flags) {
        std::cerr << "SDL_image初始化错误: " << IMG_GetError() << std::endl;
        return false;
    }

    sInitialized = true;
    return true;
}

