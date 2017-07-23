#include "Mesh.h"


//  Default
Mesh::Mesh(const std::string& meshName) : ResourceVirtual(meshName, ResourceVirtual::MESH), configuration(0x00) {}
Mesh::Mesh(const std::string& meshName, const std::vector<glm::vec3>& verticesArray, const std::vector<glm::vec3>& normalesArray, const std::vector<glm::vec3>& colorArray, const std::vector<unsigned int>& facesArray)
	: ResourceVirtual(meshName, ResourceVirtual::MESH), configuration(VALID), vertices(verticesArray), normales(normalesArray), colors(colorArray), faces(facesArray)
{
	computeBoundingBoxDimension();
	initializeVBO();
	initializeVAO();
}
Mesh::~Mesh()
{
	vertices.clear();
	normales.clear();
	colors.clear();
	faces.clear();

	glDeleteBuffers(1, &verticesBuffer);
	glDeleteBuffers(1, &normalsBuffer);
	glDeleteBuffers(1, &colorsBuffer);
	glDeleteBuffers(1, &facesBuffer);
	glDeleteVertexArrays(1, &vao);
}
//


//	Public functions
void Mesh::initializeVBO()
{
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normales.size() * sizeof(glm::vec3), normales.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);
}
void Mesh::initializeVAO()
{
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
}

void Mesh::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, NULL);
}
//

//  Set/get functions
unsigned int Mesh::getNumberVertices() const { return vertices.size(); }
unsigned int Mesh::getNumberFaces() const { return faces.size(); }
bool Mesh::isValid() const
{
	if (configuration & VALID)
		return glIsBuffer(verticesBuffer) && glIsBuffer(normalsBuffer) && glIsBuffer(colorsBuffer) && glIsBuffer(facesBuffer) && glIsVertexArray(vao);
	else return false;
}
bool Mesh::hasSkeleton() const
{
	return (configuration & HAS_SKELETON) != 0;
}
bool Mesh::isAnimable() const
{
	return (configuration & IS_ANIMABLE) != 0;
}
//

//	Protected functions
void Mesh::computeBoundingBoxDimension()
{
	sizeX = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	sizeY = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	sizeZ = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		sizeX.x = std::min(sizeX.x, vertices[i].x);
		sizeX.y = std::max(sizeX.y, vertices[i].x);

		sizeY.x = std::min(sizeY.x, vertices[i].y);
		sizeY.y = std::max(sizeY.y, vertices[i].y);

		sizeZ.x = std::min(sizeZ.x, vertices[i].z);
		sizeZ.y = std::max(sizeZ.y, vertices[i].z);
	}
}
//
