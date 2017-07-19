#pragma once

#include <vector>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"

class Skeleton : public ResourceVirtual
{
	friend class Renderer;
	public:
		//	Default
		Skeleton(const std::string& skeletonName, const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList, const glm::mat4& globalMatrix);
		Skeleton(const std::string& path, const std::string& skeletonName);
		~Skeleton();
		//

		//	Public functions
		bool isValid() const;
		//

		///	Debug
		void debug();
		//

		//	Attributes
		static std::string extension;   //!< Default extension
		//

	protected:
		//	Attributes
		glm::mat4 global;
		std::vector<unsigned int> roots;
		std::vector<Joint> joints;
		//

		///	Debug
		void printJoint(unsigned int joint,int depth);

		//
		void computeLocalBindTransform(unsigned int joint, glm::mat4 localParentTransform);
		//
};
