#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>       // 用于向量计算
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <string>  // 新增：用于OBJ文件路径的string类型
#include <tuple>
#include <map>
#include "../wrapper/checkError.h"

class Geometry {
public:
    // 构造/析构
    Geometry();
    Geometry(
        const std::vector<float>& positions,
        const std::vector<float>& normals,
        const std::vector<float>& uvs,
        const std::vector<unsigned int>& indices
        );
    ~Geometry();

    // 静态工厂方法：创建立方体（指定边长）
    static Geometry* createBox(float size);
    // 静态工厂方法：创建球体（指定半径，经纬线数量默认60）
    static Geometry* createSphere(float radius, int numLatLines = 60, int numLongLines = 60);
    // 静态工厂方法：创建固定大小的三角形
    static Geometry* createTriangle();

    // 静态工厂方法：创建2D平面
    static Geometry* createPlane(float width, float height);
    static Geometry* createScreenPlane();
    // 静态工厂方法：创建2D画布（指定宽高）
    static Geometry* createCanvas(float width, float height);
    // 静态工厂方法：从OBJ文件路径创建Geometry 
    static Geometry* createFromOBJ(const std::string& objFilePath);
    static Geometry* createFromOBJ_nvn(const std::string& objFilePath);
    static Geometry* createFromOBJwithTangent(const std::string& objFilePath);

    // 静态工厂方法：从STL文件路径创建Geometry 
    // 支持二进制STL解析，新增useSmoothNormals参数控制是否启用平滑法线
    static Geometry* createFromSTL(const std::string& stlFilePath, bool useSmoothNormals = true);

    // 获取VAO（供渲染时绑定）
    GLuint getVAO() const { return mVao; }
    // 获取索引数量（供glDrawElements使用）
    GLsizei getIndicesCount() const { return mIndicesCount; }

private:
    GLuint mVao;        // 顶点数组对象（管理VBO/EBO状态）
    GLuint mPosVbo;     // 位置数据VBO
    GLuint mUvVbo;      // UV纹理坐标VBO
	GLuint mNormalVbo;  // 法线数据VBO（若需要法线可启用）
    GLuint mEbo;        // 索引EBO
    GLuint mTangentVbo; // 切线VBO
    GLsizei mIndicesCount;  // 索引总数（绘制时需传入）
};

#endif // GEOMETRY_H