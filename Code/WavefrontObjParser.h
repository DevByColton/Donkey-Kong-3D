#ifndef WAVEFRONT_OBJ_PARSER_H
#define WAVEFRONT_OBJ_PARSER_H

#define VERTEX 'v'
#define TEXCOORD 'vt'
#define INDEX 'f'
#define SPACE ' '
#define EOL '\n'

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
ParseMeshFromWavefrontFile(Mesh* mesh, const char* objFileName)
{
	//
	// NOTE: Parse obj file for vertices and indices
	//
	if (objFileName)
	{
		char line[512];
		FILE* objFile = {0};
		errno_t openFileResult = 0;

		openFileResult = fopen_s(&objFile, objFileName, "r");
		if (openFileResult == 0)
		{
			//
			// NOTE: Initialize the vertices and indices
			// Set the total count default values, the counts will grow as needed while parsing
			//
			mesh->vertexTotalCount = 64;
			mesh->indexTotalCount = 512;
			mesh->vertices = (VertexPositionColor*)malloc(mesh->vertexTotalCount * sizeof(VertexPositionColor));
			mesh->indices = (unsigned short*)malloc(mesh->indexTotalCount * sizeof(unsigned short));

			while (fgets(line, sizeof line, objFile)) 
			{
				//
				// NOTE: Vertices
				//
				if (line[0] == VERTEX)
				{
					// NOTE: Checking if vertices need more memory allocated
					// vertexCount gets incremented after vertex data has been read, so no need for + 1
					mesh->vertices = (VertexPositionColor*)CheckExpandSize(mesh->vertices, mesh->vertexCount, &mesh->vertexTotalCount, sizeof(VertexPositionColor));

					unsigned int vertexColorIndex = 1;
					char* startOfNumber = line + 2;

					for (char* scan = startOfNumber; *scan; ++scan)
					{
						if (*scan == SPACE || *scan == EOL)
						{
							size_t sizeOfNumber = (size_t)(scan - startOfNumber);
							char* numberString = (char*)malloc(sizeOfNumber + 1);
							memcpy(numberString, startOfNumber, sizeOfNumber);

							//
							// NOTE: Values 1,2,3 are x,y,z and 4,5,6 are r,g,b
							//
							float value = strtof(numberString, 0);
							switch (vertexColorIndex)
							{
								case 1: 
								{ 
									mesh->vertices[mesh->vertexCount].position.x = value; 
								} break;
								
								case 2: 
								{ 
									mesh->vertices[mesh->vertexCount].position.y = value;
								} break;
								
								case 3: 
								{ 
									mesh->vertices[mesh->vertexCount].position.z = value;
								} break;

								case 4:
								{
									mesh->vertices[mesh->vertexCount].color.x = value; 
								} break;

								case 5:
								{
									mesh->vertices[mesh->vertexCount].color.y = value; 
								} break;

								case 6:
								{
									mesh->vertices[mesh->vertexCount].color.z = value; 
								} break;
							}
							
							free(numberString);
							startOfNumber = scan + 1;
							++vertexColorIndex;
						}
					}

					++mesh->vertexCount;
				}

				//
				// NOTE: Indices
				//
				if (line[0] == INDEX)
				{
					char* startOfNumber = line + 2;

					for (char* scan = startOfNumber; *scan; ++scan)
					{
						if (*scan == SPACE || *scan == EOL)
						{
							long long indexLong = strtoll(startOfNumber, &scan, 10);
							
							if ((indexLong >= 0) || (indexLong < INT_MAX))
							{
								// NOTE: Checking if indices need more memory allocated
								// indexCount gets incremented after index data has been read, so no need for + 1
								mesh->indices = (unsigned short*)CheckExpandSize(mesh->indices, mesh->indexCount, &mesh->indexTotalCount, sizeof(unsigned short));

								// NOTE: - 1 because blender export is not zero based
								mesh->indices[mesh->indexCount] = ((unsigned short)indexLong) - 1;

								++mesh->indexCount;
							}
							else
							{
								assert(indexLong >= 0);
								assert(indexLong < INT_MAX);
							}

							startOfNumber = scan + 1;
						}
					}
				}
			}

			//
			// NOTE: Shrink arrays to only what is needed
			//
			mesh->vertices = (VertexPositionColor*)ShrinkSize(mesh->vertices, mesh->vertexCount, &mesh->vertexTotalCount, sizeof(VertexPositionColor));
			mesh->indices = (unsigned short*)ShrinkSize(mesh->indices, mesh->indexCount, &mesh->indexTotalCount, sizeof(unsigned short));
		}

		fclose(objFile);
	}
}

#endif // WAVEFRONT_OBJ_PARSER_H