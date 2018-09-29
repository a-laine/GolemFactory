#pragma once

#include <vector>
#include <map>
#include <list>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Resources/ResourceManager.h>


/*
	TODO :
		- issue : bug in roof mesh generation : visible on house1612_99_99
		- issue : bug in roof mesh generation : visible on house519_20_30
*/


class Entity;

class HouseGenerator
{
    public:
        //  Default
		HouseGenerator();
        ~HouseGenerator();
        //

        //  Public functions
		void getHouse(Entity* house, unsigned int seed, int _density = -1, int _prosperity = -1);
        //

		//  Attributes
		static glm::vec3 stoneColor;
		static glm::vec3 groundRoof;
		static const int massiveRadius;
		//
	
	protected:
		//  Miscellaneous
		enum HouseTypeBlock
		{
			None = 0,
			House,
			Door,
			Window,
			Fireplace
		};
		struct HouseVoxel
		{
			int house;
			int blockReference;

			int roof;
			int roofReference;
		};
		struct MarkedPosition
		{
			float mark;
			glm::ivec3 position;
			glm::ivec3 size;
			bool operator<(const MarkedPosition& mp) { return mark < mp.mark; }
			MarkedPosition(float m, glm::ivec3 p, glm::ivec3 s) : mark(m), position(p), size(s) {}
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
		void initHouseField(const int& newSize);
		void createAndPlaceDebugHouseBlock(int config);
		void createAndPlaceHouseBlock();
		void createAndPlaceDecorationBlock();
		void createAndPlaceRoofBlock();
		void constructHouseMesh();
		void constructRoofMesh();
		void optimizeMesh();


		bool searchBlockPartition(const int& superficy, const int& testIndex);
		bool freePlace(const unsigned int& i, const unsigned int& j, const unsigned int& k) const;
		bool freeFloor(const unsigned int& k) const;
		bool markDecorative(const unsigned int& i, const unsigned int& j, const unsigned int& k) const;
		glm::ivec3 getRandomPosition(const int& safeOffset, const int& maxZ = 0);

		void markAll(const glm::ivec3& p, const glm::ivec3& s);
		float markFree(const glm::ivec3& p, const glm::ivec3& s) const;
		float markSupport(const glm::ivec3& p, const glm::ivec3& s) const;
		float markAdjacent(const glm::ivec3& p, const glm::ivec3& s) const;
		float markMassive(const glm::ivec3& p, const glm::ivec3& s) const;

		void addHouseBlocks(const glm::ivec3& p, const glm::ivec3& s, const int& houseType, const unsigned int& blockReference);
		void updateAvailableBlockPosition(const glm::ivec3& p, const glm::ivec3& s);
		char getRelativePosRoof(const int& ref, const unsigned int& i, const unsigned int& j, const unsigned int& k) const;

		void pushMesh(Mesh* m, const glm::vec3& p, const glm::vec3& o, const glm::vec3& s = glm::vec3(1.f, 1.f, 1.f));
		void pushGround(float px1, float py1, float pz1, float px2, float py2, float pz2, glm::vec3 color);
		
		//



        //  Attributes
		int density, prosperity, superficy;
		int houseFieldSize;
		int houseFieldFloor;
		HouseVoxel*** houseField;
		glm::vec3 houseOrigin;

		std::vector<glm::vec3> verticesArray;
		std::vector<glm::vec3> normalesArray;
		std::vector<glm::vec3> colorArray;
		std::vector<unsigned short> facesArray;

		std::map<std::string, Mesh*> assetLibrary;
		std::vector<glm::ivec3> blockLibrary;

		std::vector<std::pair<glm::ivec3, glm::ivec3> > blockList;
		std::list<glm::ivec3> availableBlockPosition;
		std::list<MarkedPosition> benchmarkPosition;

		std::vector<std::pair<glm::ivec3, glm::ivec3> > roofBlockList;

		std::mt19937 randomEngine;
		//
};
