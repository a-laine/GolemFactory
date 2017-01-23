#pragma once

#include <vector>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"

class Skeleton : public ResourceVirtual
{
	public:
		//	Default
		Skeleton(const std::string& skeletonName, const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList);
		Skeleton(const std::string& path, const std::string& skeletonName);
		~Skeleton();
		//

		//	Public functions
		bool isValid() const;

		std::vector<unsigned int> getRoots() const;
		std::vector<Joint> getJointHierarchy() const;
		//

		///	Debug
		void debug();
		//

		//	Attributes
		static std::string extension;   //!< Default extension
		//

	protected:
		//	Attributes
		std::vector<unsigned int> roots;
		std::vector<Joint> joints;
		//

		///	Debug
		void printJoint(unsigned int joint,int depth);
		//
};
