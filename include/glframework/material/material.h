#pragma once
#include "../core.h"
#include "../shader.h"

//使用C++的枚举类型
enum class MaterialType {
    PhongMaterial,
	WhiteMaterial,
    PBRMaterial,
	DepthMaterial,
	SreenMaterial
};

class Material {
public:
    Material();
    ~Material();

	void setShader(Shader* shader) {
		mShader = shader;
    }
    Shader* getShader() const {
        return mShader;
    }
public:
    MaterialType mType;

    // 深度检测相关
	bool mDepthTest{ true };        // 是否开启深度测试
	GLenum mDepthFunc{ GL_LESS };   // 深度测试函数
	bool mDepthWrite{ true };       // 是否允许写入深度缓冲

    // polygonOffset相关
    bool mPolygonOffset{ false };
    unsigned int mPolygonOffsetType{ GL_POLYGON_OFFSET_FILL };      // 针对多边形
    float mFactor{ 0.0f };
    float mUnit{ 0.0f };

    // stencil相关
    bool mStencilTest{ false };
        // glStencilOp
    unsigned int mSFail{ GL_KEEP };     // 模板测试失败怎么办
    unsigned int mZFail{ GL_KEEP };     // 模板测试通过，但深度测试没有怎么办
    unsigned int mZPass{ GL_KEEP };     // 模板和深度都通过怎么办
        // glStencilMask
    unsigned int mStencilMask{ 0xFF };  // 用于控制模板写入

        // glStencilFunc
    unsigned int mStencilFunc{ GL_ALWAYS };
    unsigned int mStencilRef{ 0 };
    unsigned int mStencilFuncMask{ 0xFF };

    // 颜色混合相关
	bool mBlend{ false };               // 是否开启颜色混合
	unsigned int mBlendSFactor{ GL_SRC_ALPHA };    // 源因子
	unsigned int mBlendDFactor{ GL_ONE_MINUS_SRC_ALPHA }; // 目标因子

	float mOpacity{ 1.0f }; // 透明度, 1.0为不透明，0.0为完全透明, 用于颜色混合

    // 面剔除
    bool mFaceCulling{ false };
	unsigned int mCullFace{ GL_BACK }; // 剔除哪一面
	unsigned int mFrontFace{ GL_CCW }; // 定义正面顶点的顺序，逆时针为正面

private:
    // shader着色器
    Shader* mShader{ nullptr };
};