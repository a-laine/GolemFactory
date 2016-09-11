#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ResourceVirtual.h"
#include "Loader/MeshLoader.h"

class Skeleton : public ResourceVirtual
{
	public:
		//  Miscellaneous
		enum ConfigurationFlags
		{
			VALID = 1 << 0
		};

		struct Bone
		{
			unsigned int parent;
			std::vector<unsigned int> children;
		};

		struct BonePose
		{
			glm::vec3 position;
			glm::fquat orientation;
			glm::vec3 size;
		};
		//

		//	Default
		Skeleton(std::string path, std::string skeletonName);
		~Skeleton();
		//

		//	Public functions
		void addBone(const Bone& b);
		void addBone(unsigned int parent,unsigned int nbChildren, unsigned int* children);

		bool isValid() const;
		//

		//	Attributes
		static std::string extension;
		//

	protected:
		//	Attributes
		uint8_t configuration;
		std::vector<Bone> boneList;
		//
};
