#include "Skeleton.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Skeleton::extension = ".skeleton";
//

//	Default
Skeleton::Skeleton(const std::string& skeletonName, const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList)
	: ResourceVirtual(skeletonName, ResourceVirtual::SKELETON), roots(rootsList), joints(jointsList)
{
	//	compute bind pose and inverse bind pose matrix lists
	inverseBindPose.assign(joints.size(), glm::mat4(1.f));
	bindPose.assign(joints.size(), glm::mat4(1.f));
	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(glm::mat4(1.f), roots[i]);
}

Skeleton::Skeleton(const std::string& path, const std::string& skeletonName) : ResourceVirtual(skeletonName, ResourceVirtual::SKELETON)
{
	std::cerr << "ERROR : loading skeleton : " << skeletonName << "\n" << extension << "format not yet implemented" << std::endl;
}
Skeleton::~Skeleton() {}
//

//	Public functions
bool Skeleton::isValid() const { return !(roots.empty() || joints.empty()); }

std::vector<glm::mat4> Skeleton::getInverseBindPose() const { return inverseBindPose; }
std::vector<glm::mat4> Skeleton::getBindPose() const { return bindPose; }
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