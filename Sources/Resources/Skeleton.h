#pragma once

#include <vector>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"
#include "Utiles/Parser/Variant.h"

class Skeleton : public ResourceVirtual
{
	friend class AnimationComponent;
	friend class SkeletonComponent;

	public:
		//	Default
		Skeleton(const std::string& skeletonName, const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList);
		Skeleton(const std::string& path, const std::string& skeletonName);
		~Skeleton();
		//

		//	Set/get functions
		bool isValid() const;

		std::vector<glm::mat4x4> getInverseBindPose() const;
		std::vector<glm::mat4x4> getBindPose() const;
		std::vector<Joint> getJoints() const;
		//

		//	Attributes
		static std::string extension;   //!< Default extension
		//

	protected:
		//	Protected functions
		void computeBindPose(const glm::mat4& parentPose, unsigned int joint);
		//

		//	Attributes
		std::vector<unsigned int> roots;
		std::vector<Joint> joints;

		std::vector<glm::mat4> inverseBindPose;
		std::vector<glm::mat4> bindPose;
		//
};
