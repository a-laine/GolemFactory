#include "HouseGenerator.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>

//  Attributes
glm::vec3 HouseGenerator::stoneColor = glm::vec3(0.2f, 0.2f, 0.2f);
const int HouseGenerator::massiveRadius = 4;
//

//  Default
HouseGenerator::HouseGenerator()
{
	houseField = nullptr;
	houseFieldSize = 0;
	houseFieldFloor = 0;
	density = 0;
	prosperity = 0;
	superficy = 0;

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
		for (int j = 0; j < houseFieldFloor; j++)
		{
			for (int i = 0; i < houseFieldSize; i++)
				delete[] houseField[j][i];
			delete[] houseField[j];
		}
		delete[] houseField;
	}
}
//

//  Public functions
InstanceVirtual* HouseGenerator::getHouse(unsigned int seed, int d, int p)
{
	// initialize
	randomEngine.seed(seed);

	if (d < 0)  density = (randomEngine() % 100);
	else density = d;
	if (p <= 0) prosperity = std::max(density / 3 + (int)(randomEngine() % 100), 1);
	else prosperity = p;

	std::string houseName = "house" + std::to_string(seed) + '_' + std::to_string(density) + '_' + std::to_string(prosperity);
	std::cout << houseName << std::endl;

	initHouseField(100);

	

	//	generate house
	createAndPlaceHouseBlock();
	constructHouseMesh();

	createAndPlaceRoofBlock();
	constructRoofMesh();
	
	optimizeMesh();

	std::cout << "   vertex count : " << verticesArray.size() << std::endl;
	std::cout << "   faces count : " << facesArray.size() << std::endl << std::endl;

	Mesh* mesh = new Mesh(houseName, verticesArray, normalesArray, colorArray, facesArray);
	ResourceManager::getInstance()->addMesh(mesh);
	InstanceDrawable* house = new InstanceDrawable(houseName);
	ResourceManager::getInstance()->release(mesh);
	return house;
}
//

//  Protected functions
void HouseGenerator::initHouseField(const int& newSize)
{
	//	clear mesh attributes lists
	verticesArray.clear();
	normalesArray.clear();
	colorArray.clear();
	facesArray.clear();

	//	clear house construction lists
	blockList.clear();
	roofBlockList.clear();
	availableBlockPosition.clear();

	//	delete last house field
	if (newSize != houseFieldSize && houseField)
	{
		for (int j = 0; j < houseFieldFloor; j++)
		{
			for (int i = 0; i < houseFieldSize; i++)
				delete[] houseField[j][i];
			delete[] houseField[j];
		}
		delete[] houseField;
		houseField = nullptr;
	}

	//	Create new house field
	if (newSize != houseFieldSize)
	{
		const int safeOffset = 2 * std::max(blockLibrary[0].x, blockLibrary[0].y) + 2 * massiveRadius + 5;
		houseFieldSize = std::max(newSize, 2 * safeOffset);
		if (prosperity > 100) houseFieldFloor = 6;
		else houseFieldFloor = 5 - std::min(3, (int)(0.04f * std::sqrtf(0.5f * (density - 100.f)*(density - 100.f) + (prosperity - 100.f)*(prosperity - 100.f))));

		houseField = new HouseVoxel**[houseFieldFloor];
		for (int j = 0; j < houseFieldFloor; j++)
		{
			houseField[j] = new HouseVoxel*[houseFieldSize];
			for (int i = 0; i < houseFieldSize; i++)
				houseField[j][i] = new HouseVoxel[houseFieldSize];
		}
	}

	//	Initialize new house field
	for (int k = 0; k < houseFieldFloor; k++)
		for (int i = 0; i < houseFieldSize; i++)
			for (int j = 0; j < houseFieldSize; j++)
			{
				houseField[k][i][j].house = HouseTypeBlock::None;
				houseField[k][i][j].blockReference = -1;

				houseField[k][i][j].roof = -1;
				houseField[k][i][j].roofReference = -1;
			}

	//	Compute superficy
	superficy = std::max((prosperity / 5) * (prosperity / 5) + (int)(randomEngine() % prosperity), 15);
	std::cout << "   density : " << density << std::endl;
	std::cout << "   prosperity : " << prosperity << std::endl;
	std::cout << "   house field size : " << houseFieldSize << std::endl;
	std::cout << "   house field floor count : " << houseFieldFloor << std::endl;
	std::cout << "   superficy : " << superficy << std::endl;
}
void HouseGenerator::createAndPlaceHouseBlock()
{
	//	generate block list used for constructions
	for (unsigned int i = 0; i < blockLibrary.size(); i++)
		if (searchBlockPartition(superficy, i)) break;

	//	place first block
	if (!blockList.empty())
	{
		blockList[0].first = getRandomPosition(2 * std::max(blockLibrary[0].x, blockLibrary[0].y) + 2 * massiveRadius);
		addHouseBlocks(blockList[0].first, blockList[0].second, HouseTypeBlock::House, 0);
		updateAvailableBlockPosition(blockList[0].first, blockList[0].second);
	}

	//	place blocks
	glm::ivec3 vmax(0, 0, 0);
	glm::ivec3 vmin(houseFieldSize, houseFieldSize, houseFieldFloor);
	for (unsigned int i = 1; i < blockList.size(); i++)
	{
		benchmarkPosition.clear();
		for (auto it = availableBlockPosition.begin(); it != availableBlockPosition.end(); it++)
		{
			markAll(*it, blockList[i].second);
			markAll(*it,  glm::ivec3(blockList[i].second.y, blockList[i].second.x, blockList[i].second.z));
			markAll(*it - glm::ivec3(blockList[i].second.x - 1, 0, 0), blockList[i].second);
			markAll(*it - glm::ivec3(blockList[i].second.y - 1, 0, 0), glm::ivec3(blockList[i].second.y, blockList[i].second.x, blockList[i].second.z));
			markAll(*it - glm::ivec3(0, blockList[i].second.y - 1, 0), blockList[i].second);
			markAll(*it - glm::ivec3(0, blockList[i].second.x - 1, 0), glm::ivec3(blockList[i].second.y, blockList[i].second.x, blockList[i].second.z));
			markAll(*it - glm::ivec3(blockList[i].second.x - 1, blockList[i].second.y - 1, 0), blockList[i].second);
			markAll(*it - glm::ivec3(blockList[i].second.y - 1, blockList[i].second.x - 1, 0), glm::ivec3(blockList[i].second.y, blockList[i].second.x, blockList[i].second.z));
		}

		if (!benchmarkPosition.empty())
		{
			benchmarkPosition.sort();
			blockList[i].first = benchmarkPosition.back().position;
			blockList[i].second = benchmarkPosition.back().size;

			vmax.x = std::max(vmax.x, blockList[i].first.x + blockList[i].second.x);
			vmax.y = std::max(vmax.y, blockList[i].first.y + blockList[i].second.y);
			vmax.z = std::max(vmax.z, blockList[i].first.z + blockList[i].second.z);

			vmin.x = std::min(vmin.x, blockList[i].first.x + blockList[i].second.x);
			vmin.y = std::min(vmin.y, blockList[i].first.y + blockList[i].second.y);
			vmin.z = std::min(vmin.z, blockList[i].first.z + blockList[i].second.z);

			addHouseBlocks(blockList[i].first, blockList[i].second, HouseTypeBlock::House, i);
			updateAvailableBlockPosition(blockList[i].first, blockList[i].second);
		}
		else std::cout << "!!!!!!!!!!!!!!!!fail to place block!!!!!!!!!!!!!!!!" << std::endl;
	}

	houseOrigin = 0.5f * (glm::vec3)(vmax + vmin);
	houseOrigin.x = std::floor(houseOrigin.x);
	houseOrigin.y = std::floor(houseOrigin.y);
	houseOrigin.z = 0.1f;

	//	prevent massive : add pillars
	for (int k = vmin.z; k <= vmax.z; k++)
		for (int i = vmin.x; i <= vmax.x; i++)
			for (int j = vmin.y; j <= vmax.y; j++)
			{
				if (freePlace(i, j, k)) continue;
				bool reachOut = false;
				for (int l = -massiveRadius; l <= massiveRadius; l++)
				{
					for (int m = -massiveRadius; m <= massiveRadius; m++)
					{
						if (freePlace(i + l, j + m, k)) { reachOut = true; break; }
					}
					if (reachOut) break;
				}
				if (!reachOut)
				{
					houseField[k][i][j].house = HouseTypeBlock::None;
					houseField[k][i][j].blockReference = -1;
				}
			}
}
void HouseGenerator::createAndPlaceRoofBlock()
{
	//	simple merge roof
	for (unsigned int i = 0; i < blockList.size(); i++)
	{
		bool mergeSuccess = false;
		int floor = blockList[i].first.z + blockList[i].second.z - 1;
		for (unsigned int j = 0; j < roofBlockList.size(); j++)
		{
			if (floor != roofBlockList[j].first.z + roofBlockList[j].second.z - 1) break;

			if (blockList[i].first.x == roofBlockList[j].first.x && blockList[i].second.x == roofBlockList[j].second.x)
			{
				if(roofBlockList[j].first.y + roofBlockList[j].second.y == blockList[i].first.y)
				{
					//	augment roof size
					roofBlockList[j].second.y += blockList[i].second.y;
					mergeSuccess = true;
				}
				else if (blockList[i].first.y + blockList[i].second.y == roofBlockList[j].first.y)
				{
					//	augment roof size
					roofBlockList[j].first.y  -= blockList[i].second.y;
					roofBlockList[j].second.y += blockList[i].second.y;
					mergeSuccess = true;
				}
			}
			else if (blockList[i].first.y == roofBlockList[j].first.y && blockList[i].second.y == roofBlockList[j].second.y)
			{
				if (roofBlockList[j].first.x + roofBlockList[j].second.x == blockList[i].first.x)
				{
					//	augment roof size
					roofBlockList[j].second.x += blockList[i].second.x;
					mergeSuccess = true;
				}
				else if (blockList[i].first.x + blockList[i].second.x == roofBlockList[j].first.x)
				{
					//	augment roof size
					roofBlockList[j].first.x  -= blockList[i].second.x;
					roofBlockList[j].second.x += blockList[i].second.x;
					mergeSuccess = true;
				}
			}

			if (mergeSuccess)
			{
				//	update roof reference
				for (int k = blockList[i].first.z; k < blockList[i].first.z + blockList[i].second.z; k++)
					for (int l = blockList[i].first.x; l < blockList[i].first.x + blockList[i].second.x; l++)
						for (int m = blockList[i].first.y; m < blockList[i].first.y + blockList[i].second.y; m++)
							houseField[k][l][m].roofReference = j;
				break;
			}
		}
		if (!mergeSuccess) roofBlockList.push_back(std::pair<glm::ivec3, glm::ivec3>(blockList[i].first, blockList[i].second));
	}

	//	Place V roof
	for (unsigned int m = 0; m < roofBlockList.size(); m++)
	{
		for (int i = 0; i < roofBlockList[m].second.x; i++)
			for (int j = 0; j < roofBlockList[m].second.y; j++)
			{
				int k = roofBlockList[m].first.z + roofBlockList[m].second.z - 1;
				if (k + 1 < houseFieldFloor && houseField[k + 1][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].house != HouseTypeBlock::None) continue;

				if (roofBlockList[m].second.x > roofBlockList[m].second.y)
				{
					if(j < roofBlockList[m].second.y / 2) houseField[k][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].roof = j;
					else if ((roofBlockList[m].second.y % 2) && j == roofBlockList[m].second.y / 2) houseField[k][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].roof = roofBlockList[m].second.y / 2;
					else  houseField[k][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].roof = roofBlockList[m].second.y - j - 1;
				}
				else
				{
					if (i < roofBlockList[m].second.x / 2) houseField[k][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].roof = i;
					else if ((roofBlockList[m].second.x % 2) && i == roofBlockList[m].second.x / 2) houseField[k][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].roof = roofBlockList[m].second.x / 2;
					else  houseField[k][roofBlockList[m].first.x + i][roofBlockList[m].first.y + j].roof = roofBlockList[m].second.x - i - 1;
				}
			}
	}
}
void HouseGenerator::constructHouseMesh()
{
	//	Real stuff begin here
	Mesh* meshToPush = nullptr;
	for (int k = 0; k < houseFieldFloor; k++)
		for (int i = 1; i < houseFieldSize - 1; i++)
			for (int j = 1; j < houseFieldSize - 1; j++)
			{
				if (houseField[k][i][j].house == HouseTypeBlock::None) continue;

				//	Ground quad or debug roof
				pushGround(i - houseOrigin.x, j - houseOrigin.y, 2.5f * k + houseOrigin.z, i + 1 - houseOrigin.x, j + 1 - houseOrigin.y, 2.5f * k + houseOrigin.z, stoneColor);

				//	right wall
				if (houseField[k][i + 1][j].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:  meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:   meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window: meshToPush = assetLibrary["window"];	break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 1 - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				}

				//	left wall
				if (houseField[k][i - 1][j].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:  meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:   meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window: meshToPush = assetLibrary["window"];	break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(0.f, 1.f, 0.f));
				}

				//	lower wall
				if (houseField[k][i][j + 1].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:  meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:   meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window: meshToPush = assetLibrary["window"];	break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(1.f, 0.f, 0.f));
				}

				//	upper wall
				if (houseField[k][i][j - 1].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:  meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:   meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window: meshToPush = assetLibrary["window"];	break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(-1.f, 0.f, 0.f));
				}

				//	corners
				if (houseField[k][i + 1][j].house == HouseTypeBlock::None && houseField[k][i][j + 1].house == HouseTypeBlock::None)
					pushMesh(assetLibrary["corner"], glm::vec3(i + 1 - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(1.f, 0.f, 0.f));
				if (houseField[k][i - 1][j].house == HouseTypeBlock::None && houseField[k][i][j + 1].house == HouseTypeBlock::None)
					pushMesh(assetLibrary["corner"], glm::vec3(i - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, 1.f, 0.f));
				if (houseField[k][i + 1][j].house == HouseTypeBlock::None && houseField[k][i][j - 1].house == HouseTypeBlock::None)
					pushMesh(assetLibrary["corner"], glm::vec3(i + 1 - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				if (houseField[k][i - 1][j].house == HouseTypeBlock::None && houseField[k][i][j - 1].house == HouseTypeBlock::None)
					pushMesh(assetLibrary["corner"], glm::vec3(i - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(-1.f, 0.f, 0.f));
			}
}
void HouseGenerator::constructRoofMesh()
{
	houseOrigin.z += 2.5f;
	for (int k = 0; k < houseFieldFloor - 1; k++)
		for (int i = 1; i < houseFieldSize - 1; i++)
			for (int j = 1; j < houseFieldSize - 1; j++)
			{
				if (houseField[k][i][j].roof < 0) continue;
				if (roofBlockList[houseField[k][i][j].roofReference].second.x > roofBlockList[houseField[k][i][j].roofReference].second.y)
				{

				}
				else
				{

				}

				//if (houseField[k][i+1][j].roof < 0 || houseField[k][i][j].roof - houseField[k][i + 1][j].roof == -1)
				//	pushMesh(assetLibrary["roof1"], glm::vec3(i + 1 - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k + houseField[k][i][j].roof), glm::vec3(0.f,-1.f, 0.f));
				pushMesh(assetLibrary["debug"], glm::vec3(i + 0.5f - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 0.5f + 2.5f * k + houseField[k][i][j].roof), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.45f, 0.45f, 0.45f));
			}
}








bool HouseGenerator::searchBlockPartition(const int& superficy, const int& testIndex)
{
	if (blockLibrary[testIndex].z >= houseFieldFloor) return false;

	int s = superficy - blockLibrary[testIndex].x * blockLibrary[testIndex].y * blockLibrary[testIndex].z;
	if (s < 0) return false;
	else if (s < 3)
	{
		blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(0, 0, 0), blockLibrary[testIndex]));
		return true;
	}
	else
	{
		blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(0, 0, 0), blockLibrary[testIndex]));
		for (unsigned int i = testIndex; i < blockLibrary.size(); i++)
			if (searchBlockPartition(s, i)) return true;
		return false;
	}
}
bool HouseGenerator::freePlace(const unsigned int& i, const unsigned int& j, const unsigned int& k) const
{
	if (houseField[k][i][j].blockReference >=0 ) return false;
	else if (houseField[k][i][j].roofReference >= 0) return false;
	else return true;
}
bool HouseGenerator::freeFloor(const unsigned int& k) const
{
	for (int i = 0; i < houseFieldSize; i++)
		for (int j = 0; j < houseFieldSize; j++)
			if (!freePlace(i,j,k)) return false;
	return true;
}
glm::ivec3 HouseGenerator::getRandomPosition(const int& safeOffset, const int& maxZ)
{
	return glm::ivec3(	(randomEngine() % (houseFieldSize - 2 * safeOffset)) + safeOffset,
						(randomEngine() % (houseFieldSize - 2 * safeOffset)) + safeOffset,
						randomEngine() % (maxZ + 1) );
}


void HouseGenerator::markAll(const glm::ivec3& p, const glm::ivec3& s)
{
	float mark = markFree(p, s);	if (mark <= 0.f) return;
	mark *= markSupport(p, s);		if (mark <= 0.f) return;
	mark *= markAdjacent(p, s);		if (mark <= 0.f) return;
	mark *= markMassive(p, s);		if (mark <= 0.f) return;

	benchmarkPosition.push_back(markedPosition(mark + (randomEngine() % std::max(s.x, s.y)), p, s));
}
float HouseGenerator::markFree(const glm::ivec3& p, const glm::ivec3& s) const
{
	//	Check out of range
	if (p.x < 0 || p.y < 0 || p.z < 0)
		return 0.f;
	else if (p.x + s.x >= houseFieldSize || p.y + s.y >= houseFieldSize || p.z + s.z >= houseFieldFloor)
		return 0.f;

	//	Check available
	for (int k = p.z; k < p.z + s.z; k++)
		for (int i = p.x; i < p.x + s.x; i++)
			for (int j = p.y; j < p.y + s.y; j++)
				if (!freePlace(i, j, k)) return 0.f;
	return 1.f;
}
float HouseGenerator::markSupport(const glm::ivec3& p, const glm::ivec3& s) const
{
	if (p.z > 0)
	{
		int support = 0;
		for (int i = p.x; i < p.x + s.x; i++)
			for (int j = p.y; j < p.y + s.y; j++)
				if (!freePlace(i, j, p.z - 1)) support++;

		//	check if enough support
		if (support < 0.7f * s.x * s.y) return 0.f;
		return (float)support;
	}
	else if (p.z == 0) return s.x * s.y * (1.1f - density / 100.f);
	else return 0.f;
}
float HouseGenerator::markAdjacent(const glm::ivec3& p, const glm::ivec3& s) const
{
	if (!freeFloor(p.z))
	{
		int blockCount = 0;
		for (int i = p.x; i < p.x + s.x; i++)
		{
			if (!freePlace(i, p.y - 1, p.z))   blockCount++;
			if (!freePlace(i, p.y + s.y, p.z)) blockCount++;
		}
		for (int i = p.y; i < p.y + s.y; i++)
		{
			if (!freePlace(p.x - 1, i, p.z))   blockCount++;
			if (!freePlace(p.x + s.x, i, p.z)) blockCount++;
		}
		if (blockCount < 3) return 0.f;
		return (float) blockCount;
	}
	else if (s.x * s.y < 12) return 0.f;
	else return (float)(s.x * s.y);
}
float HouseGenerator::markMassive(const glm::ivec3& p, const glm::ivec3& s) const
{
	bool reachOut;
	for (int i = 0; i < houseFieldSize; i++)
		for (int j = 0; j < houseFieldSize; j++)
		{
			if ((i >= p.x && i < p.x + s.x && j >= p.y && j < p.y + s.y) || !freePlace(i, j, p.z))
			{
				reachOut = false;
				for (int l = -massiveRadius; l <= massiveRadius; l++)
				{
					for (int m = -massiveRadius; m <= massiveRadius; m++)
					{
						if ((i + l >= p.x && i + l < p.x + s.x && j + m >= p.y && j + m < p.y + s.y) || !freePlace(i + l, j + m, p.z));
						else
						{
							reachOut = true; break;
						}
					}
					if (reachOut) break;
				}
				if (!reachOut) return 0.01f;
			}
		}
	return 1.f;
}

void HouseGenerator::addHouseBlocks(const glm::ivec3& p, const glm::ivec3& s, const int& houseType, const unsigned int& blockReference)
{
	for (int k = p.z; k < p.z + s.z; k++)
		for (int i = p.x; i < p.x + s.x; i++)
			for (int j = p.y; j < p.y + s.y; j++)
			{
				houseField[k][i][j].house = houseType;
				houseField[k][i][j].blockReference = blockReference;
			}
}
void HouseGenerator::updateAvailableBlockPosition(const glm::ivec3& p, const glm::ivec3& s)
{
	//	adjacent block
	for (int k = p.z; k < p.z + s.z; k++)
	{
		for (int i = p.x; i < p.x + s.x; i++)
			availableBlockPosition.push_back(glm::ivec3(i, p.y - 1,   k));
		for (int i = p.x; i < p.x + s.x; i++)
			availableBlockPosition.push_back(glm::ivec3(i, p.y + s.y, k));
		for (int i = p.y; i < p.y + s.y; i++)
			availableBlockPosition.push_back(glm::ivec3(p.x - 1,   i, k));
		for (int i = p.y; i < p.y + s.y; i++)
			availableBlockPosition.push_back(glm::ivec3(p.x + s.x, i, k));
	}

	//	top
	if (p.z + s.z < (int)houseFieldFloor)
	{
		for (int i = p.x - 1; i <= p.x + s.x; i++)
			for (int j = p.y - 1; j <= p.y + s.y; j++)
				availableBlockPosition.push_back(glm::ivec3(i, j, p.z + s.z));
	}

	//	clear pass
	const int safeOffset = std::max(blockLibrary[0].x, blockLibrary[0].y) + massiveRadius + 8;
	auto it = availableBlockPosition.begin();
	while (it != availableBlockPosition.end())
	{
		if (it->x < safeOffset || it->y < safeOffset || it->z < 0)
			it = availableBlockPosition.erase(it);
		else if (it->x >= (int)houseFieldSize - safeOffset || it->y >= (int)houseFieldSize - safeOffset || it->z >= (int)houseFieldFloor)
			it = availableBlockPosition.erase(it);
		else if (!freePlace(it->x, it->y, it->z))
			it = availableBlockPosition.erase(it);
		else it++;
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




/*
inline void HouseGenerator::createAndPlaceRoof()
{
	glm::ivec3 origin = findRoofSeed();
	glm::ivec3 size;
	while (origin.x >= 0)
	{
		size = glm::ivec3(1, 1, 1);
		//growRoofSeed(origin, size);
		//addBlocks(origin, size, Roof);
		roofBlockList.push_back(std::pair<glm::ivec3, glm::ivec3>(origin, size));
		origin = findRoofSeed();
	}
	
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
*/

void HouseGenerator::optimizeMesh()
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
