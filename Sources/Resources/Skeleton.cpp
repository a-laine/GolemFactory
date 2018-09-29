#include "Skeleton.h"

#include <Utiles/Assert.hpp>
#include <Resources/SkeletonLoader.h>

//  Static attributes
char const * const Skeleton::directory = "Skeletons/";
char const * const Skeleton::extension = ".skeleton";
std::string Skeleton::defaultName;
//

//	Default
Skeleton::Skeleton(const std::string& skeletonName) : ResourceVirtual(skeletonName) {}
Skeleton::~Skeleton() {}
//


void Skeleton::initialize(const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList)
{
    GF_ASSERT(state == INVALID);
    if(!rootsList.empty() && !jointsList.empty())
    {
        state = LOADING;

        roots = rootsList;
        joints = jointsList;

        //	compute bind pose and inverse bind pose matrix lists
        inverseBindPose.assign(joints.size(), glm::mat4(1.f));
        bindPose.assign(joints.size(), glm::mat4(1.f));
        for(unsigned int i = 0; i < roots.size(); i++)
            computeBindPose(glm::mat4(1.f), roots[i]);

        state = VALID;
    }
}

void Skeleton::initialize(std::vector<unsigned int>&& rootsList, std::vector<Joint>&& jointsList)
{
    GF_ASSERT(state == INVALID);
    if(!rootsList.empty() && !jointsList.empty())
    {
        state = LOADING;

        roots = std::move(rootsList);
        joints = std::move(jointsList);

        //	compute bind pose and inverse bind pose matrix lists
        inverseBindPose.assign(joints.size(), glm::mat4(1.f));
        bindPose.assign(joints.size(), glm::mat4(1.f));
        for(unsigned int i = 0; i < roots.size(); i++)
            computeBindPose(glm::mat4(1.f), roots[i]);

        state = VALID;
    }
}

std::string Skeleton::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
std::string Skeleton::getIdentifier() const
{
    return getIdentifier(name);
}

std::string Skeleton::getLoaderId(const std::string& resourceName) const
{
    return extension;
}

const std::string& Skeleton::getDefaultName() { return defaultName; }
void Skeleton::setDefaultName(const std::string& name) { defaultName = name; }

std::vector<glm::mat4> Skeleton::getInverseBindPose() const { return inverseBindPose; }
std::vector<glm::mat4> Skeleton::getBindPose() const { return bindPose; }
std::vector<Joint> Skeleton::getJoints() const { return joints; }
//

//	Private functions
void Skeleton::computeBindPose(const glm::mat4& parentPose, unsigned int joint)
{
	bindPose[joint] = parentPose * joints[joint].relativeBindTransform;
	inverseBindPose[joint] = glm::inverse(bindPose[joint]);
	
	for (unsigned int i = 0; i < joints[joint].sons.size(); i++)
		computeBindPose(bindPose[joint], joints[joint].sons[i]);
}
//