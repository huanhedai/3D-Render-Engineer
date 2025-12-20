#include "assimpLoader.h"
#include "../glframework/tools/tools.h"
#include "../glframework/material/phongMaterial.h"
Object* AssimpLoader::load(const std::string& path) {
	// 拿出模型所在目录
	std::size_t lastIndex = path.find_last_of("//");
	auto rootpath = path.substr(0, lastIndex + 1);

	Object* rootnode = new Object();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);	
	// aiProcess_Triangulate: 以三角形绘制顶点数据
	// aiProcess_GenNormals: 若模型没有法线，自己生成法线

	// 验证读取的是否正确顺利
	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
		std::cerr << "Error: Model Read Failed!" << std::endl;	// 无缓冲（会立即显示）cout可能会缓冲，等运行完才出来
		return nullptr;
	}

	processNode(scene->mRootNode, rootnode, scene, rootpath);
	return rootnode;
}

void AssimpLoader::processNode(aiNode* ainode, Object* parent, const aiScene* scene, const std::string& rootpath) {
	Object* node = new Object();
	parent->addChild(node);

	glm::mat4 localMatrix = getMat4f(ainode->mTransformation);	// 需要将 aiMatrix4x4 类型转为 glm::mat4
	// 位置 旋转 缩放
	glm::vec3 position, eulerAngle, scale;
	Tools::decompose(localMatrix, position, eulerAngle, scale);
	node->setPosition(position);
	node->setAngleX(eulerAngle.x);
	node->setAngleY(eulerAngle.y);

	// 检查有没有mesh，并且解析
	for(int i = 0; i < ainode->mNumMeshes; i++) {
		int meshID = ainode->mMeshes[i];	// ainode中存储了mesh的索引
		aiMesh* aimesh = scene->mMeshes[meshID];		// scene中存储了所有的mesh
		auto mesh = processMesh(aimesh, scene, rootpath);
		node->addChild(mesh);
	}

	for (int i = 0; i < ainode->mNumChildren; i++) {
		processNode(ainode->mChildren[i], node, scene, rootpath);
	}
}

/*
Mesh* AssimpLoader::processMesh(aiMesh* aimesh) {
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;

	for (int i = 0; i < aimesh->mNumVertices; i++) {
		// 位置
		positions.push_back(aimesh->mVertices[i].x);
		positions.push_back(aimesh->mVertices[i].y);
		positions.push_back(aimesh->mVertices[i].z);

		// 法线
		normals.push_back(aimesh->mNormals[i].x);
		normals.push_back(aimesh->mNormals[i].y);
		normals.push_back(aimesh->mNormals[i].z);

		// 纹理坐标
		// 一个顶点可以有多套纹理坐标，这里我们关注其第一套uv，一般情况下第一套uv是贴图uv
		if (aimesh->mTextureCoords[0]) {	// 判断是否有纹理坐标
			uvs.push_back(aimesh->mTextureCoords[0][i].x);
			uvs.push_back(aimesh->mTextureCoords[0][i].y);
		} else {
			uvs.push_back(0.0f);
			uvs.push_back(0.0f);
		}
	}
	// 解析mesh中的索引
	for (int i = 0; i < aimesh->mNumFaces; i++) {	// 一个mesh由多个face组成
		aiFace face = aimesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++) {	// 一个face可能是三角形，也可能是四边形，mNumIndices表示这个face有多少个顶点
			indices.push_back(face.mIndices[j]);
		}
	}

	auto geometry = new Geometry(positions, normals, uvs, indices);
	auto material = new PhongMaterial();
	material->mDiffuse = new Texture(std::string(TEXTURE_DIR) + +"/container2.png", 0);
	material->mBlinn = GL_TRUE;
	material->mSpecularMask = new Texture(std::string(TEXTURE_DIR) + "/container2_specular.png", 1);  // 与区分开来

	return new Mesh(geometry, material);
}
*/

Mesh* AssimpLoader::processMesh(aiMesh* aimesh, const aiScene* scene, const std::string& rootpath) {
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;

	for (int i = 0; i < aimesh->mNumVertices; i++) {
		// 位置
		positions.push_back(aimesh->mVertices[i].x);
		positions.push_back(aimesh->mVertices[i].y);
		positions.push_back(aimesh->mVertices[i].z);

		// 法线
		normals.push_back(aimesh->mNormals[i].x);
		normals.push_back(aimesh->mNormals[i].y);
		normals.push_back(aimesh->mNormals[i].z);

		// 纹理坐标
		// 一个顶点可以有多套纹理坐标，这里我们关注其第一套uv，一般情况下第一套uv是贴图uv
		if (aimesh->mTextureCoords[0]) {	// 判断是否有纹理坐标
			uvs.push_back(aimesh->mTextureCoords[0][i].x);
			uvs.push_back(aimesh->mTextureCoords[0][i].y);
		}
		else {
			uvs.push_back(0.0f);
			uvs.push_back(0.0f);
		}
	}
	// 解析mesh中的索引
	for (int i = 0; i < aimesh->mNumFaces; i++) {	// 一个mesh由多个face组成
		aiFace face = aimesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++) {	// 一个face可能是三角形，也可能是四边形，mNumIndices表示这个face有多少个顶点
			indices.push_back(face.mIndices[j]);
		}
	}

	auto geometry = new Geometry(positions, normals, uvs, indices);
	auto material = new PhongMaterial();
	//material->mDiffuse = new Texture(std::string(TEXTURE_DIR) + +"/container2.png", 0);
	//material->mBlinn = GL_TRUE;
	//material->mSpecularMask = new Texture(std::string(TEXTURE_DIR) + "/container2_specular.png", 1);  // 与区分开来

	if (aimesh->mMaterialIndex >= 0) {	// 说明scene中有材质/贴图
		Texture* texture = nullptr;
		aiMaterial* aiMat = scene->mMaterials[aimesh->mMaterialIndex];
		// 获取图片读取路径
		aiString aipath;
		aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), aipath);	// AI_MATKEY_TEXTURE表示获取的是纹理TEXTURE的路径，(aiTextureType_DIFFUSE, 0)表示获取第0张diffuse贴图

		// 判断是否是嵌入fbx的图片
		const aiTexture* aitexture = scene->GetEmbeddedTexture(aipath.C_Str());
		if (aitexture) {
			// 纹理图片是内嵌的
			unsigned char* dataIn = reinterpret_cast<unsigned char*>(aitexture->pcData);	// 这里拿的是带格式头的图片数据
			uint32_t widthIn = aitexture->mWidth;	// 通常情况下（png、jpg），代表的是整张图片的大小
			uint32_t heightIn = aitexture->mHeight;
			//texture = Texture::createTextureFromMemory(aipath.C_Str(), 0, dataIn, widthIn, heightIn);
			std::string fullpath = std::string(TEXTURE_DIR) + "/container2.png";
			texture = Texture::createTexture(fullpath, 0);
		}
		else {
			// 纹理图片在硬盘上
			//std::string fullpath = rootpath + std::string(aipath.C_Str());
			std::string fullpath = std::string(TEXTURE_DIR) + "/container2.png";
			texture = Texture::createTexture(fullpath, 0);
		}
		material->mDiffuse = texture;
	}
	else {	// 没有贴图，使用默认贴图
		material->mDiffuse = new Texture(std::string(TEXTURE_DIR) + "/container2.png", 0);
	}

	material->setBlinn(GL_TRUE);
	material->mSpecularMask = new Texture(std::string(TEXTURE_DIR) + "/container2_specular.png", 1);  // 与区分开来

	return new Mesh(geometry, material);
}

glm::mat4 AssimpLoader::getMat4f(aiMatrix4x4 value) {
	glm::mat4 to(
		value.a1, value.a2, value.a3, value.a4,
		value.b1, value.b2, value.b3, value.b4,
		value.c1, value.c2, value.c3, value.c4,
		value.d1, value.d2, value.d3, value.d4
	);
	return to;
}