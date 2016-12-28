#include "HouseGenerator.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>

//  Attributes
glm::vec3 HouseGenerator::stoneColor = glm::vec3(0.2f, 0.2f, 0.2f);
//


//  Default
HouseGenerator::HouseGenerator()
{
	houseField = nullptr;
	houseFieldSize = 0;
	houseFieldFloor = 0;

	assetLibrary["wall"] = ResourceManager::getInstance()->getMesh("House/HouseWall.obj");
	assetLibrary["corner"] = ResourceManager::getInstance()->getMesh("House/HouseCorner.obj");
	assetLibrary["door"] = ResourceManager::getInstance()->getMesh("House/HouseDoor.obj");
	assetLibrary["window"] = ResourceManager::getInstance()->getMesh("House/HouseWindow.obj");

	blockLibrary.push_back(glm::ivec3(8, 5, 3));	// 120 m²
	blockLibrary.push_back(glm::ivec3(8, 5, 2));	// 80 m²
	blockLibrary.push_back(glm::ivec3(6, 4, 3));	// 72 m²
	blockLibrary.push_back(glm::ivec3(6, 4, 2));	// 48 m²
	blockLibrary.push_back(glm::ivec3(6, 4, 1));	// 24 m²
	blockLibrary.push_back(glm::ivec3(4, 3, 1));	// 12 m²
	blockLibrary.push_back(glm::ivec3(3, 2, 1));	// 6 m²
	blockLibrary.push_back(glm::ivec3(3, 1, 1));	// 3 m²
}
HouseGenerator::~HouseGenerator()
{
	if (houseField)
	{
		for (unsigned int j = 0; j < houseFieldFloor; j++)
		{
			for (unsigned int i = 0; i < houseFieldSize; i++)
				delete[] houseField[j][i];
			delete[] houseField[j];
		}
		delete[] houseField;
		houseField = nullptr;
	}
}
//

//  Public functions
InstanceVirtual* HouseGenerator::getHouse(unsigned int seed, int density, int prosperity)
{
	// initialize
	if (density < 0)
		density = (rand() % 100);
	if (prosperity < 0)
	{
		prosperity = density / 3 + (rand() % 100);
		if (prosperity > 100) prosperity = 100;
	}
	initHouseField(std::max(prosperity, 10));
	std::string houseName = "house" + std::to_string(seed) + 'd' + std::to_string(density) + 'p' + std::to_string(prosperity);
	unsigned int superficy = std::max((prosperity / 5) * (prosperity / 5) + (rand() % prosperity), 15);

	//	Debug
	std::cout << "House name" << houseName << std::endl;
	std::cout << "   seed : " << seed << std::endl;
	std::cout << "   density : " << density << std::endl;
	std::cout << "   prosperity : " << prosperity << std::endl;
	std::cout << "   house field size : " << houseFieldSize << std::endl;
	std::cout << "   house field floor count : " << houseFieldFloor << std::endl;
	std::cout << "   superficy : " << superficy << std::endl;

	//	generate block list used for constructions
	createBlocs(superficy);
	
	int ox = houseFieldSize / 2;
	int oy = houseFieldSize / 2;
	srand(seed);

	for (unsigned int i = 0; i < blockList.size(); i++)
	{
		for (int j = 0; j < 1000; j++)
		{
			int x = (rand() % (houseFieldSize - 4) + 2);
			int y = (rand() % (houseFieldSize - 4) + 2);
			if (!freePlace(x, y, 0, blockList[i].x, blockList[i].y, blockList[i].z)) continue;
			if (i != 0 && adjacentBlock(x, y, 0, blockList[i].x, blockList[i].y) < 3) continue;
			else
			{
				addBlocks(x, y, 0, blockList[i].x, blockList[i].y, blockList[i].z, 1);
				j = 1000;
			}
		}
	}

	// Add dummy bloc for test house generator
	

	/*
	addBlocks(ox - 4, oy,     oz, 4, 4, 1, 1);		// house empty
	addBlocks(ox,     oy - 1, oz, 6, 7, 2, 1);		// house empty
	addBlocks(ox + 6, oy,     oz, 4, 4, 1, 1);		// house empty

	addBlocks(ox + 3, oy + 5, oz, 1, 1, 1, 2);		// house door

	addBlocks(ox + 1, oy + 5, oz, 1, 1, 1, 3);		// house window
	addBlocks(ox + 7, oy,     oz, 1, 1, 1, 3);		// house window
	addBlocks(ox + 7, oy + 3, oz, 1, 1, 1, 3);		// house window
	addBlocks(ox + 9, oy + 2, oz, 1, 1, 1, 3);		// house window
	addBlocks(ox - 2, oy,     oz, 1, 1, 1, 3);		// house window
	addBlocks(ox - 2, oy + 3, oz, 1, 1, 1, 3);		// house window
	addBlocks(ox - 4, oy + 2, oz, 1, 1, 1, 3);		// house window
	addBlocks(ox + 1, oy + 5, oz + 1, 1, 1, 1, 3);	// house window
	addBlocks(ox + 4, oy + 5, oz + 1, 1, 1, 1, 3);	// house window
	addBlocks(ox + 4, oy - 1, oz + 1, 1, 1, 1, 3);	// house window
	*/

	//	Construct Mesh
	constructMesh();
	optimizeMesh();

	std::cout << "   vertex count : " << verticesArray.size() << std::endl;
	std::cout << "   faces count : " << facesArray.size() << std::endl;

	ResourceManager::getInstance()->addMesh(new Mesh(houseName, verticesArray, normalesArray, colorArray, facesArray));
	return new InstanceDrawable(houseName);
}
//

//  Protected functions
inline void HouseGenerator::initHouseField(unsigned int newSize)
{
	//  Clear mesh temp variable
	verticesArray.clear();
	normalesArray.clear();
	colorArray.clear();
	facesArray.clear();

	//	Clear house block list available for construction
	blockList.clear();

	//	Delete last house field
	if (newSize != houseFieldSize && houseField)
	{
		for (unsigned int j = 0; j < houseFieldFloor; j++)
		{
			for (unsigned int i = 0; i < houseFieldSize; i++)
				delete[] houseField[j][i];
			delete[] houseField[j];
		}
		delete[] houseField;
		houseField = nullptr;
	}

	//	Create new house field
	if (newSize != houseFieldSize)
	{
		houseFieldSize = newSize;
		houseFieldFloor = 1 + newSize / 25;

		houseField = new HouseVoxel**[houseFieldFloor];
		for (unsigned int j = 0; j < houseFieldFloor; j++)
		{
			houseField[j] = new HouseVoxel*[houseFieldSize];
			for (unsigned int i = 0; i < houseFieldSize; i++)
				houseField[j][i] = new HouseVoxel[houseFieldSize];
		}
	}

	//	Initialize new house field
	HouseVoxel dummy = {true, 0};
	for (unsigned int k = 0; k < houseFieldFloor; k++)
		for (unsigned int i = 0; i < houseFieldSize; i++)
			for (unsigned int j = 0; j < houseFieldSize; j++)
				houseField[k][i][j] = dummy;
}
bool HouseGenerator::freePlace(int px, int py, int pz, int sx, int sy, int sz)
{
	//	Check out of range
	if (px < 0 || py < 0 || pz < 0)
		return false;
	else if (px + sx > (int)houseFieldSize - 2 || py + sy > (int)houseFieldSize - 2 || pz + sz > (int)houseFieldFloor)
		return false;

	//	Check available
	for (int k = pz; k < pz + sz; k++)
		for (int i = px; i < px + sx; i++)
			for (int j = py; j < py + sy; j++)
				if(!houseField[k][i][j].available) return false;
	return true;
}
int HouseGenerator::adjacentBlock(int px, int py, int pz, int sx, int sy)
{
	int blockCount = 0;
	for (int i = px; i < px + sx; i++)
		if (!houseField[pz][i][py - 1].available) blockCount++;
	for (int i = px; i < px + sx; i++)
		if (!houseField[pz][i][py + sy].available) blockCount++;
	for (int i = py; i < py + sy; i++)
		if (!houseField[pz][px - 1][i].available) blockCount++;
	for (int i = py; i < py + sy; i++)
		if (!houseField[pz][px + sx][i].available) blockCount++;
	return blockCount;
}
void HouseGenerator::addBlocks(int px, int py, int pz, int sx, int sy, int sz, unsigned int blockType)
{
	for (int k = pz; k < pz + sz; k++)
		for (int i = px; i < px + sx; i++)
			for (int j = py; j < py + sy; j++)
			{
				houseField[k][i][j].available = false;
				houseField[k][i][j].voxelType = blockType;
			}
}


inline void HouseGenerator::createBlocs(int superficy)
{
	for (unsigned int i = 0; i < blockLibrary.size(); i++)
		if (soustractBlock(superficy, i)) break;
}
bool HouseGenerator::soustractBlock(int superficy, int testIndex)
{
	if (blockLibrary[testIndex].z >= (int)houseFieldFloor) return false;

	int s = superficy - blockLibrary[testIndex].x * blockLibrary[testIndex].y * blockLibrary[testIndex].z;
	if (s < 0) return false;
	else if (s < 3)
	{
		blockList.push_back(blockLibrary[testIndex]);
		return true;
	}
	else
	{
		blockList.push_back(blockLibrary[testIndex]);
		for (unsigned int i = testIndex; i < blockLibrary.size(); i++)
			if (soustractBlock(s, i)) return true;
		return false;
	}
}


inline void HouseGenerator::constructMesh()
{	
	float ox = houseFieldSize / 2.f + 0.5f;
	float oy = houseFieldSize / 2.f + 0.5f;
	float oz = 0.1f;
	Mesh* meshToPush = nullptr;

	for (unsigned int k = 0; k < houseFieldFloor; k++)
	{
		//	house
		for (unsigned int i = 0; i < houseFieldSize; i++)
			for (unsigned int j = 0; j < houseFieldSize; j++)
			{
				if (houseField[k][i][j].available) continue;

				//	Ground quad
				pushGround(i - ox, j - oy, 2.5f * k + oz, i + 1 - ox, j + 1 - oy, 2.5f * k + oz, stoneColor);

				//	right wall
				if (i + 1 < houseFieldSize && houseField[k][i+1][j].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 1 - ox, j + 0.5f - oy, oz + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				}

				//	left wall
				if (i - 1 >= 0 && houseField[k][i - 1][j].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i - ox, j + 0.5f - oy, oz + 2.5f*k), glm::vec3(0.f, 1.f, 0.f));
				}
					
				//	lower wall
				if (j + 1 < houseFieldSize && houseField[k][i][j + 1].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - ox, j + 1 - oy, oz + 2.5f*k), glm::vec3(1.f, 0.f, 0.f));
				}

				//	upper wall
				if (j - 1 >= 0 && houseField[k][i][j - 1].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - ox, j - oy, oz + 2.5f*k), glm::vec3(-1.f, 0.f, 0.f));
				}

				//	corners
				if (i + 1 < houseFieldSize && j + 1 < houseFieldSize && houseField[k][i + 1][j].available && houseField[k][i][j + 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i + 1 - ox, j + 1 - oy, oz + 2.5f * k), glm::vec3(1.f, 0.f, 0.f));
				if (i - 1 >= 0 && j + 1 < houseFieldSize && houseField[k][i - 1][j].available && houseField[k][i][j + 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i - ox, j + 1 - oy, oz + 2.5f * k), glm::vec3(0.f, 1.f, 0.f));
				if (i + 1 < houseFieldSize && j - 1 >= 0 && houseField[k][i + 1][j].available && houseField[k][i][j - 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i + 1 - ox, j - oy, oz + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				if (i - 1 >= 0 && j - 1 >= 0 && houseField[k][i - 1][j].available && houseField[k][i][j - 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i - ox, j - oy, oz + 2.5f * k), glm::vec3(-1.f, 0.f, 0.f));
			}
	}
}
void HouseGenerator::pushMesh(Mesh* m, glm::vec3 p, glm::vec3 o, glm::vec3 s)
{
	if (!m) return;
	unsigned int facesStart = verticesArray.size();

	//	Compute orientation matrix
	glm::mat4 orientation(1.f);
	if (glm::dot(o, glm::vec3(0.f, 1.f, 0.f)) == -1.f)
		orientation = glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(0.f, 0.f, 1.f));
	else
		orientation = glm::orientation(o, glm::vec3(0.f, 1.f, 0.f));

	//	Compute mesh model matrix
	glm::mat4 model = glm::translate(glm::mat4(1.f), p);
	model = model * orientation;
	model = glm::scale(model, s);

	//	push modified data
	for (unsigned int i = 0; i < m->vertices.size(); i++)
	{
		glm::vec4 v = model * glm::vec4(m->vertices[i], 1.f);
		verticesArray.push_back(glm::vec3(v.x, v.y, v.z));
	}
	for (unsigned int i = 0; i < m->normales.size(); i++)
	{
		glm::vec4 v = orientation * glm::vec4(m->normales[i], 1.f);
		normalesArray.push_back(glm::vec3(v.x, v.y, v.z));
	}
	for (unsigned int i = 0; i < m->color.size(); i++)
		colorArray.push_back(m->color[i]);

	for (unsigned int i = 0; i < m->faces.size(); i++)
		facesArray.push_back(facesStart + m->faces[i]);
}
void HouseGenerator::pushGround(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color)
{
	verticesArray.push_back(glm::vec3(px1, py1, pz1));
	verticesArray.push_back(glm::vec3(px2, py1, (pz1 + pz2) / 2.f));
	verticesArray.push_back(glm::vec3(px2, py2, pz2));
	verticesArray.push_back(glm::vec3(px1, py2, (pz1 + pz2) / 2.f));

	glm::vec3 n = glm::cross(glm::normalize(glm::vec3(px2 - px1, py2 - py1, pz2 - pz1)),
							 glm::normalize(glm::vec3(0.f, py2 - py1, (pz1 + pz2) / 2.f)));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
}


inline void HouseGenerator::optimizeMesh()
{
	std::vector<glm::vec3> verticesBuffer;
	std::vector<glm::vec3> normalesBuffer;
	std::vector<glm::vec3> colorBuffer;
	std::vector<unsigned int> facesBuffer;

	std::map<OrderedVertex, unsigned int> vertexAlias;
	std::map<OrderedVertex, unsigned int>::iterator alias;
	OrderedVertex current;

	for (unsigned int i = 0; i < facesArray.size(); i++)
	{
		current.position = verticesArray[facesArray[i]];
		current.normal = normalesArray[facesArray[i]];
		current.color = colorArray[facesArray[i]];

		alias  = vertexAlias.find(current);
		if (alias == vertexAlias.end())
		{
			verticesBuffer.push_back(verticesArray[facesArray[i]]);
			normalesBuffer.push_back(normalesArray[facesArray[i]]);
			colorBuffer.push_back(colorArray[facesArray[i]]);
			facesBuffer.push_back(vertexAlias.size());

			vertexAlias[current] = vertexAlias.size();
		}
		else facesBuffer.push_back(alias->second);
	}

	verticesArray.swap(verticesBuffer);
	normalesArray.swap(normalesBuffer);
	colorArray.swap(colorBuffer);
	facesArray.swap(facesBuffer);
}
//
