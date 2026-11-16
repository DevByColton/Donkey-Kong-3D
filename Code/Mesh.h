#ifndef MESH_H
#define MESH_H

struct MeshData
{
    unsigned int vertexCount = 0;    
    Vertex* vertices = {0};

    unsigned int indexCount = 0;
    unsigned short* indices = {0};

    DirectX::XMFLOAT3 ambient = {};
	DirectX::XMFLOAT3 diffuse = {};
	DirectX::XMFLOAT3 specular = {};
	Texture2D texture = {};
    RenderData renderData = {};
};

struct Meshs
{
	MeshData suzanne = {};
	MeshData crate = {};
	MeshData light = {};
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

	char line[512];
	char mtlFileName[128] = {0};
	bool hasMtlFile = false;

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

	while (fgets(line, sizeof(line), objFile)) 
	{
		// IMPORTANT: %s "Matches a sequence of non-whitespace characters", so no file names with spaces in them!
		if (!hasMtlFile && sscanf_s(line, "mtllib %127s", mtlFileName, (unsigned int)sizeof(mtlFileName)) == 1)
		{
			hasMtlFile = true;
		}

		switch(line[0])
		{
			case 'v':
			{
				switch (line[1])
				{
					// NOTE: Position and color
					case ' ':
					{
						float x = 0.f;
						float y = 0.f;
						float z = 0.f;
						float r = 0.f;
						float g = 0.f;
						float b = 0.f;

						if (sscanf_s(line, "v %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b) == 6)
						{
							vertexColors = (VertexColor*)CheckExpandSize(
								vertexColors, 
								vertexColorCount, 
								&vertexColorTotalCount, 
								sizeof(VertexColor)
							);

							vertexColors[vertexColorCount].position.x = x;
							vertexColors[vertexColorCount].position.y = y;
							vertexColors[vertexColorCount].position.z = z;
							vertexColors[vertexColorCount].color.x = r;
							vertexColors[vertexColorCount].color.y = g;
							vertexColors[vertexColorCount].color.z = b;

							++vertexColorCount;
						}
						else
						{
							OutputDebugStringA("Unable to read position colors for file: ");
							OutputDebugStringA(objFileName);
							OutputDebugStringA("\n");
						}
					} break;

					// NOTE: Normals
					case 'n':
					{
						float x = 0.f;
						float y = 0.f;
						float z = 0.f;

						if (sscanf_s(line, "vn %f %f %f", &x, &y, &z) == 3)
						{
							normals = (DirectX::XMFLOAT3*)CheckExpandSize(normals, normalCount, &noramlTotalCount, sizeof(DirectX::XMFLOAT3));

							normals[normalCount].x = x;
							normals[normalCount].y = y;
							normals[normalCount].z = z;

							++normalCount;
						}
						else
						{
							OutputDebugStringA("Unable to read normals for file: ");
							OutputDebugStringA(objFileName);
							OutputDebugStringA("\n");
						}
					} break;

					// NOTE: Texture coordinates
					case 't':
					{
						float x = 0.f;
						float y = 0.f;

						if (sscanf_s(line, "vt %f %f", &x, &y) == 2)
						{
							texCoords = (DirectX::XMFLOAT2*)CheckExpandSize(texCoords, texCoordCount, &texCoordTotalCount, sizeof(DirectX::XMFLOAT2));

							texCoords[texCoordCount].x = x;
							texCoords[texCoordCount].y = y;

							++texCoordCount;
						}
						else
						{
							OutputDebugStringA("Unable to read texture coordinates for file: ");
							OutputDebugStringA(objFileName);
							OutputDebugStringA("\n");
						}
					} break;
				}
			} break;

			case 'f':
			{
				int v1, t1, n1 = 0;
				int v2, t2, n2 = 0;
				int v3, t3, n3 = 0;

				if (sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3) == 9)
				{
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

					mesh->vertices[mesh->vertexCount++] = Vertex{vertexColors[v1].position, vertexColors[v1].color, texCoords[t1], normals[n1]};
					mesh->vertices[mesh->vertexCount++] = Vertex{vertexColors[v2].position, vertexColors[v2].color, texCoords[t2], normals[n2]};
					mesh->vertices[mesh->vertexCount++] = Vertex{vertexColors[v3].position, vertexColors[v3].color, texCoords[t3], normals[n3]};
				}
				else
				{
					OutputDebugStringA("Unable to read faces for file: ");
					OutputDebugStringA(objFileName);
					OutputDebugStringA("\n");
				}
			} break;
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

	//
	// NOTE: Parse mtl file
	//
	if (hasMtlFile)
	{
		FILE* mtlFile = {0};
		errno_t mtlFileOpenResult = fopen_s(&mtlFile, mtlFileName, "r");
		assert(mtlFileOpenResult == 0);

		char textureFileName[256];
		char lineKey[16];
		while (fgets(line, sizeof(line), mtlFile))
		{
			// IMPORTANT: %s "Matches a sequence of non-whitespace characters", so no file names with spaces in them!
			if (sscanf_s(line, " %15s", lineKey, (unsigned)sizeof(lineKey)) != 1)
			{
				continue;
			}

			if (strcmp("Ka", lineKey) == 0)
			{
				float r = {};
				float g = {};
				float b = {};
				
				int ambientResult = sscanf_s(line, "Ka %f %f %f", &r, &g, &b);
				assert(ambientResult == 3);

				mesh->ambient.x = r;
				mesh->ambient.y = g;
				mesh->ambient.z = b;
			}
			else if (strcmp("Ks", lineKey) == 0)
			{
				float r = {};
				float g = {};
				float b = {};
				
				int specularResult = sscanf_s(line, "Ks %f %f %f", &r, &g, &b);
				assert(specularResult == 3);

				mesh->specular.x = r;
				mesh->specular.y = g;
				mesh->specular.z = b;
			}
			else if (strcmp("Ke", lineKey) == 0)
			{
				float r = {};
				float g = {};
				float b = {};
				
				int diffuseResult = sscanf_s(line, "Ke %f %f %f", &r, &g, &b);
				assert(diffuseResult == 3);

				mesh->diffuse.x = r;
				mesh->diffuse.y = g;
				mesh->diffuse.z = b;
			}
			else if (strcmp("map_Kd", lineKey) == 0)
			{
				int textureFileNameResult = sscanf_s(line, " map_Kd %255s", textureFileName, (unsigned)sizeof(textureFileName));
				assert(textureFileNameResult == 1);
				mesh->texture = GenerateTexture(device, 1, textureFileName);
			}
		}

		fclose(mtlFile);
	}
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

	meshs.suzanne = CreateMeshFromWavefrontFile(device, "Suzanne.obj");
	meshs.suzanne.renderData.vertexShader = shaders->baseVertexShader.shader;
	meshs.suzanne.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	meshs.suzanne.renderData.pixelShader = shaders->lightingPixelShader;
	
	meshs.crate = CreateMeshFromWavefrontFile(device, "Crate.obj");
	meshs.crate.renderData.vertexShader = shaders->baseVertexShader.shader;
	meshs.crate.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	meshs.crate.renderData.pixelShader = shaders->lightingTexturePixelShader;
	
	meshs.light = CreateMeshFromWavefrontFile(device, "LightSource.obj");
	meshs.light.renderData.vertexShader = shaders->baseVertexShader.shader;
	meshs.light.renderData.inputLayout = shaders->baseVertexShader.inputLayout;
	meshs.light.renderData.pixelShader = shaders->basePixelShader;

	return meshs;
}

internal_link void
UpdateVertexShaderData(MeshData* mesh)
{
	DirectX::FXMMATRIX model = DirectX::XMLoadFloat4x4(&mesh->renderData.modelViewProjectionData.model);
	DirectX::XMMATRIX inverseModel = DirectX::XMMatrixInverse(0, model);
	DirectX::XMMATRIX normalModel = DirectX::XMMatrixTranspose(inverseModel);

	for (unsigned int vertexIndex = 0; vertexIndex < mesh->vertexCount; ++vertexIndex)
	{
		Vertex* vertex = mesh->vertices + vertexIndex;

		DirectX::XMVECTOR normal = DirectX::XMLoadFloat3(&vertex->normal);
		normal = DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(normal, normalModel));
		DirectX::XMStoreFloat3(&vertex->normal, normal);
	}
}

internal_link void
UpdateRenderMesh(Mesh* mesh, Camera* camera, D3DResource* d3dResource)
{
	ApplyScaleRotationTranslation(&mesh->transform, &mesh->data->renderData.modelViewProjectionData);
	ApplyView(&mesh->data->renderData.modelViewProjectionData, camera);
	PerspectiveProjection(&mesh->data->renderData.modelViewProjectionData, d3dResource->backBufferDescription.Width, d3dResource->backBufferDescription.Height);
	UpdateVertexShaderData(mesh->data);
	D3DRender(d3dResource->context, &mesh->data->renderData, &mesh->data->texture);
}

#endif // MESH_H