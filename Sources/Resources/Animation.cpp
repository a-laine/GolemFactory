#include "Animation.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Animation::extension = ".animation";
//

//  Default
Animation::Animation(const std::string& animationName, const std::vector<KeyFrame>& animations)
	: ResourceVirtual(animationName, ResourceVirtual::ANIMATION), timeLine(animations)
{}

Animation::Animation(const std::string& path, const std::string& animationName) : ResourceVirtual(animationName, ResourceVirtual::ANIMATION)
{
	std::cerr << "Animation loading in GF format not supported yet" << std::endl;
}
Animation::~Animation() {}
//

//	Public functions
bool Animation::isValid() const
{
	return timeLine.empty();
}
std::vector<glm::mat4> Animation::getInverseBindPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy)
{
	if (inverseBindPose.empty())
	{
		for (unsigned int i = 0; i < hierarchy.size(); i++)
			inverseBindPose.push_back(hierarchy[i].inverseLocalBindTransform);
	}
	return inverseBindPose;
}
std::vector<glm::mat4> Animation::getBindPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4> pose(timeLine[0].poses.size());
	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
std::vector<glm::mat4> Animation::getPose(const unsigned int& keyFramePose, const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4> pose(timeLine[0].poses.size(), glm::mat4(1.f));
	for (unsigned int i = 0; i < roots.size() && keyFramePose < timeLine.size(); i++)
		computePose(keyFramePose, pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
//

//	Protected functions
void Animation::computeBindPose(std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const
{
	pose[joint] = parentPose * hierarchy[joint].relativeBindTransform;
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computeBindPose(pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}
void Animation::computePose(const unsigned int& keyFramePose, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const
{
	glm::mat4 t = glm::translate(timeLine[keyFramePose].poses[joint].position);
	glm::mat4 r = glm::toMat4(timeLine[keyFramePose].poses[joint].rotation);
	glm::mat4 s = glm::scale(timeLine[keyFramePose].poses[joint].scale);

	pose[joint] = parentPose *t * r * s;
	//pose[joint] = parentPose * hierarchy[joint].relativeBindTransform;

	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computePose(keyFramePose, pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}
//