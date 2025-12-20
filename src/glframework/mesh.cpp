#include "mesh.h"

Mesh::Mesh(Geometry* geometry, Material* material) {
    mGeometry = geometry;
    mMaterial = material;
    mType = ObjectType::Mesh;   // 设置object的类型，不同的object有不同的操作
}

Mesh::~Mesh() {
}