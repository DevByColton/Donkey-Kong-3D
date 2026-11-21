#ifndef MESH_H
#define MESH_H

struct Material
{
	char id[32];
	
	DirectX::XMFLOAT3 diffuse = {}; // Kd
	float roughness = {};           // Pr
};

struct MeshData
{
    unsigned int vertexCount = 0;    
    Vertex* vertices = {0};

    unsigned int indexCount = 0;
    unsigned short* indices = {0};

    RenderData renderData = {};
};

struct Meshs
{
	MeshData suzanne = {};
	MeshData torus = {};
	MeshData crate = {};
};

struct Mesh
{
	MeshData* data = {};
	Transform transform = {};
};

inline void*
CheckExpandSize(void* array, unsigned int currentCount, unsigned int* totalCount, size_t typeSize)
{
	if (currentCount >= *totalCount)
	{
		unsigned int previousTotalCount = *totalCount;
		unsigned int newTotalCount = previousTotalCount * 2;
		
		void* newArray = (void*)malloc(newTotalCount * typeSize);
		if (newArray == 0)
		{
			return array;
		}

		memcpy(newArray, array, previousTotalCount * typeSize);
		free(array);
		*totalCount = newTotalCount;
		
		return newArray;
	}

	return array;
}

inline void*
ShrinkSize(void* array, unsigned int currentCount, unsigned int* totalCount, size_t typeSize)
{
	if (currentCount < *totalCount)
	{	
		void* newArray = (void*)malloc(currentCount * typeSize);
		if (newArray == 0)
		{
			return array;
		}

		memcpy(newArray, array, currentCount * typeSize);
		free(array);
		*totalCount = currentCount;
		
		return newArray;
	}

	return array;
}

internal_link void
ParseMeshFromWavefrontFile(ID3D11Device* device, MeshData* mesh, const char* objFileName)
{
	assert(objFileName);

	FILE* objFile = {0};
	errno_t objFileOpenResult = fopen_s(&objFile, objFileName, "r");
	assert(objFileOpenResult == 0);

	unsigned int vertexTotalCount = 512;
	mesh->vertexCount = 0;
	mesh->vertices = (Vertex*)malloc(vertexTotalCount * sizeof(Vertex));
	
	unsigned int indexTotalCount = 512;
	mesh->indexCount = 0;
	mesh->indices = (unsigned short*)malloc(indexTotalCount * sizeof(unsigned short));

	unsigned int vertexColorTotalCount = 128;
	unsigned int vertexColorCount = 0;
	VertexColor* vertexColors = (VertexColor*)malloc(vertexTotalCount * sizeof(VertexColor));
	
	unsigned int noramlTotalCount = 128;
	unsigned int normalCount = 0;
	DirectX::XMFLOAT3* normals = (DirectX::XMFLOAT3*)malloc(noramlTotalCount * sizeof(DirectX::XMFLOAT3));
	
	unsigned int texCoordTotalCount = 128;
	unsigned int texCoordCount = 0;
	DirectX::XMFLOAT2* texCoords = (DirectX::XMFLOAT2*)malloc(texCoordTotalCount * sizeof(DirectX::XMFLOAT2));
	
	Material* currentMaterial = {0};
	unsigned int materialTotalCount = 4;
	unsigned int currentMaterialIndex = 0;
	unsigned int materialCount = 0;
	Material* materials = (Material*)malloc(materialTotalCount * sizeof(Material));

	char lineKey[16];
	char line[512];
	while (fgets(line, sizeof(line), objFile)) 
	{
		if (sscanf_s(line, " %15s", lineKey, (unsigned)sizeof(lineKey)) != 1)
		{
			continue;
		}

		//
		// NOTE: Read mtl file
		//
		if (strcmp("mtllib", lineKey) == 0)
		{
			char mtlFileName[128];
			int materialFileNameResult = sscanf_s(line, "mtllib %127s", mtlFileName, (unsigned int)sizeof(mtlFileName));
			assert(materialFileNameResult == 1);

			FILE* mtlFile = {0};
			errno_t mtlFileOpenResult = fopen_s(&mtlFile, mtlFileName, "r");
			assert(mtlFileOpenResult == 0);

			char mtlLineKey[16];
			char mtlLine[512];
			while (fgets(mtlLine, sizeof(mtlLine), mtlFile))
			{
				if (sscanf_s(mtlLine, " %15s", mtlLineKey, (unsigned)sizeof(mtlLineKey)) != 1)
				{
					continue;
				}

				if (strcmp("newmtl", mtlLineKey) == 0)
				{
					materials = (Material*)CheckExpandSize(materials, materialCount, &materialTotalCount, sizeof(Material));

					currentMaterialIndex = materialCount;

					int materialIdResult = sscanf_s(
						mtlLine, 
						"newmtl %31s",
						materials[currentMaterialIndex].id, 
						(unsigned)sizeof(materials[currentMaterialIndex].id)
					);
					assert(materialIdResult == 1);

					++materialCount;
				}
				else if (strcmp("Kd", mtlLineKey) == 0)
				{
					float r = {};
					float g = {};
					float b = {};
					
					int diffuseResult = sscanf_s(mtlLine, "Kd %f %f %f", &r, &g, &b);
					assert(diffuseResult == 3);

					materials[currentMaterialIndex].diffuse.x = r;
					materials[currentMaterialIndex].diffuse.y = g;
					materials[currentMaterialIndex].diffuse.z = b;
				}

				else if (strcmp("Pr", mtlLineKey) == 0)
				{
					float roughness = {};
					
					int roughnessResult = sscanf_s(mtlLine, "Pr %f", &roughness);
					assert(roughnessResult == 1);

					materials[currentMaterialIndex].roughness = roughness;
				}
			}

			free(materials);
			fclose(mtlFile);
		}
		else if (strcmp("usemtl", lineKey) == 0)
		{
			char materialId[32];
			int materialIdResult = sscanf_s(line, "usemtl %31s", materialId, (unsigned)sizeof(materialId));
			assert(materialIdResult == 1);

			for (unsigned int materialIndex = 0; materialIndex < materialCount; ++materialIndex)
			{
				Material* material = materials + materialIndex;
				if (strcmp(materialId, material->id) == 0)
				{
					currentMaterial = material;
					break;
				}
			}
		}
		else if (strcmp("vn", lineKey) == 0)
		{
			float x = 0.f;
			float y = 0.f;
			float z = 0.f;

			int normalResult = sscanf_s(line, "vn %f %f %f", &x, &y, &z);
			assert(normalResult == 3);

			normals = (DirectX::XMFLOAT3*)CheckExpandSize(normals, normalCount, &noramlTotalCount, sizeof(DirectX::XMFLOAT3));

			normals[normalCount].x = x;
			normals[normalCount].y = y;
			normals[normalCount].z = z;

			++normalCount;
		}
		else if (strcmp("vt", lineKey) == 0)
		{
			float x = 0.f;
			float y = 0.f;

			int texCoordsResult = sscanf_s(line, "vt %f %f", &x, &y);
			assert(texCoordsResult == 2);

			texCoords = (DirectX::XMFLOAT2*)CheckExpandSize(texCoords, texCoordCount, &texCoordTotalCount, sizeof(DirectX::XMFLOAT2));

			texCoords[texCoordCount].x = x;
			texCoords[texCoordCount].y = y;

			++texCoordCount;
		}
		else if (strcmp("v", lineKey) == 0)
		{
			float x = 0.f;
			float y = 0.f;
			float z = 0.f;
			float r = 0.f;
			float g = 0.f;
			float b = 0.f;

			// TODO: Remove reading the color
			int vertexColorResult = sscanf_s(line, "v %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
			assert(vertexColorResult == 6 || vertexColorResult == 3);

			vertexColors = (VertexColor*)CheckExpandSize(vertexColors, vertexColorCount, &vertexColorTotalCount, sizeof(VertexColor));

			vertexColors[vertexColorCount].position.x = x;
			vertexColors[vertexColorCount].position.y = y;
			vertexColors[vertexColorCount].position.z = z;
			vertexColors[vertexColorCount].color.x = r;
			vertexColors[vertexColorCount].color.y = g;
			vertexColors[vertexColorCount].color.z = b;

			++vertexColorCount;
		}
		else if (strcmp("f", lineKey) == 0)
		{
			int v1, t1, n1 = 0;
			int v2, t2, n2 = 0;
			int v3, t3, n3 = 0;

			int faceResult = sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
			assert(faceResult == 9);

			mesh->vertices = (Vertex*)CheckExpandSize(mesh->vertices, (mesh->vertexCount + 3), &vertexTotalCount, sizeof(Vertex));
			mesh->indices = (unsigned short*)CheckExpandSize(mesh->indices, (mesh->indexCount + 3), &indexTotalCount, sizeof(unsigned short));

			// NOTE: Decretment all values by 1 becuase blender exports 1 based
			v1 -= 1; t1 -= 1; n1 -= 1;
			v2 -= 1; t2 -= 1; n2 -= 1;
			v3 -= 1; t3 -= 1; n3 -= 1;
			
			// NOTE: Indices need to be where the unique vertex is store in the mesh->vertices
			// Since im mapping normals, indices and vertices should be the same size 
			// so they just need to be mapped in order
			unsigned short index = (unsigned short)mesh->vertexCount;
			mesh->indices[mesh->indexCount++] = index;
			mesh->indices[mesh->indexCount++] = index + 1;
			mesh->indices[mesh->indexCount++] = index + 2;	

			mesh->vertices[mesh->vertexCount++] =
			{
				vertexColors[v1].position, 
				currentMaterial->diffuse, 
				texCoords[t1], 
				normals[n1],
				currentMaterial->roughness
			};

			mesh->vertices[mesh->vertexCount++] =
			{
				vertexColors[v2].position, 
				currentMaterial->diffuse, 
				texCoords[t2], 
				normals[n2],
				currentMaterial->roughness
			};

			mesh->vertices[mesh->vertexCount++] = 
			{
				vertexColors[v3].position, 
				currentMaterial->diffuse, 
				texCoords[t3], 
				normals[n3],
				currentMaterial->roughness
			};				
		}
	}

	//
	// NOTE: Shrink arrays to only what is needed and free refs
	//
	mesh->vertices = (Vertex*)ShrinkSize(mesh->vertices, mesh->vertexCount, &vertexTotalCount, sizeof(Vertex));
	mesh->indices = (unsigned short*)ShrinkSize(mesh->indices, mesh->indexCount, &indexTotalCount, sizeof(unsigned short));
	
	free(vertexColors);
	free(normals);
	free(texCoords);
	fclose(objFile);
}

internal_link MeshData
CreateMeshFromWavefrontFile(ID3D11Device* device, const char* objFileName)
{
	MeshData mesh = {};
	
	ParseMeshFromWavefrontFile(device, &mesh, objFileName);
	CreateVertexBuffer(device, mesh.vertices, mesh.vertexCount, &mesh.renderData);
	CreateIndexBuffer(device,  mesh.indices, mesh.indexCount, &mesh.renderData);
	CreateConstantBuffer(device, &mesh.renderData.modelViewProjectionBuffer, sizeof(ModelViewProjectionData));

	return mesh;
}

internal_link Meshs
LoadAllMeshs(ID3D11Device* device, Shaders* shaders)
{
	Meshs meshs = {};

	meshs.crate = CreateMeshFromWavefrontFile(device, "Crate.obj");
	meshs.crate.renderData.vertexShader = shaders->baseVertexShader.shader;
	meshs.crate.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	meshs.crate.renderData.pixelShader = shaders->lightingPixelShader;

	meshs.torus = CreateMeshFromWavefrontFile(device, "Torus.obj");
	meshs.torus.renderData.vertexShader = shaders->baseVertexShader.shader;
	meshs.torus.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	meshs.torus.renderData.pixelShader = shaders->lightingPixelShader;

	meshs.suzanne = CreateMeshFromWavefrontFile(device, "Suzanne.obj");
	meshs.suzanne.renderData.vertexShader = shaders->baseVertexShader.shader;
	meshs.suzanne.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	meshs.suzanne.renderData.pixelShader = shaders->lightingPixelShader;

	return meshs;
}

internal_link void
UpdateVertexShaderData(MeshData* mesh)
{
	DirectX::FXMMATRIX model = DirectX::XMLoadFloat4x4(&mesh->renderData.modelViewProjectionData.model);
	DirectX::XMMATRIX inverseModel = DirectX::XMMatrixInverse(0, model);
	DirectX::XMMATRIX normalModel = DirectX::XMMatrixTranspose(inverseModel);
	DirectX::XMStoreFloat4x4(&mesh->renderData.modelViewProjectionData.normalModel, normalModel);
}

internal_link void
UpdateRenderMesh(Mesh* mesh, Camera* camera, D3DResource* d3dResource)
{
	ApplyScaleRotationTranslation(&mesh->transform, &mesh->data->renderData.modelViewProjectionData);
	ApplyView(&mesh->data->renderData.modelViewProjectionData, camera);
	PerspectiveProjection(&mesh->data->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);
	UpdateVertexShaderData(mesh->data);
	D3DRender(d3dResource->context, &mesh->data->renderData);
}

#endif // MESH_H