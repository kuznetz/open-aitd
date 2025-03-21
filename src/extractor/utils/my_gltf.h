#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <vector>
#include <raymath.h>
#include <cstring>

using namespace std;

extern const uint8_t cubeIndices[];
extern const float cubeVertices[];
extern const int cubeVertSize;

int createCubeMesh(tinygltf::Model& m);
int createLineMesh(tinygltf::Model& m, const vector<float>& line);
int createPolyMesh(tinygltf::Model& m, const vector<Vector3>& vertexes, const vector<unsigned int>& indices);
int createVertexes(tinygltf::Model& m, const vector<Vector3>& vertexes);
void addVertexSkin(tinygltf::Model& m, vector<unsigned char> vecBoneAffect);
tinygltf::Primitive createPolyPrimitive(tinygltf::Model& m, const vector<unsigned int>& indices, int vertAccIdx, int material);

const uint8_t cubeIndices[] = {
    //Top
    2, 6, 7, 2, 3, 7,
    //Bottom
    0, 4, 5, 0, 1, 5,
    //Left
    0, 2, 6, 0, 4, 6,
    //Right
    1, 3, 7, 1, 5, 7,
    //Front
    0, 2, 3, 0, 1, 3,
    //Back
    4, 6, 7, 4, 5, 7
}; //36

const float cubeVertices[] = {
    0, 0, 1, //0
    1, 0, 1, //1
    0, 1, 1, //2
    1, 1, 1, //3
    0, 0, 0, //4
    1, 0, 0, //5
    0, 1, 0, //6
    1, 1, 0  //7
}; //24
const int cubeVertSize = 24 * sizeof(float);

int createCubeMesh(tinygltf::Model& m) {
    //Store Cube
    tinygltf::Buffer cubeBuffer;
    cubeBuffer.data.resize(cubeVertSize + 36);
    memcpy(cubeBuffer.data.data(), cubeVertices, cubeVertSize);
    memcpy(cubeBuffer.data.data() + cubeVertSize, cubeIndices, 36);
    m.buffers.push_back(cubeBuffer);

    tinygltf::BufferView cubeVertVw;
    cubeVertVw.buffer = 0;
    cubeVertVw.byteOffset = 0;
    cubeVertVw.byteLength = cubeVertSize;
    cubeVertVw.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    m.bufferViews.push_back(cubeVertVw);

    tinygltf::Accessor cubeVertAcc;
    cubeVertAcc.bufferView = 0;
    cubeVertAcc.byteOffset = 0;
    cubeVertAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    cubeVertAcc.count = 8;
    cubeVertAcc.type = TINYGLTF_TYPE_VEC3;
    //lineAcc.maxValues = { 1.0, 1.0, 0.0 };
    //lineAcc.minValues = { 0.0, 0.0, 0.0 };
    m.accessors.push_back(cubeVertAcc);

    tinygltf::BufferView cubeIdxVw;
    cubeIdxVw.buffer = 0;
    cubeIdxVw.byteOffset = cubeVertSize;
    cubeIdxVw.byteLength = 36;
    cubeIdxVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(cubeIdxVw);

    tinygltf::Accessor cubeIdxAcc;
    cubeIdxAcc.bufferView = 1;
    cubeIdxAcc.byteOffset = 0;
    cubeIdxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    cubeIdxAcc.count = 36;
    cubeIdxAcc.type = TINYGLTF_TYPE_SCALAR;
    //weightsAcc.maxValues.push_back(2);
    //weightsAcc.minValues.push_back(0);
    m.accessors.push_back(cubeIdxAcc);

    tinygltf::Primitive primitive;
    primitive.indices = 1;                 // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = 0;  // The index of the accessor for positions
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Mesh mesh;
    mesh.primitives.push_back(primitive);
    m.meshes.push_back(mesh);
    return m.meshes.size() - 1;
}

int createLineMesh(tinygltf::Model& m, const vector<float>& line) {
    int lineBytes = line.size() * sizeof(float);
    tinygltf::Buffer buffer;
    buffer.data.resize(lineBytes);
    memcpy(buffer.data.data(), line.data(), lineBytes);
    m.buffers.push_back(buffer);
    int bufIdx = m.buffers.size() - 1;

    tinygltf::BufferView lineVw;
    lineVw.buffer = bufIdx;
    lineVw.byteOffset = 0;
    lineVw.byteLength = lineBytes;
    lineVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(lineVw);
    int vwIdx = m.bufferViews.size() - 1;

    tinygltf::Accessor lineAcc;
    lineAcc.bufferView = vwIdx;
    lineAcc.byteOffset = 0;
    lineAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    lineAcc.count = line.size() / 3;
    lineAcc.type = TINYGLTF_TYPE_VEC3;
    m.accessors.push_back(lineAcc);
    int accIdx = m.accessors.size() - 1;

    tinygltf::Primitive primitive;
    //primitive.indices = 1;                   // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = accIdx; // The index of the accessor for positions
    primitive.mode = TINYGLTF_MODE_LINE_LOOP;
    tinygltf::Mesh mesh;
    mesh.primitives.push_back(primitive);
    m.meshes.push_back(mesh);
    return m.meshes.size() - 1;
}

int createVertexes(tinygltf::Model& m, const vector<Vector3>& vertexes) {
    int vertBytes = vertexes.size() * sizeof(Vector3);
    tinygltf::Buffer buffer;
    buffer.data.resize(vertBytes);
    memcpy(buffer.data.data(), vertexes.data(), vertBytes);
    m.buffers.push_back(buffer);
    int bufIdx = m.buffers.size() - 1;

    tinygltf::BufferView vertVw;
    vertVw.buffer = bufIdx;
    vertVw.byteOffset = 0;
    vertVw.byteLength = vertBytes;
    vertVw.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    m.bufferViews.push_back(vertVw);

    tinygltf::Accessor vertAcc;
    vertAcc.bufferView = m.bufferViews.size() - 1;
    vertAcc.byteOffset = 0;
    vertAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    vertAcc.count = vertexes.size();
    vertAcc.type = TINYGLTF_TYPE_VEC3;
    m.accessors.push_back(vertAcc);
    auto vertAccIdx = m.accessors.size() - 1;
    return vertAccIdx;
}

tinygltf::Primitive createPolyPrimitive(tinygltf::Model& m, const vector<unsigned int>& indices, int vertAccIdx, int material) {
    int indxBytes = indices.size() * sizeof(unsigned int);
    tinygltf::Buffer buffer;
    buffer.data.resize(indxBytes);
    memcpy(buffer.data.data(), indices.data(), indxBytes);
    m.buffers.push_back(buffer);
    int bufIdx = m.buffers.size() - 1;

    tinygltf::BufferView idxVw;
    idxVw.buffer = bufIdx;
    idxVw.byteOffset = 0;
    idxVw.byteLength = indxBytes;
    idxVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(idxVw);

    tinygltf::Accessor idxAcc;
    idxAcc.bufferView = m.bufferViews.size() - 1;
    idxAcc.byteOffset = 0;
    idxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAcc.count = indices.size();
    idxAcc.type = TINYGLTF_TYPE_SCALAR;
    m.accessors.push_back(idxAcc);
    auto idxAccIdx = m.accessors.size() - 1;

    tinygltf::Primitive primitive;
    primitive.material = material;
    primitive.indices = idxAccIdx;                 // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = vertAccIdx; // The index of the accessor for positions
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    return primitive;
}

int createPolyMesh(tinygltf::Model& m, const vector<Vector3>& vertexes, const vector<unsigned int>& indices) {
    int vertBytes = vertexes.size() * sizeof(Vector3);
    int indxBytes = indices.size() * sizeof(unsigned int);
    tinygltf::Buffer buffer;
    buffer.data.resize(vertBytes + indxBytes);
    memcpy(buffer.data.data(), vertexes.data(), vertBytes);
    memcpy(buffer.data.data() + vertBytes, indices.data(), indxBytes);
    m.buffers.push_back(buffer);
    int bufIdx = m.buffers.size() - 1;

    tinygltf::BufferView vertVw;
    vertVw.buffer = bufIdx;
    vertVw.byteOffset = 0;
    vertVw.byteLength = vertBytes;
    vertVw.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    m.bufferViews.push_back(vertVw);

    tinygltf::Accessor vertAcc;
    vertAcc.bufferView = m.bufferViews.size() - 1;
    vertAcc.byteOffset = 0;
    vertAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    vertAcc.count = vertexes.size();
    vertAcc.type = TINYGLTF_TYPE_VEC3;
    m.accessors.push_back(vertAcc);
    auto vertAccIdx = m.accessors.size() - 1;

    tinygltf::BufferView idxVw;
    idxVw.buffer = bufIdx;
    idxVw.byteOffset = vertBytes;
    idxVw.byteLength = indxBytes;
    idxVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(idxVw);

    tinygltf::Accessor idxAcc;
    idxAcc.bufferView = m.bufferViews.size() - 1;
    idxAcc.byteOffset = 0;
    idxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    idxAcc.count = indices.size();
    idxAcc.type = TINYGLTF_TYPE_SCALAR;
    m.accessors.push_back(idxAcc);
    auto idxAccIdx = m.accessors.size() - 1;

    tinygltf::Primitive primitive;
    primitive.indices = idxAccIdx;                 // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = vertAccIdx; // The index of the accessor for positions
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    tinygltf::Mesh mesh;
    mesh.primitives.push_back(primitive);
    m.meshes.push_back(mesh);
    return m.meshes.size() - 1;
}

void addVertexSkin(tinygltf::Model& m, vector<unsigned char> vecBoneAffect) {
    int jointsBytes = vecBoneAffect.size() * 4;
    int weightsBytes = vecBoneAffect.size() * 4;
    tinygltf::Buffer buffer;
    buffer.data.resize(jointsBytes + weightsBytes);
    unsigned char* b = buffer.data.data();
    memset(b, 0, sizeof(jointsBytes + weightsBytes));
    for (int i = 0; i < vecBoneAffect.size(); i++) {
        b[i * 4] = vecBoneAffect[i];
    }
    for (int i = 0; i < vecBoneAffect.size(); i++) {
        b[jointsBytes + i * 4] = 1;
    }
    m.buffers.push_back(buffer);
    int bufIdx = m.buffers.size() - 1;

    tinygltf::BufferView jointsVw;
    jointsVw.buffer = bufIdx;
    jointsVw.byteOffset = 0;
    jointsVw.byteLength = jointsBytes;
    //jointsVw.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    m.bufferViews.push_back(jointsVw);

    tinygltf::Accessor jointsAcc;
    jointsAcc.bufferView = m.bufferViews.size() - 1;
    jointsAcc.byteOffset = 0;
    jointsAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    jointsAcc.count = vecBoneAffect.size();
    jointsAcc.type = TINYGLTF_TYPE_VEC4;
    m.accessors.push_back(jointsAcc);
    auto jointsAccIdx = m.accessors.size() - 1;

    tinygltf::BufferView weightsVw;
    weightsVw.buffer = bufIdx;
    weightsVw.byteOffset = jointsBytes;
    weightsVw.byteLength = weightsBytes;
    //weightsVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(weightsVw);

    tinygltf::Accessor weightsAcc;
    weightsAcc.bufferView = m.bufferViews.size() - 1;
    weightsAcc.byteOffset = 0;
    weightsAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    weightsAcc.count = vecBoneAffect.size();
    weightsAcc.type = TINYGLTF_TYPE_VEC4;
    m.accessors.push_back(weightsAcc);
    auto weightsAccIdx = m.accessors.size() - 1;

    for (int i = 0; i < m.meshes[0].primitives.size(); i++) {
        auto& prim = m.meshes[0].primitives[i];
        prim.attributes["JOINTS_0"] = jointsAccIdx;
        prim.attributes["WEIGHTS_0"] = weightsAccIdx;
    }
}