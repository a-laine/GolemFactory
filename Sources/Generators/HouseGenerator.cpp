#include "HouseGenerator.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>

//  Attributes
glm::vec3 HouseGenerator::stoneColor = glm::vec3(0.2f, 0.2f, 0.2f);
const int HouseGenerator::massiveRadius = 3;
const int HouseGenerator::adjacentOptiRadius = 7;
//

//  Default
HouseGenerator::HouseGenerator()
{
	houseField = nullptr;
	houseFieldSize = 0;
	houseFieldFloor = 0;

	assetLibrary["debug"] = ResourceManager::getInstance()->getMesh("cube2.obj");

	assetLibrary["wall"] = ResourceManager::getInstance()->getMesh("House/HouseWall.obj");
	assetLibrary["corner"] = ResourceManager::getInstance()->getMesh("House/HouseCorner.obj");
	assetLibrary["door"] = ResourceManager::getInstance()->getMesh("House/HouseDoor.obj");
	assetLibrary["window"] = ResourceManager::getInstance()->getMesh("House/HouseWindow.obj");

	assetLibrary["roof1"] = ResourceManager::getInstance()->getMesh("House/HouseRoof1.obj");
	assetLibrary["roof2"] = ResourceManager::getInstance()->getMesh("House/HouseRoof2.obj");
	assetLibrary["roofend1"] = ResourceManager::getInstance()->getMesh("House/HouseRoofEnd1.obj");
	assetLibrary["roofend2"] = ResourceManager::getInstance()->getMesh("House/HouseRoofEnd2.obj");
	assetLibrary["roofend3"] = ResourceManager::getInstance()->getMesh("House/HouseRoofEnd3.obj");
	assetLibrary["roofend4"] = ResourceManager::getInstance()->getMesh("House/HouseRoofEnd4.obj");

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
	randomEngine.seed(seed);
	if (density < 0)
		density = (randomEngine() % 100);
	if (prosperity <= 0)
		prosperity = std::max(density / 3 + (int)(randomEngine() % 100), 1);
	initHouseField(prosperity, density, prosperity);
	std::string houseName = "house" + std::to_string(seed) + 'd' + std::to_string(density) + 'p' + std::to_string(prosperity);
	int superficy =  std::max((prosperity / 5) * (prosperity / 5) + (int)(randomEngine() % prosperity), 15);

	//	Debug
	std::cout << houseName << std::endl;
	std::cout << "   seed : " << seed << std::endl;
	std::cout << "   density : " << density << std::endl;
	std::cout << "   prosperity : " << prosperity << std::endl;
	std::cout << "   house field size : " << houseFieldSize << std::endl;
	std::cout << "   house field floor count : " << houseFieldFloor << std::endl;
	std::cout << "   superficy : " << superficy << std::endl;
	//

	//	generate block list used for constructions
	for (unsigned int i = 0; i < blockLibrary.size(); i++)
		if (searchBlockPartition(superficy, i)) break;
	availableBlockPosition.clear();

	std::cout << "   blocks : " << std::endl;
	for (unsigned int i = 0; i < blockList.size(); i++)
		std::cout << "       " << blockList[i].second.x << " " << blockList[i].second.y << " " << blockList[i].second.z << std::endl;

	//	place blocks
	const int safeOffset = std::max(blockLibrary[0].x, blockLibrary[0].y) + massiveRadius + adjacentOptiRadius;
	for (unsigned int i = 0; i < blockList.size(); i++)
	{
		//	construct the available position list & shuffle it
		availableBlockPosition.clear();
		for (int k = 0; k < (int)houseFieldFloor; k++)
			for (int l = safeOffset; l < (int)houseFieldSize - safeOffset; l++)
				for (int j = safeOffset; j < (int)houseFieldSize - safeOffset; j++)
					if (houseField[0][l][j].available) availableBlockPosition.push_back(glm::ivec3(l, j, k));
		std::shuffle(availableBlockPosition.begin(), availableBlockPosition.end(), randomEngine);

		//	try to place block
		for (auto it = availableBlockPosition.begin(); it != availableBlockPosition.end(); it++)
		{
			if (!freePlace(*it, blockList[i].second)) continue;
			if (!supportedBlock(*it, blockList[i].second)) continue;
			if (massiveStruct(*it, blockList[i].second)) continue;
			if (i !=0 && adjacentBlock(*it, blockList[i].second) < 3) continue;
			else
			{
				glm::ivec3 finalPos = optimizeAdjacent(*it, blockList[i].second);
				addHouseBlocks(finalPos, blockList[i].second, 1, i);
				blockList[i].first = finalPos;
				break;
			}
		}
	}

	//	compute house origin
	glm::ivec3 vmax(0, 0, 0);
	glm::ivec3 vmin(houseFieldSize, houseFieldSize, houseFieldFloor);
	for (int k = 0; k < (int)houseFieldFloor; k++)
		for (int i = 0; i < (int)houseFieldSize; i++)
			for (int j = 0; j < (int)houseFieldSize; j++)
			{
				if (houseField[k][i][j].available) continue;
				vmax.x = std::max(vmax.x, i);
				vmax.y = std::max(vmax.y, j);
				vmax.z = std::max(vmax.z, k);

				vmin.x = std::min(vmin.x, i);
				vmin.y = std::min(vmin.y, j);
				vmin.z = std::min(vmin.z, k);
			}
	houseOrigin = 0.5f * (glm::vec3)(vmax + vmin);
	houseOrigin.z = 0.1f;


	//	generate house
	constructHouseMesh();

	//createAndPlaceRoof();
	//constructRoofMesh();

	optimizeMesh();

	std::cout << "   vertex count : " << verticesArray.size() << std::endl;
	std::cout << "   faces count : " << facesArray.size() << std::endl;

	Mesh* mesh = new Mesh(houseName, verticesArray, normalesArray, colorArray, facesArray);
	ResourceManager::getInstance()->addMesh(mesh);
	InstanceDrawable* house = new InstanceDrawable(houseName);
	ResourceManager::getInstance()->release(mesh);
	return house;
}
//

//  Protected functions
void HouseGenerator::initHouseField(const int& newSize, const int& density, const int& prosperity)
{
	//	clear mesh attributes lists
	verticesArray.clear();
	normalesArray.clear();
	colorArray.clear();
	facesArray.clear();
	blockList.clear();

	//	delete last house field
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
		const int safeOffset = std::max(blockLibrary[0].x, blockLibrary[0].y) + massiveRadius + adjacentOptiRadius;
		houseFieldSize = std::max(newSize, 2 * safeOffset + 10);
		if (prosperity > 100) houseFieldFloor = 5;
		else houseFieldFloor = 5 - std::min(3, (int)(0.04f * std::sqrtf(0.5f * (density - 100.f)*(density - 100.f) + (prosperity - 100.f)*(prosperity - 100.f))));

		houseField = new HouseVoxel**[houseFieldFloor];
		for (unsigned int j = 0; j < houseFieldFloor; j++)
		{
			houseField[j] = new HouseVoxel*[houseFieldSize];
			for (unsigned int i = 0; i < houseFieldSize; i++)
				houseField[j][i] = new HouseVoxel[houseFieldSize];
		}
	}

	//	Initialize new house field
	for (unsigned int k = 0; k < houseFieldFloor; k++)
		for (unsigned int i = 0; i < houseFieldSize; i++)
			for (unsigned int j = 0; j < houseFieldSize; j++)
			{
				houseField[k][i][j].available = true;
				houseField[k][i][j].houseType = None;
				houseField[k][i][j].roofType = None;
				houseField[k][i][j].blockReference = 0;
			}
}

bool HouseGenerator::searchBlockPartition(const int& superficy, const int& testIndex)
{
	if (blockLibrary[testIndex].z >= (int)houseFieldFloor) return false;

	int s = superficy - blockLibrary[testIndex].x * blockLibrary[testIndex].y * blockLibrary[testIndex].z;
	if (s < 0) return false;
	else if (s < 3)
	{
		if (randomEngine() % 2)
			blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(0, 0, 0), blockLibrary[testIndex]));
		else
			blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(0, 0, 0), glm::ivec3(blockLibrary[testIndex].y, blockLibrary[testIndex].x, blockLibrary[testIndex].z)));
		return true;
	}
	else
	{
		if (randomEngine() % 2)
			blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(0, 0, 0), blockLibrary[testIndex]));
		else
			blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(0, 0, 0), glm::ivec3(blockLibrary[testIndex].y, blockLibrary[testIndex].x, blockLibrary[testIndex].z)));
		for (unsigned int i = testIndex; i < blockLibrary.size(); i++)
			if (searchBlockPartition(s, i)) return true;
		return false;
	}
}
bool HouseGenerator::freePlace(const glm::ivec3& p, const glm::ivec3& s) const
{
	//	Check out of range
	if (p.x < 0 || p.y < 0 || p.z < 0)
		return false;
	else if (p.x + s.x >= (int)houseFieldSize || p.y + s.y >= (int)houseFieldSize || p.z + s.z >= (int)houseFieldFloor)
		return false;

	//	Check available
	for (int k = p.z; k < p.z + s.z; k++)
		for (int i = p.x; i < p.x + s.x; i++)
			for (int j = p.y; j < p.y + s.y; j++)
				if (!houseField[k][i][j].available) return false;
	return true;
}
bool HouseGenerator::supportedBlock(const glm::ivec3& p, const glm::ivec3& s) const
{
	if (p.z == 0) return true;
	int support = 0;
	for (int i = p.x; i < p.x + s.x; i++)
		for (int j = p.y; j < p.y + s.y; j++)
			if (!houseField[p.z - 1][i][j].available) support++;
	return support > 0.8f * s.x * s.y;
}
bool HouseGenerator::massiveStruct(const glm::ivec3& p, const glm::ivec3& s) const
{
	bool reachOut;
	for (int i = p.x - massiveRadius; i < p.x + s.x + massiveRadius; i++)
		for (int j = p.y - massiveRadius; j < p.y + s.y + massiveRadius; j++)
		{
			reachOut = false;
			for (int l = massiveRadius; l <= massiveRadius; l++)
				for (int m = massiveRadius; m <= massiveRadius; m++)
					if (houseField[p.z][i+l][j+m].available)
					{
						reachOut = true;
						break;
					}
			if (!reachOut) return true;
		}
	return false;
}
int HouseGenerator::adjacentBlock(const glm::ivec3& p, const glm::ivec3& s) const
{
	int blockCount = 0;
	for (int i = p.x; i < p.x + s.x; i++)
		if (!houseField[p.z][i][p.y - 1].available) blockCount++;
	for (int i = p.x; i < p.x + s.x; i++)
		if (!houseField[p.z][i][p.y + s.y].available) blockCount++;
	for (int i = p.y; i < p.y + s.y; i++)
		if (!houseField[p.z][p.x - 1][i].available) blockCount++;
	for (int i = p.y; i < p.y + s.y; i++)
		if (!houseField[p.z][p.x + s.x][i].available) blockCount++;
	return blockCount;
}
glm::ivec3 HouseGenerator::optimizeAdjacent(const glm::ivec3& p, const glm::ivec3& s) const
{
	int offset = (adjacentOptiRadius - 1) / 2;
	int rate[adjacentOptiRadius][adjacentOptiRadius];

	for (int i = 0; i < adjacentOptiRadius; i++)
		for (int j = 0; j < adjacentOptiRadius; j++)
		{
			if (!freePlace(p + glm::ivec3(i - offset, j - offset, 0), s)) rate[i][j] = 0;
			else if (!supportedBlock(p + glm::ivec3(i - offset, j - offset, 0), s)) rate[i][j] = 0;
			//else if (massiveStruct(p + glm::ivec3(i - offset, j - offset, 0), s)) rate[i][j] = 0;
			else rate[i][j] = adjacentBlock(p + glm::ivec3(i - offset, j - offset, 0), s);
		}

	int maxi = 0; int maxj = 0;
	for (int i = 0; i < adjacentOptiRadius; i++)
		for (int j = 0; j < adjacentOptiRadius; j++)
		{
			if (rate[i][j] > rate[maxi][maxj])
			{
				maxi = i;
				maxj = j;
			}
		}

	return p + glm::ivec3(maxi - offset, maxj - offset, 0);
}
void HouseGenerator::addHouseBlocks(const glm::ivec3& p, const glm::ivec3& s, const unsigned int& houseType, const unsigned int& blockReference)
{
	for (int k = p.z; k < p.z + s.z; k++)
		for (int i = p.x; i < p.x + s.x; i++)
			for (int j = p.y; j < p.y + s.y; j++)
			{
				houseField[k][i][j].available = false;
				houseField[k][i][j].houseType = houseType;
				houseField[k][i][j].blockReference = blockReference;
			}
}

void HouseGenerator::constructHouseMesh()
{
	Mesh* meshToPush = nullptr;

	for (unsigned int k = 0; k < houseFieldFloor; k++)
		for (unsigned int i = 0; i < houseFieldSize; i++)
			for (unsigned int j = 0; j < houseFieldSize; j++)
			{
				if (houseField[k][i][j].available || houseField[k][i][j].houseType == HouseType::None) continue;

				//	Ground quad or debug roof
				pushGround(i - houseOrigin.x, j - houseOrigin.y, 2.5f * k + houseOrigin.z, i + 1 - houseOrigin.x, j + 1 - houseOrigin.y, 2.5f * k + houseOrigin.z, stoneColor);

				//	right wall
				if (i + 1 < houseFieldSize && houseField[k][i+1][j].available)
				{
					switch (houseField[k][i][j].houseType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 1 - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				}

				//	left wall
				if (i - 1 >= 0 && houseField[k][i - 1][j].available)
				{
					switch (houseField[k][i][j].houseType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(0.f, 1.f, 0.f));
				}
					
				//	lower wall
				if (j + 1 < houseFieldSize && houseField[k][i][j + 1].available)
				{
					switch (houseField[k][i][j].houseType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(1.f, 0.f, 0.f));
				}

				//	upper wall
				if (j - 1 >= 0 && houseField[k][i][j - 1].available)
				{
					switch (houseField[k][i][j].houseType)
					{
						case HouseEmpty: meshToPush = assetLibrary["wall"]; break;
						case Door: meshToPush = assetLibrary["door"]; break;
						case Window: meshToPush = assetLibrary["window"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(-1.f, 0.f, 0.f));
				}

				//	corners
				if (i + 1 < houseFieldSize && j + 1 < houseFieldSize && houseField[k][i + 1][j].available && houseField[k][i][j + 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i + 1 - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(1.f, 0.f, 0.f));
				if (i - 1 >= 0 && j + 1 < houseFieldSize && houseField[k][i - 1][j].available && houseField[k][i][j + 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, 1.f, 0.f));
				if (i + 1 < houseFieldSize && j - 1 >= 0 && houseField[k][i + 1][j].available && houseField[k][i][j - 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i + 1 - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				if (i - 1 >= 0 && j - 1 >= 0 && houseField[k][i - 1][j].available && houseField[k][i][j - 1].available)
					pushMesh(assetLibrary["corner"], glm::vec3(i - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(-1.f, 0.f, 0.f));
			}
}
void HouseGenerator::pushMesh(Mesh* m, const glm::vec3& p, const glm::vec3& o, const glm::vec3& s)
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
	model = glm::scale(model, s);
	model = model * orientation;

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





inline void HouseGenerator::createAndPlaceRoof()
{
	/*glm::ivec3 origin = findRoofSeed();
	glm::ivec3 size;
	while (origin.x >= 0)
	{
		size = glm::ivec3(1, 1, 1);
		//growRoofSeed(origin, size);
		//addBlocks(origin, size, Roof);
		roofBlockList.push_back(std::pair<glm::ivec3, glm::ivec3>(origin, size));
		origin = findRoofSeed();
	}
	*/
}
float HouseGenerator::benchmarkRoof(glm::ivec3 p, glm::ivec3 s)
{
	float coveredBlockMark = 1.f;
	float uncoveredBlockMark = 2.f;
	float result = 0.f;

	for (int i = p.x; i < p.x + s.x; i++)
		for (int j = p.y; j < p.y + s.y; j++)
		{
			//if (!houseField[p.z][i][j].available || houseField[p.z - 1][i][j].voxelType == Roof) return 0.f;
			if (houseField[p.z - 1][i][j].available) result -= p.z * uncoveredBlockMark;
			else result += coveredBlockMark;
		}
	return result;
}


inline void HouseGenerator::constructRoofMesh()
{
	float ox = houseFieldSize / 2.f + 0.5f;
	float oy = houseFieldSize / 2.f + 0.5f;
	float oz = 0.1f;

	for (unsigned int k = 0; k < houseFieldFloor; k++)
		for (unsigned int i = 0; i < houseFieldSize; i++)
			for (unsigned int j = 0; j < houseFieldSize; j++)
			{
				if (houseField[k][i][j].available) continue;

				//	Ground roof
				//if (houseField[k][i][j].voxelType == Roof)
				//	pushGround(i - ox, j - oy, 2.5f * k + oz, i + 1 - ox, j + 1 - oy, 2.5f * k + oz, groundRoof);
			}

	for (unsigned int i = 0; i < roofBlockList.size(); i++)
	{
		roofSlope(roofBlockList[i].first, roofBlockList[i].second);
		roofEnd(roofBlockList[i].first, roofBlockList[i].second);
	}
}
void HouseGenerator::roofSlope(const glm::ivec3& p, const glm::ivec3& s)
{
	glm::vec3 houseOrigin(houseFieldSize / 2.f, houseFieldSize / 2.f, 0.1f);
	glm::vec3 tmp;
	if (s.y > s.x)
	{
		for (int i = 0; i < s.x / 2; i++)
			for (int j = p.y; j < p.y + s.y; j++)
			{
				tmp = glm::vec3(p.x + i - houseOrigin.x - 0.5f, j - houseOrigin.y, i + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roof1"], tmp, glm::vec3(0.f, 1.f, 0.f));

				tmp = glm::vec3(p.x + s.x - i - houseOrigin.x - 0.5f, j - houseOrigin.y, i + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roof1"], tmp, glm::vec3(0.f, -1.f, 0.f));
			}
		if (s.x % 2)
		{
			for (int j = p.y; j < p.y + s.y; j++)
			{
				tmp = glm::vec3(p.x + s.x / 2 + 1 - houseOrigin.x - 0.5f, j - houseOrigin.y, s.x / 2 + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roof2"], tmp, glm::vec3(0.f, -1.f, 0.f));
			}
		}
	}
	else
	{
		for (int i = p.x; i < p.x + s.x; i++)
			for (int j = 0; j < s.y / 2; j++)
			{
				tmp = glm::vec3(i - houseOrigin.x, p.y + j - houseOrigin.y - 0.5f, j + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roof1"], tmp, glm::vec3(-1.f, 0.f, 0.f));

				tmp = glm::vec3(i - houseOrigin.x, p.y + s.y - j - houseOrigin.y - 0.5f, j + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roof1"], tmp, glm::vec3(1.f, 0.f, 0.f));
			}
		if (s.y % 2)
		{
			for (int i = p.x; i < p.x + s.x; i++)
			{
				tmp = glm::vec3(i - houseOrigin.x, p.y + s.y / 2 + 1 - houseOrigin.y - 0.5f, s.y / 2 + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roof2"], tmp, glm::vec3(1.f, 0.f, 0.f));
			}
		}
	}
}
void HouseGenerator::roofEnd(const glm::ivec3& p, const glm::ivec3& s)
{
	glm::vec3 houseOrigin(houseFieldSize / 2.f, houseFieldSize / 2.f, 0.1f);
	glm::vec3 tmp;

	if (s.y > s.x)
	{
		//	front end
		for (int i = 0; i < s.x / 2; i++)
		{
			if (houseField[p.z][p.x + i][p.y - 1].available)
			{
				tmp = glm::vec3(p.x + i - houseOrigin.x - 0.5f, p.y - 0.5f - houseOrigin.y, i + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend1"], tmp, glm::vec3(0.f, 1.f, 0.f));
			}
			if (houseField[p.z][p.x + s.x - 1 - i][p.y - 1].available)
			{
				tmp = glm::vec3(p.x + s.x - i - houseOrigin.x - 0.5f, p.y - 0.5f - houseOrigin.y, i + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend4"], tmp, glm::vec3(0.f, 1.f, 0.f));
			}
		}
		for (int i = 0; i < s.x / 2; i++)
		{
			if (houseField[p.z][p.x + i][p.y + s.y].available)
			{
				tmp = glm::vec3(p.x + i - houseOrigin.x - 0.5f, p.y + s.y - 0.5f - houseOrigin.y, i + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend4"], tmp, glm::vec3(0.f, -1.f, 0.f));
			}
			if (houseField[p.z][p.x + s.x - 1 - i][p.y + s.y].available)
			{
				tmp = glm::vec3(p.x + s.x - i - houseOrigin.x - 0.5f, p.y + s.y - 0.5f - houseOrigin.y, i + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend1"], tmp, glm::vec3(0.f, -1.f, 0.f));
			}
		}

		//	slope end border
		for (int j = p.y; j < p.y + s.y; j++)
		{
			if (!houseField[p.z][p.x - 1][j].available) continue;
			tmp = glm::vec3(p.x - 0.5f - houseOrigin.x, j - houseOrigin.y, 2.5f * p.z + houseOrigin.z);
			pushMesh(assetLibrary["roofend3"], tmp, glm::vec3(0.f, 1.f, 0.f));
		}
		for (int j = p.y; j < p.y + s.y; j++)
		{
			if (!houseField[p.z][p.x + s.x][j].available) continue;
			tmp = glm::vec3(p.x + s.x - 0.5f - houseOrigin.x, j - houseOrigin.y, 2.5f * p.z + houseOrigin.z);
			pushMesh(assetLibrary["roofend3"], tmp, glm::vec3(0.f, -1.f, 0.f));
		}
	}
	else
	{
		//	front end
		for (int j = 0; j < s.y / 2; j++)
		{
			if (houseField[p.z][p.x - 1][p.y + j].available)
			{
				tmp = glm::vec3(p.x - 0.5f - houseOrigin.x, p.y + j - houseOrigin.y - 0.5f, j + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend4"], tmp, glm::vec3(1.f, 0.f, 0.f));
			}
			if (houseField[p.z][p.x - 1][p.y + s.y - 1 - j].available)
			{
				tmp = glm::vec3(p.x - 0.5f - houseOrigin.x, p.y + s.y - j - houseOrigin.y - 0.5f, j + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend1"], tmp, glm::vec3(1.f, 0.f, 0.f));
			}
		}
		for (int j = 0; j < s.y / 2; j++)
		{
			if (houseField[p.z][p.x + s.x][p.y + j].available)
			{
				tmp = glm::vec3(p.x + s.x - 0.5f - houseOrigin.x, p.y + j - houseOrigin.y - 0.5f, j + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend1"], tmp, glm::vec3(-1.f, 0.f, 0.f));
			}
			if (houseField[p.z][p.x + s.x][p.y + s.y - 1 - j].available)
			{
				tmp = glm::vec3(p.x + s.x - 0.5f - houseOrigin.x, p.y + s.y - j - houseOrigin.y - 0.5f, j + 2.5f * p.z + houseOrigin.z);
				pushMesh(assetLibrary["roofend4"], tmp, glm::vec3(-1.f, 0.f, 0.f));
			}
		}

		//	slope end border
		for (int i = p.x; i < p.x + s.x; i++)
		{
			if (!houseField[p.z][i][p.y - 1].available) continue;
			tmp = glm::vec3(i - houseOrigin.x, p.y - 0.5f - houseOrigin.y, 2.5f * p.z + houseOrigin.z);
			pushMesh(assetLibrary["roofend3"], tmp, glm::vec3(-1.f, 0.f, 0.f));
		}
		for (int i = p.x; i < p.x + s.x; i++)
		{
			if (!houseField[p.z][i][p.y + s.y].available) continue;
			tmp = glm::vec3(i - houseOrigin.x, p.y + s.y - 0.5f - houseOrigin.y, 2.5f * p.z + houseOrigin.z);
			pushMesh(assetLibrary["roofend3"], tmp, glm::vec3(1.f, 0.f, 0.f));
		}
	}
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
