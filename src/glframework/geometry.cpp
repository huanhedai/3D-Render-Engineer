#include "geometry.h"
#include <fstream>
#include <sstream>
#include <stdexcept> // 用于抛出文件读取错误
#include <cstring>

// 构造函数：初始化OpenGL对象为0
Geometry::Geometry()
    : mVao(0), mPosVbo(0), mUvVbo(0), mEbo(0), mIndicesCount(0), mNormalVbo(0){
}

Geometry::Geometry(
    const std::vector<float>& positions,
    const std::vector<float>& normals,
    const std::vector<float>& uvs,
    const std::vector<unsigned int>& indices
) {
    // 1. 初始化索引数量（后续绘制用）
    mIndicesCount = static_cast<unsigned int>(indices.size());

    // 2. 【核心】先创建并绑定 VAO（所有 VBO/EBO 配置都将被 VAO 记录）
    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);

    // 3. 配置位置 VBO（location = 0）
    glGenBuffers(1, &mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
    // 传入位置数据到 VBO
    glBufferData(GL_ARRAY_BUFFER,
        positions.size() * sizeof(float),
        positions.data(),
        GL_STATIC_DRAW);
    // 步骤1：配置顶点属性格式（3个float，步长为单个顶点字节数，偏移0）
    glVertexAttribPointer(0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0);
    // 步骤2：启用该顶点属性（配置完成后启用，规范顺序）
    glEnableVertexAttribArray(0);
    // 解绑当前 VBO（避免后续操作污染，VAO 已记录关联关系）
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 4. 配置 UV VBO（location = 1）
    glGenBuffers(1, &mUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mUvVbo);
    glBufferData(GL_ARRAY_BUFFER,
        uvs.size() * sizeof(float),
        uvs.data(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(1,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 5. 配置法线 VBO（location = 2）
    glGenBuffers(1, &mNormalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mNormalVbo);
    glBufferData(GL_ARRAY_BUFFER,
        normals.size() * sizeof(float),
        normals.data(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(2,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 6. 配置 EBO（索引缓冲区，VAO 绑定期间绑定 EBO 会被 VAO 记录）
    glGenBuffers(1, &mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW);
    // 注意：EBO 无需提前解绑！VAO 绑定状态下，GL_ELEMENT_ARRAY_BUFFER 的绑定会被 VAO 持久化；
    // 若此时解绑 EBO，VAO 会记录「空的 EBO 绑定」，导致绘制失败

    // 7. 最后解绑 VAO（核心：VAO 解绑后，后续操作不会污染 VAO 内的状态）
    glBindVertexArray(0);

    // 8. 清理上下文残留的 EBO 绑定（可选，但推荐，避免影响后续其他 Buffer 操作）
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    /***********************************************************/
    /*          Buffer 数据流程
	* VBO/EBO 数据流程：CPU 内存 --> 显存（GPU 内存）
	* 这里采样的是分离的VBO形式，每种属性一个VBO；推荐使用一个VBO存储所有属性以减少绑定次数
	* 一个属性传入的流程如下：
	* 1. 生成 VBO             glGenBuffers: 在GPU显存中申请一块缓冲区。
	* 2. 绑定 VBO 到目标     glBindBuffer: 绑定到 GL_ARRAY_BUFFER 目标。将该缓冲区标记为“活跃”。
	* 3. 传输数据到 VBO      glBufferData: CPU内存 --> 显存。将包含顶点数据的数组复制进当前“活跃”的缓冲区。
	* 4. 配置顶点属性格式    glVertexAttribPointer: 配置顶点属性格式，告诉OpenGL如何解析VBO数据。
                                                    “告诉 OpenGL：当渲染时，从这个 VBO 的显存里，按这个规则读数据。
	* 5. 启用顶点属性        glEnableVertexAttribArray: 启用该属性，相当于接上管子的水龙头打开了，前一步是接管子。
	* 6. 解绑 VBO           （可选，避免污染，VAO已记录关联关系）
    
    
    * 解绑的逻辑:
    * 1、VBO 可以在配置完成后立即解绑（无需等 VAO 解绑）。这里每个 VBO 配置完成后调用 glBindBuffer(GL_ARRAY_BUFFER, 0)，
         是为了避免后续代码（如其他 VBO 配置）误操作当前绑定的 VBO。
    * 2、EBO 必须等 VAO 解绑后，才能清理上下文的 EBO 绑定（VAO 绑定期间解绑 EBO 会导致 VAO 记录空的 EBO）。
         VAO 绑定期间，GL_ELEMENT_ARRAY_BUFFER 的绑定状态是「VAO 的一部分」—— 如果此时解绑 EBO，VAO 会把「当前绑定的 EBO = 空」记录下来；
    * 3、所有 VBO/EBO 配置完成后执行，保存所有配置，避免后续操作污染 VAO 状态。    
    */
}

// 析构函数：释放OpenGL资源（必须在OpenGL上下文有效时调用）
Geometry::~Geometry() {
    glDeleteBuffers(1, &mPosVbo);
    glDeleteBuffers(1, &mUvVbo);
    glDeleteBuffers(1, &mNormalVbo);
    glDeleteBuffers(1, &mTangentVbo);
    glDeleteBuffers(1, &mEbo);
    glDeleteVertexArrays(1, &mVao);
}

// 1. 创建立方体
Geometry* Geometry::createBox(float size) {
    Geometry* geometry = new Geometry();
    geometry->mIndicesCount = 36; // 6个面，每个面2个三角形（3*2），共6*6=36索引
    float halfSize = size / 2.0f;

    // 1.1 顶点位置数据（6个面，每个面4个顶点，共24个顶点）
    const float positions[] = {
        // 前面
        -halfSize, -halfSize,  halfSize,
         halfSize, -halfSize,  halfSize,
         halfSize,  halfSize,  halfSize,
        -halfSize,  halfSize,  halfSize,
        // 后面
        -halfSize, -halfSize, -halfSize,
         halfSize, -halfSize, -halfSize,
         halfSize,  halfSize, -halfSize,
        -halfSize,  halfSize, -halfSize,
        // 右面
         halfSize, -halfSize,  halfSize,
         halfSize, -halfSize, -halfSize,
         halfSize,  halfSize, -halfSize,
         halfSize,  halfSize,  halfSize,
         // 左面
         -halfSize, -halfSize, -halfSize,
         -halfSize, -halfSize,  halfSize,
         -halfSize,  halfSize,  halfSize,
         -halfSize,  halfSize, -halfSize,
         // 上面
         -halfSize,  halfSize,  halfSize,
          halfSize,  halfSize,  halfSize,
          halfSize,  halfSize, -halfSize,
         -halfSize,  halfSize, -halfSize,
         // 下面
         -halfSize, -halfSize, -halfSize,
          halfSize, -halfSize, -halfSize,
          halfSize, -halfSize,  halfSize,
         -halfSize, -halfSize,  halfSize
    };

    // 1.2 UV纹理坐标（每个面4个顶点，共24个UV对）
    const float uvs[] = {
        // 前面
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        // 后面
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        // 右面
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        // 左面
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        // 上面
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        // 下面
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    const float normals[]= {
        // 前面
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        // 后面
        0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        // 右面
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        // 左面
        -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        // 上面
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        // 下面
        0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f
    };

    // 1.3 索引数据（每个面2个三角形，共36个索引）
    const unsigned int indices[] = {
        0,1,2, 2,3,0,  // 前面
        4,5,6, 6,7,4,  // 后面
        8,9,10, 10,11,8,// 右面
        12,13,14,14,15,12,// 左面
        16,17,18,18,19,16,// 上面
        20,21,22,22,23,20 // 下面
    };

    // 1.4 创建VBO（位置+UV+法线）
    GLuint& posVbo = geometry->mPosVbo;
    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    GLuint& uvVbo = geometry->mUvVbo;
    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    GLuint& normalVbo = geometry->mNormalVbo;
    glGenBuffers(1, &normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

    // 1.5 创建EBO（索引）
    GLuint& ebo = geometry->mEbo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 1.6 创建VAO（管理VBO/EBO状态）
    GLuint& vao = geometry->mVao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 绑定位置VBO，设置顶点属性（location=0：位置，3个float）
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 绑定UV VBO，设置顶点属性（location=1：UV，2个float）
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 绑定法线VBO, 设置顶点属性（location=2：法线，3个float)
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 绑定EBO（VAO会记录EBO状态）
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // 解绑VAO（避免后续操作污染）
    glBindVertexArray(0);

    return geometry;
}

// 2. 创建球体
Geometry* Geometry::createSphere(float radius, int numLatLines, int numLongLines) {
    Geometry* geometry = new Geometry();
    std::vector<GLfloat> positions{};  // 顶点位置
    std::vector<GLfloat> uvs{};        // UV坐标
    std::vector<GLuint> indices{};     // 索引
    std::vector<GLfloat> normals{};     // 法线

    // 2.1 生成顶点位置和UV（经纬线遍历）
    for (int lat = 0; lat <= numLatLines; lat++) { // 纬线（0~π，含上下极点）
        for (int lon = 0; lon <= numLongLines; lon++) { // 经线（0~2π）
            // 球坐标转笛卡尔坐标（phi=纬度，theta=经度）
            float phi = lat * glm::pi<float>() / numLatLines;    // 0~π（上下极点）
            float theta = lon * 2 * glm::pi<float>() / numLongLines; // 0~2π（绕Y轴）

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi); // Y轴为球体上下方向
            float z = radius * sin(phi) * sin(theta);

            // UV坐标（U：经度方向，V：纬度方向，原点在左上角）
            float u = 1.0f - (float)lon / numLongLines; // 经度反向（避免纹理翻转）
            float v = 1.0f - (float)lat / numLatLines; // 纬度反向（极点在顶部）

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
            uvs.push_back(u);
            uvs.push_back(v);

            // 注意：法线方向没问题，但是法线的长度不为1，所以在fragment内要先归一化
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }
    }

    // 2.2 生成索引（每个经纬网格拆分为2个三角形）
    for (int lat = 0; lat < numLatLines; lat++) {
        for (int lon = 0; lon < numLongLines; lon++) {
            // 当前网格的4个顶点索引（lat行lon列、lat+1行lon列、lat行lon+1列、lat+1行lon+1列）
            GLuint p1 = lat * (numLongLines + 1) + lon;
            GLuint p2 = (lat + 1) * (numLongLines + 1) + lon;
            GLuint p3 = lat * (numLongLines + 1) + (lon + 1);
            GLuint p4 = (lat + 1) * (numLongLines + 1) + (lon + 1);

            // 两个三角形（p1-p2-p3 和 p3-p2-p4）
            indices.push_back(p1);
            indices.push_back(p2);
            indices.push_back(p3);
            indices.push_back(p3);
            indices.push_back(p2);
            indices.push_back(p4);
        }
    }

    // 2.3 记录索引数量
    geometry->mIndicesCount = static_cast<GLsizei>(indices.size());

    // 2.4 创建VBO（位置+UV）
    GLuint& posVbo = geometry->mPosVbo;
    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        positions.size() * sizeof(GLfloat),
        positions.data(),
        GL_STATIC_DRAW
    );

    GLuint& uvVbo = geometry->mUvVbo;
    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        uvs.size() * sizeof(GLfloat),
        uvs.data(),
        GL_STATIC_DRAW
    );

    GLuint& normalVbo = geometry->mNormalVbo;
    glGenBuffers(1, &normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        normals.size() * sizeof(GLfloat),
        normals.data(),
        GL_STATIC_DRAW
    );

    // 2.5 创建EBO（索引）
    GLuint& ebo = geometry->mEbo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(GLuint),
        indices.data(),
        GL_STATIC_DRAW
    );

    // 2.6 创建VAO
    GLuint& vao = geometry->mVao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 位置属性（location=0）
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // UV属性（location=1）
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);

    // 法线属性（location=2）
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // 绑定EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // 解绑VAO
    glBindVertexArray(0);

    return geometry;
}

// 创建固定大小的三角形
Geometry* Geometry::createTriangle() {
    Geometry* geometry = new Geometry();
    geometry->mIndicesCount = 3; // 三角形仅3个顶点，无需索引（直接用glDrawArrays，这里兼容原EBO逻辑，设为3）

    // 三角形顶点数据：3个顶点，每个顶点包含（x,y,z）位置 + （u,v）UV
    // 顶点顺序：左下 → 右下 → 顶部（逆时针，符合OpenGL正面剔除规则）
    const float vertices[] = {
        // 位置（x,y,z）        UV（u,v）
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,  // 左下顶点
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f,  // 右下顶点
         0.0f,  0.5f, 0.0f,    0.5f, 1.0f   // 顶部顶点
    };

    // 1. 创建VBO（顶点数据：位置+UV合并存储，避免多VBO）
    GLuint& posUvVbo = geometry->mPosVbo; // 复用mPosVbo存储合并数据（原mUvVbo可闲置，或在析构中正常释放）
    glGenBuffers(1, &posUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posUvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 2. 创建VAO（管理顶点属性）
    GLuint& vao = geometry->mVao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 3. 设置顶点属性：位置（location=0，3个float，步长=5*float，偏移=0）
    glBindBuffer(GL_ARRAY_BUFFER, posUvVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // 属性位置：对应Shader的aPos（location=0）
        3,                  // 组件数：x,y,z → 3个float
        GL_FLOAT,           // 数据类型
        GL_FALSE,           // 是否归一化
        5 * sizeof(float),  // 步长：每个顶点占5个float（3位置+2UV）
        (void*)0            // 偏移：位置数据在顶点开头
    );

    // 4. 设置顶点属性：UV（location=1，2个float，步长=5*float，偏移=3*float）
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                  // 属性位置：对应Shader的aTexCoord（location=1）
        2,                  // 组件数：u,v → 2个float
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),  // 步长同上（与位置共享顶点数据）
        (void*)(3 * sizeof(float)) // 偏移：UV在位置数据之后（3个float）
    );

    // 5. 解绑VAO（避免后续污染）
    glBindVertexArray(0);

    return geometry;
}


Geometry* Geometry::createPlane(float width, float height)
{
    Geometry* geometry = new Geometry();

    // 计算半宽和半高，确保平面以原点(0,0)为中心
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    // 顶点位置：围绕原点(0,0)分布，形成中心对称的平面
    const float positions[] = {
        -halfWidth, -halfHeight, 0.0f,  // 左下
         halfWidth, -halfHeight, 0.0f,  // 右下
         halfWidth,  halfHeight, 0.0f,  // 右上
        -halfWidth,  halfHeight, 0.0f   // 左上
    };

    // 纹理坐标：与顶点位置一一对应
    const float uvs[] = {
        0.0f, 0.0f,  // 左下
        1.0f, 0.0f,  // 右下
        1.0f, 1.0f,  // 右上
        0.0f, 1.0f   // 左上
    };

    const float normals[] = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    // 三角形索引：两个三角形组成矩形
    const unsigned int indices[] = {
        0, 1, 2,  // 第一个三角形
        0, 2, 3   // 第二个三角形
    };

    geometry->mIndicesCount = sizeof(indices) / sizeof(unsigned int);   // 必不可少

    // 创建位置VBO
    GLuint& posVbo = geometry->mPosVbo;
    glGenBuffers(1, &posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    // 创建UV VBO
    GLuint& uvVbo = geometry->mUvVbo;
    glGenBuffers(1, &uvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    // 创建法线VBO
    GLuint& normalVbo = geometry->mNormalVbo;
    glGenBuffers(1, &normalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

    // 创建EBO
    GLuint& ebo = geometry->mEbo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 创建并配置VAO
    GLuint& vao = geometry->mVao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 配置位置属性（location=0）
    glBindBuffer(GL_ARRAY_BUFFER, posVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 配置UV属性（location=1）
    glBindBuffer(GL_ARRAY_BUFFER, uvVbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 配置法线属性（location=2）
    glBindBuffer(GL_ARRAY_BUFFER, normalVbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 绑定EBO（会被VAO记录）
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // 解绑VAO
    glBindVertexArray(0);

    // 解绑缓冲区（可选，出于安全考虑）
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return geometry;
}

Geometry* Geometry::createScreenPlane() {
	Geometry* geometry = new Geometry();
	geometry->mIndicesCount = 6; // 2个三角形，共6个顶点

	// 构建数据：位置(x,y,z)
    const float positions[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f
	};  // 为什么这里特意忽略z？为了之后在shader中把其当作NDC坐标传入下一阶段。

	// 纹理坐标(u,v)
    const float uvs[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
	};

    // 索引数据
    const unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
	};

	// 创建位置VBO
	GLuint& posVbo = geometry->mPosVbo;
	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);  // 绑定位置VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);    // 上传位置数据
	// 创建UV VBO
	GLuint& uvVbo = geometry->mUvVbo;
    glGenBuffers(1, &uvVbo);
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);    // 绑定UV VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);    // 上传UV数据
	// 创建EBO
    GLuint& ebo = geometry->mEbo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);  // 绑定EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 上传索引数据

    // 创建并配置VAO
	GLuint& vao = geometry->mVao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);     // 绑定VAO,开始配置当前VAO - vao，记录后续绑定的VBO/EBO和顶点属性状态
	// 配置位置属性（location=0）
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	// 配置UV属性（location=1）
	glBindBuffer(GL_ARRAY_BUFFER, uvVbo);   // 绑定UV VBO
	glEnableVertexAttribArray(1);   // 启用UV属性
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // 设置UV属性指针
	// 绑定EBO（会被VAO记录）
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	// 解绑VAO
	glBindVertexArray(0);       // 解绑VAO，避免后续操作污染

	return geometry;
}

// 新增：创建2D画布
Geometry* Geometry::createCanvas(float width, float height) {
    Geometry* geometry = new Geometry();

    // 顶点数据：(x, y, z) + (u, v)
    // 采用正交投影坐标（原点在中心，范围与窗口一致）
    std::vector<float> vertices = {
        // 左下角
        0.0f, 0.0f , 0.0f,  0.0f, 0.0f,
        // 右下角
         width , 0.0f , 0.0f,  1.0f, 0.0f,
         // 右上角
          width ,  height , 0.0f,  1.0f, 1.0f,
          // 左上角
          0.0f ,  height , 0.0f,  0.0f, 1.0f
    };

    // 索引数据（两个三角形组成矩形）
    std::vector<unsigned int> indices = {
        0, 1, 2,  // 第一个三角形
        0, 2, 3   // 第二个三角形
    };
    geometry->mIndicesCount = indices.size();

    // 创建VAO
    GL_CALL(glGenVertexArrays(1, &geometry->mVao));
    GL_CALL(glBindVertexArray(geometry->mVao));

    // 创建顶点VBO（位置+UV）
    GL_CALL(glGenBuffers(1, &geometry->mPosVbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW));

    // 创建EBO
    GL_CALL(glGenBuffers(1, &geometry->mEbo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW));

    // 启用顶点属性（位置：3个float，步长5*float，偏移0）
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));

    // 启用UV属性（纹理坐标：2个float，步长5*float，偏移3*float）
    GL_CALL(glEnableVertexAttribArray(1));
    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));

    // 解绑VAO
    GL_CALL(glBindVertexArray(0));

    return geometry;
}


// 辅助结构体：存储OBJ面的单个顶点索引（v:顶点索引, vt:UV索引）
struct OBJFaceIndex {
    unsigned int v;  // 对应obj中"v"的索引（1-based）
    unsigned int vt; // 对应obj中"vt"的索引（1-based）
    unsigned int vn; // 对应obj中"vn"的索引（1-based）
};

// 从OBJ文件路径创建Geometry
Geometry* Geometry::createFromOBJ(const std::string& objFilePath) {
    // 1. 初始化解析所需的容器（新增法线相关容器）
    std::vector<glm::vec3> objVertices;    // 存储OBJ中的"v"顶点
    std::vector<glm::vec2> objUVs;         // 存储OBJ中的"vt"纹理坐标
    std::vector<glm::vec3> objNormals;     // 存储OBJ中的"vn"法线（新增）
    std::vector<OBJFaceIndex> faceIndices; // 存储面的顶点索引组合（v/vt/vn）
    std::vector<GLfloat> outVertices;      // 最终传入VBO的顶点坐标
    std::vector<GLfloat> outUVs;           // 最终传入VBO的UV坐标
    std::vector<GLfloat> outNormals;       // 最终传入VBO的法线（新增）
    std::vector<GLuint> outIndices;        // 最终传入EBO的索引

    // 2. 打开并读取OBJ文件
    std::ifstream objFile(objFilePath);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + objFilePath);
    }

    std::string line;
    while (std::getline(objFile, line)) {
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;

        // 2.1 解析顶点坐标（格式：v x y z）
        if (token == "v") {
            glm::vec3 vertex;
            lineStream >> vertex.x >> vertex.y >> vertex.z;
            objVertices.push_back(vertex);
        }
        // 2.2 解析纹理坐标（格式：vt u v）
        else if (token == "vt") {
            glm::vec2 uv;
            lineStream >> uv.x >> uv.y;
            objUVs.push_back(uv);
        }
        // 2.3 解析法线（格式：vn x y z）（新增）
        else if (token == "vn") {
            glm::vec3 normal;
            lineStream >> normal.x >> normal.y >> normal.z;
            objNormals.push_back(normal);
        }
        // 2.4 解析面（格式：f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...）
        else if (token == "f") {
            std::vector<OBJFaceIndex> faceVertexIndices;
            std::string faceToken;
            while (lineStream >> faceToken) {
                OBJFaceIndex idx = { 0, 0, 0 }; // 初始化包含法线索引
                std::istringstream faceIdxStream(faceToken);
                
                // 按"/"分割（格式：v/vt/vn）
                faceIdxStream >> idx.v;
                if (faceIdxStream.peek() == '/') {
                    faceIdxStream.ignore(); // 跳过第一个"/"
                    
                    // 解析纹理坐标索引
                    if (faceIdxStream.peek() != '/') {
                        faceIdxStream >> idx.vt;
                    }
                    
                    // 解析法线索引（新增）
                    if (faceIdxStream.peek() == '/') {
                        faceIdxStream.ignore(); // 跳过第二个"/"
                        faceIdxStream >> idx.vn;
                    }
                }
                faceVertexIndices.push_back(idx);
            }

            // 使用扇形分割法拆分多边形为三角形
            if (faceVertexIndices.size() >= 3) {
                for (size_t i = 1; i < faceVertexIndices.size() - 1; ++i) {
                    faceIndices.push_back(faceVertexIndices[0]);
                    faceIndices.push_back(faceVertexIndices[i]);
                    faceIndices.push_back(faceVertexIndices[i + 1]);
                }
            } else {
                throw std::runtime_error("Invalid face: requires at least 3 vertices");
            }
        }
        // 忽略其他指令
    }
    objFile.close();

    // 3. 去重顶点+UV+法线（新增法线因素）
    // 扩展key计算，包含法线索引
    std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> vertexUVNormalMap;
    unsigned int uniqueVertexCount = 0;

    for (const auto& idx : faceIndices) {
        // 转换为0-based索引
        unsigned int vIdx = idx.v - 1;
        unsigned int vtIdx = idx.vt - 1;
        unsigned int vnIdx = idx.vn - 1; // 新增
        
        // 检查索引有效性
        if (vIdx >= objVertices.size()) {
            throw std::runtime_error("Vertex index out of range");
        }
        if (vtIdx >= objUVs.size()) {
            throw std::runtime_error("UV index out of range");
        }
        if (vnIdx >= objNormals.size()) {
            throw std::runtime_error("Normal index out of range");
        }
        
        // 使用顶点、UV和法线索引的组合作为key
        auto key = std::make_tuple(vIdx, vtIdx, vnIdx);
        
        // 若该组合未出现过，添加到输出容器
        if (vertexUVNormalMap.find(key) == vertexUVNormalMap.end()) {
            vertexUVNormalMap[key] = uniqueVertexCount++;
            
            // 添加顶点坐标
            outVertices.push_back(objVertices[vIdx].x);
            outVertices.push_back(objVertices[vIdx].y);
            outVertices.push_back(objVertices[vIdx].z);
            
            // 添加UV坐标
            outUVs.push_back(objUVs[vtIdx].x);
            outUVs.push_back(objUVs[vtIdx].y);
            
            // 添加法线（新增）
            outNormals.push_back(objNormals[vnIdx].x);
            outNormals.push_back(objNormals[vnIdx].y);
            outNormals.push_back(objNormals[vnIdx].z);
        }
        
        // 记录当前面的索引
        outIndices.push_back(vertexUVNormalMap[key]);
    }

    // 4. 创建Geometry对象并初始化缓冲
    Geometry* geometry = new Geometry();
    geometry->mIndicesCount = static_cast<GLsizei>(outIndices.size());

    // 4.1 创建顶点VBO
    glGenBuffers(1, &geometry->mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outVertices.size() * sizeof(GLfloat),
        outVertices.data(),
        GL_STATIC_DRAW
    );

    // 4.2 创建UV VBO
    glGenBuffers(1, &geometry->mUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mUvVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outUVs.size() * sizeof(GLfloat),
        outUVs.data(),
        GL_STATIC_DRAW
    );

    // 4.3 创建法线VBO（新增）
    glGenBuffers(1, &geometry->mNormalVbo);  // 假设Geometry类有mNormalVbo成员
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mNormalVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outNormals.size() * sizeof(GLfloat),
        outNormals.data(),
        GL_STATIC_DRAW
    );

    // 4.4 创建EBO
    glGenBuffers(1, &geometry->mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        outIndices.size() * sizeof(GLuint),
        outIndices.data(),
        GL_STATIC_DRAW
    );

    // 4.5 创建VAO并配置属性
    glGenVertexArrays(1, &geometry->mVao);
    glBindVertexArray(geometry->mVao);

    // 配置顶点属性（location=0）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // 配置UV属性（location=1）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mUvVbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);

    // 配置法线属性（location=2）（新增）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mNormalVbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // 绑定EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);

    // 解绑VAO
    glBindVertexArray(0);

    return geometry;
}


Geometry* Geometry::createFromOBJ_nvn(const std::string& objFilePath) {
    // 1. 初始化解析所需的容器
    std::vector<glm::vec3> objVertices;    // 存储OBJ中的"v"顶点（原始数据）
    std::vector<glm::vec2> objUVs;         // 存储OBJ中的"vt"纹理坐标（原始数据）
    std::vector<OBJFaceIndex> faceIndices; // 存储面的顶点索引组合（v/vt）
    std::vector<GLfloat> outVertices;      // 最终传入VBO的顶点坐标（去重后）
    std::vector<GLfloat> outUVs;           // 最终传入VBO的UV坐标（去重后）
    std::vector<GLuint> outIndices;        // 最终传入EBO的索引（0-based）

    // 2. 打开并读取OBJ文件
    std::ifstream objFile(objFilePath);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + objFilePath);
    }

    std::string line;
    while (std::getline(objFile, line)) {
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;

        // 2.1 解析顶点坐标（格式：v x y z）
        if (token == "v") {
            glm::vec3 vertex;
            lineStream >> vertex.x >> vertex.y >> vertex.z;
            objVertices.push_back(vertex);
        }
        // 2.2 解析纹理坐标（格式：vt u v）
        else if (token == "vt") {
            glm::vec2 uv;
            lineStream >> uv.x >> uv.y;
            // OBJ的UV原点在左下角，OpenGL默认相同，无需翻转（若纹理上下颠倒可改为 uv.y = 1.0f - uv.y）
            objUVs.push_back(uv);
        }
        // 2.3 解析面（格式：f v1/vt1 v2/vt2 v3/vt3 ...）
        else if (token == "f") {
            // 读取面的每个顶点索引（支持任意n边形，使用扇形分割法拆分为三角形）
            std::vector<OBJFaceIndex> faceVertexIndices;
            std::string faceToken;
            while (lineStream >> faceToken) {
                OBJFaceIndex idx = { 0, 0 };
                std::istringstream faceIdxStream(faceToken);
                // 按"/"分割（格式：v/vt 或 v/vt/vn，此处只取前两个）
                faceIdxStream >> idx.v;
                if (faceIdxStream.peek() == '/') {
                    faceIdxStream.ignore(); // 跳过第一个"/"
                    if (faceIdxStream.peek() != '/') { // 存在vt索引
                        faceIdxStream >> idx.vt;
                    }
                    faceIdxStream.ignore(); // 跳过第二个"/"（忽略vn）
                }
                faceVertexIndices.push_back(idx);
            }

            // 使用扇形分割法拆分多边形为三角形
            // 原理：以第一个顶点为公共顶点，与其他顶点组成三角形 (v0, v1, v2), (v0, v2, v3), ..., (v0, vn-1, vn)
            if (faceVertexIndices.size() >= 3) { // 至少需要3个顶点才能组成多边形
                for (size_t i = 1; i < faceVertexIndices.size() - 1; ++i) {
                    // 添加第一个三角形 (v0, v1, v2)，然后是(v0, v2, v3)，以此类推
                    faceIndices.push_back(faceVertexIndices[0]);       // 公共顶点（第一个顶点）
                    faceIndices.push_back(faceVertexIndices[i]);       // 当前顶点
                    faceIndices.push_back(faceVertexIndices[i + 1]);   // 下一个顶点
                }
            }
            else {
                // 处理无效面（顶点数不足3个）
                throw std::runtime_error("Invalid face: requires at least 3 vertices");
            }
        }
        // 忽略其他指令（如mtllib、vn、o等）
    }
    objFile.close();

    // 3. 去重顶点+UV（避免重复数据，减少VBO大小）
    // 核心逻辑：用"顶点索引+UV索引"的组合作为key，记录唯一顶点的位置
    std::vector<unsigned int> vertexUVMap(objVertices.size() * objUVs.size(), -1);
    unsigned int uniqueVertexCount = 0;

    for (const auto& idx : faceIndices) {
        // 计算key（若OBJ索引从1开始→减1转为0-based；若OBJ索引从0开始，不用操作）
        unsigned int vIdx = idx.v - 1;
        unsigned int vtIdx = idx.vt - 1;
        if (vIdx >= objVertices.size() || vtIdx >= objUVs.size()) {
            throw std::runtime_error("OBJ index out of range (invalid face data)");
        }
        unsigned int key = vIdx * objUVs.size() + vtIdx;

        // 若该顶点+UV组合未出现过，添加到输出容器
        if (vertexUVMap[key] == -1) {
            vertexUVMap[key] = uniqueVertexCount++;
            // 添加顶点坐标（x,y,z）
            outVertices.push_back(objVertices[vIdx].x);
            outVertices.push_back(objVertices[vIdx].y);
            outVertices.push_back(objVertices[vIdx].z);
            // 添加UV坐标（u,v）
            outUVs.push_back(objUVs[vtIdx].x);
            outUVs.push_back(objUVs[vtIdx].y);
        }
        // 记录当前面的索引（指向唯一顶点的位置）
        outIndices.push_back(vertexUVMap[key]);
    }

    // 4. 创建Geometry对象并初始化缓冲
    Geometry* geometry = new Geometry();
    geometry->mIndicesCount = static_cast<GLsizei>(outIndices.size());

    // 4.1 创建顶点VBO（位置数据）
    glGenBuffers(1, &geometry->mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outVertices.size() * sizeof(GLfloat),
        outVertices.data(),
        GL_STATIC_DRAW
    );

    // 4.2 创建UV VBO（纹理坐标数据）
    glGenBuffers(1, &geometry->mUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mUvVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outUVs.size() * sizeof(GLfloat),
        outUVs.data(),
        GL_STATIC_DRAW
    );

    // 4.3 创建EBO（索引数据）
    glGenBuffers(1, &geometry->mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        outIndices.size() * sizeof(GLuint),
        outIndices.data(),
        GL_STATIC_DRAW
    );

    // 4.4 创建VAO（管理顶点属性）
    glGenVertexArrays(1, &geometry->mVao);
    glBindVertexArray(geometry->mVao);

    // 绑定位置VBO，设置顶点属性（location=0：对应着色器aPos）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // 属性位置
        3,                  // 组件数（x,y,z）
        GL_FLOAT,           // 数据类型
        GL_FALSE,           // 不归一化
        3 * sizeof(GLfloat),// 步长（每个顶点占3个float）
        (void*)0            // 偏移（从开头读取）
    );

    // 绑定UV VBO，设置顶点属性（location=1：对应着色器aTexCoord）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mUvVbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                  // 属性位置
        2,                  // 组件数（u,v）
        GL_FLOAT,           // 数据类型
        GL_FALSE,           // 不归一化
        2 * sizeof(GLfloat),// 步长（每个UV占2个float）
        (void*)0            // 偏移（从开头读取）
    );

    // 绑定EBO（VAO会记录EBO状态）
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);

    // 解绑VAO（避免后续操作污染）
    glBindVertexArray(0);

    return geometry;
}


Geometry* Geometry::createFromOBJwithTangent(const std::string& objFilePath) {
    // 1. 初始化解析所需的容器
    std::vector<glm::vec3> objVertices;
    std::vector<glm::vec2> objUVs;
    std::vector<glm::vec3> objNormals;
    std::vector<OBJFaceIndex> faceIndices;
    std::vector<GLfloat> outVertices;
    std::vector<GLfloat> outUVs;
    std::vector<GLfloat> outNormals;
    std::vector<GLuint> outIndices;

    // 2. 打开并读取OBJ文件
    std::ifstream objFile(objFilePath);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + objFilePath);
    }

    std::string line;
    while (std::getline(objFile, line)) {
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;

        if (token == "v") {
            glm::vec3 vertex;
            lineStream >> vertex.x >> vertex.y >> vertex.z;
            objVertices.push_back(vertex);
        }
        else if (token == "vt") {
            glm::vec2 uv;
            lineStream >> uv.x >> uv.y;
            objUVs.push_back(uv);
        }
        else if (token == "vn") {
            glm::vec3 normal;
            lineStream >> normal.x >> normal.y >> normal.z;
            objNormals.push_back(normal);
        }
        else if (token == "f") {
            std::vector<OBJFaceIndex> faceVertexIndices;
            std::string faceToken;
            while (lineStream >> faceToken) {
                OBJFaceIndex idx = { 0, 0, 0 };
                std::istringstream faceIdxStream(faceToken);

                faceIdxStream >> idx.v;
                if (faceIdxStream.peek() == '/') {
                    faceIdxStream.ignore();
                    if (faceIdxStream.peek() != '/') {
                        faceIdxStream >> idx.vt;
                    }
                    if (faceIdxStream.peek() == '/') {
                        faceIdxStream.ignore();
                        faceIdxStream >> idx.vn;
                    }
                }
                faceVertexIndices.push_back(idx);
            }

            if (faceVertexIndices.size() >= 3) {
                for (size_t i = 1; i < faceVertexIndices.size() - 1; ++i) {
                    faceIndices.push_back(faceVertexIndices[0]);
                    faceIndices.push_back(faceVertexIndices[i]);
                    faceIndices.push_back(faceVertexIndices[i + 1]);
                }
            }
            else {
                throw std::runtime_error("Invalid face: requires at least 3 vertices");
            }
        }
    }
    objFile.close();

    // 3. 去重顶点+UV+法线
    std::map<std::tuple<unsigned int, unsigned int, unsigned int>, unsigned int> vertexUVNormalMap;
    unsigned int uniqueVertexCount = 0;

    for (const auto& idx : faceIndices) {
        unsigned int vIdx = idx.v - 1;
        unsigned int vtIdx = idx.vt - 1;
        unsigned int vnIdx = idx.vn - 1;

        if (vIdx >= objVertices.size() || vtIdx >= objUVs.size() || vnIdx >= objNormals.size()) {
            throw std::runtime_error("Index out of range in OBJ file.");
        }

        auto key = std::make_tuple(vIdx, vtIdx, vnIdx);

        if (vertexUVNormalMap.find(key) == vertexUVNormalMap.end()) {
            vertexUVNormalMap[key] = uniqueVertexCount++;

            outVertices.push_back(objVertices[vIdx].x);
            outVertices.push_back(objVertices[vIdx].y);
            outVertices.push_back(objVertices[vIdx].z);

            outUVs.push_back(objUVs[vtIdx].x);
            outUVs.push_back(objUVs[vtIdx].y);

            outNormals.push_back(objNormals[vnIdx].x);
            outNormals.push_back(objNormals[vnIdx].y);
            outNormals.push_back(objNormals[vnIdx].z);
        }

        outIndices.push_back(vertexUVNormalMap[key]);
    }

    // =================================================================
    // 新增：计算切线向量
    // =================================================================
    std::vector<glm::vec3> tangents(uniqueVertexCount, glm::vec3(0.0f));

    for (size_t i = 0; i < outIndices.size(); i += 3) {
        GLuint idx0 = outIndices[i];
        GLuint idx1 = outIndices[i + 1];
        GLuint idx2 = outIndices[i + 2];

        glm::vec3 v0(outVertices[idx0 * 3], outVertices[idx0 * 3 + 1], outVertices[idx0 * 3 + 2]);
        glm::vec3 v1(outVertices[idx1 * 3], outVertices[idx1 * 3 + 1], outVertices[idx1 * 3 + 2]);
        glm::vec3 v2(outVertices[idx2 * 3], outVertices[idx2 * 3 + 1], outVertices[idx2 * 3 + 2]);

        glm::vec2 uv0(outUVs[idx0 * 2], outUVs[idx0 * 2 + 1]);
        glm::vec2 uv1(outUVs[idx1 * 2], outUVs[idx1 * 2 + 1]);
        glm::vec2 uv2(outUVs[idx2 * 2], outUVs[idx2 * 2 + 1]);

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        if (std::isinf(f) || std::isnan(f)) {
            f = 1.0f; // 处理纹理坐标相同导致的除零错误
        }
        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        tangents[idx0] += tangent;
        tangents[idx1] += tangent;
        tangents[idx2] += tangent;
    }

    std::vector<GLfloat> outTangents;
    for (const auto& t : tangents) {
        glm::vec3 normalizedTangent = glm::normalize(t);
        outTangents.push_back(normalizedTangent.x);
        outTangents.push_back(normalizedTangent.y);
        outTangents.push_back(normalizedTangent.z);
    }
    // =================================================================
    // 新增代码结束
    // =================================================================

    // 4. 创建Geometry对象并初始化缓冲
    Geometry* geometry = new Geometry();
    geometry->mIndicesCount = static_cast<GLsizei>(outIndices.size());

    glGenBuffers(1, &geometry->mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glBufferData(GL_ARRAY_BUFFER, outVertices.size() * sizeof(GLfloat), outVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &geometry->mUvVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mUvVbo);
    glBufferData(GL_ARRAY_BUFFER, outUVs.size() * sizeof(GLfloat), outUVs.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &geometry->mNormalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mNormalVbo);
    glBufferData(GL_ARRAY_BUFFER, outNormals.size() * sizeof(GLfloat), outNormals.data(), GL_STATIC_DRAW);

    // 新增：创建切线VBO
    glGenBuffers(1, &geometry->mTangentVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mTangentVbo);
    glBufferData(GL_ARRAY_BUFFER, outTangents.size() * sizeof(GLfloat), outTangents.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &geometry->mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, outIndices.size() * sizeof(GLuint), outIndices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &geometry->mVao);
    glBindVertexArray(geometry->mVao);

    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, geometry->mUvVbo);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, geometry->mNormalVbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // 新增：配置切线属性 (location=3)
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mTangentVbo);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return geometry;
}



// 辅助结构体：存储STL顶点与法线索引的关联
struct STLFaceVertex {
    glm::vec3 pos;    // 顶点坐标
    unsigned int nIdx; // 关联的法线索引（0-based）
};

// 工具函数：处理小端序（二进制STL默认小端存储）
template <typename T>
T readLittleEndian(std::ifstream& file) {
    T data;
    file.read(reinterpret_cast<char*>(&data), sizeof(T));
    // 判断当前平台是否为小端，非小端则字节交换（适配Linux/macOS大端场景）
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    char* bytes = reinterpret_cast<char*>(&data);
    for (size_t i = 0; i < sizeof(T) / 2; ++i) {
        std::swap(bytes[i], bytes[sizeof(T) - 1 - i]);
    }
#endif
    return data;
}

// 自定义比较器结构体
struct Vec3Compare {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    }
};


Geometry* Geometry::createFromSTL(const std::string& stlFilePath, bool useSmoothNormals) {
    // 1. 初始化解析容器
    std::vector<glm::vec3> stlNormals;       // 存储所有面片的法线
    std::vector<STLFaceVertex> faceVertices; // 存储所有顶点（含原始法线索引）
    std::vector<GLfloat> outVertices;        // 最终VBO顶点数据
    std::vector<GLfloat> outNormals;         // 最终VBO法线数据
    std::vector<GLuint> outIndices;          // 最终EBO索引数据

    // 2. 以二进制模式打开文件
    std::ifstream stlFile(stlFilePath, std::ios::in | std::ios::binary);
    if (!stlFile.is_open()) {
        throw std::runtime_error("Failed to open binary STL file: " + stlFilePath);
    }

    // 3. 解析文件头（80字节，直接跳过）
    char header[80];
    stlFile.read(header, sizeof(header));
    if (stlFile.gcount() != sizeof(header)) {
        throw std::runtime_error("Invalid binary STL: incomplete header");
    }

    // 4. 解析面片数量（4字节unsigned int，小端序）
    unsigned int faceCount = readLittleEndian<unsigned int>(stlFile);
    if (faceCount == 0) {
        throw std::runtime_error("Binary STL has no faces (face count = 0)");
    }

    // 5. 解析每个面片的数据（50字节/个）
    const size_t FACE_DATA_SIZE = 50;
    char faceBuffer[FACE_DATA_SIZE];

    for (unsigned int i = 0; i < faceCount; ++i) {
        stlFile.read(faceBuffer, FACE_DATA_SIZE);
        if (stlFile.gcount() != FACE_DATA_SIZE) {
            throw std::runtime_error("Invalid binary STL: incomplete face data at index " + std::to_string(i));
        }

        // 5.1 解析面片法线
        glm::vec3 faceNormal;
        memcpy(&faceNormal.x, faceBuffer + 0, sizeof(float));
        memcpy(&faceNormal.y, faceBuffer + 4, sizeof(float));
        memcpy(&faceNormal.z, faceBuffer + 8, sizeof(float));
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        faceNormal.x = readLittleEndian<float>(std::istringstream(std::string(faceBuffer + 0, 4)));
        faceNormal.y = readLittleEndian<float>(std::istringstream(std::string(faceBuffer + 4, 4)));
        faceNormal.z = readLittleEndian<float>(std::istringstream(std::string(faceBuffer + 8, 4)));
#endif
        stlNormals.push_back(faceNormal);
        unsigned int normalIdx = stlNormals.size() - 1;

        // 5.2 解析3个顶点
        glm::vec3 vertices[3];
        memcpy(&vertices[0].x, faceBuffer + 12, sizeof(float));
        memcpy(&vertices[0].y, faceBuffer + 16, sizeof(float));
        memcpy(&vertices[0].z, faceBuffer + 20, sizeof(float));
        memcpy(&vertices[1].x, faceBuffer + 24, sizeof(float));
        memcpy(&vertices[1].y, faceBuffer + 28, sizeof(float));
        memcpy(&vertices[1].z, faceBuffer + 32, sizeof(float));
        memcpy(&vertices[2].x, faceBuffer + 36, sizeof(float));
        memcpy(&vertices[2].y, faceBuffer + 40, sizeof(float));
        memcpy(&vertices[2].z, faceBuffer + 44, sizeof(float));
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        for (int v = 0; v < 3; ++v) {
            vertices[v].x = readLittleEndian<float>(std::istringstream(std::string(faceBuffer + 12 + v * 12, 4)));
            vertices[v].y = readLittleEndian<float>(std::istringstream(std::string(faceBuffer + 16 + v * 12, 4)));
            vertices[v].z = readLittleEndian<float>(std::istringstream(std::string(faceBuffer + 20 + v * 12, 4)));
        }
#endif

        // 5.3 存储当前面片的3个顶点（关联原始法线索引）
        faceVertices.push_back({ vertices[0], normalIdx });
        faceVertices.push_back({ vertices[1], normalIdx });
        faceVertices.push_back({ vertices[2], normalIdx });
    }

    stlFile.close();

    // 6. 计算平滑顶点法线（若启用）
    std::vector<glm::vec3> smoothVertexNormals;
    // 使用自定义比较器定义 map（替换原来的 vertexIndexMap）
    std::map<glm::vec3, unsigned int, Vec3Compare> vertexIndexMap;// 顶点位置→索引的映射
    if (useSmoothNormals) {
        // 步骤1：为每个顶点收集所有关联的面法线
        std::vector<std::vector<glm::vec3>> vertexNormalLists;
        unsigned int vertexCount = 0;
        for (const auto& fv : faceVertices) {
            if (vertexIndexMap.find(fv.pos) == vertexIndexMap.end()) {
                vertexIndexMap[fv.pos] = vertexCount++;
                vertexNormalLists.emplace_back();
            }
            unsigned int idx = vertexIndexMap[fv.pos];
            vertexNormalLists[idx].push_back(stlNormals[fv.nIdx]);
        }

        // 步骤2：对每个顶点的法线列表求平均并归一化
        smoothVertexNormals.resize(vertexCount);
        for (size_t i = 0; i < vertexCount; ++i) {
            glm::vec3 avgNormal(0.0f);
            for (const auto& n : vertexNormalLists[i]) {
                avgNormal += n;
            }
            // 避免零向量， fallback到默认方向
            if (glm::length(avgNormal) > 1e-6f) {
                avgNormal = glm::normalize(avgNormal);
            }
            else {
                avgNormal = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            smoothVertexNormals[i] = avgNormal;
        }
    }

    // 7. 顶点去重并生成最终缓冲数据
    using VertexKey = std::tuple<float, float, float, unsigned int>;
    std::map<VertexKey, unsigned int> vertexNormalMap;
    unsigned int uniqueVertexCount = 0;

    for (const auto& faceVert : faceVertices) {
        VertexKey key = std::make_tuple(
            faceVert.pos.x, faceVert.pos.y, faceVert.pos.z,
            faceVert.nIdx
        );

        if (vertexNormalMap.find(key) == vertexNormalMap.end()) {
            vertexNormalMap[key] = uniqueVertexCount++;
            // 顶点坐标
            outVertices.push_back(faceVert.pos.x);
            outVertices.push_back(faceVert.pos.y);
            outVertices.push_back(faceVert.pos.z);
            // 法线：根据是否启用平滑法线选择
            glm::vec3 normal;
            if (useSmoothNormals) {
                auto it = vertexIndexMap.find(faceVert.pos);
                if (it != vertexIndexMap.end()) {
                    normal = smoothVertexNormals[it->second];
                }
                else {
                    normal = stlNormals[faceVert.nIdx]; // 理论上不会触发
                }
            }
            else {
                normal = stlNormals[faceVert.nIdx];
            }
            outNormals.push_back(normal.x);
            outNormals.push_back(normal.y);
            outNormals.push_back(normal.z);
        }

        outIndices.push_back(vertexNormalMap[key]);
    }

    // 8. 创建并初始化GPU缓冲
    Geometry* geometry = new Geometry();
    geometry->mIndicesCount = static_cast<GLsizei>(outIndices.size());

    // 8.1 顶点VBO
    glGenBuffers(1, &geometry->mPosVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outVertices.size() * sizeof(GLfloat),
        outVertices.data(),
        GL_STATIC_DRAW
    );

    // 8.2 法线VBO
    glGenBuffers(1, &geometry->mNormalVbo);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mNormalVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        outNormals.size() * sizeof(GLfloat),
        outNormals.data(),
        GL_STATIC_DRAW
    );

    // 8.3 索引EBO
    glGenBuffers(1, &geometry->mEbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        outIndices.size() * sizeof(GLuint),
        outIndices.data(),
        GL_STATIC_DRAW
    );

    // 8.4 VAO配置
    glGenVertexArrays(1, &geometry->mVao);
    glBindVertexArray(geometry->mVao);

    // 顶点属性（location=0）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mPosVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // 法线属性（location=2）
    glBindBuffer(GL_ARRAY_BUFFER, geometry->mNormalVbo);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

    // 绑定EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->mEbo);

    glBindVertexArray(0);

    return geometry;
}