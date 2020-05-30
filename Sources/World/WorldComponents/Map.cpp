#include "Map.h"

#include "Resources/Loader/ImageLoader.h"

#include <iostream>


//	Default
Map::Map() : height(0), width(0), heightmap(nullptr), amplitude(1.f), cellSize(10.f), vao(0), facesCount(0)
{}
Map::~Map()
{
	if (heightmap)
	{
		for (int h = 0; h < height; h++)
			delete[] heightmap[h];
		delete[] heightmap;
	}

	glDeleteBuffers(1, &verticesBuffer);
	glDeleteBuffers(1, &normalsBuffer);
	glDeleteBuffers(1, &colorsBuffer);
	glDeleteBuffers(1, &facesBuffer);
	glDeleteVertexArrays(1, &vao);
}
//

//	Public functions
bool Map::loadFromHeightmap(const std::string& resourceDirectory, const std::string& fileName)
{
	//	import
	int channel;
	ImageLoader imgload;
	uint8_t* data = imgload.loadFromFile(resourceDirectory + fileName, width, height, channel, ImageLoader::GREY);
	if (data == nullptr || width < 1 || height < 1)
		return false;

	//	allocate and load
	int offset = 128;
	heightmap = new float*[height];
	for (int h = 0; h < height; h++)
	{
		heightmap[h] = new float[width];
	}
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++)
		{
			heightmap[h][w] = amplitude * ((int)data[h * width + w] - offset) * 0.00390625f; // = 1/256
		}

	//	create mesh data
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<unsigned int> faces;
	glm::vec3 color = glm::vec3(1.f, 1.f, 1.f);
	
	for (int h = 0; h < height - 1; h++)
		for (int w = 0; w < width - 1; w++)
		{
			vertices.push_back(glm::vec3((float)w / (width - 1) - 0.5f, (float)h / (height - 1) - 0.5f, heightmap[h][w]));
			vertices.push_back(glm::vec3((float)w / (width - 1) - 0.5f, (float)(h + 1) / (height - 1) - 0.5f, heightmap[h + 1][w]));
			vertices.push_back(glm::vec3((float)(w + 1) / (width - 1) - 0.5f, (float)h / (height - 1) - 0.5f, heightmap[h][w + 1]));

			glm::vec3 n1 = glm::cross(vertices[vertices.size() - 2] - vertices[vertices.size() - 1], vertices[vertices.size() - 3] - vertices[vertices.size() - 1]);
			normals.push_back(n1);  colors.push_back(color);
			normals.push_back(n1);  colors.push_back(color);
			normals.push_back(n1);  colors.push_back(color);

			faces.push_back((int)vertices.size() - 3);
			faces.push_back((int)vertices.size() - 2);
			faces.push_back((int)vertices.size() - 1);

			vertices.push_back(glm::vec3((float)w / (width - 1) - 0.5f, (float)(h + 1) / (height - 1) - 0.5f, heightmap[h + 1][w]));
			vertices.push_back(glm::vec3((float)(w + 1) / (width - 1) - 0.5f, (float)h / (height - 1) - 0.5f, heightmap[h][w + 1]));
			vertices.push_back(glm::vec3((float)(w + 1) / (width - 1) - 0.5f, (float)(h + 1) / (height - 1) - 0.5f, heightmap[h + 1][w + 1]));

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
//

//	Set / Get
unsigned int Map::getFacesCount() const { return facesCount; }
GLuint Map::getVAO() const { return vao; }
//