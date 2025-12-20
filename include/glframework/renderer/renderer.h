#pragma once
#include <vector>
#include "../core.h"
#include "../mesh.h"
#include "../../camera/camera.h"
#include "../light/directionalLight.h"
#include "../light/pointLight.h"
#include "../light/ambientLight.h"
#include "../light/spotLight.h"
#include "../shader.h"
#include "../scene.h"

class Renderer
{
public:
	Renderer();
	Renderer(const std::string &vertexshaderPath, const std::string& fragmentshaderPath);
	Renderer(const std::string& phongVertexPath , const std::string& phongFragmentPath ,
		const std::string& whiteVertexPath , const std::string& whiteFragmentPath ,
		const std::string& pbrVertexPath , const std::string& pbrFragmentPath);
	~Renderer();

	// 渲染功能函数
	// 每次调用都会渲染一帧
	void render(
		Scene* scene,
		Camera* camera,
		const DirectionalLight* dirLight,
		const std::vector<PointLight*>& pointLights,
		const AmbientLight* ambLight,
		unsigned int fbo = 0	// 默认是0，0号是系统默认的FBO
	);

	void renderObject(
		Object* object,
		Camera* camera,
		const DirectionalLight* dirLight,
		const std::vector<PointLight*>& pointLights,
		const AmbientLight* ambLight
	);

	void render(
		const std::vector<Mesh*>& meshes,
		Camera* camera,
		DirectionalLight* dirLight,
		AmbientLight* ambLight
	);

	void render(
		const std::vector<Mesh*>& meshes,
		Camera* camera,
		PointLight* pointLight,
		AmbientLight* ambLight
	);

	void render(
		const std::vector<Mesh*>& meshes,
		Camera* camera,
		SpotLight* spotLight,
		AmbientLight* ambLight
	);

	void render(
		const std::vector<Mesh*>& meshes,
		Camera* camera,
		DirectionalLight* dirLight,
		PointLight* pointLight,
		SpotLight* spotLight,
		AmbientLight* ambLight
	);

	// 点光源数组的render函数
	void render(
		const std::vector<Mesh*>& meshes,
		Camera* camera,
		DirectionalLight* dirLight,
		const std::vector<PointLight*>& pointLights,
		SpotLight* spotLight,
		AmbientLight* ambLight,
		unsigned int fbo = 0	// 默认是0，0号是系统默认的FBO
	);

	void render(
		const std::vector<Mesh*>& meshes,
		Camera* camera,
		const std::vector<DirectionalLight*>& directionLights,
		const std::vector<PointLight*>& pointLights,
		const std::vector<SpotLight*>& spotLights,
		AmbientLight* ambLight,
		unsigned int fbo = 0    // 默认是0，0号是系统默认的FBO, 这里传入非0值表示渲染到自定义FBO；0号表示渲染到屏幕
	);


	void setClearColor(glm::vec3 color);

private:
	Shader* pickShader(MaterialType type);
	void setDepthState(Material* material);
	void setPolygonOffsetState(Material* material);
	void setStencilState(Material* material);
	void setBlenderState(Material* material);
	void setFaceCullingState(Material* material);

	void projectObject(Object* obj);	// 判断物体内的mesh是透明还是不透明，放入对应队列
private:
	// 生成多种不同的shader对象，看情况使用多种，每次记得在构造函数内生成即可
	// 根据材质类型的不同，挑选使用哪一个shader对象
	Shader* mPhongShader{ nullptr };
	Shader* mWhiteShader{ nullptr };
	Shader* mPBRShader{ nullptr };

	// 不透明物体与透明物体的队列
	// 注意！！每一帧绘制前，需要清空两个队列
	std::vector<Mesh*> mOpaqueObjects{};		// 不透明物体队列
	std::vector<Mesh*> mTransparentObjects{};	// 透明物体队列
};
