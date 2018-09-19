#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>

#include "HouseGenerator.h"
#include "Resources/ComponentResource.h"


//  Attributes
glm::vec3 HouseGenerator::stoneColor = glm::vec3(0.2f, 0.2f, 0.2f);
const int HouseGenerator::massiveRadius = 3;
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

	//	House mesh
	assetLibrary["wall"] = ResourceManager::getInstance()->getMesh("House/HouseWall.obj");
	assetLibrary["corner"] = ResourceManager::getInstance()->getMesh("House/HouseCorner.obj");
	assetLibrary["door"] = ResourceManager::getInstance()->getMesh("House/HouseDoor.obj");
	assetLibrary["window"] = ResourceManager::getInstance()->getMesh("House/HouseWindow.obj");
	assetLibrary["fireplace"] = ResourceManager::getInstance()->getMesh("House/HouseFireplace.obj");

	//	Roof slope meshes
	assetLibrary["pic"] = ResourceManager::getInstance()->getMesh("House/HouseRoofPic.obj");
	assetLibrary["slope"] = ResourceManager::getInstance()->getMesh("House/HouseRoofSlope.obj");
	assetLibrary["inner"] = ResourceManager::getInstance()->getMesh("House/HouseRoofDualSlopeInner.obj");
	assetLibrary["outter"] = ResourceManager::getInstance()->getMesh("House/HouseRoofDualSlopeOutter.obj");
	

	//	Roof top meshes
	assetLibrary["top"] = ResourceManager::getInstance()->getMesh("House/HouseRoofTop.obj");
	assetLibrary["topEnd"] = ResourceManager::getInstance()->getMesh("House/HouseRoofTopSlope.obj");
	assetLibrary["topAngle"] = ResourceManager::getInstance()->getMesh("House/HouseRoofTopAngle.obj");
	assetLibrary["topTee"] = ResourceManager::getInstance()->getMesh("House/HouseRoofTopTee.obj");

	//	Roof top meshes
	assetLibrary["slopeTop"] = ResourceManager::getInstance()->getMesh("House/HouseRoofSlopeTop.obj");
	assetLibrary["topDualSlope"] = ResourceManager::getInstance()->getMesh("House/HouseRoofTopDualSlope.obj");
	assetLibrary["topDualSlope2"] = ResourceManager::getInstance()->getMesh("House/HouseRoofTopDualSlope2.obj");
	assetLibrary["dualTopDualSlope"] = ResourceManager::getInstance()->getMesh("House/HouseRoofDualTopDualSlope.obj");

	//	Roof end
	assetLibrary["roofEnd"] = ResourceManager::getInstance()->getMesh("House/HouseRoofEnd3.obj");
	assetLibrary["roofEndCorner"] = ResourceManager::getInstance()->getMesh("House/HouseRoofEnd4.obj");

	//	Elementary blocks
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
	for (auto it = assetLibrary.begin(); it != assetLibrary.end(); it++)
		ResourceManager::getInstance()->release(it->second);

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
	initHouseField(prosperity);

	//	generate house
	createAndPlaceHouseBlock();
	createAndPlaceDecorationBlock();
	createAndPlaceRoofBlock();

	//	generate mesh
	constructHouseMesh();
	constructRoofMesh();
	optimizeMesh();


	//	end
	Mesh* mesh = new Mesh(houseName, verticesArray, normalesArray, colorArray, facesArray);
		ResourceManager::getInstance()->addMesh(mesh);
	InstanceVirtual* house = new InstanceVirtual();
		house->addComponent(new ComponentResource<Mesh>(ResourceManager::getInstance()->getMesh(houseName)));
		house->addComponent(new ComponentResource<Shader>(ResourceManager::getInstance()->getShader("default")));
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
	/*std::cout << "   density : " << density << std::endl;
	std::cout << "   prosperity : " << prosperity << std::endl;
	std::cout << "   house field size : " << houseFieldSize << std::endl;
	std::cout << "   house field floor count : " << houseFieldFloor << std::endl;
	std::cout << "   superficy : " << superficy << std::endl;*/
}
void HouseGenerator::createAndPlaceDebugHouseBlock(int config)
{
	if (config == 1)
	{
		addHouseBlocks(glm::ivec3(50, 50, 0), glm::ivec3(17, 7, houseFieldFloor - 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(50, 50, 0), glm::ivec3(17, 7, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(50, 57, 0), glm::ivec3(7,  3, houseFieldFloor - 1), HouseTypeBlock::House, 1); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(50, 57, 0), glm::ivec3(7, 3, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(60, 57, 0), glm::ivec3(7,  3, houseFieldFloor - 1), HouseTypeBlock::House, 2); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(60, 57, 0), glm::ivec3(7, 3, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(50, 60, 0), glm::ivec3(17, 7, houseFieldFloor - 1), HouseTypeBlock::House, 3); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(50, 60, 0), glm::ivec3(17, 7, houseFieldFloor - 1)));

		addHouseBlocks(glm::ivec3(67, 57, 0), glm::ivec3(4, 3, houseFieldFloor - 1), HouseTypeBlock::House, 4);  blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(67, 57, 0), glm::ivec3(4, 3, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(57, 67, 0), glm::ivec3(3, 4, houseFieldFloor - 1), HouseTypeBlock::House, 5);  blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(57, 67, 0), glm::ivec3(3, 4, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(46, 57, 0), glm::ivec3(4, 3, houseFieldFloor - 1), HouseTypeBlock::House, 6);  blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(46, 57, 0), glm::ivec3(4, 3, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(57, 46, 0), glm::ivec3(3, 4, houseFieldFloor - 1), HouseTypeBlock::House, 7);  blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(57, 46, 0), glm::ivec3(3, 4, houseFieldFloor - 1)));

		addHouseBlocks(glm::ivec3(50, 43, 0), glm::ivec3(17, 3, houseFieldFloor - 1), HouseTypeBlock::House, 8); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(50, 43, 0), glm::ivec3(17, 3, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(50, 70, 0), glm::ivec3(17, 3, houseFieldFloor - 1), HouseTypeBlock::House, 9); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(50, 70, 0), glm::ivec3(17, 3, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(43, 50, 0), glm::ivec3(3, 17, houseFieldFloor - 1), HouseTypeBlock::House, 10); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(43, 50, 0), glm::ivec3(3, 17, houseFieldFloor - 1)));
		addHouseBlocks(glm::ivec3(70, 50, 0), glm::ivec3(3, 17, houseFieldFloor - 1), HouseTypeBlock::House, 11); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(70, 50, 0), glm::ivec3(3, 17, houseFieldFloor - 1)));

		superficy = 532 * (houseFieldFloor - 1);
	}
	else
	{
		addHouseBlocks(glm::ivec3(54, 54, 0), glm::ivec3(5, 5, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(54, 54, 0), glm::ivec3(5, 5, 1)));
		addHouseBlocks(glm::ivec3(56, 52, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(56, 52, 0), glm::ivec3(3, 2, 1)));
		addHouseBlocks(glm::ivec3(59, 56, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(59, 56, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(52, 56, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(52, 56, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(54, 59, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(54, 59, 0), glm::ivec3(3, 2, 1)));

		addHouseBlocks(glm::ivec3(41, 54, 0), glm::ivec3(5, 5, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(41, 54, 0), glm::ivec3(5, 5, 1)));
		addHouseBlocks(glm::ivec3(41, 52, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(41, 52, 0), glm::ivec3(3, 2, 1)));
		addHouseBlocks(glm::ivec3(39, 56, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(39, 56, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(46, 56, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(46, 56, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(43, 59, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(43, 59, 0), glm::ivec3(3, 2, 1)));


		addHouseBlocks(glm::ivec3(54, 41, 0), glm::ivec3(5, 5, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(54, 41, 0), glm::ivec3(5, 5, 1)));
		addHouseBlocks(glm::ivec3(56, 46, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(54, 54, 0), glm::ivec3(3, 2, 1)));
		addHouseBlocks(glm::ivec3(59, 41, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(59, 41, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(52, 41, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(52, 41, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(54, 39, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(54, 39, 0), glm::ivec3(3, 2, 1)));

		addHouseBlocks(glm::ivec3(41, 41, 0), glm::ivec3(5, 5, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(41, 41, 0), glm::ivec3(5, 5, 1)));
		addHouseBlocks(glm::ivec3(41, 46, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(41, 46, 0), glm::ivec3(3, 2, 1)));
		addHouseBlocks(glm::ivec3(39, 41, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(39, 41, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(46, 41, 0), glm::ivec3(2, 3, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(46, 41, 0), glm::ivec3(2, 3, 1)));
		addHouseBlocks(glm::ivec3(43, 39, 0), glm::ivec3(3, 2, 1), HouseTypeBlock::House, 0); blockList.push_back(std::pair<glm::ivec3, glm::ivec3>(glm::ivec3(43, 39, 0), glm::ivec3(3, 2, 1)));

		superficy = 196;
	}

	houseOrigin.x = 50;
	houseOrigin.y = 50;
	houseOrigin.z = 0.1f;
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
			addHouseBlocks(blockList[i].first, blockList[i].second, HouseTypeBlock::House, i);
			updateAvailableBlockPosition(blockList[i].first, blockList[i].second);
		}
		else std::cout << "!!!!!!!!!!!!!!!!fail to place block!!!!!!!!!!!!!!!!" << std::endl;
	}

	//	Compute house center
	glm::ivec3 vmax(0, 0, 0);
	glm::ivec3 vmin(houseFieldSize, houseFieldSize, houseFieldFloor);
	for (unsigned int i = 0; i < blockList.size(); i++)
	{
		vmax.x = std::max(vmax.x, blockList[i].first.x + blockList[i].second.x);
		vmax.y = std::max(vmax.y, blockList[i].first.y + blockList[i].second.y);
		vmax.z = std::max(vmax.z, blockList[i].first.z + blockList[i].second.z);

		vmin.x = std::min(vmin.x, blockList[i].first.x);
		vmin.y = std::min(vmin.y, blockList[i].first.y);
		vmin.z = std::min(vmin.z, blockList[i].first.z);
	}
	houseOrigin.x = 0.5f * (vmax.x + vmin.x);
	houseOrigin.y = 0.5f * (vmax.y + vmin.y);
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
					houseField[k][i][j].roofReference = -1;
					houseField[k][i][j].roof = -1;
				}
			}
}
void HouseGenerator::createAndPlaceDecorationBlock()
{
	unsigned int index = (unsigned int)blockList.size();
	int doorCount = 1 + superficy / houseFieldFloor / 40;
	int fireplaceCount = 1 + superficy / 70;
	int factor = 0;

	/*std::cout << "   Door count : " << doorCount << std::endl;
	std::cout << "   Fireplace count : " << fireplaceCount << std::endl;*/

	for (unsigned int m = 0; m < blockList.size(); m++)
	{
		for (int k = blockList[m].first.z; k < blockList[m].first.z + blockList[m].second.z; k++)
			for (int i = blockList[m].first.x; i < blockList[m].first.x + blockList[m].second.x; i++)
				for (int j = blockList[m].first.y; j < blockList[m].first.y + blockList[m].second.y; j++, factor++)
				{
					if (!markDecorative(i, j, k)) continue;
					int rand = randomEngine() % 100;;

					if (k == 0 && doorCount && rand < 10 + 90.f * factor/superficy)
					{
						addHouseBlocks(glm::ivec3(i, j, k), glm::ivec3(1, 1, 1), HouseTypeBlock::Door, index++);
						doorCount--;
					}
					else if (fireplaceCount && rand < 10 + 90.f * factor / superficy)
					{
						addHouseBlocks(glm::ivec3(i, j, k), glm::ivec3(1, 1, 1), HouseTypeBlock::Fireplace, index++);
						fireplaceCount--;
					}
					else if (rand < 40)
					{
						addHouseBlocks(glm::ivec3(i, j, k), glm::ivec3(1, 1, 1), HouseTypeBlock::Window, index++);
					}
				}
	}
}
void HouseGenerator::createAndPlaceRoofBlock()
{
	const int offset = 10;
	for (int k = 0; k < houseFieldFloor - 1; k++)
		for (int i = 1; i < houseFieldSize - 1; i++)
			for (int j = 1; j < houseFieldSize - 1; j++)
			{
				if (freePlace(i, j, k)) continue;
				int r = massiveRadius + offset;
				for (int l = -massiveRadius - offset; l <= massiveRadius + offset; l++)
					for (int m = -massiveRadius - offset; m <= massiveRadius + offset; m++)
					{
						if (freePlace(i + l, j + m, k))
							r = std::min(r, std::max(std::abs(l), std::abs(m)) - 1);
					}
				houseField[k][i][j].roof = r;
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
						case HouseTypeBlock::House:		meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:		meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window:	meshToPush = assetLibrary["window"];	break;
						case HouseTypeBlock::Fireplace: meshToPush = assetLibrary["fireplace"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 1 - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
				}

				//	left wall
				if (houseField[k][i - 1][j].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:		meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:		meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window:	meshToPush = assetLibrary["window"];	break;
						case HouseTypeBlock::Fireplace: meshToPush = assetLibrary["fireplace"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(0.f, 1.f, 0.f));
				}

				//	lower wall
				if (houseField[k][i][j + 1].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:		meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:		meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window:	meshToPush = assetLibrary["window"];	break;
						case HouseTypeBlock::Fireplace: meshToPush = assetLibrary["fireplace"]; break;
						default: meshToPush = nullptr; break;
					}
					pushMesh(meshToPush, glm::vec3(i + 0.5f - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f*k), glm::vec3(1.f, 0.f, 0.f));
				}

				//	upper wall
				if (houseField[k][i][j - 1].house == HouseTypeBlock::None)
				{
					switch (houseField[k][i][j].house)
					{
						case HouseTypeBlock::House:		meshToPush = assetLibrary["wall"];		break;
						case HouseTypeBlock::Door:		meshToPush = assetLibrary["door"];		break;
						case HouseTypeBlock::Window:	meshToPush = assetLibrary["window"];	break;
						case HouseTypeBlock::Fireplace: meshToPush = assetLibrary["fireplace"]; break;
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
				if (houseField[k][i][j].roof < 0)
				{
					if (!freePlace(i - 1, j, k))
						pushMesh(assetLibrary["roofEnd"], glm::vec3(i - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
					if (!freePlace(i + 1, j, k))
						pushMesh(assetLibrary["roofEnd"], glm::vec3(i + 1 - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, 1.f, 0.f));
					if (!freePlace(i, j - 1, k))
						pushMesh(assetLibrary["roofEnd"], glm::vec3(i + 0.5f - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(1.f, 0.f, 0.f));
					if (!freePlace(i, j + 1, k))
						pushMesh(assetLibrary["roofEnd"], glm::vec3(i + 0.5f - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(-1.f, 0.f, 0.f));
				
					if (!freePlace(i - 1, j - 1, k) && freePlace(i - 1, j, k) && freePlace(i, j - 1, k))
						pushMesh(assetLibrary["roofEndCorner"], glm::vec3(i - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, 1.f, 0.f));
					if (!freePlace(i + 1, j + 1, k) && freePlace(i + 1, j, k) && freePlace(i, j + 1, k))
						pushMesh(assetLibrary["roofEndCorner"], glm::vec3(i + 1 - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(0.f, -1.f, 0.f));
					if (!freePlace(i - 1, j + 1, k) && freePlace(i - 1, j, k) && freePlace(i, j + 1, k))
						pushMesh(assetLibrary["roofEndCorner"], glm::vec3(i - houseOrigin.x, j + 1 - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(1.f, 0.f, 0.f));
					if (!freePlace(i + 1, j - 1, k) && freePlace(i + 1, j, k) && freePlace(i, j - 1, k))
						pushMesh(assetLibrary["roofEndCorner"], glm::vec3(i + 1 - houseOrigin.x, j - houseOrigin.y, houseOrigin.z + 2.5f * k), glm::vec3(-1.f, 0.f, 0.f));
				}
				else if (k + 1 < houseFieldFloor && houseField[k + 1][i][j].house == HouseTypeBlock::None)
				{
					///	init
					char north = getRelativePosRoof(houseField[k][i][j].roof, i, j - 1, k);
					char south = getRelativePosRoof(houseField[k][i][j].roof, i, j + 1, k);
					char west  = getRelativePosRoof(houseField[k][i][j].roof, i + 1, j, k);
					char east   = getRelativePosRoof(houseField[k][i][j].roof, i - 1, j, k);

					char northEast = getRelativePosRoof(houseField[k][i][j].roof, i - 1, j - 1, k);
					char southEast = getRelativePosRoof(houseField[k][i][j].roof, i - 1, j + 1, k);
					char northWest = getRelativePosRoof(houseField[k][i][j].roof, i + 1, j - 1, k);
					char southWest = getRelativePosRoof(houseField[k][i][j].roof, i + 1, j + 1, k);

					glm::vec3 p = glm::vec3(i + 0.5f - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k + houseField[k][i][j].roof);

					///	Pic
					if (north == '-' && south == '-' && east == '-' && west == '-')
						pushMesh(assetLibrary["pic"], glm::vec3(i + 0.5f - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 2.5f * k + houseField[k][i][j].roof), glm::vec3(1.f, 0.f, 0.f));
					
					///	Merge-part two top & two slope
					else if (north == '0' && south == '0' && east == '0' && west == '0' && northEast == '-' && (northWest == '0' || northWest == '+') && southEast == '-' && southWest == '-')
						pushMesh(assetLibrary["dualTopDualSlope"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && east == '0' && west == '0' && (northEast == '0' || northEast == '+') && northWest == '-' && southEast == '-' && southWest == '-')
						pushMesh(assetLibrary["dualTopDualSlope"], p, glm::vec3(0.f, 1.f, 0.f));
					else if (north == '0' && south == '0' && east == '0' && west == '0' && northEast == '-' && northWest == '-' && (southEast == '0' || southEast == '+') && southWest == '-')
						pushMesh(assetLibrary["dualTopDualSlope"], p, glm::vec3(1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && east == '0' && west == '0' && northEast == '-' && northWest == '-' && southEast == '-' && (southWest == '0' || southWest == '+'))
						pushMesh(assetLibrary["dualTopDualSlope"], p, glm::vec3(0.f, -1.f, 0.f));
					
					///	Merge-part one top & two slope
					else if (north == '0' && south == '0' && east == '-' && west == '0' && (northWest == '0' || northWest == '+') && southEast == '-' && southWest == '-')
						pushMesh(assetLibrary["topDualSlope2"], p, glm::vec3(0.f, 1.f, 0.f));
					else if (north == '0' && south == '-' && east == '0' && west == '0' && (northEast == '0' || northEast == '+') && southWest == '-' && northWest == '-')
						pushMesh(assetLibrary["topDualSlope2"], p, glm::vec3(1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && east == '0' && west == '-' && (southEast == '0' || southEast == '+') && northEast == '-' && northWest == '-')
						pushMesh(assetLibrary["topDualSlope2"], p, glm::vec3(0.f, -1.f, 0.f));
					else if (north == '-' && south == '0' && east == '0' && west == '0' && (southWest == '0' || southWest == '+') && northEast == '-' && southEast == '-')
						pushMesh(assetLibrary["topDualSlope2"], p, glm::vec3(-1.f, 0.f, 0.f));

					else if (north == '0' && south == '0' && east == '0' && west == '-' && (northEast == '0' || northEast == '+') && southEast == '-' && southWest == '-')
						pushMesh(assetLibrary["topDualSlope"], p, glm::vec3(0.f, 1.f, 0.f));
					else if (north == '-' && south == '0' && east == '0' && west == '0' && (southEast == '0' || southEast == '+') && northWest == '-' && southWest == '-')
						pushMesh(assetLibrary["topDualSlope"], p, glm::vec3(1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && east == '-' && west == '0' && (southWest == '0' || southWest == '+') && northWest == '-' && northEast == '-')
						pushMesh(assetLibrary["topDualSlope"], p, glm::vec3(0.f, -1.f, 0.f));
					else if (north == '0' && south == '-' && east == '0' && west == '0' && (northWest == '0' || northWest == '+') && southEast == '-' && northEast == '-')
						pushMesh(assetLibrary["topDualSlope"], p, glm::vec3(-1.f, 0.f, 0.f));

					///	Top tee
					else if (north == '0' && south == '0' && east == '0' && west == '-' && southEast == '-' && northEast == '-')
						pushMesh(assetLibrary["topTee"], p, glm::vec3(0.f, 1.f, 0.f));
					else if (north == '0' && south == '-' && east == '0' && west == '0' && northEast == '-' && northWest == '-')
						pushMesh(assetLibrary["topTee"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && east == '-' && west == '0' && northWest == '-' && southWest == '-')
						pushMesh(assetLibrary["topTee"], p, glm::vec3(0.f, -1.f, 0.f));
					else if (north == '-' && south == '0' && east == '0' && west == '0' && southWest == '-' && southEast == '-')
						pushMesh(assetLibrary["topTee"], p, glm::vec3(1.f, 0.f, 0.f));

					///	Merge-part top slope
					else if ((north == '0' || north == '+') && south == '0' && east == '0' && west == '0' && southWest == '-' && southEast == '-')
						pushMesh(assetLibrary["slopeTop"], p, glm::vec3(1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && (east == '0' || east == '+') && west == '0' && southWest == '-' && northWest == '-')
						pushMesh(assetLibrary["slopeTop"], p, glm::vec3(0.f, -1.f, 0.f));
					else if (north == '0' && (south == '0' || south == '+') && east == '0' && west == '0' && northEast == '-' && northWest == '-')
						pushMesh(assetLibrary["slopeTop"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if (north == '0' && south == '0' && east == '0' && (west == '0' || west == '+') && northEast == '-' && southEast == '-')
						pushMesh(assetLibrary["slopeTop"], p, glm::vec3(0.f, 1.f, 0.f));

					///	Top angles
					else if (north == '-' && south == '0' && east == '-' && west == '0' && northWest == '-' && southEast == '-' && southWest == '-')
						pushMesh(assetLibrary["topAngle"], p, glm::vec3(0.f, -1.f, 0.f));
					else if (north == '0' && south == '-' && east == '-' && west == '0' && northWest == '-' && northEast == '-' && southWest == '-')
						pushMesh(assetLibrary["topAngle"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if (north == '0' && south == '-' && east == '0' && west == '-' && northWest == '-' && northEast == '-' && southEast == '-')
						pushMesh(assetLibrary["topAngle"], p, glm::vec3(0.f, 1.f, 0.f));
					else if (north == '-' && south == '0' && east == '0' && west == '-' && northEast == '-' && southEast == '-' && southWest == '-')
						pushMesh(assetLibrary["topAngle"], p, glm::vec3(1.f, 0.f, 0.f));

					///	Top end
					else if (north == '0' && (south == '-' || south == '1') && (east == '-' || east == '1') && (west == '-' || west == '1'))
						pushMesh(assetLibrary["topEnd"], p, glm::vec3(0.f, -1.f, 0.f));
					else if ((north == '-' || north == '1') && (south == '-' || south == '1') && east == '0' && (west == '-' || west == '1'))
						pushMesh(assetLibrary["topEnd"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if ((north == '-' || north == '1') && south == '0' && (east == '-' || east == '1') && (west == '-' || west == '1'))
						pushMesh(assetLibrary["topEnd"], p, glm::vec3(0.f, 1.f, 0.f));
					else if ((north == '-' || north == '1') && (south == '-' || south == '1') && (east == '-' || east == '1') && west == '0')
						pushMesh(assetLibrary["topEnd"], p, glm::vec3(1.f, 0.f, 0.f));

					///	Top
					else if (north == '0' && south == '0' && east == '-' && west == '-')
						pushMesh(assetLibrary["top"], p, glm::vec3(0.f, 1.f, 0.f));
					else if (north == '-' && south == '-' && east == '0' && west == '0')
						pushMesh(assetLibrary["top"], p, glm::vec3(1.f, 0.f, 0.f));

					///	Dual slope outter
					else if (north == '0' && (south == '-' || south == '1') && (east == '-' || east == '1') && west == '0' && (northWest == '0' || northWest == '+'))
						pushMesh(assetLibrary["outter"], p, glm::vec3(1.f, 0.f, 0.f));
					else if (north == '0' && (south == '-' || south == '1') && east == '0'  && (west == '-' || west == '1')&& (northEast == '0' || northEast == '+'))
						pushMesh(assetLibrary["outter"], p, glm::vec3(0.f, -1.f, 0.f));
					else if ((north == '-' || north == '1') && south == '0' && east == '0' && (west == '-' || west == '1') && (southEast == '0' || southEast == '+'))
						pushMesh(assetLibrary["outter"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if ((north == '-' || north == '1') && south == '0' && (east == '-' || east == '1') && west == '0' && (southWest == '0' || southWest == '+'))
						pushMesh(assetLibrary["outter"], p, glm::vec3(0.f, 1.f, 0.f));

					///	Dual slope inner
					else if ((north == '+' || north == '0') && south == '0' && east == '0' && (west == '+' || west == '0') && (southEast == '-' || southEast == '1'))
						pushMesh(assetLibrary["inner"], p, glm::vec3(1.f, 0.f, 0.f));
					else if ((north == '+' || north == '0') && south == '0' && (east == '+' || east == '0') && west == '0'  && (southWest == '-' || southWest == '1'))
						pushMesh(assetLibrary["inner"], p, glm::vec3(0.f, -1.f, 0.f));
					else if (north == '0' && (south == '+' || south == '0') && (east == '+' || east == '0') && west == '0' && (northWest == '-' || northWest == '1'))
						pushMesh(assetLibrary["inner"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if (north == '0' && (south == '+' || south == '0') && east == '0' && (west == '+' || west == '0') && (northEast == '-' || northEast == '1'))
						pushMesh(assetLibrary["inner"], p, glm::vec3(0.f, 1.f, 0.f));

					///	Simple slope
					else if ((north == '+' || north == '0') && (south == '-' || south == '1'))
						pushMesh(assetLibrary["slope"], p, glm::vec3(1.f, 0.f, 0.f));
					else if ((east == '-' || east == '1') && (west == '+' || west == '0'))
						pushMesh(assetLibrary["slope"], p, glm::vec3(0.f, 1.f, 0.f));
					else if ((north == '-' || north == '1') && (south == '+' || south == '0'))
						pushMesh(assetLibrary["slope"], p, glm::vec3(-1.f, 0.f, 0.f));
					else if ((east == '+' || east == '0') && (west == '-' || west == '1'))
						pushMesh(assetLibrary["slope"], p, glm::vec3(0.f, -1.f, 0.f));

					///	Error
					else pushMesh(assetLibrary["debug"], glm::vec3(i + 0.5f - houseOrigin.x, j + 0.5f - houseOrigin.y, houseOrigin.z + 0.5f + 2.5f * k + houseField[k][i][j].roof), glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.3f, 0.3f, 0.3f));
				}
			}
}
void HouseGenerator::optimizeMesh()
{
	std::vector<glm::vec3> verticesBuffer;
	std::vector<glm::vec3> normalesBuffer;
	std::vector<glm::vec3> colorBuffer;
	std::vector<unsigned short> facesBuffer;

	std::map<OrderedVertex, unsigned short> vertexAlias;
	std::map<OrderedVertex, unsigned short>::iterator alias;
	OrderedVertex current;

	for (unsigned int i = 0; i < facesArray.size(); i++)
	{
		current.position = verticesArray[facesArray[i]];
		current.normal = normalesArray[facesArray[i]];
		current.color = colorArray[facesArray[i]];

		alias = vertexAlias.find(current);
		if (alias == vertexAlias.end())
		{
			verticesBuffer.push_back(verticesArray[facesArray[i]]);
			normalesBuffer.push_back(normalesArray[facesArray[i]]);
			colorBuffer.push_back(colorArray[facesArray[i]]);
			facesBuffer.push_back((unsigned short)vertexAlias.size());

			vertexAlias[current] = (unsigned short)vertexAlias.size();
		}
		else facesBuffer.push_back(alias->second);
	}

	verticesArray.swap(verticesBuffer);
	normalesArray.swap(normalesBuffer);
	colorArray.swap(colorBuffer);
	facesArray.swap(facesBuffer);
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
bool HouseGenerator::markDecorative(const unsigned int& i, const unsigned int& j, const unsigned int& k) const
{
	//	on a wall
	char orientation;
	int corner = 0;
	for (;;)
	{
		if (houseField[k][i][j - 1].house == HouseTypeBlock::None) { orientation = 'n'; corner++; }
		if (houseField[k][i][j + 1].house == HouseTypeBlock::None) { orientation = 's'; corner++; }
		if (houseField[k][i - 1][j].house == HouseTypeBlock::None) { orientation = 'e'; corner++; }
		if (houseField[k][i + 1][j].house == HouseTypeBlock::None) { orientation = 'w'; corner++; }
		if (corner == 1) break;
		return false;
	}
	
	//	enough place front
	switch (orientation)
	{
		case 'w':
			if (houseField[k][i + 2][j].house != HouseTypeBlock::None) return false;
			if (houseField[k][i + 1][j + 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i + 1][j - 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i][j - 1].house != HouseTypeBlock::None && houseField[k][i][j - 1].house != HouseTypeBlock::House) return false;
			if (houseField[k][i][j + 1].house != HouseTypeBlock::None && houseField[k][i][j + 1].house != HouseTypeBlock::House) return false;
			break;

		case 's':
			if (houseField[k][i][j + 2].house != HouseTypeBlock::None) return false;
			if (houseField[k][i + 1][j + 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i - 1][j + 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i - 1][j].house != HouseTypeBlock::None && houseField[k][i - 1][j].house != HouseTypeBlock::House) return false;
			if (houseField[k][i + 1][j].house != HouseTypeBlock::None && houseField[k][i + 1][j].house != HouseTypeBlock::House) return false;
			break;

		case 'e':
			if (houseField[k][i - 2][j].house != HouseTypeBlock::None) return false;
			if (houseField[k][i - 1][j + 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i - 1][j - 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i][j - 1].house != HouseTypeBlock::None && houseField[k][i][j - 1].house != HouseTypeBlock::House) return false;
			if (houseField[k][i][j + 1].house != HouseTypeBlock::None && houseField[k][i][j + 1].house != HouseTypeBlock::House) return false;
			break;

		case 'n':
			if (houseField[k][i][j - 2].house != HouseTypeBlock::None) return false;
			if (houseField[k][i + 1][j - 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i - 1][j - 1].house != HouseTypeBlock::None) return false;
			if (houseField[k][i - 1][j].house != HouseTypeBlock::None && houseField[k][i - 1][j].house != HouseTypeBlock::House) return false;
			if (houseField[k][i + 1][j].house != HouseTypeBlock::None && houseField[k][i + 1][j].house != HouseTypeBlock::House) return false;
			break;

		default:
			return false;
	}

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

	benchmarkPosition.push_back(MarkedPosition(mark + (randomEngine() % std::max(s.x, s.y)), p, s));
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
char HouseGenerator::getRelativePosRoof(const int& ref, const unsigned int& i, const unsigned int& j, const unsigned int& k) const
{
	int pos = houseField[k][i][j].roof;
	if (pos < 0) return '1'; // out
	else if (ref - pos == 0)  return '0';
	else if (ref - pos == 1)  return '-';
	else if (ref - pos == -1) return '+';
	else return 'e';	// error
}


void HouseGenerator::pushMesh(Mesh* m, const glm::vec3& p, const glm::vec3& o, const glm::vec3& s)
{
	if (!m) return;
	unsigned int facesStart = (unsigned int)verticesArray.size();

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
	for (unsigned int i = 0; i < m->colors.size(); i++)
		colorArray.push_back(m->colors[i]);

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
	facesArray.push_back((unsigned short)verticesArray.size() - 4);
	facesArray.push_back((unsigned short)verticesArray.size() - 3);
	facesArray.push_back((unsigned short)verticesArray.size() - 2);

	facesArray.push_back((unsigned short)verticesArray.size() - 4);
	facesArray.push_back((unsigned short)verticesArray.size() - 2);
	facesArray.push_back((unsigned short)verticesArray.size() - 1);
}
//
