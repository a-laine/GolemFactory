#include "Chunk.h"
#include <iostream>

//	Shared attributes
uint8_t Chunk::maxSubdivide = 6;
float Chunk::rugosity = 1.f;
//

//	Default
Chunk::Chunk(unsigned int randomSeed, glm::mat4 model, const float& cornerHeight) :
	needVBOUpdate(false), initialized(false), lod(1), seed(randomSeed), next(randomSeed), modelMatrix(model), corner(cornerHeight), 
	vao(0), verticesBuffer(0), colorsBuffer(0), normalsBuffer(0), facesBuffer(0)
{

}
Chunk::~Chunk()
{
	free();
}
//

//	Public functions
void Chunk::initialize(const float& topLeft, const float& topRight, const float& bottomRight)
{
	// primer square vertices
	vertices.push_back(glm::vec3( 0.5f,  0.5f, corner)); // bottomLeft
	vertices.push_back(glm::vec3(-0.5f,  0.5f, topLeft));
	vertices.push_back(glm::vec3( 0.5f, -0.5f, bottomRight));
	vertices.push_back(glm::vec3(-0.5f, -0.5f, topRight));

	colors.push_back(glm::vec3(1, 0, 0));
	colors.push_back(glm::vec3(0, 1, 0));
	colors.push_back(glm::vec3(0.2, 0.2, 1));
	colors.push_back(glm::vec3(1, 1, 0));

	normals.push_back(glm::vec3(0, 0, 1));
	normals.push_back(glm::vec3(0, 0, 1));
	normals.push_back(glm::vec3(0, 0, 1));
	normals.push_back(glm::vec3(0, 0, 1));

	//indexes
	indexes[gfvertex(vertices[0])] = 0;
	indexes[gfvertex(vertices[1])] = 1;
	indexes[gfvertex(vertices[2])] = 2;
	indexes[gfvertex(vertices[3])] = 3;

	// faces
	faces.push_back(0); faces.push_back(3); faces.push_back(1);
	faces.push_back(0); faces.push_back(2); faces.push_back(3);

	// end
	lodVerticesCount.push_back((unsigned int)vertices.size());
}
void Chunk::free()
{
	vertices.clear();
	colors.clear();
	normals.clear();
	faces.clear();
	indexes.clear();
	lodVerticesCount.clear();

	lod = 1;
	needVBOUpdate = false;
	initialized = false;

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
void Chunk::addLOD()	// N(4logN + 5logN)
{
	if (lod < maxSubdivide)
	{
		// initialization
		needVBOUpdate = true;
		float step = 1.f / (int)glm::pow(2, lod - 1);
		lod++;
		unsigned int initialVerticesCount = (unsigned int)vertices.size();

		// clear old data
		faces.clear();

		// split all squares
		for (float x = -0.5f; x < 0.5f; x += step)
			for (float y = -0.5f; y < 0.5f; y += step)
			{
				unsigned int i0 = indexes[gfvertex(x, y)];
				unsigned int i1 = indexes[gfvertex(x, y + step)];
				unsigned int i2 = indexes[gfvertex(x + step, y)];
				unsigned int i3 = indexes[gfvertex(x + step, y + step)];

				splitFace(i0, i1, i2, i3, 0.1f* step);
			}

		// save sizes
		lodVerticesCount.push_back((unsigned int)vertices.size() - initialVerticesCount);
	}
}
void Chunk::removeLOD()   // N(4logN + 5logN)
{
	if (lod > 1)
	{
		// initialization
		needVBOUpdate = true;
		lod--;
		float step = 1.f / ((int)glm::pow(2, lod) - 1);

		// clear old data
		vertices.erase(vertices.begin() + lodVerticesCount.back(), vertices.end());
		colors.erase(colors.begin() + lodVerticesCount.back(), colors.end());
		normals.erase(normals.begin() + lodVerticesCount.back(), normals.end());
		faces.clear();
		lodVerticesCount.pop_back();

		// merge all squares
		for (float x = -0.5f; x <= 0.5f; x += step)
			for (float y = -0.5f; y <= 0.5f; y += step)
			{
				unsigned int i0 = indexes[gfvertex(x, y)];
				unsigned int i1 = indexes[gfvertex(x, y + step)];
				unsigned int i2 = indexes[gfvertex(x + step, y)];
				unsigned int i3 = indexes[gfvertex(x + step, y + step)];

				mergeFace(i0, i1, i2, i3);
			}
	}
}
//

//	Set / Get
bool Chunk::getNeedVBOUpdate() const { return needVBOUpdate; }
bool Chunk::isInitialized() const { return initialized; }


void Chunk::setModelMatrix(glm::mat4 model) { modelMatrix = model; }


glm::mat4 Chunk::getModelMatrix() const { return modelMatrix; }
float* Chunk::getModelMatrixPtr() { return &modelMatrix[0][0]; }
uint8_t Chunk::getLod() const { return lod; }


float Chunk::getCorner()
{
	return corner;
}
std::vector<float> Chunk::getLeftBorder(const uint8_t& targetLod)
{
	std::vector<float> result;
	next = seed;
	result.push_back(randf());
	for (uint8_t i = 0; i < targetLod; i++)
		result.push_back(randf());
	return result;
}
std::vector<float> Chunk::getBottomBorder(const uint8_t& targetLod)
{
	std::vector<float> result;
	next = seed;
	result.push_back(randf());
	randJump((unsigned int)glm::pow(2, maxSubdivide));
	for (uint8_t i = 0; i < targetLod; i++)
		result.push_back(randf());
	return result;
}


unsigned int Chunk::getFacesCount() const { return (unsigned int)faces.size(); }
GLuint Chunk::getVAO() const { return vao; }
//



//	Private functions
void Chunk::initializeVBO()
{
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_DYNAMIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_DYNAMIC_DRAW);
}
void Chunk::initializeVAO()
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

	needVBOUpdate = false;
	initialized = glIsBuffer(verticesBuffer) && glIsBuffer(normalsBuffer) && glIsBuffer(colorsBuffer) && glIsBuffer(facesBuffer) && glIsVertexArray(vao);
}
void Chunk::updateVBO()
{
	needVBOUpdate = false;

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());

	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(glm::vec3), normals.data());

	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(glm::vec3), colors.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, faces.size() * sizeof(unsigned int), faces.data());
}


void Chunk::splitFace(const unsigned int& i0, const unsigned int& i1, const unsigned int& i2, const unsigned int& i3, const float& amplitude) // 5logN
{
	// generate new vertices
	glm::vec3 v0 = vertices[i0];
	glm::vec3 v1 = vertices[i1];
	glm::vec3 v2 = vertices[i2];
	glm::vec3 v3 = vertices[i3];

	// get indexes
	unsigned int i4 = instantiateVertex(0.5f * (v0 + v1), amplitude);
	unsigned int i5 = instantiateVertex(0.5f * (v2 + v3), amplitude);
	unsigned int i6 = instantiateVertex(0.5f * (v0 + v2), amplitude);
	unsigned int i7 = instantiateVertex(0.5f * (v3 + v1), amplitude);
	unsigned int i8 = instantiateVertex(0.25f * (v0 + v1 + v2 + v3), amplitude);
		
	// create new faces
	faces.push_back(i0); faces.push_back(i8); faces.push_back(i4);
	faces.push_back(i0); faces.push_back(i6); faces.push_back(i8);
	faces.push_back(i4); faces.push_back(i7); faces.push_back(i1);
	faces.push_back(i4); faces.push_back(i8); faces.push_back(i7);
	faces.push_back(i6); faces.push_back(i5); faces.push_back(i8);
	faces.push_back(i6); faces.push_back(i2); faces.push_back(i5);
	faces.push_back(i8); faces.push_back(i3); faces.push_back(i7);
	faces.push_back(i8); faces.push_back(i5); faces.push_back(i3);
}
void Chunk::mergeFace(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3) // 5logN
{	
	// generate corner vertices
	glm::vec3 v0 = vertices[i0];
	glm::vec3 v1 = vertices[i1];
	glm::vec3 v2 = vertices[i2];
	glm::vec3 v3 = vertices[i3];

	// search and remove sub square vertices
	auto it = indexes.find(gfvertex(0.5f * (v0 + v1)));
	if (it != indexes.end())
		indexes.erase(it);
	it = indexes.find(gfvertex(0.5f * (v2 + v3)));
	if (it != indexes.end())
		indexes.erase(it);
	it = indexes.find(gfvertex(0.5f * (v0 + v2)));
	if (it != indexes.end())
		indexes.erase(it);
	it = indexes.find(gfvertex(0.5f * (v1 + v3)));
	if (it != indexes.end())
		indexes.erase(it);
	it = indexes.find(gfvertex(0.25f * (v0 + v1 + v2 + v3)));
	if (it != indexes.end())
		indexes.erase(it);

	// create new triangles
	faces.push_back(i0); faces.push_back(i3); faces.push_back(i1);
	faces.push_back(i0); faces.push_back(i2); faces.push_back(i3);
}


float Chunk::randf(const float& min, const float& max, const unsigned int& quantum) const
{
	return min + (rand() % quantum) * (max - min) / quantum;
}
unsigned int Chunk::randi() // RAND_MAX assumed to be 32767 
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % 32768;
}
void Chunk::randJump(const unsigned int& jumpCount)
{
	for(unsigned int i = 0; i<jumpCount; i++)
		next = next * 1103515245 + 12345;
}

unsigned int Chunk::instantiateVertex(const float& x, const float& y, const float& z, const float& amplitude)
{
	return instantiateVertex(glm::vec3(x, y, z), amplitude);
}
unsigned int Chunk::instantiateVertex(const glm::vec3& base, const float& amplitude)
{
	auto it = indexes.find(gfvertex(base));
	if (it == indexes.end())
	{
		glm::vec3 v = glm::vec3(base.x, base.y, base.z + amplitude * randf());
		vertices.push_back(v);
		colors.push_back(glm::vec3(1, 1, 1));
		normals.push_back(glm::vec3(0, 0, 1));
		indexes[gfvertex(base)] = (unsigned int)vertices.size() - 1;
		return (unsigned int)vertices.size() - 1;
	}
	else return it->second;
}
//
