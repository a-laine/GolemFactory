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

		struct Joint
		{
			glm::vec3 position;
			glm::fquat orientation;

			unsigned int parent;
			std::vector<unsigned int> sons;
			std::string name;
		};
		//

		//	Default
		Skeleton(std::string path, std::string skeletonName);
		~Skeleton();
		//

		//	Public functions
		bool isValid() const;
		void debug();
		//

		//	Attributes
		static std::string extension;
		//

	//protected:
		//	Attributes
		uint8_t configuration;
		unsigned int root;
		std::vector<Joint> jointList;
		//

		//
		void printJoint(unsigned int joint,int depth);
		//
};
