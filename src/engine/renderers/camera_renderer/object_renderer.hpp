#pragma once
#include <vector>
#include <string>
#include "engine/world/world.h"
#include "engine/resources/resources.h"
#include "common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

    class ObjectRenderer {
    public:
        World& world;
        Resources& resources;

        ObjectRenderer(World& world) :
            world(world),
            resources(*world.resources)
        {
            shaderLoaded = false;
        }

        // Main object rendering method
        void renderObject(GameObject& gobj) {
            // Lazy shader loading on first call
            ensureShaderLoaded();

            RModel* rmodel = resources.models.getModel(gobj.modelId, world.altModels);
            Model& model = rmodel->model;
            if (!model.data) return;

            const Vector3 pos = gobj.getPosition();
            const Vector3 roomPos = world.curStage->rooms[gobj.getRoomId()].origPosition;

						YCut = world.waterLevel - pos.y;

            Matrix matr = MatrixIdentity();
            matr = MatrixMultiply(MatrixTranslate(roomPos.x, roomPos.y, roomPos.z), matr);
            matr = MatrixMultiply(MatrixTranslate(pos.x, pos.y, pos.z), matr);
            matr = MatrixMultiply(gobj.getRotMatrix(), matr);
            matr = MatrixMultiply(MatrixRotateY(PI), matr);

            auto m = rlGetMatrixModelview();
            rlSetMatrixModelview(MatrixMultiply(matr, m));

            // Render each mesh of the model with our custom shader
            auto& rlModel = model.model;
            for (int i = 0; i < rlModel.meshCount; i++) {
                Mesh mesh = rlModel.meshes[i];
                Material material = rlModel.materials[rlModel.meshMaterial[i]];
                Matrix transform = rlModel.transform;
                drawMesh(mesh, material, transform);
            }

            rlSetMatrixModelview(m);
        }

    private:
        // Custom shader and load flag
        Shader customShader;
        bool shaderLoaded;
        float YCut = -10000.0f;   // by default very low value – clipping disabled

        // Store uniform locations for quick access
        int locUseTexture = -1;
        int locYCut = -1;

        // Inline shader code (GLSL 330)
        static constexpr const char* vertexShader = R"(
            #version 330
            in vec3 vertexPosition;
            in vec2 vertexTexCoord;
            in vec3 vertexNormal;

            uniform mat4 mvp;
            uniform mat4 matModel;
            uniform mat4 matNormal;

            out vec2 fragTexCoord;
            out vec3 fragNormal;
            out vec3 fragPosition;

            void main() {
                fragTexCoord = vertexTexCoord;
                fragNormal = normalize(mat3(matNormal) * vertexNormal);
                vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
                fragPosition = worldPos.xyz;
                gl_Position = mvp * vec4(vertexPosition, 1.0);
            }
        )";

        static constexpr const char* fragmentShader = R"(
            #version 330
            in vec2 fragTexCoord;
            in vec3 fragNormal;
            in vec3 fragPosition;

            uniform sampler2D texture0;
            uniform vec4 colDiffuse;
            uniform int useTexture;
            uniform float YCut;

            out vec4 finalColor;

            void main() {
                if (fragPosition.y < YCut) discard;
                vec4 texColor = texture(texture0, fragTexCoord);
                if (useTexture == 0) {
                    finalColor = colDiffuse;
                } else {
                    finalColor = texColor * colDiffuse;
                }
                finalColor.a = 1.0;
            }
        )";

        // Load shader from inline strings
        void ensureShaderLoaded() {
            if (shaderLoaded) return;

            customShader = LoadShaderFromMemory(vertexShader, fragmentShader);
            if (customShader.id == 0) {
                TraceLog(LOG_ERROR, "Failed to load model shader from memory");
                return;
            }

            // Get locations for our uniforms
            locUseTexture = GetShaderLocation(customShader, "useTexture");
            if (locUseTexture == -1) {
                TraceLog(LOG_WARNING, "Uniform 'useTexture' not found in model shader");
            }
            locYCut = GetShaderLocation(customShader, "YCut");
            if (locYCut == -1) {
                TraceLog(LOG_WARNING, "Uniform 'YCut' not found in model shader");
            }

            shaderLoaded = true;
        }

        // Draw a single mesh with the custom shader
        void drawMesh(Mesh mesh, Material material, Matrix transform) {
            // Ensure shader is loaded (just in case)
            ensureShaderLoaded();

            rlEnableShader(customShader.id);
            //rlEnableBackfaceCulling();

            // --- Matrices ---
            Matrix matModel = MatrixIdentity();
            Matrix matView = rlGetMatrixModelview();
            Matrix matProjection = rlGetMatrixProjection();

            if (customShader.locs[SHADER_LOC_MATRIX_VIEW] != -1)
                rlSetUniformMatrix(customShader.locs[SHADER_LOC_MATRIX_VIEW], matView);
            if (customShader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1)
                rlSetUniformMatrix(customShader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

            matModel = MatrixMultiply(transform, rlGetMatrixTransform());
            if (customShader.locs[SHADER_LOC_MATRIX_MODEL] != -1)
                rlSetUniformMatrix(customShader.locs[SHADER_LOC_MATRIX_MODEL], matModel);

            Matrix matModelView = MatrixMultiply(matModel, matView);
            if (customShader.locs[SHADER_LOC_MATRIX_NORMAL] != -1)
                rlSetUniformMatrix(customShader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));

            Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);
            if (customShader.locs[SHADER_LOC_MATRIX_MVP] != -1)
                rlSetUniformMatrix(customShader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

            // --- Color (colDiffuse) ---
            if (customShader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1) {
                Color color = material.maps[MATERIAL_MAP_DIFFUSE].color;
                float values[4] = {
                    (float)color.r / 255.0f,
                    (float)color.g / 255.0f,
                    (float)color.b / 255.0f,
                    (float)color.a / 255.0f
                };
                rlSetUniform(customShader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
            }

            // --- Texture and useTexture flag ---
            int diffuseSlot = 0;
            bool hasTexture = (material.maps[MATERIAL_MAP_DIFFUSE].texture.id > 0);

            if (hasTexture) {
                rlActiveTextureSlot(diffuseSlot);
                rlEnableTexture(material.maps[MATERIAL_MAP_DIFFUSE].texture.id);
                if (customShader.locs[SHADER_LOC_MAP_DIFFUSE] != -1) {
                    rlSetUniform(customShader.locs[SHADER_LOC_MAP_DIFFUSE], &diffuseSlot, SHADER_UNIFORM_INT, 1);
                }
            }

            // Set useTexture (our custom location)
            if (locUseTexture != -1) {
                int useTex = hasTexture ? 1 : 0;
                rlSetUniform(locUseTexture, &useTex, SHADER_UNIFORM_INT, 1);
            }

            // Set YCut
            if (locYCut != -1) {
                rlSetUniform(locYCut, &YCut, SHADER_UNIFORM_FLOAT, 1);
            }

            // --- Vertex attribute setup ---
            if (!rlEnableVertexArray(mesh.vaoId)) {
                rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION]);
                rlSetVertexAttribute(customShader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
                rlEnableVertexAttribute(customShader.locs[SHADER_LOC_VERTEX_POSITION]);

                if (customShader.locs[SHADER_LOC_VERTEX_TEXCOORD01] != -1) {
                    rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD]);
                    rlSetVertexAttribute(customShader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
                    rlEnableVertexAttribute(customShader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);
                }

                if (customShader.locs[SHADER_LOC_VERTEX_NORMAL] != -1) {
                    rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL]);
                    rlSetVertexAttribute(customShader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
                    rlEnableVertexAttribute(customShader.locs[SHADER_LOC_VERTEX_NORMAL]);
                }

                if (mesh.indices != NULL)
                    rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
            }

            // --- Drawing ---
            if (mesh.indices != NULL)
                rlDrawVertexArrayElements(0, mesh.triangleCount * 3, 0);
            else
                rlDrawVertexArray(0, mesh.vertexCount);

            // --- Cleanup ---
            if (hasTexture) {
                rlActiveTextureSlot(diffuseSlot);
                rlDisableTexture();
            }

            rlDisableVertexArray();
            rlDisableVertexBuffer();
            rlDisableVertexBufferElement();
            //rlDisableBackfaceCulling();
            rlDisableShader();

            // Restore matrices (not strictly necessary, but for good order)
            rlSetMatrixModelview(matView);
            rlSetMatrixProjection(matProjection);
        }
    };
}