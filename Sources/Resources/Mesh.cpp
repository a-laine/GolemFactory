#include "Mesh.h"

//  Default
Mesh::Mesh(std::string path,std::string meshName) : ResourceVirtual(path,meshName,ResourceVirtual::MESH), configuration(0x00)
{
    std::string file = path + meshName;
	if (MeshLoader::loadMesh(file, vertices, normales, color, faces)) return;
	configuration |= 0x01;
	computeBoundingBoxDimension();

	initializeVBO();
	initializeVAO();
}
Mesh::Mesh() : ResourceVirtual(ResourceVirtual::MESH), configuration(0x00)
{

}
Mesh::~Mesh()
{
	vertices.clear();
	normales.clear();
	color.clear();
	faces.clear();

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteBuffers(1, &colorBuffer);
	glDeleteBuffers(1, &arraybuffer);
	glDeleteVertexArrays(1, &vao);
}
//


//
void Mesh::initializeVBO()
{
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normales.size() * sizeof(glm::vec3), normales.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(glm::vec3), color.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &arraybuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);
}
void Mesh::initializeVAO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBindVertexArray(0);
}

void Mesh::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, NULL);
}
bool Mesh::isValid() const
{
	if (configuration & 0x01)
		return glIsBuffer(vertexbuffer) && glIsBuffer(normalBuffer) && glIsBuffer(colorBuffer) && glIsBuffer(arraybuffer) && glIsVertexArray(vao);
	else return false;
}
//

//  Set/get functions
unsigned int Mesh::getNumberVertices() const{return vertices.size();}
unsigned int Mesh::getNumberFaces() const{return faces.size();}
//

//	Protected functions
void Mesh::computeBoundingBoxDimension()
{
	sizeX = glm::vec2(0, 0);
	sizeY = glm::vec2(0, 0);
	sizeZ = glm::vec2(0, 0);

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
