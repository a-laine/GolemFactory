#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <random>
#include <algorithm>

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
		static glm::vec3 stoneColor;
		static glm::vec3 groundRoof;
		static const int massiveRadius;
		static const int adjacentOptiRadius;
		//
	
	protected:
		//  Miscellaneous
		enum HouseType
		{
			None = 0,
			HouseEmpty,
			Door,
			Window
		};
		enum RoofType
		{
			Slope_xp,
			Slope_xm,
			Slope_yp,
			Slope_ym
		};
		struct HouseVoxel
		{
			bool available;
			unsigned int houseType;
			unsigned int roofType;
			unsigned int blockReference;
		};
		struct OrderedVertex
		{
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 color;

			int inf(const glm::vec3& l, const glm::vec3& r) const
			{
				//	return 1 if equals, 2 if l < r and 3 if l > r
				if (l.x != r.x) return (l.x < r.x ? 2 : 3);
				if (l.y != r.y) return (l.y < r.y ? 2 : 3);
				if (l.z != r.z) return (l.z < r.z ? 2 : 3);
				return 1;
			}
			bool operator< (const OrderedVertex& r) const
			{
				int res = inf(position, r.position);
				if (res == 2) return true;
				if (res == 3) return false;
				
				res = inf(normal, r.normal);
				if (res == 2) return true;
				if (res == 3) return false;

				res = inf(color, r.color);
				if (res == 2) return true;
				return false;
			};
		};
		//

		//  Protected functions
		void initHouseField(const int& newSize, const int& density, const int& prosperity);
		
		bool searchBlockPartition(const int& superficy, const int& testIndex);
		bool freePlace(const glm::ivec3& p, const glm::ivec3& s) const;
		bool supportedBlock(const glm::ivec3& p, const glm::ivec3& s) const;
		bool massiveStruct(const glm::ivec3& p, const glm::ivec3& s) const;
		int adjacentBlock(const glm::ivec3& p, const glm::ivec3& s) const;
		glm::ivec3 optimizeAdjacent(const glm::ivec3& p, const glm::ivec3& s) const;
		void addHouseBlocks(const glm::ivec3& p, const glm::ivec3& s, const unsigned int& houseType, const unsigned int& blockReference);


			
		//
		void constructHouseMesh();
			void pushMesh(Mesh* m, const glm::vec3& p, const glm::vec3& o, const glm::vec3& s = glm::vec3(1.f, 1.f,1.f));
			void pushGround(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color);
		//
		inline void createAndPlaceRoof();
			float benchmarkRoof(glm::ivec3 p, glm::ivec3 s);
		//
		inline void constructRoofMesh();
			void roofSlope(const glm::ivec3& p, const glm::ivec3& s);
			void roofEnd(const glm::ivec3& p, const glm::ivec3& s);
		//
		inline void optimizeMesh();
		//

        //  Attributes
		unsigned int houseFieldSize;
		unsigned int houseFieldFloor;
		HouseVoxel*** houseField;
		glm::vec3 houseOrigin;

		std::vector<glm::vec3> verticesArray;
		std::vector<glm::vec3> normalesArray;
		std::vector<glm::vec3> colorArray;
		std::vector<unsigned int> facesArray;

		std::map<std::string, Mesh*> assetLibrary;
		std::vector<glm::ivec3> blockLibrary;
		std::vector<std::pair<glm::ivec3, glm::ivec3> > blockList;
		std::vector<glm::ivec3> availableBlockPosition;
		std::vector<std::pair<glm::ivec3, glm::ivec3> > roofBlockList;

		std::mt19937 randomEngine;
		//
};
