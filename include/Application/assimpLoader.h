#pragma once
#include "../glframework/core.h"
#include "../glframework/object.h"

#include "../../thirdParty/include/assimp/Importer.hpp"
#include "../../thirdParty/include/assimp/scene.h"
#include "../../thirdParty/include/assimp/postprocess.h"

#include "../glframework/mesh.h"
class AssimpLoader {
public:
	static Object* load(const std::string& path);
private:
	static void processNode(aiNode* ainode, Object* parent, const aiScene* scene, const std::string& rootpath);
	static Mesh* processMesh(aiMesh* aimesh, const aiScene* scene, const std::string& rootpath);	// scene中的纹理贴图数据
	static Mesh* processMesh(aiMesh* aimesh);	// 自己的纹理贴图数据
	

	static glm::mat4 getMat4f(aiMatrix4x4 value);
};