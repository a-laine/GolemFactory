#include "Map.h"

#include "Resources/Loader/ImageLoader.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


//	Default
Map::Map() : height(0), width(0), chunks(nullptr), amplitude(16.f), scale(1.f), vao(0), facesCount(0), lastPlayerPos(-1, -1)
{}
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
			glm::vec3 chunkPos = glm::vec3(w - 0.5f * width, h - 0.5f * height, 0);
			glm::mat4 m = glm::translate(glm::mat4(1.f), chunkPos); // glm::scale(glm::mat4(1.f), chunkSize)
			chunks[w][h] = new Chunk(rand(), m, z0);
		}

	//	create mesh data
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<unsigned int> faces;
	glm::vec3 color = glm::vec3(0.3f, 0.3f, 0.3f);
	
	for (int w = 0; w < width - 1; w++)
		for (int h = 0; h < height - 1; h++)
		{
			vertices.push_back(glm::vec3((float)w / (width - 1) - 0.5f, (float)h / (height - 1) - 0.5f, chunks[w][h]->getCorner()));
			vertices.push_back(glm::vec3((float)w / (width - 1) - 0.5f, (float)(h + 1) / (height - 1) - 0.5f, chunks[w][h + 1]->getCorner()));
			vertices.push_back(glm::vec3((float)(w + 1) / (width - 1) - 0.5f, (float)h / (height - 1) - 0.5f, chunks[w + 1][h]->getCorner()));

			glm::vec3 n1 = glm::cross(vertices[vertices.size() - 2] - vertices[vertices.size() - 1], vertices[vertices.size() - 3] - vertices[vertices.size() - 1]);
			normals.push_back(n1);  colors.push_back(color);
			normals.push_back(n1);  colors.push_back(color);
			normals.push_back(n1);  colors.push_back(color);

			faces.push_back((int)vertices.size() - 3);
			faces.push_back((int)vertices.size() - 2);
			faces.push_back((int)vertices.size() - 1);

			vertices.push_back(glm::vec3((float)w / (width - 1) - 0.5f, (float)(h + 1) / (height - 1) - 0.5f, chunks[w][h + 1]->getCorner()));
			vertices.push_back(glm::vec3((float)(w + 1) / (width - 1) - 0.5f, (float)h / (height - 1) - 0.5f, chunks[w + 1][h]->getCorner()));
			vertices.push_back(glm::vec3((float)(w + 1) / (width - 1) - 0.5f, (float)(h + 1) / (height - 1) - 0.5f, chunks[w + 1][h + 1]->getCorner()));

			glm::vec3 n2 = glm::cross(vertices[vertices.size() - 3] - vertices[vertices.size() - 2], vertices[vertices.size() - 1] - vertices[vertices.size() - 2]);
			normals.push_back(n2);  colors.push_back(color);
			normals.push_back(n2);  colors.push_back(color);
			normals.push_back(n2);  colors.push_back(color);
			
			faces.push_back((int)vertices.size() - 3);
			faces.push_back((int)vertices.size() - 2);
			faces.push_back((int)vertices.size() - 1);
		}
	facesCount = (unsigned int)faces.size();

	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);

	//	free and finish
	imgload.freeImage(data);
	if (!glIsBuffer(verticesBuffer) || !glIsBuffer(normalsBuffer) || !glIsBuffer(colorsBuffer) || !glIsBuffer(facesBuffer) || !glIsVertexArray(vao))
		return false;
	return true;
}


void Map::update(const glm::vec3& playerPosition)
{
	//std::cout << "Map update center : " << playerPosition.x << " " << playerPosition.y << std::endl;

	glm::ivec2 p = worldToChunk(playerPosition);
	if (lastPlayerPos != p)
	{
		lastPlayerPos = p;
		drawableChunks.clear();
		int checkingSquareRaduis = 7;

		for (int i = -checkingSquareRaduis; i < checkingSquareRaduis + 1; i++)
			for (int j = -checkingSquareRaduis; j < checkingSquareRaduis + 1; j++)
			{
				glm::ivec2 v = glm::ivec2(p.x + i, p.y + j);
				if (v.x < width && v.y < height && v.x >= 0 && v.y >= 0)
				{
					Chunk* chunk = chunks[v.x][v.y];
					float d = glm::length(glm::vec2(i, j));
					if (d < 5.f) // radius (in chunk)
					{
						drawableChunks.push_back(v);
						if (!chunk->initialized)
						{
							float z0 = chunks[v.x][v.y]->getCorner();
							float z1 = inBound(v.x + 1, v.y) ? chunks[v.x + 1][v.y]->getCorner() : z0;
							float z2 = inBound(v.x + 1, v.y) ? chunks[v.x][v.y + 1]->getCorner() : z0;
							float z3 = inBound(v.x + 1, v.y) ? chunks[v.x + 1][v.y + 1]->getCorner() : z0;

							chunk->initialize(z0, z0, z0);
							/*while(chunk->getLod() < 4)
							{
								chunk->addLOD();
							}*/
							chunk->initializeVBO();
							chunk->initializeVAO();

							if(!chunk->initialized)
								std::cout << "error" << std::endl;
						}
					}
					else
					{
						chunk->free();
					}
				}
			}

		std::cout << "Map update center : " << p.x << " " << p.y << " " << drawableChunks.size() << std::endl;
	}
}
//

//	Set / Get
unsigned int Map::getFacesCount() const { return facesCount; }
GLuint Map::getVAO() const { return vao; }
Chunk* Map::getChunk(const int& w, const int& h) { return chunks[w][h]; }
std::vector<glm::ivec2> Map::getDrawableChunks() { return drawableChunks; }
glm::mat4 Map::getModelMatrix() const { return glm::scale(glm::mat4(1.f), glm::vec3(scale * (height - 1), scale * (width - 1), scale)); }


glm::ivec2 Map::worldToChunk(glm::vec3 p) const
{
	glm::vec3 firstChunkPos = glm::vec3(-0.5f * width - 0.5f, -0.5f * height - 0.5f, 0);
	glm::vec3 v = p - firstChunkPos;
	return glm::ivec2((int)v.x, (int)v.y);
}
bool Map::inBound(const int& x, const int& y) const
{
	if (x >= width || x < 0)
		return false;
	else if (y >= height || y < 0)
		return false;
	else return true;
}
//