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

		ObjectRenderer(World& world):
      world(world),
      resources(*world.resources)
    {
		}

		void renderObject(GameObject& gobj)
		{
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
			
			auto& rlModel = model.model;
			for (int i = 0; i < rlModel.meshCount; i++)
			{
					DrawMesh(rlModel.meshes[i], rlModel.materials[rlModel.meshMaterial[i]], rlModel.transform);
			}			

			rlSetMatrixModelview(m);
		}

		void DrawMesh(Mesh mesh, Material material, Matrix transform)
		{
			rlEnableShader(material.shader.id);

			if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
			{
				float values[4] = {
					(float)material.maps[MATERIAL_MAP_DIFFUSE].color.r / 255.0f,
					(float)material.maps[MATERIAL_MAP_DIFFUSE].color.g / 255.0f,
					(float)material.maps[MATERIAL_MAP_DIFFUSE].color.b / 255.0f,
					(float)material.maps[MATERIAL_MAP_DIFFUSE].color.a / 255.0f
				};
				rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
			}

			Matrix matModel = MatrixIdentity();
			Matrix matView = rlGetMatrixModelview();
			Matrix matProjection = rlGetMatrixProjection();

			if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1)
				rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
			if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1)
				rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

			matModel = MatrixMultiply(transform, rlGetMatrixTransform());
			if (material.shader.locs[SHADER_LOC_MATRIX_MODEL] != -1)
				rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MODEL], matModel);

			Matrix matModelView = MatrixMultiply(matModel, matView);
			if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1)
				rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));

			int diffuseSlot = 0;
			if (material.maps[MATERIAL_MAP_DIFFUSE].texture.id > 0)
			{
				rlActiveTextureSlot(diffuseSlot);
				rlEnableTexture(material.maps[MATERIAL_MAP_DIFFUSE].texture.id);
				if (material.shader.locs[SHADER_LOC_MAP_DIFFUSE] != -1)
					rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE], &diffuseSlot, SHADER_UNIFORM_INT, 1);
			}

			if (!rlEnableVertexArray(mesh.vaoId))
			{
				rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION]);
				rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
				rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

				if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] != -1)
				{
					rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD]);
					rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
					rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);
				}

				if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
				{
					rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL]);
					rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
					rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
				}

				if (mesh.indices != NULL)
					rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
			}

			Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);
			if (material.shader.locs[SHADER_LOC_MATRIX_MVP] != -1)
				rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

			if (mesh.indices != NULL)
				rlDrawVertexArrayElements(0, mesh.triangleCount * 3, 0);
			else
				rlDrawVertexArray(0, mesh.vertexCount);

			if (material.maps[MATERIAL_MAP_DIFFUSE].texture.id > 0)
			{
				rlActiveTextureSlot(diffuseSlot);
				rlDisableTexture();
			}

			rlDisableVertexArray();
			rlDisableVertexBuffer();
			rlDisableVertexBufferElement();
			rlDisableShader();

			rlSetMatrixModelview(matView);
			rlSetMatrixProjection(matProjection);
		}

	};

}
