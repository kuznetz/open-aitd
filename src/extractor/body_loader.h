#pragma once
#include <string>
#include "structs/body.h"
#include "pak/pak.h"

void loadBodiesPak(char* filename) {

}

void loadBodyBones(PakBody &body, char* data) {

}

void loadBodyPrimitives(PakBody& body, char* data) {
	int i = 0;
	int primCount = READ_LE_U16(data + 0);
	i += 2;
	body.primitives.resize(primCount);
	for (int n = 0; n < primCount; n++) {
		auto& prim = body.primitives[n];
		prim.type = data[i + 0];
		i++;
		switch (prim.type)
		{
		/*
		//line
		case 0:
		{
			i++;
			int colorIndex = buffer[i + 0];
			i += 2;

			Color32 color = paletteColors[colorIndex];
			int pointIndexA = buffer.ReadShort(i + 0) / 6;
			int pointIndexB = buffer.ReadShort(i + 2) / 6;
			Vector3 directionVector = vertices[pointIndexA] - vertices[pointIndexB];
			Vector3 middle = (vertices[pointIndexA] + vertices[pointIndexB]) / 2.0f;
			Quaternion rotation = Quaternion.LookRotation(directionVector);

			uv.AddRange(CubeMesh.uv);
			uvDepth.AddRange(CubeMesh.vertices.Select(x = > Vector2.zero));
			indices[0].AddRange(CubeMesh.triangles.Select(x = > x + allVertices.Count));
			allVertices.AddRange(CubeMesh.vertices.Select(x = >
				rotation * Vector3.Scale(x, new Vector3(linesize, linesize, directionVector.magnitude))
				+ middle));
			colors.AddRange(CubeMesh.vertices.Select(x = > color));
			colorsRaw.AddRange(CubeMesh.vertices.Select(x = > color));
			boneWeights.AddRange(CubeMesh.vertices.Select(x = > new BoneWeight(){ boneIndex0 = bonesPerVertex[x.z > 0 ? pointIndexA : pointIndexB], weight0 = 1 }));

			i += 4;
			break;
		}
		*/
		//polygon
		case 1:
		{
			int numPoints = READ_LE_U8(data + i);
			prim.polyType = READ_LE_U8(data + i + 1);
			prim.colorIndex = READ_LE_U8(data + i + 2);
			i += 3;

			prim.vertexIdxs.resize(numPoints);
			for (int m = 0; m < numPoints; m++)
			{
				prim.vertexIdxs[m] = READ_LE_U16(data + i);
				i += 2;
			}
			break;
		}
		/*
		//sphere
		case 3:
		{
			int polyType = buffer[i];
			i++;
			int colorIndex = buffer[i];
			Color32 color = Palette.GetPaletteColor(paletteColors, colorIndex, polyType, DetailsLevel.BoolValue);
			Color32 colorRaw = Palette.GetRawPaletteColor(paletteColors, colorIndex, polyType);
			List<int> triangleList = indices[GetTriangleListIndex(polyType)];

			i += 2;

			int size = buffer.ReadShort(i + 0);
			i += 2;
			int pointSphereIndex = buffer.ReadShort(i + 0) / 6;
			i += 2;

			Vector3 position = vertices[pointSphereIndex];
			float scale = size / 500.0f;
			float uvScale = noisesize * size / 200.0f;

			if ((polyType == 3 || polyType == 4 || polyType == 5 || polyType == 6) && DetailsLevel.BoolValue)
			{
				gradientPolygonType.Add(polyType);
				gradientPolygonList.Add(Enumerable.Range(allVertices.Count, SphereMesh.vertices.Length).ToList());
			}

			uv.AddRange(SphereMesh.uv.Select(x = > x * uvScale));
			uvDepth.AddRange(SphereMesh.vertices.Select(x = > Vector2.zero));
			triangleList.AddRange(SphereMesh.triangles.Select(x = > x + allVertices.Count));
			allVertices.AddRange(SphereMesh.vertices.Select(x = > x * scale + position));
			colors.AddRange(SphereMesh.vertices.Select(x = > color));
			colorsRaw.AddRange(SphereMesh.vertices.Select(x = > colorRaw));
			boneWeights.AddRange(SphereMesh.vertices.Select(x = > new BoneWeight(){ boneIndex0 = bonesPerVertex[pointSphereIndex], weight0 = 1 }));
			break;
		}
		*/
		/*
		case 2: //1x1 pixel
		case 6: //2x2 square
		case 7: //NxN square, size depends projected z-value
		{
			i++;
			int colorIndex = buffer[i];
			i += 2;
			int cubeIndex = buffer.ReadShort(i + 0) / 6;
			i += 2;

			Color32 color = paletteColors[colorIndex];
			Vector3 position = vertices[cubeIndex];

			float pointsize = linesize;
			switch (prim.type)
			{
			case 6:
				pointsize = linesize * 2.5f;
				break;
			case 7:
				pointsize = linesize * 5.0f;
				break;
			}

			uv.AddRange(CubeMesh.uv);
			uvDepth.AddRange(CubeMesh.vertices.Select(x = > Vector2.zero));
			indices[0].AddRange(CubeMesh.triangles.Select(x = > x + allVertices.Count));
			allVertices.AddRange(CubeMesh.vertices.Select(x = > x * pointsize + position));
			colors.AddRange(CubeMesh.vertices.Select(x = > color));
			colorsRaw.AddRange(CubeMesh.vertices.Select(x = > color));
			boneWeights.AddRange(CubeMesh.vertices.Select(x = > new BoneWeight(){ boneIndex0 = bonesPerVertex[cubeIndex], weight0 = 1 }));
			break;
		}
		*/
		
		//triangle
		case 8:  //texture
		case 9:  //normals
		case 10: //normals + texture
		{
			if (prim.type == 8 || prim.type == 10) {
				//uvOffset = uvStart + buffer.ReadUnsignedShort(i + 1) / 16 * 3;
				u16 uvOffset = READ_LE_U16(data + i);
			}
			prim.colorIndex = READ_LE_U8(data + i + 2);
			i += 3;

			prim.vertexIdxs.resize(3);
			for (int k = 0; k < 3; k++)
			{
				i += 2;
				prim.vertexIdxs[k] = READ_LE_U16(data + i + 0);
				if (prim.type == 8 || prim.type == 10)
				{
					/*uv.Add(new Vector2(
						buffer.ReadShort(uvIndex + 0) / 256.0f,
						buffer.ReadShort(uvIndex + 2) / textureHeight));
					uvIndex += 4;*/
				}
			}
			//??? - skip 3 bytes
			i += 3;
			if (prim.type == 9 || prim.type == 10)
			{
				i += 6; //normals
			}
		}
		break;

		case 4:
		case 5: //should be ignored
			break;

		default:
			auto m = std::string("unknown primitive ") + std::to_string(prim.type) + " at " + std::to_string(i);
			throw new std::exception(m.c_str());
		}

	}
}

void loadBody(char* data, int size) {
	//header
	PakBody body;
	body.modelFlags = READ_LE_U16(data);	
	body.bounds.ZVX1 = READ_LE_S16(data + 2);
	body.bounds.ZVX2 = READ_LE_S16(data + 4);
	body.bounds.ZVY1 = READ_LE_S16(data + 6);
	body.bounds.ZVY2 = READ_LE_S16(data + 8);
    body.bounds.ZVZ1 = READ_LE_S16(data + 10);
	body.bounds.ZVZ2 = READ_LE_S16(data + 12);
	int vertsOffs = READ_LE_S16(data + 14) + 16;

	int vertsCount = READ_LE_S16(data + vertsOffs);
	int i = vertsOffs+2;
	body.vertices.resize(vertsCount*3);
	for (int j = 0; j < vertsCount; j++)
	{
		body.vertices[j + 0] = READ_LE_S16(data + i + 0);
		body.vertices[j + 1] = READ_LE_S16(data + i + 2);
		body.vertices[j + 2] = READ_LE_S16(data + i + 4);
		i += 6;
	}

	//check if model has bones
	if ((body.modelFlags & 2) == 2)
	{
		loadBodyBones(body, data + i);
	}
	loadBodyPrimitives(body, data + i);
}

/*
void LoadBody(bool resetcamera = true)
	{
		string filePath = modelFolders[modelFolderIndex];

		//camera
		if (resetcamera)
		{
			cameraPosition = Vector2.zero;
		}

		//clear model
		SkinnedMeshRenderer filter = GetComponent<SkinnedMeshRenderer>();
		filter.sharedMesh = null;

		//delete all bones
		foreach (Transform child in transform)
		{
			if (child.gameObject != BoundingBox)
			{
				Destroy(child.gameObject);
			}
		}

		//load data
		byte[] buffer;
		using (var pak = new PakArchive(filePath))
		{
			buffer = pak[modelIndex].Read();
		}
		int i = 0;

		//header
		modelFlags = buffer.ReadShort(i + 0);

		//bounding box
		LoadBoundingBox(buffer, i + 2);

		i += 0xE;
		i += buffer.ReadShort(i + 0) + 2;

		//vertexes
		int count = buffer.ReadShort(i + 0);
		i += 2;

		List<Vector3> vertices = new List<Vector3>();
		for (int j = 0; j < count; j++)
		{
			Vector3 position = new Vector3(buffer.ReadShort(i + 0), -buffer.ReadShort(i + 2), buffer.ReadShort(i + 4));
			vertices.Add(position / 1000.0f);
			i += 6;
		}

		bones = new List<Transform>();
		List<Matrix4x4> bindPoses = new List<Matrix4x4>();
		Dictionary<int, int> bonesPerVertex = new Dictionary<int, int>();
		List<Vector3> vertexNoTransform = vertices.ToList();

		if ((modelFlags & 2) == 2) //check if model has bones
		{
			//bones
			count = buffer.ReadShort(i + 0);
			i += 2;
			i += count * 2;

			Dictionary<int, Transform> bonesPerIndex = new Dictionary<int, Transform>();

			bonesPerIndex.Add(255, transform);
			for (int n = 0; n < count; n++)
			{
				int startindex = buffer.ReadShort(i + 0) / 6;
				int numpoints = buffer.ReadShort(i + 2);
				int vertexindex = buffer.ReadShort(i + 4) / 6;
				int parentindex = buffer[i + 6];
				int boneindex = buffer[i + 7];

				//create bone
				Transform bone = new GameObject("BONE").transform;
				bonesPerIndex.Add(boneindex, bone);

				bone.parent = bonesPerIndex[parentindex];
				bone.localRotation = Quaternion.identity;
				bone.localPosition = vertexNoTransform[vertexindex];
				bones.Add(bone);

				//create pose
				Matrix4x4 bindPose = new Matrix4x4();
				bindPose = bone.worldToLocalMatrix * transform.localToWorldMatrix;
				bindPoses.Add(bindPose);

				//apply bone transformation
				Vector3 position = vertices[vertexindex];
				for (int u = 0; u < numpoints; u++)
				{
					vertices[startindex] += position;
					bonesPerVertex.Add(startindex, bones.Count - 1);
					startindex++;
				}

				if ((modelFlags & 8) == 8)
				{
					i += 0x18;
				}
				else
				{
					i += 0x10;
				}
			}
		}
		else
		{
			//if no bones add dummy values
			for (int u = 0; u < vertices.Count; u++)
			{
				bonesPerVertex.Add(u, 0);
			}
		}

		//compute line size
		Bounds bounds = new Bounds();
		foreach (Vector3 vector in vertices)
		{
			bounds.Encapsulate(vector);
		}
		float linesize = bounds.size.magnitude / 250.0f;
		float noisesize = 0.8f / bounds.size.magnitude;

		//primitives
		count = buffer.ReadUnsignedShort(i + 0);
		i += 2;

		//load palette
		Color32[] paletteColors = paletteTexture.GetPixels32();

		//load texture
		int texAHeight = 1;
		int texBHeight = 1;
		int uvStart = 0;
		if (File.Exists(textureFolder)) //TIMEGATE
		{
			Texture2D texA, texB;
			Texture.LoadTextures(buffer, paletteColors, textureFolder, textureCount, DetailsLevel.BoolValue, out uvStart, out texAHeight, out texBHeight, out texA, out texB);

			var materials = GetComponent<SkinnedMeshRenderer>().materials;
			materials[5].mainTexture = texA;
			materials[6].mainTexture = texB;
		}

		List<BoneWeight> boneWeights = new List<BoneWeight>();
		allVertices = new List<Vector3>();
		uv = new List<Vector2>();
		uvDepth = new List<Vector2>();
		List<Color32> colors = new List<Color32>();
		List<int>[] indices = new List<int>[7];

		for (int n = 0 ; n < indices.Length ; n++)
		{
			indices[n] = new List<int>();
		}

		gradientPolygonList = new List<List<int>>();
		gradientPolygonType = new List<int>();
		colorsRaw = new List<Color32>();

		for (int n = 0; n < count; n++)
		{
			int primitiveType = buffer[i + 0];
			i++;

			switch (primitiveType)
			{
				//line
				case 0:
					{
						i++;
						int colorIndex = buffer[i + 0];
						i += 2;

						Color32 color = paletteColors[colorIndex];
						int pointIndexA = buffer.ReadShort(i + 0) / 6;
						int pointIndexB = buffer.ReadShort(i + 2) / 6;
						Vector3 directionVector = vertices[pointIndexA] - vertices[pointIndexB];
						Vector3 middle = (vertices[pointIndexA] + vertices[pointIndexB]) / 2.0f;
						Quaternion rotation = Quaternion.LookRotation(directionVector);

						uv.AddRange(CubeMesh.uv);
						uvDepth.AddRange(CubeMesh.vertices.Select(x => Vector2.zero));
						indices[0].AddRange(CubeMesh.triangles.Select(x => x + allVertices.Count));
						allVertices.AddRange(CubeMesh.vertices.Select(x =>
							rotation * Vector3.Scale(x, new Vector3(linesize, linesize, directionVector.magnitude))
							+ middle));
						colors.AddRange(CubeMesh.vertices.Select(x => color));
						colorsRaw.AddRange(CubeMesh.vertices.Select(x => color));
						boneWeights.AddRange(CubeMesh.vertices.Select(x => new BoneWeight() { boneIndex0 = bonesPerVertex[x.z > 0 ? pointIndexA : pointIndexB], weight0 = 1 }));

						i += 4;
						break;
					}
				//polygon
				case 1:
					{
						int numPoints = buffer[i + 0];
						int polyType = buffer[i + 1];
						int colorIndex = buffer[i + 2];
						i += 3;

						Color32 color = Palette.GetPaletteColor(paletteColors, colorIndex, polyType, DetailsLevel.BoolValue);
						Color32 colorRaw = Palette.GetRawPaletteColor(paletteColors, colorIndex, polyType);
						List<int> triangleList = indices[GetTriangleListIndex(polyType)];

						//add vertices
						List<int> polyVertices = new List<int>();
						int verticesCount = allVertices.Count;
						for (int m = 0; m < numPoints; m++)
						{
							int pointIndex = buffer.ReadShort(i + 0) / 6;
							i += 2;

							colors.Add(color);
							colorsRaw.Add(colorRaw);
							polyVertices.Add(allVertices.Count);
							allVertices.Add(vertices[pointIndex]);
							boneWeights.Add(new BoneWeight() { boneIndex0 = bonesPerVertex[pointIndex], weight0 = 1 });
						}

						gradientPolygonType.Add(polyType);
						gradientPolygonList.Add(polyVertices);

						if (polyType == 1 && DetailsLevel.BoolValue)
						{
							Vector3 forward, left;
							ComputeUV(polyVertices, out forward, out left);

							foreach (int pointIndex in polyVertices)
							{
								Vector3 poly = allVertices[pointIndex];

								uv.Add(new Vector2(
									Vector3.Dot(poly, left) * noisesize,
									Vector3.Dot(poly, forward) * noisesize
								));
							}
						}
						else
						{
							uv.AddRange(polyVertices.Select(x => Vector2.zero));
						}

						uvDepth.AddRange(polyVertices.Select(x => Vector2.zero));

						//triangulate
						int v0 = 0;
						int v1 = 1;
						int v2 = numPoints - 1;
						bool swap = true;

						while (v1 < v2)
						{
							triangleList.Add(verticesCount + v0);
							triangleList.Add(verticesCount + v1);
							triangleList.Add(verticesCount + v2);

							if (swap)
							{
								v0 = v1;
								v1++;
							}
							else
							{
								v0 = v2;
								v2--;
							}

							swap = !swap;
						}

						break;
					}
				//sphere
				case 3:
					{
						int polyType = buffer[i];
						i++;
						int colorIndex = buffer[i];
						Color32 color = Palette.GetPaletteColor(paletteColors, colorIndex, polyType, DetailsLevel.BoolValue);
						Color32 colorRaw = Palette.GetRawPaletteColor(paletteColors, colorIndex, polyType);
						List<int> triangleList = indices[GetTriangleListIndex(polyType)];

						i += 2;

						int size = buffer.ReadShort(i + 0);
						i += 2;
						int pointSphereIndex = buffer.ReadShort(i + 0) / 6;
						i += 2;

						Vector3 position = vertices[pointSphereIndex];
						float scale = size / 500.0f;
						float uvScale = noisesize * size / 200.0f;

						if ((polyType == 3 || polyType == 4 || polyType == 5 || polyType == 6) && DetailsLevel.BoolValue)
						{
							gradientPolygonType.Add(polyType);
							gradientPolygonList.Add(Enumerable.Range(allVertices.Count, SphereMesh.vertices.Length).ToList());
						}

						uv.AddRange(SphereMesh.uv.Select(x => x * uvScale));
						uvDepth.AddRange(SphereMesh.vertices.Select(x => Vector2.zero));
						triangleList.AddRange(SphereMesh.triangles.Select(x => x + allVertices.Count));
						allVertices.AddRange(SphereMesh.vertices.Select(x => x * scale + position));
						colors.AddRange(SphereMesh.vertices.Select(x => color));
						colorsRaw.AddRange(SphereMesh.vertices.Select(x => colorRaw));
						boneWeights.AddRange(SphereMesh.vertices.Select(x => new BoneWeight() { boneIndex0 = bonesPerVertex[pointSphereIndex], weight0 = 1 }));
						break;
					}

				case 2: //1x1 pixel
				case 6: //2x2 square
				case 7: //NxN square, size depends projected z-value
					{
						i++;
						int colorIndex = buffer[i];
						i += 2;
						int cubeIndex = buffer.ReadShort(i + 0) / 6;
						i += 2;

						Color32 color = paletteColors[colorIndex];
						Vector3 position = vertices[cubeIndex];

						float pointsize = linesize;
						switch (primitiveType)
						{
							case 6:
								pointsize = linesize * 2.5f;
								break;
							case 7:
								pointsize = linesize * 5.0f;
								break;
						}

						uv.AddRange(CubeMesh.uv);
						uvDepth.AddRange(CubeMesh.vertices.Select(x => Vector2.zero));
						indices[0].AddRange(CubeMesh.triangles.Select(x => x + allVertices.Count));
						allVertices.AddRange(CubeMesh.vertices.Select(x => x * pointsize + position));
						colors.AddRange(CubeMesh.vertices.Select(x => color));
						colorsRaw.AddRange(CubeMesh.vertices.Select(x => color));
						boneWeights.AddRange(CubeMesh.vertices.Select(x => new BoneWeight() { boneIndex0 = bonesPerVertex[cubeIndex], weight0 = 1 }));
						break;
					}

				//triangle
				case 8:  //texture
				case 9:  //normals
				case 10: //normals + texture
					{
						float textureHeight = 1.0f;
						int uvIndex = 0, indicesIndex = 0;
						Color color = Color.white;

						if (primitiveType == 8 || primitiveType == 10)
						{
							uvIndex = uvStart + buffer.ReadUnsignedShort(i + 1) / 16 * 3;
							bool texModel = (buffer[i + 1] & 0xF) == 0;
							textureHeight = texModel ? texAHeight : texBHeight;
							indicesIndex = texModel ? 5 : 6;
						}
						else
						{
							int colorIndex = buffer[i + 2];
							color = paletteColors[colorIndex];
						}

						i += 3;

						for (int k = 0 ; k < 3 ; k++)
						{
							int pointIndex = buffer.ReadShort(i + 0) / 6;
							i += 2;

							uvDepth.Add(Vector2.zero);
							indices[indicesIndex].Add(allVertices.Count);
							colors.Add(color);
							colorsRaw.Add(color);
							allVertices.Add(vertices[pointIndex]);
							boneWeights.Add(new BoneWeight() { boneIndex0 = bonesPerVertex[pointIndex], weight0 = 1 });

							if (primitiveType == 8 || primitiveType == 10)
							{
								uv.Add(new Vector2(
									buffer.ReadShort(uvIndex + 0) / 256.0f,
									buffer.ReadShort(uvIndex + 2) / textureHeight));
								uvIndex += 4;
							}
							else
							{
								uv.Add(Vector2.zero);
							}
						}

						if (primitiveType == 9 || primitiveType == 10)
						{
							i +=6; //normals
						}
					}
					break;

				case 4:
				case 5: //should be ignored
					break;

				default:
					throw new UnityException("unknown primitive " + primitiveType.ToString() + " at " + i.ToString());
			}

		}

		// Create the mesh
		Mesh msh = new Mesh();
		msh.vertices = allVertices.ToArray();
		msh.colors32 = colors.ToArray();

		//separate triangles depending their material
		msh.subMeshCount = 7;
		msh.SetTriangles(indices[0], 0);
		msh.SetTriangles(indices[1], 1);
		msh.SetTriangles(indices[2], 2);
		msh.SetTriangles(indices[3], 3);
		msh.SetTriangles(indices[4], 4);
		msh.SetTriangles(indices[5], 5);
		msh.SetTriangles(indices[6], 6);
		msh.SetUVs(0, uv);
		msh.SetUVs(1, uvDepth);
		msh.RecalculateNormals();
		msh.RecalculateBounds();

		//apply bones
		if (bones.Count > 0)
		{
			msh.boneWeights = boneWeights.ToArray();
			msh.bindposes = bindPoses.ToArray();
			GetComponent<SkinnedMeshRenderer>().bones = bones.ToArray();
			initialBonesPosition = bones.Select(x => x.localPosition).ToList();
		}

		filter.localBounds = msh.bounds;
		filter.sharedMesh = msh;

		RefreshLeftText();
		if (resetcamera)
		{
			frameDistance = Vector3Int.Zero;
		}
	}
*/