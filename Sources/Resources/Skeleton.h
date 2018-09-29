#pragma once

#include <vector>

#include "ResourceVirtual.h"
#include "Joint.h"


class Skeleton : public ResourceVirtual
{
	friend class AnimationComponent;
	friend class SkeletonComponent;

	public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        //	Default
		Skeleton(const std::string& skeletonName);
		~Skeleton();
		//

        void initialize(const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList);
        void initialize(std::vector<unsigned int>&& rootsList, std::vector<Joint>&& jointsList);
		std::vector<glm::mat4x4> getInverseBindPose() const;
		std::vector<glm::mat4x4> getBindPose() const;
		std::vector<Joint> getJoints() const;
        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
		//

	protected:
        static std::string defaultName;

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
