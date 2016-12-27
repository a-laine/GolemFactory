#include "HouseGenerator.h"


//  Attributes
float HouseGenerator::floorHeight = 2.5f;
glm::vec3 HouseGenerator::wallColor = glm::vec3(0.36f, 0.23f, 0.15f);
glm::vec3 HouseGenerator::woodColor = glm::vec3(0.24f, 0.15f, 0.10f);
glm::vec3 HouseGenerator::doorColor = glm::vec3(0.36f, 0.23f, 0.15f);
glm::vec3 HouseGenerator::glassColor = glm::vec3(0.59f, 0.78f, 0.81f);
glm::vec3 HouseGenerator::roofColor = glm::vec3(0.49f, 0.31f, 0.20f);
//


//  Default
HouseGenerator::HouseGenerator()
{
	houseField = nullptr;
	houseFieldSize = 0;
	houseFieldFloor = 0;
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
	
	/*
	std::cout << "House name" << houseName << std::endl;
	std::cout << "   seed : " << seed << std::endl;
	std::cout << "   density : " << density << std::endl;
	std::cout << "   prosperity : " << prosperity << std::endl;
	std::cout << "   house field size : " << houseFieldSize << std::endl;
	std::cout << "   house field floor count : " << houseFieldFloor << std::endl;
	std::cout << "   superficy : " << superficy << std::endl;
	*/

	// Add dummy bloc for test house generator
	int ox = houseFieldSize / 2;
	int oy = houseFieldSize / 2;
	int oz = 0;
	addBlocksNoCheck(ox - 4, oy,     oz, 4, 4, 1, 1);		// house empty
	addBlocksNoCheck(ox,     oy - 1, oz, 6, 7, 2, 1);		// house empty
	addBlocksNoCheck(ox + 6, oy,     oz, 4, 4, 1, 1);		// house empty

	addBlocksNoCheck(ox + 3, oy + 5, oz, 1, 1, 1, 2);		// house door

	addBlocksNoCheck(ox + 1, oy + 5, oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox + 7, oy,     oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox + 7, oy + 3, oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox + 9, oy + 2, oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox - 2, oy,     oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox - 2, oy + 3, oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox - 4, oy + 2, oz, 1, 1, 1, 3);		// house window
	addBlocksNoCheck(ox + 1, oy + 5, oz + 1, 1, 1, 1, 3);	// house window
	addBlocksNoCheck(ox + 4, oy + 5, oz + 1, 1, 1, 1, 3);	// house window
	addBlocksNoCheck(ox + 4, oy - 1, oz + 1, 1, 1, 1, 3);	// house window


	//	Construct Mesh
	Mesh* m = constructMesh(houseName);
	//std::cout << "   vertex count : " << m->getNumberVertices() << std::endl;
	//std::cout << "   faces count : " << m->getNumberFaces() << std::endl;
	ResourceManager::getInstance()->addMesh(m);
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
bool HouseGenerator::addBlocks(int px, int py, int pz, int sx, int sy, int sz, unsigned int blockType)
{
	//	Check out of range
	if (px < 0 || py < 0 || pz < 0)
		return false;
	else if (px + sx > (int)houseFieldSize || py + sy >(int)houseFieldSize || pz + sz >(int)houseFieldFloor)
		return false;
	else if (!blockType)
		return false;

	//	Check available
	for (int k = pz; k < pz + sz; k++)
		for (int i = px; i < px + sx; i++)
			for (int j = py; j < py + sy; j++)
				if(!houseField[k][i][j].available) return false;

	//	Place block
	for (int k = pz; k < pz + sz; k++)
		for (int i = px; i < px + sx; i++)
			for (int j = py; j < py + sy; j++)
			{
				houseField[k][i][j].available = false;
				houseField[k][i][j].voxelType = blockType;
			}
	return true;
}
void HouseGenerator::addBlocksNoCheck(int px, int py, int pz, int sx, int sy, int sz, unsigned int blockType)
{
	for (int k = pz; k < pz + sz; k++)
		for (int i = px; i < px + sx; i++)
			for (int j = py; j < py + sy; j++)
			{
				houseField[k][i][j].available = false;
				houseField[k][i][j].voxelType = blockType;
			}
}
inline Mesh* HouseGenerator::constructMesh(std::string meshName)
{	
	float ox = houseFieldSize / 2.f + 0.5f;
	float oy = houseFieldSize / 2.f + 0.5f;
	float oz = 0.0f;

	for (unsigned int k = 0; k < houseFieldFloor; k++)
	{
		//	house
		for (unsigned int i = 0; i < houseFieldSize; i++)
			for (unsigned int j = 0; j < houseFieldSize; j++)
			{
				if (houseField[k][i][j].available) continue;

				//	Ground quad
				pushHQuad(i - ox, j - oy, floorHeight * k + oz, i + 1 - ox, j + 1 - oy, floorHeight * k + oz, wallColor);

				//	right wall
				if (i + 1 < houseFieldSize && houseField[k][i+1][j].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty:
							pushVQuad( i + 1 - ox, j - oy, floorHeight * k + oz, i + 1 - ox, j + 1 - oy, floorHeight * (k + 1) + oz, wallColor);
							break;
						case Door:
							pushDoor(  i + 1 - ox, j - oy,     i + 1 - ox, j + 1 - oy, floorHeight * k + oz, wallColor);
							break;
						case Window:
							pushWindow(i + 1 - ox, j - oy, i + 1 - ox, j + 1 - oy,     floorHeight * k + oz, wallColor);
							break;
						default:
							break;
					}
				}

				//	left wall
				if (i - 1 >= 0 && houseField[k][i - 1][j].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty:
							pushVQuad (i - ox, j + 1 - oy, floorHeight * (k + 1) + oz, i - ox, j - oy, floorHeight * k + oz, wallColor);
							break;
						case Door:
							pushDoor(  i - ox, j - oy,     i - ox, j + 1 - oy, floorHeight * k + oz, wallColor);
							break;
						case Window:
							pushWindow(i - ox, j + 1 - oy, i - ox, j - oy,     floorHeight * k + oz, wallColor);
							break;
						default:
							break;
					}
				}
					
				//	lower wall
				if (j + 1 < houseFieldSize && houseField[k][i][j + 1].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty:
							pushVQuad(i - ox, j + 1 - oy, floorHeight * k + oz, i + 1 - ox, j + 1 - oy, floorHeight * (k + 1) + oz, wallColor);
							break;
						case Door:
							pushDoor(i + 1 - ox, j + 1 - oy, i - ox, j + 1 - oy, floorHeight * k + oz, wallColor);
							break;
						case Window:
							pushWindow(i - ox, j + 1 - oy, i + 1 - ox, j + 1 - oy, floorHeight * k + oz, wallColor);
							break;
						default:
							break;
					}
				}

				//	upper wall
				if (j - 1 >= 0 && houseField[k][i][j - 1].available)
				{
					switch (houseField[k][i][j].voxelType)
					{
						case HouseEmpty:
							pushVQuad(i + 1 - ox, j - oy, floorHeight * k + oz, i - ox, j - oy, floorHeight * (k + 1) + oz, wallColor);
							break;
						case Door:
							pushDoor(i + 1 - ox, j - oy, i - ox, j - oy, floorHeight * k + oz, wallColor);
							break;
						case Window:
							pushWindow(i + 1 - ox, j - oy, i - ox, j - oy, floorHeight * k + oz, wallColor);
							break;
						default:
							break;
					}
				}

				//	inner roof
				if (k + 1 < houseFieldFloor && houseField[k + 1][i][j].available)
					pushHQuad(i - ox, j - oy, floorHeight * (k + 1) + oz, i + 1 - ox, j + 1 - oy, floorHeight * (k + 1) + oz, wallColor);
				else if (k + 1 == houseFieldFloor)
					pushHQuad(i - ox, j - oy, floorHeight * (k + 1) + oz, i + 1 - ox, j + 1 - oy, floorHeight * (k + 1) + oz, wallColor);
			}
	}

	pushRoofX(glm::vec3(-3, 1, 2.5f), glm::vec3(2, 2, 2), roofColor);
	pushRoofX(glm::vec3( 7, 1, 2.5f), glm::vec3(2, 2, 2), roofColor);
	pushRoofY(glm::vec3( 2, 1.5f, 5.f),  glm::vec3(3, 3.5f, 3), roofColor);

	return new Mesh(meshName, verticesArray, normalesArray, colorArray, facesArray);
}


void HouseGenerator::pushHQuad(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color)
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
void HouseGenerator::pushVQuad(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color)
{
	verticesArray.push_back(glm::vec3(px1, py1, pz1));
	verticesArray.push_back(glm::vec3(px2, py2, pz1));
	verticesArray.push_back(glm::vec3(px2, py2, pz2));
	verticesArray.push_back(glm::vec3(px1, py1, pz2));

	glm::vec3 n = glm::cross(glm::normalize(glm::vec3(px2 - px1, py2 - py1, pz2 - pz1)),
		glm::normalize(glm::vec3(0.f, 0.f, 1.f)));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
}
void HouseGenerator::pushDoor(float px1, float py1, float px2, float py2, float pz, glm::vec3 color)
{
	glm::vec3 orig(px1, py1, pz);
	glm::vec3 h = glm::vec3(px2 - px1, py2 - py1, 0.0f);		//	local horizontal vector
	glm::vec3 v = glm::normalize(glm::vec3(0.0f, 0.0f, 1.f));	//	local vertical
	glm::vec3 t = glm::cross(v, h);

	//	wall
		verticesArray.push_back(orig + 0.00f * h + 0.00f * v);
		verticesArray.push_back(orig + 0.05f * h + 0.00f * v);
		verticesArray.push_back(orig + 0.05f * h + 1.90f * v);
		verticesArray.push_back(orig + 0.00f * h + 1.90f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
		//
		verticesArray.push_back(orig + 0.95f * h + 0.00f * v);
		verticesArray.push_back(orig + 1.00f * h + 0.00f * v);
		verticesArray.push_back(orig + 1.00f * h + 1.90f * v);
		verticesArray.push_back(orig + 0.95f * h + 1.90f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
		//
		verticesArray.push_back(orig + 0.00f * h + 2.00f * v);
		verticesArray.push_back(orig + 1.00f * h + 2.00f * v);
		verticesArray.push_back(orig + 1.00f * h + floorHeight * v);
		verticesArray.push_back(orig + 0.00f * h + floorHeight * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	
	//	door
		verticesArray.push_back(orig + 0.10f * h + 0.00f * v);
		verticesArray.push_back(orig + 0.90f * h + 0.00f * v);
		verticesArray.push_back(orig + 0.90f * h + 1.90f * v);
		verticesArray.push_back(orig + 0.10f * h + 1.90f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(doorColor); colorArray.push_back(doorColor); colorArray.push_back(doorColor); colorArray.push_back(doorColor);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);

	//	wood part
		pushBox(orig + 0.5f * h   + 1.95f * v, 0.5f * h   + 0.1f * t  + 0.05f * v,  woodColor);
		pushBox(orig + 0.075f * h + 0.95f * v, 0.025f * h + 0.07f * t + 0.95f * v,  woodColor);
		pushBox(orig + 0.925f * h + 0.95f * v, 0.025f * h + 0.07f * t + 0.95f * v,  woodColor);
		pushBox(orig + 0.8f * h   + 1.00f * v, 0.025f * h + 0.05f * t + 0.025f * v, glm::vec3(0.1f, 0.1f, 0.1f));
}
void HouseGenerator::pushWindow(float px1, float py1, float px2, float py2, float pz, glm::vec3 color)
{
	glm::vec3 orig(px1, py1, pz);
	glm::vec3 h = glm::vec3(px2 - px1, py2 - py1, 0.0f);		//	local horizontal vector
	glm::vec3 v = glm::normalize(glm::vec3(0.0f, 0.0f, 1.f));	//	local vertical
	glm::vec3 t = glm::cross(h, v);

	//	wall
		verticesArray.push_back(orig + 0.00f * h + 0.00f * v);
		verticesArray.push_back(orig + 1.00f * h + 0.00f * v);
		verticesArray.push_back(orig + 1.00f * h + 0.90f * v);
		verticesArray.push_back(orig + 0.00f * h + 0.90f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
		//
		verticesArray.push_back(orig + 0.00f * h + 2.10f * v);
		verticesArray.push_back(orig + 1.00f * h + 2.10f * v);
		verticesArray.push_back(orig + 1.00f * h + 2.50f * v);
		verticesArray.push_back(orig + 0.00f * h + 2.50f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
		//
		verticesArray.push_back(orig + 0.00f * h  + 1.00f * v);
		verticesArray.push_back(orig + 0.025f * h + 1.00f * v);
		verticesArray.push_back(orig + 0.025f * h + 2.00f * v);
		verticesArray.push_back(orig + 0.00f * h  + 2.00f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
		//
		verticesArray.push_back(orig + 0.975f * h + 1.00f * v);
		verticesArray.push_back(orig + 1.00f * h  + 1.00f * v);
		verticesArray.push_back(orig + 1.00f * h  + 2.00f * v);
		verticesArray.push_back(orig + 0.975f * h + 2.00f * v);

		normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);  normalesArray.push_back(t);
		colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);

	//	wood
		pushBox(orig + 0.5f * h + 0.95f * v, 0.5f * h   + 0.10f * t + 0.05f * v, woodColor);
		pushBox(orig + 0.5f * h + 2.05f * v, 0.5f * h   + 0.10f * t + 0.05f * v, woodColor);
		pushBox(orig + 0.05f * h + 1.5f * v, 0.025f * h + 0.07f * t + 0.50f * v, woodColor);
		pushBox(orig + 0.95f * h + 1.5f * v, 0.025f * h + 0.07f * t + 0.50f * v, woodColor);

		pushBox(orig + 0.125f * h + 1.5f * v, 0.05f * h + 0.02f * t + 0.50f * v, woodColor);
		pushBox(orig + 0.875f * h + 1.5f * v, 0.05f * h + 0.02f * t + 0.50f * v, woodColor);
		pushBox(orig + 0.5f * h + 1.5f * v, 0.05f * h + 0.02f * t + 0.50f * v, woodColor);

		pushBox(orig + 0.5f * h + 1.35f * v, 0.35f * h + 0.02f * t + 0.05f * v, woodColor);
		pushBox(orig + 0.5f * h + 1.65f * v, 0.35f * h + 0.02f * t + 0.05f * v, woodColor);
		pushBox(orig + 0.5f * h + 1.975f * v, 0.35f * h + 0.02f * t + 0.025f * v, woodColor);
		pushBox(orig + 0.5f * h + 1.025f * v, 0.35f * h + 0.02f * t + 0.025f * v, woodColor);

	//	glass
		verticesArray.push_back(orig + 0.875f * h + 1.05f * v);
		verticesArray.push_back(orig + 0.15f * h + 1.05f * v);
		verticesArray.push_back(orig + 0.15f * h + 2.05f * v);
		verticesArray.push_back(orig + 0.875f * h + 2.05f * v);

		normalesArray.push_back(t);		  normalesArray.push_back(t);		normalesArray.push_back(t);		  normalesArray.push_back(t);
		colorArray.push_back(glassColor); colorArray.push_back(glassColor); colorArray.push_back(glassColor); colorArray.push_back(glassColor);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
		facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
}
void HouseGenerator::pushBox(glm::vec3 position, glm::vec3 size, glm::vec3 color)
{
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, -size.z));
	verticesArray.push_back(position + glm::vec3(-size.x,  size.y, -size.z));
	verticesArray.push_back(position + glm::vec3(-size.x,  size.y,  size.z));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y,  size.z));

	normalesArray.push_back(glm::vec3(-1.f, 0.0f,0.0f));		normalesArray.push_back(glm::vec3(-1.f, 0.0f, 0.0f));
	normalesArray.push_back(glm::vec3(-1.f, 0.0f, 0.0f));		normalesArray.push_back(glm::vec3(-1.f, 0.0f, 0.0f));
	colorArray.push_back(color); colorArray.push_back(color);	colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(size.x, -size.y, -size.z));
	verticesArray.push_back(position + glm::vec3(size.x, size.y, -size.z));
	verticesArray.push_back(position + glm::vec3(size.x, size.y, size.z));
	verticesArray.push_back(position + glm::vec3(size.x, -size.y, size.z));

	normalesArray.push_back(glm::vec3(1.f, 0.0f, 0.0f));		normalesArray.push_back(glm::vec3(1.f, 0.0f, 0.0f));
	normalesArray.push_back(glm::vec3(1.f, 0.0f, 0.0f));		normalesArray.push_back(glm::vec3(1.f, 0.0f, 0.0f));
	colorArray.push_back(color); colorArray.push_back(color);	colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, size.y, -size.z));
	verticesArray.push_back(position + glm::vec3( size.x, size.y, -size.z));
	verticesArray.push_back(position + glm::vec3( size.x, size.y, size.z));
	verticesArray.push_back(position + glm::vec3(-size.x, size.y, size.z));

	normalesArray.push_back(glm::vec3(0.f, 1.0f, 0.0f));		normalesArray.push_back(glm::vec3(0.f, 1.0f, 0.0f));
	normalesArray.push_back(glm::vec3(0.f, 1.0f, 0.0f));		normalesArray.push_back(glm::vec3(0.f, 1.0f, 0.0f));
	colorArray.push_back(color); colorArray.push_back(color);	colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, -size.z));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y, -size.z));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y,  size.z));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y,  size.z));

	normalesArray.push_back(glm::vec3(0.f, -1.0f, 0.0f));		normalesArray.push_back(glm::vec3(0.f, -1.0f, 0.0f));
	normalesArray.push_back(glm::vec3(0.f, -1.0f, 0.0f));		normalesArray.push_back(glm::vec3(0.f, -1.0f, 0.0f));
	colorArray.push_back(color); colorArray.push_back(color);	colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x,  size.y, size.z));
	verticesArray.push_back(position + glm::vec3( size.x,  size.y, size.z));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y, size.z));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, size.z));

	normalesArray.push_back(glm::vec3(0.f, 0.0f, 1.0f));		normalesArray.push_back(glm::vec3(0.f, 0.0f, 1.0f));
	normalesArray.push_back(glm::vec3(0.f, 0.0f, 1.0f));		normalesArray.push_back(glm::vec3(0.f, 0.0f, 1.0f));
	colorArray.push_back(color); colorArray.push_back(color);	colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x,  size.y, -size.z));
	verticesArray.push_back(position + glm::vec3( size.x,  size.y, -size.z));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y, -size.z));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, -size.z));

	normalesArray.push_back(glm::vec3(0.f, 0.0f, -1.0f));		normalesArray.push_back(glm::vec3(0.f, 0.0f, -1.0f));
	normalesArray.push_back(glm::vec3(0.f, 0.0f, -1.0f));		normalesArray.push_back(glm::vec3(0.f, 0.0f, -1.0f));
	colorArray.push_back(color); colorArray.push_back(color);	colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
}


void HouseGenerator::pushRoofX(glm::vec3 position, glm::vec3 size, glm::vec3 color)
{
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x,  0,  size.z));
	verticesArray.push_back(position + glm::vec3(-size.x,  0,  size.z));

	glm::vec3 n = glm::normalize(glm::vec3(0.f, size.z, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, 0, size.z));
	verticesArray.push_back(position + glm::vec3(-size.x, 0, size.z));

	n = glm::normalize(glm::vec3(0.f, -size.z, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	out upper
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y - 0.3f, 0));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y - 0.3f, 0));

	n = glm::normalize(glm::vec3(0.f, 0.f, -1.f));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	out lower
	verticesArray.push_back(position + glm::vec3(-size.x, size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x, size.y + 0.3f, 0));
	verticesArray.push_back(position + glm::vec3(-size.x, size.y + 0.3f, 0));

	n = glm::normalize(glm::vec3(0.f, 0.f, -1.f));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	side upper down
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y - 0.1f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y - 0.1f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, -0.2f, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3(-size.x, -0.2f, size.z + 0.1f));

	n = glm::normalize(glm::vec3(0.f, size.z, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	side lower down
	verticesArray.push_back(position + glm::vec3(-size.x, size.y + 0.1f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, size.y + 0.1f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, 0.2f, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3(-size.x, 0.2f, size.z + 0.1f));

	n = glm::normalize(glm::vec3(0.f, -size.z, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	Top
	verticesArray.push_back(position + glm::vec3(-size.x, -0.2f, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3( size.x, -0.2f, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3( size.x, 0.2f,  size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3(-size.x, 0.2f,  size.z + 0.1f));

	n = glm::vec3(0.f, 0.f, 1.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y - 0.3f, 0.0f));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y - 0.3f, 0.0f));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y - 0.3f, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y - 0.3f, 0.2f));

	n = glm::vec3(0.f, -1.f, 0.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, size.y + 0.3f, 0.0f));
	verticesArray.push_back(position + glm::vec3( size.x, size.y + 0.3f, 0.0f));
	verticesArray.push_back(position + glm::vec3( size.x, size.y + 0.3f, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x, size.y + 0.3f, 0.2f));

	n = glm::vec3(0.f, 1.f, 0.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y - 0.3f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y - 0.3f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, -size.y - 0.1f, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y - 0.1f, 0.2f));

	n = glm::vec3(0.f, 0.f, 1.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x, size.y + 0.3f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, size.y + 0.3f, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x, size.y + 0.1f, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x, size.y + 0.1f, 0.2f));

	n = glm::vec3(0.f, 0.f, 1.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
}
void HouseGenerator::pushRoofY(glm::vec3 position, glm::vec3 size, glm::vec3 color)
{
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3(-size.x,  size.y, 0));
	verticesArray.push_back(position + glm::vec3( 0,       size.y, size.z));
	verticesArray.push_back(position + glm::vec3( 0,      -size.y, size.z));

	glm::vec3 n = glm::normalize(glm::vec3(-size.z, 0.f, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3( size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3( size.x,  size.y, 0));
	verticesArray.push_back(position + glm::vec3( 0, size.y, size.z));
	verticesArray.push_back(position + glm::vec3( 0, -size.y, size.z));

	n = glm::normalize(glm::vec3(size.z, 0.f, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	out upper
	verticesArray.push_back(position + glm::vec3(-size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3(-size.x, size.y, 0));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f, size.y, 0));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f, -size.y, 0));

	n = glm::normalize(glm::vec3(0.f, 0.f, -1.f));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	out lower
	verticesArray.push_back(position + glm::vec3(size.x, -size.y, 0));
	verticesArray.push_back(position + glm::vec3(size.x, size.y, 0));
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f, size.y, 0));
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f, -size.y, 0));

	n = glm::normalize(glm::vec3(0.f, 0.f, -1.f));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	side upper down
	verticesArray.push_back(position + glm::vec3(-size.x - 0.1f,-size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x- 0.1f,size.y , 0.2f));
	verticesArray.push_back(position + glm::vec3(-0.2f,  size.y, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3(-0.2f, -size.y, size.z + 0.1f));

	n = glm::normalize(glm::vec3(-size.z, 0.f, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	side lower down
	verticesArray.push_back(position + glm::vec3( size.x + 0.1f, -size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3( size.x + 0.1f, size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3( 0.2f, size.y, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3( 0.2f, -size.y, size.z + 0.1f));

	n = glm::normalize(glm::vec3(size.z, 0.f, size.y));
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//	Top
	verticesArray.push_back(position + glm::vec3(-0.2f, -size.y, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3(-0.2f,  size.y, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3( 0.2f,  size.y, size.z + 0.1f));
	verticesArray.push_back(position + glm::vec3( 0.2f, -size.y, size.z + 0.1f));

	n = glm::vec3(0.f, 0.f, 1.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f, -size.y, 0.0f));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f,  size.y, 0.0f));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f,  size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f, -size.y, 0.2f));

	n = glm::vec3(-1.f, 0.f, 0.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f, -size.y, 0.0f));
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f,  size.y, 0.0f));
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f,  size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f, -size.y, 0.2f));

	n = glm::vec3(1.f, 0.f, 0.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f, -size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.3f,  size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.1f,  size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(-size.x - 0.1f, -size.y, 0.2f));

	n = glm::vec3(0.f, 0.f, 1.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
	//
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f,-size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(size.x + 0.3f, size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(size.x + 0.1f, size.y, 0.2f));
	verticesArray.push_back(position + glm::vec3(size.x + 0.1f,-size.y, 0.2f));

	n = glm::vec3(0.f, 0.f, 1.f);
	normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);  normalesArray.push_back(n);
	colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color); colorArray.push_back(color);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 3); facesArray.push_back(verticesArray.size() - 2);
	facesArray.push_back(verticesArray.size() - 4); facesArray.push_back(verticesArray.size() - 2); facesArray.push_back(verticesArray.size() - 1);
}
//
