#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../common/raylib_cpp.hpp"

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

        // Основной метод рендеринга объекта
        void renderObject(GameObject& gobj) {
            // Ленивая загрузка шейдера при первом вызове
            ensureShaderLoaded();

            RModel* rmodel = resources.models.getModel(gobj.modelId, world.altModels);
            Model& model = rmodel->model;
            if (!model.data) return;

            const Vector3 pos = gobj.getPosition();
            const Vector3 roomPos = world.curStage->rooms[gobj.getRoomId()].origPosition;

            Matrix matr = MatrixIdentity();
            matr = MatrixMultiply(MatrixTranslate(roomPos.x, roomPos.y, roomPos.z), matr);
            matr = MatrixMultiply(MatrixTranslate(pos.x, pos.y, pos.z), matr);
            matr = MatrixMultiply(gobj.getRotMatrix(), matr);
            matr = MatrixMultiply(MatrixRotateY(PI), matr);

            auto m = rlGetMatrixModelview();
            rlSetMatrixModelview(MatrixMultiply(matr, m));

            // Рендерим каждый меш модели с нашим кастомным шейдером
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
        // Кастомный шейдер и флаг загрузки
        Shader customShader;
        bool shaderLoaded;

        // Храним локации uniform-ов для быстрого доступа
        int locUseTexture = -1;

        // Inline-код шейдеров (GLSL 330)
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

            out vec4 finalColor;

            void main() {
                vec4 texColor = texture(texture0, fragTexCoord);
                // Если текстура не используется или её пиксель прозрачен — берём цвет из colDiffuse
                if (useTexture == 0 || texColor.a < 0.01) {
                    finalColor = colDiffuse;
                } else {
                    finalColor = texColor * colDiffuse;
                }
								finalColor.a = 1.0;
            }
        )";

        // Загрузка шейдера из inline-строк
        void ensureShaderLoaded() {
            if (shaderLoaded) return;

            customShader = LoadShaderFromMemory(vertexShader, fragmentShader);
            if (customShader.id == 0) {
                TraceLog(LOG_ERROR, "Failed to load custom shader from memory");
                return;
            }

            // Получаем локации для наших uniform-ов
            locUseTexture = GetShaderLocation(customShader, "useTexture");
            if (locUseTexture == -1) {
                TraceLog(LOG_WARNING, "Uniform 'useTexture' not found in custom shader");
            }

            shaderLoaded = true;
        }

        // Рисуем один меш с кастомным шейдером
        void drawMesh(Mesh mesh, Material material, Matrix transform) {
            // Убедимся, что шейдер загружен (на всякий случай)
            ensureShaderLoaded();

            rlEnableShader(customShader.id);
						rlEnableBackfaceCulling();

            // --- Матрицы ---
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

            // --- Цвет (colDiffuse) ---
            if (customShader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1) {
                Color color = material.maps[MATERIAL_MAP_DIFFUSE].color;
                float values[4] = {
                    (float)color.r / 255.0f,
                    (float)color.g / 255.0f,
                    (float)color.b / 255.0f,
                    (float)color.a / 255.0f
                };
								if (values[3] < 1.0f) {
									printf("alpha: %f", values[3]);
								}
                rlSetUniform(customShader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
            }

            // --- Текстура и флаг useTexture ---
            int diffuseSlot = 0;
            bool hasTexture = (material.maps[MATERIAL_MAP_DIFFUSE].texture.id > 0);

            if (hasTexture) {
                rlActiveTextureSlot(diffuseSlot);
                rlEnableTexture(material.maps[MATERIAL_MAP_DIFFUSE].texture.id);
                if (customShader.locs[SHADER_LOC_MAP_DIFFUSE] != -1) {
                    rlSetUniform(customShader.locs[SHADER_LOC_MAP_DIFFUSE], &diffuseSlot, SHADER_UNIFORM_INT, 1);
                }
            }

            // Устанавливаем useTexture (наша кастомная локация)
            if (locUseTexture != -1) {
                int useTex = hasTexture ? 1 : 0;
                rlSetUniform(locUseTexture, &useTex, SHADER_UNIFORM_INT, 1);
            }

            // --- Настройка вершинных атрибутов ---
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

            // --- Отрисовка ---
            if (mesh.indices != NULL)
                rlDrawVertexArrayElements(0, mesh.triangleCount * 3, 0);
            else
                rlDrawVertexArray(0, mesh.vertexCount);

            // --- Очистка ---
            if (hasTexture) {
                rlActiveTextureSlot(diffuseSlot);
                rlDisableTexture();
            }

            rlDisableVertexArray();
            rlDisableVertexBuffer();
            rlDisableVertexBufferElement();
						rlDisableBackfaceCulling();
            rlDisableShader();

            // Восстанавливаем матрицы (необязательно, но для порядка)
            rlSetMatrixModelview(matView);
            rlSetMatrixProjection(matProjection);
        }
    };
}