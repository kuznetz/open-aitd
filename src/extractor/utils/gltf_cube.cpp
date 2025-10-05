#pragma once
#include "my_gltf.h"

tinygltf::Primitive createCubePrim(tinygltf::Model& m, Vector3 center, Vector3 size, int material) {
    const int vertexCount = 8;
    const int triangleCount = 12; // 6 faces * 2 triangles each
    
    // Calculate half dimensions
    Vector3 halfSize = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
    
    // Generate vertices for the cube
    std::vector<Vector3> vertices(vertexCount);
    
    // Define all 8 vertices of the cube
    vertices[0] = { center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z }; // bottom-back-left
    vertices[1] = { center.x + halfSize.x, center.y - halfSize.y, center.z - halfSize.z }; // bottom-back-right
    vertices[2] = { center.x + halfSize.x, center.y + halfSize.y, center.z - halfSize.z }; // top-back-right
    vertices[3] = { center.x - halfSize.x, center.y + halfSize.y, center.z - halfSize.z }; // top-back-left
    vertices[4] = { center.x - halfSize.x, center.y - halfSize.y, center.z + halfSize.z }; // bottom-front-left
    vertices[5] = { center.x + halfSize.x, center.y - halfSize.y, center.z + halfSize.z }; // bottom-front-right
    vertices[6] = { center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z }; // top-front-right
    vertices[7] = { center.x - halfSize.x, center.y + halfSize.y, center.z + halfSize.z }; // top-front-left
    
    // Generate indices for triangles (12 triangles = 6 faces * 2 triangles each)
    std::vector<unsigned int> indices(triangleCount * 3);
    int index = 0;
    
    // Front face
    indices[index++] = 4; indices[index++] = 5; indices[index++] = 6;
    indices[index++] = 4; indices[index++] = 6; indices[index++] = 7;
    
    // Back face
    indices[index++] = 1; indices[index++] = 0; indices[index++] = 3;
    indices[index++] = 1; indices[index++] = 3; indices[index++] = 2;
    
    // Left face
    indices[index++] = 0; indices[index++] = 4; indices[index++] = 7;
    indices[index++] = 0; indices[index++] = 7; indices[index++] = 3;
    
    // Right face
    indices[index++] = 5; indices[index++] = 1; indices[index++] = 2;
    indices[index++] = 5; indices[index++] = 2; indices[index++] = 6;
    
    // Top face
    indices[index++] = 3; indices[index++] = 7; indices[index++] = 6;
    indices[index++] = 3; indices[index++] = 6; indices[index++] = 2;
    
    // Bottom face
    indices[index++] = 0; indices[index++] = 1; indices[index++] = 5;
    indices[index++] = 0; indices[index++] = 5; indices[index++] = 4;
    
    // Create vertex buffer accessor
    int vertAccIdx = createVertexes(m, vertices);
    // Create primitive
    return createPolyPrimitive(m, indices, vertAccIdx, material);
}
