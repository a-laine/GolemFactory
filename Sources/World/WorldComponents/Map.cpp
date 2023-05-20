#include "Map.h"

#include <Resources/Loader/ImageLoader.h>

#include <iostream>
#include <set>
//#include <glm/gtc/matrix_transform.hpp>


//	Default
Map::Map() : height(0), width(0), chunks(nullptr), amplitude(16.f), scale(1.f), vao(0), facesCount(0), lastPlayerCell(-1, -1)
{
	lodRadius[0] = 2.2f;
	lodRadius[1] = 3.2f;
	lodRadius[2] = 4.7f;
	lodRadius[3] = 6.2f;
	lodRadius[4] = 7.7f;
	lodRadius[5] = 9.2f;
}
Map::~Map()
{
	if (chunks)
	{
		for (int w = 0; w < width; w++)
			for (int h = 0; h < height; h++)
				delete chunks[w][h];
		for (int w = 0; w < width; w++)
			delete[] chunks[w];
		delete[] chunks;
	}

	glDeleteBuffers(1, &verticesBuffer);
	glDeleteBuffers(1, &normalsBuffer);
	glDeleteBuffers(1, &colorsBuffer);
	glDeleteBuffers(1, &facesBuffer);
	glDeleteVertexArrays(1, &vao);

	verticesBuffer = 0;
	normalsBuffer = 0;
	colorsBuffer = 0;
	facesBuffer = 0;
	vao = 0;
}
//

//	Public functions
bool Map::loadFromHeightmap(const std::string& resourceDirectory, const std::string& fileName)
{
	//	import
	int channel;
	ImageLoader imgload;
	uint8_t* data = imgload.loadFromFile(resourceDirectory + fileName, width, height, channel, ImageLoader::GREY);
	if (data == nullptr || width < 2 || height < 2)
		return false;

	//	allocate and load
	int offset = 128;
	chunks = new Chunk**[width];
	for (int w = 0; w < width; w++)
	{
		chunks[w] = new Chunk*[height];
	}
	for (int w = 0; w < width; w++)
		for (int h = 0; h < height; h++)
		{
			float z0 = amplitude * ((int)data[h * width + w] - offset) * 0.00390625f; // = 1/256
			vec4f chunkPos = vec4f(w - 0.5f * width, 0, h - 0.5f * height, 1.f);
			mat4f m = mat4f::translate(mat4f::identity, chunkPos); // glm::scale(mat4f(1.f), chunkSize)
			chunks[w][h] = new Chunk(rand(), m, z0);
		}

	//	create mesh data
	std::vector<vec4f> vertices;
	std::vector<vec4f> normals;
	std::vector<vec4f> colors;
	std::vector<unsigned int> faces;
	vec4f color = vec4f(1.f, 1.f, 1.f, 1.f);
	
	for (int w = 0; w < width; w++)
		for (int h = 0; h < height; h++)
		{
			vertices.push_back(getVertex(w, h));
			normals.push_back(getNormal(w, h));
			colors.push_back(color);
		}
	for (int w = 0; w < width - 1; w++)
		for (int h = 0; h < height - 1; h++)
		{
			faces.push_back(w * height + h);
			faces.push_back((w + 1) * height + h);
			faces.push_back(w * height + h + 1);

			faces.push_back(w * height + h + 1);
			faces.push_back((w + 1) * height + h);
			faces.push_back((w + 1) * height + h + 1);
		}
	facesCount = (unsigned int)faces.size();

	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec4f), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec4f), normals.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4f), colors.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);

	//	free and finish
	imgload.freeImage(data);
	if (!glIsBuffer(verticesBuffer) || !glIsBuffer(normalsBuffer) || !glIsBuffer(colorsBuffer) || !glIsBuffer(facesBuffer) || !glIsVertexArray(vao))
		return false;
	return true;
}


void Map::update(const vec4f& playerPosition)
{
	bool detailsEnable = true;
	float externalRadius = 10.5f;

	vec2i playerCell = worldToChunk(playerPosition);
	if (lastPlayerCell != playerCell)
	{
		lastPlayerCell = playerCell;
		drawableChunks.clear();
		int checkingSquareRadius = 10;
		jobList.clear();

		// chunk update
		for (int i = -checkingSquareRadius - 2; i < checkingSquareRadius + 3; i++)
			for (int j = -checkingSquareRadius - 2; j < checkingSquareRadius + 3; j++)
			{
				vec2i v = vec2i(playerCell.x + i, playerCell.y + j);
				if (v.x < width && v.y < height && v.x >= 0 && v.y >= 0)
				{
					Chunk* chunk = chunks[v.x][v.y];
					float d = vec2f(i, j).getNorm();
					if (i >= -checkingSquareRadius && j >= -checkingSquareRadius && i < checkingSquareRadius + 1 && j < checkingSquareRadius + 1) // radius (in chunk)
					{
						drawableChunks.push_back(v);
						if (!chunk->isInitialized() && detailsEnable)
						{
							float z0 = chunks[v.x][v.y]->getCorner();
							float z1 = inBound(v.x - 1, v.y) ? chunks[v.x - 1][v.y]->getCorner() : z0;
							float z2 = inBound(v.x, v.y - 1) ? chunks[v.x][v.y - 1]->getCorner() : z0;
							float z3 = inBound(v.x - 1, v.y - 1) ? chunks[v.x - 1][v.y - 1]->getCorner() : z0;

							unsigned int bottomSeed = inBound(v.x - 1, v.y) ? chunks[v.x - 1][v.y]->getSeed() : 0;
							unsigned int rightSeed = inBound(v.x, v.y - 1) ? chunks[v.x][v.y - 1]->getSeed() : 0;

							chunk->initialize(z1, z3, z2);
							chunk->addLOD(bottomSeed, rightSeed, false);
							chunk->initializeVBO();
							chunk->initializeVAO();

							if(!chunk->isInitialized())
								std::cout << "error" << std::endl;
						}

						int lod = getLod(d);
						if(chunk->getLod() != lod)
							jobList.push_back(vec4i(v.x, v.y, lod, 0));
					}
					else
					{
						chunk->free();
					}
				}
			}

		// discarded part of Map mesh
		exclusionZone.x = height - 1;
		exclusionZone.y = checkingSquareRadius;
		exclusionZone.w = playerCell.x - 1;
		exclusionZone.z = playerCell.y - 1;

		//std::cout << "discarded faces count : " << discardedFaces.size() << std::endl << std::endl << std::endl;
	}

	// do defered jobs
	if(!jobList.empty() && detailsEnable)
	{
		vec2i v = vec2i(jobList.back().x, jobList.back().y);
		Chunk* chunk = chunks[v.x][v.y];
		int lod = jobList.back().z;

		if (chunk->getLod() < lod && detailsEnable)
		{
			unsigned int bottomSeed = inBound(v.x - 1, v.y) ? chunks[v.x - 1][v.y]->getSeed() : 0;
			unsigned int rightSeed = inBound(v.x, v.y - 1) ? chunks[v.x][v.y - 1]->getSeed() : 0;

			for (int k = 0; k < 10 && chunk->getLod() < lod ; k++)
				chunk->addLOD(bottomSeed, rightSeed, true);
			chunk->updateVBO();
		}
		else if (chunk->getLod() > lod && detailsEnable)
		{
			for (int k = 0; k < 10 && chunk->getLod() > lod; k++)
				chunk->removeLOD();
			chunk->updateVBO();
		}

		jobList.pop_back();
	}
}
//

//	Set / Get
void Map::setShader(Shader* s) { shader = s; }


unsigned int Map::getFacesCount() const { return facesCount; }
GLuint Map::getVAO() const { return vao; }
Chunk* Map::getChunk(const int& w, const int& h) { return chunks[w][h]; }
std::vector<vec2i> Map::getDrawableChunks() { return drawableChunks; }
mat4f Map::getModelMatrix() const { return mat4f::scale(mat4f::identity, vec4f(height - 1, 1.f, width - 1, 1.f)); }
mat4f Map::getNormalMatrix() const { return mat4f::transpose(mat4f::inverse(getModelMatrix())); }
vec4f Map::getScale() const { return scale; }
Shader* Map::getShader() { return shader; }


vec2i Map::worldToChunk(vec4f p) const
{
	vec2f firstChunkPos = vec2f(-0.5f * width - 0.5f, -0.5f * height - 0.5f);
	vec2f v = vec2f(p.x / scale.x, p.z / scale.z) - firstChunkPos;
	return vec2i((int)v.x, (int)v.y);
}
bool Map::inBound(const int& x, const int& y) const
{
	if (x >= width || x < 0)
		return false;
	else if (y >= height || y < 0)
		return false;
	else return true;
}
vec4i Map::getExclusionZone() const { return exclusionZone; }
//


// Privates functions
vec4f Map::getVertex(const int& x, const int& y)
{
	return vec4f((float)x / (width - 1) - 0.5f, chunks[x][y]->getCorner(), (float)y / (height - 1) - 0.5f, 1.f);
}
vec4f Map::getNormal(const int& x, const int& y)
{
	float h0 = chunks[x][y]->getCorner();

	vec4f n = vec4f(0.f);
	int count = 0;

	if (inBound(x + 1, y))
	{
		count++;
		n += vec4f(h0 - chunks[x + 1][y]->getCorner(), 0.f, 1.f, 0.f).getNormal();
	}
	if (inBound(x - 1, y))
	{
		count++;
		n += vec4f(chunks[x - 1][y]->getCorner() - h0, 0.f, 1.f, 0.f).getNormal();
	}
	if (inBound(x, y + 1))
	{
		count++;
		n += vec4f(0.f, h0 - chunks[x][y + 1]->getCorner(), 1.f, 0.f).getNormal();
	}
	if (inBound(x, y - 1))
	{
		count++;
		n += vec4f(0.f, chunks[x][y - 1]->getCorner() - h0, 1.f, 0.f).getNormal();
	}

	return (1.f / count) * n;
}
int Map::getLod(float d)
{
	if (d < lodRadius[0]) return 8;
	else if (d < lodRadius[1]) return 7;
	else if (d < lodRadius[2]) return 6;
	else if (d < lodRadius[3]) return 5;
	else if (d < lodRadius[4]) return 4;
	else if (d < lodRadius[5]) return 3;
	else return 2;
}
//