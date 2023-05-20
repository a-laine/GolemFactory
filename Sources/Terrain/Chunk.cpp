#include "Chunk.h"
#include <iostream>

//	Shared attributes
uint8_t Chunk::maxSubdivide = 10;
float Chunk::rugosity = 1.f;
//

//	Default
Chunk::Chunk(unsigned int randomSeed, mat4f model, const float& cornerHeight) :
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
	vertices.push_back(vec4f( 0.5f,  corner, 0.5f, 1.f)); // bottomLeft
	vertices.push_back(vec4f(-0.5f,  topLeft, 0.5f, 1.f));
	vertices.push_back(vec4f( 0.5f, bottomRight, -0.5f, 1.f));
	vertices.push_back(vec4f(-0.5f, topRight, -0.5f, 1.f));

	colors.push_back(vec4f(1, 1, 1, 1.f));
	colors.push_back(vec4f(1, 1, 1, 1.f));
	colors.push_back(vec4f(1, 1, 1, 1.f));
	colors.push_back(vec4f(1, 1, 1, 1.f));

	normals.push_back(vec4f(0, 0, 1, 0));
	normals.push_back(vec4f(0, 0, 1, 0));
	normals.push_back(vec4f(0, 0, 1, 0));
	normals.push_back(vec4f(0, 0, 1, 0));

	//indexes
	indexes[gfvertex(vertices[0])] = 0;
	indexes[gfvertex(vertices[1])] = 1;
	indexes[gfvertex(vertices[2])] = 2;
	indexes[gfvertex(vertices[3])] = 3;

	// faces
	faces.push_back(0); faces.push_back(3); faces.push_back(1);
	faces.push_back(0); faces.push_back(2); faces.push_back(3);

	// end
	lodVerticesCount.push_back(4);
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

	facesCenter = 0;
	facesBorder = 0;
	facesBorderSeamless = 0;
}
void Chunk::addLOD(const unsigned int&  seed1, const unsigned int&  seed2, bool debug)	// N(4logN + 5logN)
{
	if (lod < maxSubdivide)
	{
		// initialization
		needVBOUpdate = true;
		int subdivision = (int)std::pow(2, lod - 1);
		float step = 1.f / subdivision;
		unsigned int initialVerticesCount = (unsigned int)vertices.size();
		float amplitude = 0.1f * step * rugosity;
		const int offset = (int)std::pow(2, maxSubdivide - 1);

		// clear old data
		faces.clear();

		// split all squares
		instantiateBorders(seed1, seed2, offset, subdivision, step, amplitude);
		initRandomNumberGenerator(2 * offset + subdivision - 1, seed);
		splitCenter(step, amplitude);
		generateBorders(step);
		generateSeamlessBorders(step);

		lod++;
		smoothNormals(0.5f * step);
		lodVerticesCount.push_back((unsigned int)vertices.size() - initialVerticesCount);

		if (debug)
		{
			
		}
	}
}
void Chunk::removeLOD()   // N(4logN + 5logN)
{
	if (lod > 1)
	{
		// initialization
		needVBOUpdate = true;
		lod--;
		int subdivision = (int)std::pow(2, lod - 1);
		float step = 1.f / subdivision;

		// clear old data
		vertices.erase(vertices.end() - lodVerticesCount.back(), vertices.end());
		colors.erase(colors.end() - lodVerticesCount.back(), colors.end());
		normals.erase(normals.end() - lodVerticesCount.back(), normals.end());
		faces.clear();
		lodVerticesCount.pop_back();

		// merge all squares
		for (float x = -0.5f; x < 0.5f; x += step)
			for (float y = -0.5f; y < 0.5f; y += step)
			{
				unsigned int i0 = indexes[gfvertex(x, y)];
				unsigned int i1 = indexes[gfvertex(x, y + step)];
				unsigned int i2 = indexes[gfvertex(x + step, y)];
				unsigned int i3 = indexes[gfvertex(x + step, y + step)];

				mergeFace(i0, i1, i2, i3);
			}

		// sooth normals
		for (float x = -0.5f + step; x < 0.5f - step; x += step)
			for (float y = -0.5f + step; y < 0.5f - step; y += step)
			{
				unsigned int index = indexes[gfvertex(x, y)];
				vec4f n(vertices[indexes[gfvertex(x - step, y)]].z - vertices[indexes[gfvertex(x + step, y)]].z,
					vertices[indexes[gfvertex(x, y - step)]].z - vertices[indexes[gfvertex(x, y + step)]].z,
					4 * step, 0.f);
				normals[indexes[gfvertex(x, y)]] = n.getNormal();
			}

	}
	if (lod == 1)
		free();
}
//

//	Set / Get
bool Chunk::getNeedVBOUpdate() const { return needVBOUpdate; }
bool Chunk::isInitialized() const { return initialized; }


void Chunk::setModelMatrix(mat4f model) { modelMatrix = model; }


mat4f Chunk::getModelMatrix() const { return modelMatrix; }
float* Chunk::getModelMatrixPtr() { return &modelMatrix[0][0]; }
uint8_t Chunk::getLod() const { return lod; }


float Chunk::getCorner(){ return corner; }
unsigned int Chunk::getSeed() { return seed; }


unsigned int Chunk::getCenterFacesCount() const { return facesCenter; }
unsigned int Chunk::getBorderFacesCount() const { return facesBorder; }
unsigned int Chunk::getSeamlessBorderFacesCount() const { return facesBorderSeamless; }
GLuint Chunk::getVAO() const { return vao; }
//



//	Private functions
void Chunk::initializeVBO()
{
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec4f), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec4f), vertices.data());

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec4f), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, normals.size() * sizeof(vec4f), normals.data());

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4f), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(vec4f), colors.data());

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, faces.size() * sizeof(unsigned int), faces.data());
}
void Chunk::initializeVAO()
{
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

	needVBOUpdate = false;
	initialized = glIsBuffer(verticesBuffer) && glIsBuffer(normalsBuffer) && glIsBuffer(colorsBuffer) && glIsBuffer(facesBuffer) && glIsVertexArray(vao);
}
void Chunk::updateVBO()
{
	needVBOUpdate = false;

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

	initializeVBO();
	initializeVAO();
}


void Chunk::splitFace(const unsigned int& i0, const unsigned int& i1, const unsigned int& i2, const unsigned int& i3, const float& amplitude, const float& step, const uint8_t& faceMode) // 5logN
{
	// generate new vertices
	vec4f v0 = vertices[i0];
	vec4f v1 = vertices[i1];
	vec4f v2 = vertices[i2];
	vec4f v3 = vertices[i3];

	// get indexes
	unsigned int i4 = instantiateVertexSmooth(0.5f * (v0 + v1), step, amplitude);
	unsigned int i6 = instantiateVertexSmooth(0.5f * (v0 + v2), step, amplitude);
	unsigned int i5 = instantiateVertexSmooth(0.5f * (v2 + v3), step, amplitude);
	unsigned int i7 = instantiateVertexSmooth(0.5f * (v3 + v1), step, amplitude);
	unsigned int i8 = instantiateVertex(0.25f * (v0 + v1 + v2 + v3), amplitude);

	// create new faces
	if (faceMode & 0x01) { faces.push_back(i0); faces.push_back(i8); faces.push_back(i4); }
	if (faceMode & 0x02) { faces.push_back(i0); faces.push_back(i6); faces.push_back(i8); }
	if (faceMode & 0x04) { faces.push_back(i4); faces.push_back(i8); faces.push_back(i1); }
	if (faceMode & 0x08) { faces.push_back(i1); faces.push_back(i8); faces.push_back(i7); }
	if (faceMode & 0x10) { faces.push_back(i7); faces.push_back(i8); faces.push_back(i3); }
	if (faceMode & 0x20) { faces.push_back(i3); faces.push_back(i8); faces.push_back(i5); }
	if (faceMode & 0x40) { faces.push_back(i5); faces.push_back(i8); faces.push_back(i2); }
	if (faceMode & 0x80) { faces.push_back(i2); faces.push_back(i8); faces.push_back(i6); }
}
void Chunk::mergeFace(unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3) // 5logN
{	
	// generate corner vertices
	vec4f v0 = vertices[i0];
	vec4f v1 = vertices[i1];
	vec4f v2 = vertices[i2];
	vec4f v3 = vertices[i3];

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


float Chunk::randf(const float& min, const float& max, const unsigned int& quantum)
{
	return min + (randi() % quantum) * (max - min) / quantum;
}
unsigned int Chunk::randi() // RAND_MAX assumed to be 32767 
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % 32768;
}
void Chunk::initRandomNumberGenerator(const unsigned int& offset, const unsigned int& newseed)
{
	next = newseed;
	for (unsigned int i = 0; i < offset; i++)
		next = next * 1103515245 + 12345;
}


unsigned int Chunk::instantiateVertex(const float& x, const float& y, const float& z, const float& amplitude)
{
	return instantiateVertex(vec4f(x, y, z, 1.f), amplitude);
}
unsigned int Chunk::instantiateVertex(const vec4f& base, const float& amplitude)
{
	auto it = indexes.find(gfvertex(base));
	if (it == indexes.end())
	{
		vec4f v = vec4f(base.x, base.y + amplitude * randf(), base.z, 1.f);
		vertices.push_back(v);
		colors.push_back(vec4f(1.f, 1.f, 1.f, 1.f));
		normals.push_back(vec4f(0, 1, 0, 0));
		indexes[gfvertex(base)] = (unsigned int)vertices.size() - 1;
		return (unsigned int)vertices.size() - 1;
	}
	else return it->second;
}
unsigned int Chunk::instantiateVertexSmooth(const vec4f& v, const float& step, const float&  amplitude)
{
	vec4f base = vec4f(v.x, 0.f, v.y, 1.f);
	int n = 0;

	auto it = indexes.find(gfvertex(v.x + step, v.y));
	if (it != indexes.end())
	{
		n++;
		base.y += vertices[it->second].z;
	}

	it = indexes.find(gfvertex(v.x - step, v.y));
	if (it != indexes.end())
	{
		n++;
		base.y += vertices[it->second].z;
	}

	it = indexes.find(gfvertex(v.x, v.y + step));
	if (it != indexes.end())
	{
		n++;
		base.y += vertices[it->second].z;
	}

	it = indexes.find(gfvertex(v.x, v.y - step));
	if (it != indexes.end())
	{
		n++;
		base.y += vertices[it->second].z;
	}

	base.z /= (float)n;
	return instantiateVertex(base, amplitude);
}


void Chunk::smoothNormals(const float& step)
{
	for (float x = -0.5f + step; x < 0.5f - step; x += step)
		for (float y = -0.5f + step; y < 0.5f - step; y += step)
		{
			unsigned int index = indexes[gfvertex(x, y)];
			vec4f n(vertices[indexes[gfvertex(x - step, y)]].z - vertices[indexes[gfvertex(x + step, y)]].z,
				vertices[indexes[gfvertex(x, y - step)]].z - vertices[indexes[gfvertex(x, y + step)]].z,
				4 * step, 0);
			normals[indexes[gfvertex(x, y)]] = n.getNormal();
		}
}
void Chunk::splitCenter(const float& step, const float& amplitude)
{
	for (float x = -0.5f; x < 0.5f; x += step)
		for (float y = -0.5f; y < 0.5f; y += step)
		{
			unsigned int i0 = indexes[gfvertex(x, y)];
			unsigned int i1 = indexes[gfvertex(x, y + step)];
			unsigned int i2 = indexes[gfvertex(x + step, y)];
			unsigned int i3 = indexes[gfvertex(x + step, y + step)];

			splitFace(i0, i1, i2, i3, amplitude, 0.5f * step, (x != -0.5f && y != -0.5f && x != 0.5f - step && y != 0.5f - step) ? 0xFF : 0x00);
		}
	facesCenter = (unsigned int)faces.size();
}
void Chunk::instantiateBorders(const unsigned int&  seed1, const unsigned int&  seed2, const int& offset, const int subdivision, const float& step, const float& amplitude)
{
	// inject neibors borders vertices
	initRandomNumberGenerator(subdivision - 1, seed1);
	for (float y = -0.5f; y < 0.5f; y += step)
	{
		unsigned int i0 = indexes[gfvertex(-0.5f, y)];
		unsigned int i1 = indexes[gfvertex(-0.5f, y + step)];
		instantiateVertex(0.5f * (vertices[i0] + vertices[i1]), amplitude);
	}
	initRandomNumberGenerator(offset + subdivision - 1, seed2);
	for (float x = -0.5f; x < 0.5f; x += step)
	{
		unsigned int i0 = indexes[gfvertex(x, -0.5f)];
		unsigned int i1 = indexes[gfvertex(x + step, -0.5f)];
		instantiateVertex(0.5f * (vertices[i0] + vertices[i1]), amplitude);
	}

	// inject personal border
	initRandomNumberGenerator(subdivision - 1, seed);
	for (float y = -0.5f; y < 0.5f; y += step)
	{
		unsigned int i0 = indexes[gfvertex(0.5f, y)];
		unsigned int i1 = indexes[gfvertex(0.5f, y + step)];
		instantiateVertex(0.5f * (vertices[i0] + vertices[i1]), amplitude);
	}
	initRandomNumberGenerator(offset + subdivision - 1, seed);
	for (float x = -0.5f; x < 0.5f; x += step)
	{
		unsigned int i0 = indexes[gfvertex(x, 0.5f)];
		unsigned int i1 = indexes[gfvertex(x + step, 0.5f)];
		instantiateVertex(0.5f * (vertices[i0] + vertices[i1]), amplitude);
	}
}
void Chunk::generateBorders(const float& step)
{
	/*
		(n) = vertex index
		 n  = face bit

			(1)-----------(7)-----------(3)
			 | \           |          /  |
			 |   \    4    |   5    /    |
			 |     \       |      /      |
			 |       \     |    /        |
			 |   3      \  |  /     6    |
			(1)-----------(7)-----------(3)
			 |          /  | \           |
			 |   1    /    |   \    7    |
			 |     /       |     \       |
			 |   /   2     | 8     \     |
			 | /           |          \  |
			(0)-----------(6)-----------(2)
	*/

	unsigned int facesBorderStart = (unsigned int)faces.size();
	for (float x = -0.5f; x < 0.5f; x += step)
	{
		uint16_t mode = 0b00011000;
		if (x != -0.5f && x != 0.5f - step) mode = 0b11111111;
		else if(x == -0.5f) mode = 0b01111000;
		else if (x == 0.5f - step) mode = 0b00011101;
		
		generateFaces(x, 0.5f - step, step, mode);
	}
	facesBorder = (unsigned int)(faces.size() - facesBorderStart);

	for (float x = -0.5f; x < 0.5f; x += step)
	{
		uint16_t mode = 0b10000010;
		if (x != -0.5f && x != 0.5f - step) mode = 0b11111111;
		else if (x == -0.5f) mode = 0b11100010;
		else if (x == 0.5f - step) mode = 0b10000111;

		generateFaces(x, -0.5f, step, mode);
	}

	for (float y = -0.5f; y < 0.5f; y += step)
	{
		uint16_t mode = 0b00000101;
		if (y != -0.5f && y != 0.5f - step) mode = 0b11111111;
		else if (y == -0.5f) mode = 0b00011101;
		else if (y == 0.5f - step) mode = 0b10000111;

		generateFaces(-0.5f, y, step, mode);
	}

	for (float y = -0.5f; y < 0.5f; y += step)
	{
		uint16_t mode = 0b01100000;
		if (y != -0.5f && y != 0.5f - step) mode = 0b11111111;
		else if (y == -0.5f) mode = 0b01111000;
		else if (y == 0.5f - step) mode = 0b11100010;

		generateFaces(0.5f - step, y, step, mode);
	}
}
void Chunk::generateSeamlessBorders(const float& step)
{
	/* sama as before and add degenerated faces
		(n) = vertex index
		 n  = face bit

		(1)-----------(7)-----------(3)
		 | \                      /  |
		 |   \                  /    |
		 |     \      10      /      |
		 |       \          /        |
		 |          \     /          |
		(1)    9      (7)      11   (3)
		 |          /    \           |
		 |        /        \         |
		 |     /             \       |
		 |   /        12       \     |
		 | /                      \  |
		(0)-----------(6)-----------(2)
	*/

	unsigned int facesSeamlessBorderStart = (unsigned int)faces.size();
	for (float x = -0.5f; x < 0.5f; x += step)
	{
		uint16_t mode = 0b1000000000;
		if (x != -0.5f && x != 0.5f - step) mode = 0b1011100111;
		else if (x == -0.5f) mode = 0b1001100000;
		else if (x == 0.5f - step) mode = 0b1000000101;

		generateFaces(x, 0.5f - step, step, mode);
	}
	facesBorderSeamless = (unsigned int)(faces.size() - facesSeamlessBorderStart);

	for (float x = -0.5f; x < 0.5f; x += step)
	{
		uint16_t mode = 0b100000000000;
		if (x != -0.5f && x != 0.5f - step) mode = 0b100001111101;
		else if (x == -0.5f) mode = 0b100001100000;
		else if (x == 0.5f - step) mode = 0b100000000101;

		generateFaces(x, -0.5f, step, mode);
	}

	for (float y = -0.5f; y < 0.5f; y += step)
	{
		uint16_t mode = 0b100000000;
		if (y != -0.5f && y != 0.5f - step) mode = 0b111111010;
		else if (y == -0.5f) mode = 0b100011000;
		else if (y == 0.5f - step) mode = 0b110000010;

		generateFaces(-0.5f, y, step, mode);
	}

	for (float y = -0.5f; y < 0.5f; y += step)
	{
		uint16_t mode = 0b10000000000;
		if (y != -0.5f && y != 0.5f - step) mode = 0b10010011111;
		else if (y == -0.5f) mode = 0b10000011000;
		else if (y == 0.5f - step) mode = 0b10010000010;

		generateFaces(0.5f - step, y, step, mode);
	}
}
void Chunk::generateFaces(const float& x, const float& y, const float& step, const uint16_t& faceMode)
{
	unsigned int i0 = indexes[gfvertex(x, y)];
	unsigned int i1 = indexes[gfvertex(x, y + step)];
	unsigned int i2 = indexes[gfvertex(x + step, y)];
	unsigned int i3 = indexes[gfvertex(x + step, y + step)];

	vec4f v0 = vertices[i0];
	vec4f v1 = vertices[i1];
	vec4f v2 = vertices[i2];
	vec4f v3 = vertices[i3];

	unsigned int i4 = indexes[gfvertex(0.5f * (v0 + v1))];
	unsigned int i6 = indexes[gfvertex(0.5f * (v0 + v2))];
	unsigned int i5 = indexes[gfvertex(0.5f * (v2 + v3))];
	unsigned int i7 = indexes[gfvertex(0.5f * (v3 + v1))];
	unsigned int i8 = indexes[gfvertex(0.25f * (v0 + v1 + v2 + v3))];

	if (faceMode & 0x01) { faces.push_back(i0); faces.push_back(i8); faces.push_back(i4); } //face 1
	if (faceMode & 0x02) { faces.push_back(i0); faces.push_back(i6); faces.push_back(i8); } //face 2
	if (faceMode & 0x04) { faces.push_back(i4); faces.push_back(i8); faces.push_back(i1); } //face 3
	if (faceMode & 0x08) { faces.push_back(i1); faces.push_back(i8); faces.push_back(i7); } //face 4
	if (faceMode & 0x10) { faces.push_back(i7); faces.push_back(i8); faces.push_back(i3); } //face 5
	if (faceMode & 0x20) { faces.push_back(i3); faces.push_back(i8); faces.push_back(i5); } //face 6
	if (faceMode & 0x40) { faces.push_back(i5); faces.push_back(i8); faces.push_back(i2); } //face 7
	if (faceMode & 0x80) { faces.push_back(i2); faces.push_back(i8); faces.push_back(i6); } //face 8

	if (faceMode & 0x100) { faces.push_back(i0); faces.push_back(i8); faces.push_back(i1); } //face 9
	if (faceMode & 0x200) { faces.push_back(i1); faces.push_back(i8); faces.push_back(i3); } //face 10
	if (faceMode & 0x400) { faces.push_back(i3); faces.push_back(i8); faces.push_back(i2); } //face 11
	if (faceMode & 0x800) { faces.push_back(i2); faces.push_back(i8); faces.push_back(i0); } //face 12

	//std::cout << (int)faceMode << " ";
}
//
