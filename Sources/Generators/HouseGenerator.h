#pragma once

#include <vector>
#include <map>
#include <list>
#include <random>
/*#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>*/

#include "Math/TMath.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>

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
		static vec4f stoneColor;
		static vec4f groundRoof;
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
			vec3i position;
			vec3i size;
			bool operator<(const MarkedPosition& mp) { return mark < mp.mark; }
			MarkedPosition(float m, vec3i p, vec3i s) : mark(m), position(p), size(s) {}
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


		bool searchBlockPartition(const int& superficy, const int& testIndex);
		bool freePlace(const unsigned int& i, const unsigned int& j, const unsigned int& k) const;
		bool freeFloor(const unsigned int& k) const;
		bool markDecorative(const unsigned int& i, const unsigned int& j, const unsigned int& k) const;
		vec3i getRandomPosition(const int& safeOffset, const int& maxZ = 0);

		void markAll(const vec3i& p, const vec3i& s);
		float markFree(const vec3i& p, const vec3i& s) const;
		float markSupport(const vec3i& p, const vec3i& s) const;
		float markAdjacent(const vec3i& p, const vec3i& s) const;
		float markMassive(const vec3i& p, const vec3i& s) const;

		void addHouseBlocks(const vec3i& p, const vec3i& s, const int& houseType, const unsigned int& blockReference);
		void updateAvailableBlockPosition(const vec3i& p, const vec3i& s);
		char getRelativePosRoof(const int& ref, const unsigned int& i, const unsigned int& j, const unsigned int& k) const;

		void pushMesh(Mesh* m, const vec4f& p, const vec4f& o, const vec4f& s = vec4f(1.f));
		void pushGround(float px1, float py1, float pz1, float px2, float py2, float pz2, vec4f color);
		
		//



        //  Attributes
		int density, prosperity, superficy;
		int houseFieldSize;
		int houseFieldFloor;
		HouseVoxel*** houseField;
		vec4f houseOrigin;

		std::vector<vec4f> verticesArray;
		std::vector<vec4f> normalesArray;
		std::vector<vec4f> uvArray;
		std::vector<unsigned short> facesArray;

		std::map<std::string, Mesh*> assetLibrary;
		std::vector<vec3i> blockLibrary;

		std::vector<std::pair<vec3i, vec3i> > blockList;
		std::list<vec3i> availableBlockPosition;
		std::list<MarkedPosition> benchmarkPosition;

		std::vector<std::pair<vec3i, vec3i> > roofBlockList;

		std::mt19937 randomEngine;
		//
};
