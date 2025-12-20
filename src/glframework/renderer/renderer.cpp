#include "renderer.h"
#include "../material/phongMaterial.h"
#include "../material/whiteMaterial.h"
#include "../material/PBRMaterial.h"
#include "../material/screenMaterial.h"

#include<iostream>
#include <string>
#include <algorithm>

Renderer::Renderer(){}

// 构造函数：通过传入各shader的顶点和片段路径来选择性创建
// 未传入有效路径的shader将保持nullptr状态
Renderer::Renderer(const std::string& phongVertexPath, const std::string& phongFragmentPath,
    const std::string& whiteVertexPath, const std::string& whiteFragmentPath,
    const std::string& pbrVertexPath, const std::string& pbrFragmentPath)
{
    // 只有当顶点和片段路径都有效的时候才创建对应的shader
    if (!phongVertexPath.empty() && !phongFragmentPath.empty()) {
        mPhongShader = new Shader(phongVertexPath.c_str(), phongFragmentPath.c_str());
    }
    if (!whiteVertexPath.empty() && !whiteFragmentPath.empty()) {
        mWhiteShader = new Shader(whiteVertexPath.c_str(), whiteFragmentPath.c_str());
    }
    if (!pbrVertexPath.empty() && !pbrFragmentPath.empty()) {
        mPBRShader = new Shader(pbrVertexPath.c_str(), pbrFragmentPath.c_str());
    }
}



Renderer::Renderer(const std::string& vertexshaderPath, const std::string& fragmentshaderPath)
{
    //mPhongShader = new Shader(vertexshaderPath.c_str(), fragmentshaderPath.c_str());
    mPhongShader = new Shader(vertexshaderPath.c_str(), fragmentshaderPath.c_str());
    mWhiteShader = new Shader("E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/whiteShader/white.vert",
        "E:/IT-Furnace/OpenGl/mindray/Framework/resource/shaders/whiteShader/white.frag");
}

Renderer::~Renderer()
{
    delete mPBRShader;
    delete mWhiteShader;
    delete mPhongShader;
}

Shader* Renderer::pickShader(MaterialType type) {
    Shader* result = nullptr;

    switch (type) {
    case MaterialType::PhongMaterial:
        result = mPhongShader;
        break;
	case MaterialType::WhiteMaterial:
		result = mWhiteShader;
		break;
    case MaterialType::PBRMaterial:
        result = mPBRShader;
        break;
    case MaterialType::SreenMaterial:
		// No shader assigned for ScreenMaterial yet
        break;
    default:
        std::cout << "Unknown material type to pick shader" << std::endl;
        break;
    }

    return result;
}

void Renderer::setClearColor(glm::vec3 color) {
    glClearColor(color.r, color.g, color.b, 1.0);
}


void Renderer::setDepthState(Material* material) {
    // 设置深度测试相关状态
    if (material->mDepthTest) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(material->mDepthFunc);       // 深度测试函数
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }

    if (material->mDepthWrite) {
        glDepthMask(GL_TRUE);   // 允许写入深度缓冲
    }
    else {
        glDepthMask(GL_FALSE);  // 不允许写入深度缓冲
    }
}
void Renderer::setPolygonOffsetState(Material* material) {
    // 检测polygonOffset
    if (material->mPolygonOffset) {
        glEnable(material->mPolygonOffsetType);
        glPolygonOffset(material->mFactor, material->mUnit);
        // 第一个参数是与片元深度梯度相乘的因子，同一个物体，深度值变化越快，偏移得越多
        // 第二个参数是物体整体的偏移量
    }
    else {
        glDisable(GL_POLYGON_OFFSET_FILL);      // 关闭多边形的 offset
        glDisable(GL_POLYGON_OFFSET_LINE);      // 关闭线的 offset
    }
}


void Renderer::setStencilState(Material* material) {
    if (material->mStencilTest) {
        glEnable(GL_STENCIL_TEST);
        //unsigned int mSFail{ GL_KEEP };     // 模板测试失败怎么办
        //unsigned int mZFail{ GL_KEEP };     // 模板测试通过，但深度测试没有怎么办
        //unsigned int mZPass{ GL_KEEP };     // 模板和深度都通过怎么办
        //// glStencilMask
        //unsigned int mStencilMask{ 0xFF };  // 用于控制模板写入

        //// glStencilFunc
        //unsigned int mStencilFunc{ GL_ALWAYS };
        //unsigned int mStencilRef{ 0 };
        //unsigned int mStencilFuncMask{ 0xFF };
        glStencilFunc(material->mStencilFunc, material->mStencilRef, material->mStencilFuncMask);   // 如何测试
        glStencilOp(material->mSFail, material->mZFail, material->mZPass);  // 成功、失败后如何
        glStencilMask(material->mStencilMask);  // 控制缓冲写入
    }
    else {
        glDisable(GL_STENCIL_TEST);
    }
}

void Renderer::setBlenderState(Material* material) {
    if (material->mBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(material->mBlendSFactor, material->mBlendDFactor);
    }
    else {
        glDisable(GL_BLEND);
    }
}

void Renderer::setFaceCullingState(Material* material) {
    if(material->mFaceCulling) {
        glEnable(GL_CULL_FACE);
        glFrontFace(material->mFrontFace);   
        glCullFace(material->mFaceCulling);
    }
    else {
        glDisable(GL_CULL_FACE);
	}
}

void Renderer::projectObject(Object* obj) {
    if(obj->getType() == ObjectType::Mesh) {
        Mesh* mesh = static_cast<Mesh*>(obj);
        Material* material = mesh->mMaterial;
		if (material->mBlend) {     // 开启了透明，放入透明队列
            mTransparentObjects.push_back(mesh);
        }
        else {
            mOpaqueObjects.push_back(mesh);
        }
	}

    auto children = obj->getChildren();
    for (auto child : children) {
        projectObject(child);
	}
}
void Renderer::render(
    const std::vector<Mesh*>& meshes,
    Camera* camera,
    const std::vector<DirectionalLight*>& directionLights,
    const std::vector<PointLight*>& pointLights,
    const std::vector<SpotLight*>& spotLights,
    AmbientLight* ambLight,
	unsigned int fbo   // 默认是0，0号是系统默认的FBO, 这里传入非0值表示渲染到自定义FBO；0号表示渲染到屏幕
) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);     // 绑定到指定的FBO上进行渲染

    //1 设置当前帧绘制的时候，opengl的必要状态机参数
	// 一开始全部打开深度测试，防止因为前面的绘制操作关闭了写入深度缓存导致的无法glClear深度缓存
	// 后面再去针对物体设置深度测试相关状态。
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // 深度测试功能：如果片段的深度值小于存储的深度值，则通过。
    glDepthMask(GL_TRUE);       // 允许写入深度缓存

    // 默认情况下是关闭offset的，只有对特定物体我们才开启
    glDisable(GL_POLYGON_OFFSET_FILL);      // 关闭多边形的 offset
    glDisable(GL_POLYGON_OFFSET_LINE);      // 关闭线的 offset

    // 开启测试、设置基本写入状态，打开模板测试写入
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);    // 开启模板测试写入，保证了模板缓冲可以被清理

    // 默认颜色混合
    glDisable(GL_BLEND);    // 默认关闭，耗性能

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //GL_STENCIL_BUFFER_BIT 清理模板缓冲

    //3 遍历mesh进行绘制
    for (int i = 0; i < meshes.size(); i++) {
        auto mesh = meshes[i];
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        setDepthState(material);
        setPolygonOffsetState(material);
        setStencilState(material);
		setBlenderState(material);
		setFaceCullingState(material);

        /*
        if (i == 0) {
            glStencilFunc(GL_ALWAYS, 1, 0xFF);      // 如何测试
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);      // 成功、失败后如何
            glStencilMask(0xFF);    // 控制缓冲写入
        }
        else if (i == 1) {
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);    // 逻辑优势，不允许它写入，不写也对，逻辑上表示边界覆盖的区域不应该写入 1
        }
        else if (i == 2) {
            glStencilFunc(GL_ALWAYS, 1, 0xFF);      // 如何测试
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);      // 成功、失败后如何
            glStencilMask(0xFF);    // 控制缓冲写入
        }
        else if (i == 3) {
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);    // 逻辑优势，不允许它写入，不写也对，逻辑上表示边界覆盖的区域不应该写入 1
        }
        */
        //1 决定使用哪个Shader
        Shader* shader = material->getShader();
        if (shader == nullptr) {
            throw std::runtime_error("The Shader is nullptr.");
        }

        //2 更新shader的uniform
        shader->begin();
        /* ----------------------- 先传输通用的uniform变量----------------------------*/
        /*    光源参数的uniform更新    */ 
        // spotlight的更新
        for (int i = 0; i < spotLights.size(); i++) {
            auto spotLight = spotLights[i];
            std::string baseName = "spotLights[";
            baseName.append(std::to_string(i));
            baseName.append("]");

            shader->setVector3(baseName + ".position", spotLight->getPosition());
            shader->setVector3(baseName + ".color", spotLight->getColor());
            shader->setFloat(baseName + ".specularIntensity", spotLight->getSpecularIntensity());
            shader->setFloat(baseName + ".k2", spotLight->mK2);
            shader->setFloat(baseName + ".k1", spotLight->mK1);
            shader->setFloat(baseName + ".kc", spotLight->mKc);
        }


        // directionallight的更新
        for (int i = 0; i < directionLights.size(); i++) {
            auto directionLight = directionLights[i];
            std::string baseName = "directionLights[";
            baseName.append(std::to_string(i));
            baseName.append("]");

            shader->setVector3(baseName + ".direction", directionLight->getDirection());
            shader->setVector3(baseName + ".color", directionLight->getColor());
            shader->setFloat(baseName + ".specularIntensity", directionLight->getSpecularIntensity());
            shader->setVector3(baseName + ".ambient", directionLight->getAmbient());
            shader->setVector3(baseName + ".diffuse", directionLight->getDiffuse());
            shader->setVector3(baseName + ".specular", directionLight->getSpecular());
        }

        // pointlight的更新
        for (int i = 0; i < pointLights.size(); i++) {
            auto pointLight = pointLights[i];
            std::string baseName = "pointLights[";
            baseName.append(std::to_string(i));
            baseName.append("]");

            shader->setVector3(baseName + ".position", pointLight->getPosition());
            shader->setVector3(baseName + ".color", pointLight->getColor());
            shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
            shader->setFloat(baseName + ".k2", pointLight->getK2());
            shader->setFloat(baseName + ".k1", pointLight->getK1());
            shader->setFloat(baseName + ".kc", pointLight->getKc());

            shader->setVector3(baseName + ".ambient", pointLight->getAmbient());
            shader->setVector3(baseName + ".diffuse", pointLight->getDiffuse());
            shader->setVector3(baseName + ".specular", pointLight->getSpecular());
        }

        // 环境光配置
        shader->setVector3("ambientLight.color", ambLight->getColor());
        shader->setFloat("ambientLight.Intensity", ambLight->getIntensity());

        /*    相机的uniform更新    */
        shader->setVector3("cameraPosition", camera->mPosition);
        shader->setFloat("far", camera->mFar);
        shader->setFloat("near", camera->mNear);

        /*    MVP矩阵与normalMatrix    */
        shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
        shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
        shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

        auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
        shader->setMatrix3x3("normalMatrix", normalMatrix);

        /*     物体基本材质    */
        shader->setFloat("opacityUniform", 1.0f);   // 透明度（float类型，默认1.0f，完全不透明）

        /* Phong */
        shader->setFloat("ambientIntensity", 1.0f);
        shader->setVector3("ambientColorUniform", glm::vec3(1.0, 1.0, 1.0));
        shader->setFloat("diffuseIntensity", 1.0f);
        shader->setVector3("diffuseColorUniform", glm::vec3(1.0, 1.0, 1.0));

        shader->setFloat("specularIntensity", 1.0f);    // 高光强度（float类型，默认1.0f）
        shader->setFloat("specularPowerUniform", 32.0f);    // 高光幂次（float类型，默认32.0f，控制高光光斑大小，值越大光斑越集中）
        shader->setVector3("specularColorUniform", glm::vec3(1.0, 1.0, 1.0));

        

        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            // 绑定mDiffuse到单元 phongMat->mDiffuse->getUnit()，并告诉Shader
            phongMat->mDiffuse->bind();     // 绑定纹理ID - mTexture，到对应的纹理单元 mUnit
            shader->setInt("sampler", phongMat->mDiffuse->getUnit());	// 绑定采样器到纹理单元
            // specular 贴图
            if (phongMat->mSpecularMask != nullptr) {
                phongMat->mSpecularMask->bind();     // 绑定纹理ID - mTexture，到对应的纹理单元 mUnit
                shader->setInt("specularMaskSampler", phongMat->mSpecularMask->getUnit());	// 绑定采样器到纹理单元
            }
            shader->setVector3("material.ambient", phongMat->getAmbientColor());
            shader->setVector3("material.diffuse", phongMat->getDiffuseColor());
            shader->setVector3("material.specular", phongMat->getSpecularColor());
            shader->setFloat("material.shiness", phongMat->getShiness());

            shader->setFloat("material.shiness", phongMat->getShiness());
            shader->setFloat("opacity", phongMat->mOpacity);    // 透明度
            
            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        case MaterialType::PBRMaterial: {
            PBRMaterial* PBRMat = (PBRMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PBRMaterial

            // 将纹理单元与纹理采样器进行挂钩
            // AlbedoMap
            if (PBRMat->mAlbedoMap == nullptr) {
                shader->setBool("useAlbedoMap", false);
            }
            else {
                shader->setBool("useAlbedoMap", true);
                // 绑定mAlbedoMap到单元 PBRMat->mAlbedoMap->getUnit()，并告诉Shader
                PBRMat->mAlbedoMap->bind();
                shader->setInt("albedoMap", PBRMat->mAlbedoMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // NormalMap
            if (PBRMat->mNormalMap == nullptr) {
                shader->setBool("useNormalMap", false);
            }
            else {
                shader->setBool("useNormalMap", true);
                // 绑定normalMap到单元 PBRMat->mNormalMap->getUnit()，并告诉Shader
                PBRMat->mNormalMap->bind();
                shader->setInt("normalMap", PBRMat->mNormalMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // MetallicMap
            if (PBRMat->mMetallicMap == nullptr) {
                shader->setBool("useMetallicMap", false);
            }
            else {
                shader->setBool("useMetallicMap", true);
                PBRMat->mMetallicMap->bind();
                shader->setInt("metallicMap", PBRMat->mMetallicMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // RoughnessMap
            if (PBRMat->mRoughnessMap == nullptr) {
                shader->setBool("useRoughnessMap", false);
            }
            else {
                shader->setBool("useRoughnessMap", true);
                PBRMat->mRoughnessMap->bind();
                shader->setInt("roughnessMap", PBRMat->mRoughnessMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // AoMap
            if (PBRMat->mAoMap == nullptr) {
                shader->setBool("useAoMap", false);
            }
            else {
                shader->setBool("useAoMap", true);
                PBRMat->mAoMap->bind();
                shader->setInt("aoMap", PBRMat->mAoMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // 设置非贴图的PBR参数
            shader->setVector3("albedo", glm::vec3(0.5f));  // 模型颜色
            shader->setFloat("metallic", PBRMat->getMetallic());    // 金属度（0=非金属，1=金属）
            shader->setFloat("roughness", PBRMat->getRoughness());   // 粗糙度（0=光滑，1=粗糙）
            shader->setFloat("ao", PBRMat->getAo());    // 环境光遮蔽

			shader->setFloat("opacity", PBRMat->mOpacity);    // 透明度

            /* PBR */
            shader->setFloat("normalScaleUniform", 1.0f); // 法线缩放系数（float类型，默认1.0f，不缩放法线）
            shader->setFloat("metallicUniform", 1.0f); // 材质金属度（float类型，默认1.0f，完全金属材质）
            shader->setFloat("roughnessUniform", 1.0f); // 材质粗糙度（float类型，默认1.0f，完全粗糙材质）
            shader->setVector3("emissiveFactorUniform", glm::vec3(0.0, 0.0, 0.0)); // 材质自发光因子（vec3类型，默认(0.0,0.0,0.0)，默认不发光）
            shader->setFloat("aoStrengthUniform", 1.0f); // 材质环境光遮蔽强度（float类型，默认1.0f，完全应用AO效果）

            float n1 = PBRMat->n1;
            float n2 = 1.0;
            float f0 = ((n1 - n2) * (n1 - n2)) / ((n1 + n2) * (n1 + n2));
            shader->setFloat("baseF0Uniform", f0);    // 法线入射时基础反射率（float类型，默认0.04f，非金属）
            shader->setVector3("edgeTintUniform", glm::vec3(1.0, 1.0, 1.0)); // 材质边缘色调（vec3类型，默认白色，与其他颜色uniform默认值保持统一）


            float coatn1 = PBRMat->coatn1;
            float coatn2 = 1.0;
            float coatf0 = ((coatn1 - coatn2) * (coatn1 - coatn2)) / ((coatn1 + coatn2) * (coatn1 + coatn2));
            /*  coat  */
            shader->setFloat("coatF0Uniform", coatf0); // 涂层法线入射时反射率（float类型，默认1.0f，最大涂层反射率）
            shader->setFloat("coatRoughnessUniform", PBRMat->coatRoughness); // 涂层粗糙度（float类型，默认1.0f，完全粗糙涂层）
            shader->setFloat("coatStrengthUniform", PBRMat->coatStrength); // 涂层强度（float类型，默认1.0f，完全应用涂层效果）
            shader->setVector3("coatColorUniform", PBRMat->coatColor); // 涂层颜色（vec3类型，默认白色，与其他颜色uniform默认值保持统一）

            break;
        }
        case MaterialType::SreenMaterial: {
			ScreenMaterial* screenMat = (ScreenMaterial*)material;
            // 绑定mScreenTexture到单元 screenMat->mScreenTexture->getUnit()，并告诉Shader
			// 为什么需要这一步？因为有些操作会改变原来的纹理单元绑定状态，导致之前绑定该纹理ID的纹理单元绑定了别的纹理ID
            if (screenMat->mScreenTexture != nullptr) {
                screenMat->mScreenTexture->bind();
                shader->setInt("screenTexture", screenMat->mScreenTexture->getUnit());
            }
           
            if (screenMat->mWeightSumTexture != nullptr) {
                screenMat->mWeightSumTexture->bind();
                shader->setInt("weightSumTexture", screenMat->mWeightSumTexture->getUnit());
            }
			
            if (screenMat->mColorWeightTexture != nullptr) {
                screenMat->mColorWeightTexture->bind();
                shader->setInt("colorWeightTexture", screenMat->mColorWeightTexture->getUnit());
            }
			
            break;
        }
default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            continue;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }
}


void Renderer::render(
    const std::vector<Mesh*>& meshes,
    Camera* camera,
    DirectionalLight* dirLight,
    const std::vector<PointLight*>& pointLights,
    SpotLight* spotLight,
    AmbientLight* ambLight,
    unsigned int fbo   // 默认是0，0号是系统默认的FBO, 这里传入非0值表示渲染到自定义FBO；0号表示渲染到屏幕
) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);     // 绑定到指定的FBO上进行渲染

    //1 设置当前帧绘制的时候，opengl的必要状态机参数
    // 一开始全部打开深度测试，防止因为前面的绘制操作关闭了写入深度缓存导致的无法glClear深度缓存
    // 后面再去针对物体设置深度测试相关状态。
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // 深度测试功能：如果片段的深度值小于存储的深度值，则通过。
    glDepthMask(GL_TRUE);       // 允许写入深度缓存

    // 默认情况下是关闭offset的，只有对特定物体我们才开启
    glDisable(GL_POLYGON_OFFSET_FILL);      // 关闭多边形的 offset
    glDisable(GL_POLYGON_OFFSET_LINE);      // 关闭线的 offset

    // 开启测试、设置基本写入状态，打开模板测试写入
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);    // 开启模板测试写入，保证了模板缓冲可以被清理

    // 默认颜色混合
    glDisable(GL_BLEND);    // 默认关闭，耗性能

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //GL_STENCIL_BUFFER_BIT 清理模板缓冲

    //3 遍历mesh进行绘制
    for (int i = 0; i < meshes.size(); i++) {
        auto mesh = meshes[i];
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        setDepthState(material);
        setPolygonOffsetState(material);
        setStencilState(material);
        setBlenderState(material);
        setFaceCullingState(material);

        /*
        if (i == 0) {
            glStencilFunc(GL_ALWAYS, 1, 0xFF);      // 如何测试
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);      // 成功、失败后如何
            glStencilMask(0xFF);    // 控制缓冲写入
        }
        else if (i == 1) {
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);    // 逻辑优势，不允许它写入，不写也对，逻辑上表示边界覆盖的区域不应该写入 1
        }
        else if (i == 2) {
            glStencilFunc(GL_ALWAYS, 1, 0xFF);      // 如何测试
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);      // 成功、失败后如何
            glStencilMask(0xFF);    // 控制缓冲写入
        }
        else if (i == 3) {
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);    // 逻辑优势，不允许它写入，不写也对，逻辑上表示边界覆盖的区域不应该写入 1
        }
        */
        //1 决定使用哪个Shader
        Shader* shader = material->getShader();
        if (shader == nullptr) {
            throw std::runtime_error("The Shader is nullptr.");
        }

        //2 更新shader的uniform
        shader->begin();
        /* ----------------------- 先传输通用的uniform变量----------------------------*/
        /*    光源参数的uniform更新    */
        // spotlight的更新
        shader->setVector3("spotLight.position", spotLight->getPosition());
        shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
        shader->setVector3("spotLight.color", spotLight->getColor());
        shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
        shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));   // cos 弧度值
        shader->setFloat("spotLight.outerLine", glm::cos(glm::radians(spotLight->getOuterAngle())));   // cos 弧度值
        shader->setFloat("spotLight.k2", spotLight->mK2);
        shader->setFloat("spotLight.k1", spotLight->mK1);
        shader->setFloat("spotLight.kc", spotLight->mKc);

        // directionallight的更新
        shader->setVector3("directionLight.direction", dirLight->getDirection());
        shader->setVector3("directionLight.color", dirLight->getColor());
        shader->setFloat("directionLight.specularIntensity", dirLight->getSpecularIntensity());
        shader->setVector3("directionLight.ambient", dirLight->getAmbient());
        shader->setVector3("directionLight.diffuse", dirLight->getDiffuse());
        shader->setVector3("directionLight.specular", dirLight->getSpecular());


        // pointlight的更新
        for (int i = 0; i < pointLights.size(); i++) {
            auto pointLight = pointLights[i];
            std::string baseName = "pointLights[";
            baseName.append(std::to_string(i));
            baseName.append("]");

            shader->setVector3(baseName + ".position", pointLight->getPosition());
            shader->setVector3(baseName + ".color", pointLight->getColor());
            shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
            shader->setFloat(baseName + ".k2", pointLight->getK2());
            shader->setFloat(baseName + ".k1", pointLight->getK1());
            shader->setFloat(baseName + ".kc", pointLight->getKc());

            shader->setVector3(baseName + ".ambient", pointLight->getAmbient());
            shader->setVector3(baseName + ".diffuse", pointLight->getDiffuse());
            shader->setVector3(baseName + ".specular", pointLight->getSpecular());
        }

        // 环境光配置
        shader->setVector3("ambientLight.color", ambLight->getColor());
        shader->setFloat("ambientLight.Intensity", ambLight->getIntensity());

        /*    相机的uniform更新    */
        shader->setVector3("cameraPosition", camera->mPosition);
        shader->setFloat("far", camera->mFar);
        shader->setFloat("near", camera->mNear);

        /*    MVP矩阵与normalMatrix    */
        shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
        shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
        shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

        auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
        shader->setMatrix3x3("normalMatrix", normalMatrix);

        /*     物体基本材质    */
        shader->setFloat("opacityUniform", 1.0f);   // 透明度（float类型，默认1.0f，完全不透明）

        /* Phong */
        shader->setFloat("ambientIntensity", 1.0f);
        shader->setVector3("ambientColorUniform", glm::vec3(1.0, 1.0, 1.0));
        shader->setFloat("diffuseIntensity", 1.0f);
        shader->setVector3("diffuseColorUniform", glm::vec3(1.0, 1.0, 1.0));

        shader->setFloat("specularIntensity", 1.0f);    // 高光强度（float类型，默认1.0f）
        shader->setFloat("specularPowerUniform", 32.0f);    // 高光幂次（float类型，默认32.0f，控制高光光斑大小，值越大光斑越集中）
        shader->setVector3("specularColorUniform", glm::vec3(1.0, 1.0, 1.0));

        /* PBR */
        shader->setFloat("normalScaleUniform", 1.0f); // 法线缩放系数（float类型，默认1.0f，不缩放法线）
        shader->setFloat("metallicUniform", 1.0f); // 材质金属度（float类型，默认1.0f，完全金属材质）
        shader->setFloat("roughnessUniform", 1.0f); // 材质粗糙度（float类型，默认1.0f，完全粗糙材质）
        shader->setVector3("emissiveFactorUniform", glm::vec3(0.0, 0.0, 0.0)); // 材质自发光因子（vec3类型，默认(0.0,0.0,0.0)，默认不发光）
        shader->setFloat("aoStrengthUniform", 1.0f); // 材质环境光遮蔽强度（float类型，默认1.0f，完全应用AO效果）
        shader->setFloat("baseF0Uniform", 1.0f);    // 法线入射时基础反射率（float类型，默认1.0f，最大基础反射率）
        shader->setVector3("edgeTintUniform", glm::vec3(1.0, 1.0, 1.0)); // 材质边缘色调（vec3类型，默认白色，与其他颜色uniform默认值保持统一）

        /*  coat  */
        shader->setFloat("coatF0Uniform", 1.0f); // 涂层法线入射时反射率（float类型，默认1.0f，最大涂层反射率）
        shader->setFloat("coatRoughnessUniform", 1.0f); // 涂层粗糙度（float类型，默认1.0f，完全粗糙涂层）
        shader->setFloat("coatStrengthUniform", 1.0f); // 涂层强度（float类型，默认1.0f，完全应用涂层效果）
        shader->setVector3("coatColorUniform", glm::vec3(1.0, 1.0, 1.0)); // 涂层颜色（vec3类型，默认白色，与其他颜色uniform默认值保持统一）


        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            // 绑定mDiffuse到单元 phongMat->mDiffuse->getUnit()，并告诉Shader
            phongMat->mDiffuse->bind();     // 绑定纹理ID - mTexture，到对应的纹理单元 mUnit
            shader->setInt("sampler", phongMat->mDiffuse->getUnit());	// 绑定采样器到纹理单元
            // specular 贴图
            if (phongMat->mSpecularMask != nullptr) {
                phongMat->mSpecularMask->bind();     // 绑定纹理ID - mTexture，到对应的纹理单元 mUnit
                shader->setInt("specularMaskSampler", phongMat->mSpecularMask->getUnit());	// 绑定采样器到纹理单元
            }
            shader->setVector3("material.ambient", phongMat->getAmbientColor());
            shader->setVector3("material.diffuse", phongMat->getDiffuseColor());
            shader->setVector3("material.specular", phongMat->getSpecularColor());
            shader->setFloat("material.shiness", phongMat->getShiness());

            shader->setFloat("material.shiness", phongMat->getShiness());
            shader->setFloat("opacity", phongMat->mOpacity);    // 透明度

            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        case MaterialType::PBRMaterial: {
            PBRMaterial* PBRMat = (PBRMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PBRMaterial

            // 将纹理单元与纹理采样器进行挂钩
            // AlbedoMap
            if (PBRMat->mAlbedoMap == nullptr) {
                shader->setBool("useAlbedoMap", false);
            }
            else {
                shader->setBool("useAlbedoMap", true);
                // 绑定mAlbedoMap到单元 PBRMat->mAlbedoMap->getUnit()，并告诉Shader
                PBRMat->mAlbedoMap->bind();
                shader->setInt("albedoMap", PBRMat->mAlbedoMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // NormalMap
            if (PBRMat->mNormalMap == nullptr) {
                shader->setBool("useNormalMap", false);
            }
            else {
                shader->setBool("useNormalMap", true);
                // 绑定normalMap到单元 PBRMat->mNormalMap->getUnit()，并告诉Shader
                PBRMat->mNormalMap->bind();
                shader->setInt("normalMap", PBRMat->mNormalMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // MetallicMap
            if (PBRMat->mMetallicMap == nullptr) {
                shader->setBool("useMetallicMap", false);
            }
            else {
                shader->setBool("useMetallicMap", true);
                PBRMat->mMetallicMap->bind();
                shader->setInt("metallicMap", PBRMat->mMetallicMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // RoughnessMap
            if (PBRMat->mRoughnessMap == nullptr) {
                shader->setBool("useRoughnessMap", false);
            }
            else {
                shader->setBool("useRoughnessMap", true);
                PBRMat->mRoughnessMap->bind();
                shader->setInt("roughnessMap", PBRMat->mRoughnessMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // AoMap
            if (PBRMat->mAoMap == nullptr) {
                shader->setBool("useAoMap", false);
            }
            else {
                shader->setBool("useAoMap", true);
                PBRMat->mAoMap->bind();
                shader->setInt("aoMap", PBRMat->mAoMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // 设置非贴图的PBR参数
            shader->setVector3("albedo", glm::vec3(0.5f));  // 模型颜色
            shader->setFloat("metallic", PBRMat->getMetallic());    // 金属度（0=非金属，1=金属）
            shader->setFloat("roughness", PBRMat->getRoughness());   // 粗糙度（0=光滑，1=粗糙）
            shader->setFloat("ao", PBRMat->getAo());    // 环境光遮蔽

            shader->setFloat("opacity", PBRMat->mOpacity);    // 透明度

            break;
        }
        case MaterialType::SreenMaterial: {
            ScreenMaterial* screenMat = (ScreenMaterial*)material;
            // 绑定mScreenTexture到单元 screenMat->mScreenTexture->getUnit()，并告诉Shader
            // 为什么需要这一步？因为有些操作会改变原来的纹理单元绑定状态，导致之前绑定该纹理ID的纹理单元绑定了别的纹理ID
            if (screenMat->mScreenTexture != nullptr) {
                screenMat->mScreenTexture->bind();
                shader->setInt("screenTexture", screenMat->mScreenTexture->getUnit());
            }

            if (screenMat->mWeightSumTexture != nullptr) {
                screenMat->mWeightSumTexture->bind();
                shader->setInt("weightSumTexture", screenMat->mWeightSumTexture->getUnit());
            }

            if (screenMat->mColorWeightTexture != nullptr) {
                screenMat->mColorWeightTexture->bind();
                shader->setInt("colorWeightTexture", screenMat->mColorWeightTexture->getUnit());
            }

            break;
        }
        default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            continue;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }
}

void Renderer::render(
    const std::vector<Mesh*>& meshes,
    Camera* camera,
    DirectionalLight* dirLight,
    PointLight* pointLight,
    SpotLight* spotLight,
    AmbientLight* ambLight
) {
    //1 设置当前帧绘制的时候，opengl的必要状态机参数
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // 深度测试功能：如果片段的深度值小于存储的深度值，则通过。

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //3 遍历mesh进行绘制
    for (int i = 0; i < meshes.size(); i++) {
        auto mesh = meshes[i];
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        //1 决定使用哪个Shader
        Shader* shader = pickShader(material->mType);

        //2 更新shader的uniform
        shader->begin();

        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            shader->setInt("sampler", 0);	// 绑定采样器到纹理单元0，因为前面激活了0号

            // 将纹理与纹理单元进行挂钩
            phongMat->mDiffuse->bind(); // 激活并绑定纹理单元

            // specular 贴图
            shader->setInt("specularMaskSampler", 1);	// 绑定采样器到纹理单元1，因为前面激活了1号
            phongMat->mSpecularMask->bind(); // 激活并绑定纹理单元

            // 光源参数的uniform更新
            // spotlight的更新
            shader->setVector3("spotLight.position", spotLight->getPosition());
            shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
            shader->setVector3("spotLight.color", spotLight->getColor());
            shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
            shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));   // cos 弧度值
            shader->setFloat("spotLight.outerLine", glm::cos(glm::radians(spotLight->getOuterAngle())));   // cos 弧度值
            shader->setFloat("spotLight.k2", spotLight->mK2);
            shader->setFloat("spotLight.k1", spotLight->mK1);
            shader->setFloat("spotLight.kc", spotLight->mKc);

            // directionallight的更新
            shader->setVector3("directionLight.direction", dirLight->getDirection());
            shader->setVector3("directionLight.color", dirLight->getColor());
            shader->setFloat("directionLight.specularIntensity", dirLight->getSpecularIntensity());

            // pointlight的更新
            shader->setVector3("pointLight.position", pointLight->getPosition());
            shader->setVector3("pointLight.color", pointLight->getColor());
            shader->setFloat("pointLight.specularIntensity", pointLight->getSpecularIntensity());
            shader->setFloat("pointLight.k2", pointLight->getK2());
            shader->setFloat("pointLight.k1", pointLight->getK1());
            shader->setFloat("pointLight.kc", pointLight->getKc());

            shader->setFloat("shiness", phongMat->getShiness());
            shader->setBool("blinn", phongMat->getBlinn());
            shader->setVector3("ambientColor", ambLight->getColor());

            // 相机信息更新
            shader->setVector3("cameraPosition", camera->mPosition);

            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

            // 计算normalMatrix
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
            shader->setMatrix3x3("normalMatrix", normalMatrix);

            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            continue;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }
}


void Renderer::render(
    const std::vector<Mesh*>& meshes,
    Camera* camera,
    SpotLight* spotLight,
    AmbientLight* ambLight
) {
    //1 设置当前帧绘制的时候，opengl的必要状态机参数
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // 深度测试功能：如果片段的深度值小于存储的深度值，则通过。

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //3 遍历mesh进行绘制
    for (int i = 0; i < meshes.size(); i++) {
        auto mesh = meshes[i];
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        //1 决定使用哪个Shader
        Shader* shader = pickShader(material->mType);

        //2 更新shader的uniform
        shader->begin();

        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            shader->setInt("sampler", 0);	// 绑定采样器到纹理单元0，因为前面激活了0号

            // 将纹理与纹理单元进行挂钩
            phongMat->mDiffuse->bind(); // 激活并绑定纹理单元

            // specular 贴图
            shader->setInt("specularMaskSampler", 1);	// 绑定采样器到纹理单元1，因为前面激活了1号
            phongMat->mSpecularMask->bind(); // 激活并绑定纹理单元

            // 光源参数的uniform参数   spotLight.
            //shader->setVector3("lightPosition", spotLight->getPosition());
            //shader->setVector3("targetDirection", spotLight->getTargetDirection());
            //shader->setVector3("lightColor", spotLight->getColor());
            //shader->setFloat("specularIntensity", spotLight->getSpecularIntensity());
            //shader->setFloat("innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));   // cos 弧度值
            //shader->setFloat("outerLine", glm::cos(glm::radians(spotLight->getOuterAngle())));   // cos 弧度值
            //shader->setFloat("k2", spotLight->mK2);
            //shader->setFloat("k1", spotLight->mK1);
            //shader->setFloat("kc", spotLight->mKc);

            // 封装光源的参数
            shader->setVector3("spotLight.position", spotLight->getPosition());
            shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
            shader->setVector3("spotLight.color", spotLight->getColor());
            shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
            shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));   // cos 弧度值
            shader->setFloat("spotLight.outerLine", glm::cos(glm::radians(spotLight->getOuterAngle())));   // cos 弧度值
            shader->setFloat("spotLight.k2", spotLight->mK2);
            shader->setFloat("spotLight.k1", spotLight->mK1);
            shader->setFloat("spotLight.kc", spotLight->mKc);

            shader->setFloat("shiness", phongMat->getShiness());
            shader->setBool("blinn", phongMat->getBlinn());
            shader->setVector3("ambientColor", ambLight->getColor());

            // 相机信息更新
            shader->setVector3("cameraPosition", camera->mPosition);

            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

            // 计算normalMatrix
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
            shader->setMatrix3x3("normalMatrix", normalMatrix);

            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            continue;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }
}


void Renderer::render(
    const std::vector<Mesh*>& meshes,
    Camera* camera,
    PointLight* pointLight,
    AmbientLight* ambLight
) {
    //1 设置当前帧绘制的时候，opengl的必要状态机参数
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // 深度测试功能：如果片段的深度值小于存储的深度值，则通过。

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //3 遍历mesh进行绘制
    for (int i = 0; i < meshes.size(); i++) {
        auto mesh = meshes[i];
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        //1 决定使用哪个Shader
        Shader* shader = pickShader(material->mType);

        //2 更新shader的uniform
        shader->begin();

        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            shader->setInt("sampler", 0);	// 绑定采样器到纹理单元0，因为前面激活了0号

            // 将纹理与纹理单元进行挂钩
            phongMat->mDiffuse->bind(); // 激活并绑定纹理单元

            // specular 贴图
            shader->setInt("specularMaskSampler", 1);	// 绑定采样器到纹理单元1，因为前面激活了1号
            phongMat->mSpecularMask->bind(); // 激活并绑定纹理单元

            // 光源参数的uniform参数
            shader->setVector3("lightPosition", pointLight->getPosition());
            shader->setVector3("lightColor", pointLight->getColor());
            shader->setFloat("specularIntensity", pointLight->getSpecularIntensity());
            shader->setFloat("k2", pointLight->getK2());
            shader->setFloat("k1", pointLight->getK1());
            shader->setFloat("kc", pointLight->getKc());

            shader->setFloat("shiness", phongMat->getShiness());
            shader->setBool("blinn", phongMat->getBlinn());
            shader->setVector3("ambientColor", ambLight->getColor());

            // 相机信息更新
            shader->setVector3("cameraPosition", camera->mPosition);

            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

            // 计算normalMatrix
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
            shader->setMatrix3x3("normalMatrix", normalMatrix);

            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            continue;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }
}


void Renderer::render(
    const std::vector<Mesh*>& meshes,
    Camera* camera,
    DirectionalLight* dirLight,
    AmbientLight* ambLight
) {
    //1 设置当前帧绘制的时候，opengl的必要状态机参数
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //3 遍历mesh进行绘制
    for (int i = 0; i < meshes.size(); i++) {
        auto mesh = meshes[i];
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        //1 决定使用哪个Shader
        Shader* shader = pickShader(material->mType);

        //2 更新shader的uniform
        shader->begin();

        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            shader->setInt("sampler", 0);	// 绑定采样器到纹理单元0，因为前面激活了0号

            // 将纹理与纹理单元进行挂钩
            phongMat->mDiffuse->bind(); // 激活并绑定纹理单元

			// specular 贴图
			shader->setInt("specularMaskSampler", 1);	// 绑定采样器到纹理单元1，因为前面激活了1号
			phongMat->mSpecularMask->bind(); // 激活并绑定纹理单元

            // 光源参数的uniform参数
            shader->setVector3("lightDirection", dirLight->mDirection);
            shader->setVector3("lightColor", dirLight->getColor());
            shader->setFloat("specularIntensity", dirLight->getSpecularIntensity());

            shader->setFloat("shiness", phongMat->getShiness());
            shader->setBool("blinn", phongMat->getBlinn());
            shader->setVector3("ambientColor", ambLight->getColor());

            // 相机信息更新
            shader->setVector3("cameraPosition", camera->mPosition);

            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

            // 计算normalMatrix
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
            shader->setMatrix3x3("normalMatrix", normalMatrix);

            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            continue;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }
}

void Renderer::render(
    Scene* scene,
    Camera* camera,
    const DirectionalLight* dirLight,
    const std::vector<PointLight*>& pointLights,
    const AmbientLight* ambLight,
    unsigned int fbo   // 默认是0，0号是系统默认的FBO
) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);     // 先绑定到指定的FBO上进行渲染

    //1 设置当前帧绘制的时候，opengl的必要状态机参数
    // 一开始全部打开深度测试，防止因为前面的绘制操作关闭了写入深度缓存导致的无法glClear深度缓存
    // 后面再去针对物体设置深度测试相关状态。
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);       // 深度测试功能：如果片段的深度值小于存储的深度值，则通过。
    glDepthMask(GL_TRUE);       // 允许写入深度缓存

    // 默认情况下是关闭offset的，只有对特定物体我们才开启
    glDisable(GL_POLYGON_OFFSET_FILL);      // 关闭多边形的 offset
    glDisable(GL_POLYGON_OFFSET_LINE);      // 关闭线的 offset

    // 开启测试、设置基本写入状态，打开模板测试写入
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);    // 开启模板测试写入，保证了模板缓冲可以被清理

    // 默认颜色混合
    glDisable(GL_BLEND);    // 默认关闭，耗性能

    //2 清理画布
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //GL_STENCIL_BUFFER_BIT 清理模板缓冲


    // 清空两个队列
    mOpaqueObjects.clear();
    mTransparentObjects.clear();
    projectObject(scene);

	// 多物体时候，需要对透明物体进行排序，保证从远到近渲染
    std::sort(
        mTransparentObjects.begin(),
        mTransparentObjects.end(),
        [camera](Mesh* a, Mesh* b) {
            // 计算A的相机系的Z
			auto modelMatrixA = a->getModelMatrx();
			auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);     // 从原点中心移到世界坐标中的某点
			auto cameraPositionA = camera->getViewMatrix() * worldPositionA;
            float zA = cameraPositionA.z;
			// 计算B的相机系的Z
			auto modelMatrixB = b->getModelMatrx();
			auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            auto cameraPositionB = camera->getViewMatrix() * worldPositionB;
            float zB = cameraPositionB.z;
            // 返回z值较小的在前面，升序排列
			return zA < zB;
        }
	);

	// 先渲染不透明物体
    for(int i = 0; i < mOpaqueObjects.size(); i++) {
        renderObject(mOpaqueObjects[i], camera, dirLight, pointLights, ambLight);
	}

	// 再渲染透明物体
    for (int i = 0; i < mTransparentObjects.size(); i++) {
        renderObject(mTransparentObjects[i], camera, dirLight, pointLights, ambLight);
    }
}

// 针对单个object进行渲染
void Renderer::renderObject(
    Object* object,
    Camera* camera,
    const DirectionalLight* dirLight,
    const std::vector<PointLight*>& pointLights,
    const AmbientLight* ambLight
) {
    // 1 判断是Mesh还是Object，如果是Mesh需要渲染
    if (object->getType() == ObjectType::Mesh) {
        auto mesh = (Mesh*)object;  // 由于object本身已经确定是Mesh子类，所以可以强转
        auto geometry = mesh->mGeometry;
        auto material = mesh->mMaterial;

        setDepthState(material);
        setPolygonOffsetState(material);
        setStencilState(material);
        setBlenderState(material);

        //1 决定使用哪个Shader
        Shader* shader = pickShader(material->mType);

        //2 更新shader的uniform
        shader->begin();

        switch (material->mType) {
        case MaterialType::PhongMaterial: {
            PhongMaterial* phongMat = (PhongMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PhongMaterial，外面把Texture绑到了0号单元
            // diffuse 贴图
            // 将纹理单元与纹理采样器进行挂钩
            shader->setInt("sampler", 0);	// 绑定采样器到纹理单元0，因为前面激活了0号

            // 将纹理与纹理单元进行挂钩
            phongMat->mDiffuse->bind(); // 激活并绑定纹理单元

            // specular 贴图
            shader->setInt("specularMaskSampler", 1);	// 绑定采样器到纹理单元1，因为前面激活了1号
            phongMat->mSpecularMask->bind(); // 激活并绑定纹理单元

            // 光源参数的uniform参数
            shader->setVector3("lightDirection", dirLight->mDirection);
            shader->setVector3("lightColor", dirLight->getColor());
            shader->setFloat("specularIntensity", dirLight->getSpecularIntensity());

            shader->setFloat("shiness", phongMat->getShiness());
            shader->setBool("blinn", phongMat->getBlinn());
            shader->setVector3("ambientColor", ambLight->getColor());

            // 相机信息更新
            shader->setVector3("cameraPosition", camera->mPosition);

            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

            // 计算normalMatrix
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
            shader->setMatrix3x3("normalMatrix", normalMatrix);

            break;
        }
        case MaterialType::WhiteMaterial: {
            // 使用白色Shader
            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量
            break;
        }
        case MaterialType::PBRMaterial: {
            PBRMaterial* PBRMat = (PBRMaterial*)material;     // 强转，类型安全检查？因为在外部创建material时new的是一个PBRMaterial

            // 将纹理单元与纹理采样器进行挂钩
            // AlbedoMap
            if (PBRMat->mAlbedoMap == nullptr) {
                shader->setBool("useAlbedoMap", false);
            }
            else {
                shader->setBool("useAlbedoMap", true);
                shader->setInt("albedoMap", PBRMat->mAlbedoMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // NormalMap
            if (PBRMat->mNormalMap == nullptr) {
                shader->setBool("useNormalMap", false);
            }
            else {
                shader->setBool("useNormalMap", true);
                shader->setInt("normalMap", PBRMat->mNormalMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // MetallicMap
            if (PBRMat->mMetallicMap == nullptr) {
                shader->setBool("useMetallicMap", false);
            }
            else {
                shader->setBool("useMetallicMap", true);
                shader->setInt("metallicMap", PBRMat->mMetallicMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // RoughnessMap
            if (PBRMat->mRoughnessMap == nullptr) {
                shader->setBool("useRoughnessMap", false);
            }
            else {
                shader->setBool("useRoughnessMap", true);
                shader->setInt("roughnessMap", PBRMat->mRoughnessMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // AoMap
            if (PBRMat->mAoMap == nullptr) {
                shader->setBool("useAoMap", false);
            }
            else {
                shader->setBool("useAoMap", true);
                shader->setInt("aoMap", PBRMat->mAoMap->getUnit());	// 绑定采样器到对应纹理单元
            }

            // 设置非贴图的PBR参数
            shader->setVector3("albedo", glm::vec3(0.5f));  // 模型颜色
            shader->setFloat("metallic", PBRMat->getMetallic());    // 金属度（0=非金属，1=金属）
            shader->setFloat("roughness", PBRMat->getRoughness());   // 粗糙度（0=光滑，1=粗糙）
            shader->setFloat("ao", PBRMat->getAo());    // 环境光遮蔽

            shader->setFloat("opacity", PBRMat->mOpacity);    // 透明度

            // 光源参数的uniform更新
            // pointlight的更新
            for (int i = 0; i < pointLights.size(); i++) {
                auto pointLight = pointLights[i];
                std::string baseName = "pointLights[";
                baseName.append(std::to_string(i));
                baseName.append("]");

                shader->setVector3(baseName + ".position", pointLight->getPosition());
                //std::cout << "pointlight[" << i << "].position = " << pointLight->getPosition().x<<", "<< pointLight->getPosition().y << ", "<< pointLight->getPosition().z<< std::endl;
                shader->setVector3(baseName + ".color", pointLight->getColor());
                shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
                shader->setFloat(baseName + ".k2", pointLight->getK2());
                shader->setFloat(baseName + ".k1", pointLight->getK1());
                shader->setFloat(baseName + ".kc", pointLight->getKc());

                shader->setVector3(baseName + ".ambient", pointLight->getAmbient());
                shader->setVector3(baseName + ".diffuse", pointLight->getDiffuse());
                shader->setVector3(baseName + ".specular", pointLight->getSpecular());

            }

            // directionallight的更新
            shader->setVector3("directionLight.direction", dirLight->getDirection());
            shader->setVector3("directionLight.color", dirLight->getColor());
            shader->setFloat("directionLight.specularIntensity", dirLight->getSpecularIntensity());

            // 配置环境光
            shader->setVector3("ambientLight.color", ambLight->getColor());
            shader->setFloat("ambientLight.Intensity", ambLight->getIntensity());

            // 相机信息更新
            shader->setVector3("cameraPosition", camera->mPosition);
            shader->setFloat("far", camera->mFar);
            shader->setFloat("near", camera->mNear);

            // MVP矩阵
            shader->setMatrix4x4("ProjectionMatrix", camera->getProjectionMatrix());    // 投影变换矩阵
            shader->setMatrix4x4("ViewMatrix", camera->getViewMatrix());    // 视图变换矩阵，camera类
            shader->setMatrix4x4("ModelMatrix", mesh->getModelMatrx());	// 实际项目中每一帧都要修改ModelMatrix矩阵，所以在render函数中设置该 uniform 变量

            // 计算normalMatrix
            auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrx())));
            shader->setMatrix3x3("normalMatrix", normalMatrix);

            break;
        }
        case MaterialType::SreenMaterial: {
            ScreenMaterial* screenMat = (ScreenMaterial*)material;

            shader->setInt("screenTexture", screenMat->mScreenTexture->getUnit());

            break;
        }
        default:
            std::cerr << "Unknown material type: " << static_cast<int>(material->mType) << std::endl;
            break;
        }

        //3 绑定vao
        glBindVertexArray(geometry->getVAO());

        //4 执行绘制命令
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0);
    }

    // 2 遍历object的子节点，对每个子节点都需要调用 renderObject，深度优先搜索DFS
    auto children = object->getChildren();
    for (int i = 0; i < children.size(); i++) {
        renderObject(children[i], camera, dirLight, pointLights, ambLight);
    }
}
