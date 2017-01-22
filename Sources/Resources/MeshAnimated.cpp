#include "MeshAnimated.h"


//  Default
MeshAnimated::MeshAnimated( const std::string& meshName, const std::vector<glm::vec3>& verticesArray, const std::vector<glm::vec3>& normalesArray,
							const std::vector<glm::vec3>& colorArray, const std::vector<glm::ivec3>& bonesArray, const std::vector<glm::vec3>& weightsArray,
							const std::vector<unsigned int>& facesArray)
	: Mesh(meshName)
{
	configuration = VALID | HAS_SKELETON;
	vertices = verticesArray;
	normales = normalesArray;
	colors = colorArray;
	bones = bonesArray;
	weights = weightsArray;
	faces = facesArray;
	computeBoundingBoxDimension();
	initializeVBO();
	initializeVAO();
}
MeshAnimated::~MeshAnimated()
{
	weights.clear();
	bones.clear();
	glDeleteBuffers(1, &bonesBuffer);
	glDeleteBuffers(1, &weightsBuffer);
}
//

//	Public functions
void MeshAnimated::initializeVBO()
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

	glGenBuffers(1, &bonesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
	glBufferData(GL_ARRAY_BUFFER, bones.size() * sizeof(glm::ivec3), bones.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &weightsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, weightsBuffer);
	glBufferData(GL_ARRAY_BUFFER, weights.size() * sizeof(glm::vec3), weights.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);
}
void MeshAnimated::initializeVAO()
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

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
	glVertexAttribPointer(3, 3, GL_INT, GL_FALSE, 0, NULL);
	
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, weightsBuffer);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);
}

void MeshAnimated::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, NULL);
}
//

//	Set / get functions
bool MeshAnimated::isValid() const
{
	if (!Mesh::isValid()) return false;
	else return glIsBuffer(bonesBuffer) && glIsBuffer(weightsBuffer);
}
//





