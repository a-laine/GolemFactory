#include "Mesh.h"

//  Default
Mesh::Mesh(std::string path,std::string meshName) :
    ResourceVirtual(path,meshName,ResourceVirtual::MESH)
{
    std::string file = path + meshName;
	int i = MeshLoader::loadMesh(file, vertices, normales, color, faces);
	if (i) configuration |= 0x01;
	else return;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &arraybuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
}
Mesh::~Mesh()
{
	vertices.clear();
	normales.clear();
	color.clear();
	faces.clear();

	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &arraybuffer);
	glDeleteVertexArrays(1, &vao);
}

void Mesh::draw()
{
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, NULL);
	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
}
bool Mesh::isValid() const { return !configuration; }
//

//  Set/get functions
unsigned int Mesh::getNumberVertices() const{return vertices.size();}
unsigned int Mesh::getNumberFaces() const{return faces.size();}
//
