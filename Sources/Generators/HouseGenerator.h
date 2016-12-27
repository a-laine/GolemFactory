#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <string>


#include "Resources/ResourceManager.h"
#include "Instances/InstanceManager.h"

class HouseGenerator
{
    public:
        //  Default
		HouseGenerator();
        ~HouseGenerator();
        //

        //  Public functions
		InstanceVirtual* getHouse(unsigned int seed, int density = -1, int prosperity = -1);
        //

		//  Attributes
		static float floorHeight;
		static glm::vec3 wallColor;
		static glm::vec3 woodColor;
		static glm::vec3 doorColor;
		static glm::vec3 roofColor;
		static glm::vec3 glassColor;
		//
	
	protected:
		//  Miscellaneous
		enum VoxelType
		{
			None = 0,
			HouseEmpty,
			Door,
			Window
		};
		struct HouseVoxel
		{
			bool available;
			unsigned int voxelType;
		};
		//

		//  Protected functions
		inline void initHouseField(unsigned int newSize);
		bool addBlocks(int px, int py, int pz, int sx, int sy, int sz, unsigned int blockType = 0);
		void addBlocksNoCheck(int px, int py, int pz, int sx, int sy, int sz, unsigned int blockType = 0);
		inline Mesh* constructMesh(std::string meshName);

		void pushHQuad(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color);
		void pushVQuad(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color);
		void pushDoor(float px1, float py1, float px2, float py2, float pz, glm::vec3 color);
		void pushWindow(float px1, float py1, float px2, float py2, float pz, glm::vec3 color);
		void pushBox(glm::vec3 position, glm::vec3 size, glm::vec3 color);

		void pushRoofX(glm::vec3 position, glm::vec3 size, glm::vec3 color);
		void pushRoofY(glm::vec3 position, glm::vec3 size, glm::vec3 color);
		//

        //  Attributes
		unsigned int houseFieldSize;
		unsigned int houseFieldFloor;
		HouseVoxel*** houseField;

		std::vector<glm::vec3> verticesArray;
		std::vector<glm::vec3> normalesArray;
		std::vector<glm::vec3> colorArray;
		std::vector<unsigned int> facesArray;
		//
};
