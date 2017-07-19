#include "Animation.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Animation::extension = ".animation";
//

//  Default
Animation::Animation(const std::string& animationName, const std::vector<KeyFrame>& animations)
	: ResourceVirtual(animationName, ResourceVirtual::ANIMATION), timeLine(animations)
{
	debugframe = 0;
}

Animation::Animation(const std::string& path, const std::string& animationName) : ResourceVirtual(animationName, ResourceVirtual::ANIMATION)
{
	std::cerr << "Animation loading in GF format not supported yet" << std::endl;
}
Animation::~Animation()
{

}
//

//	Public functions
bool Animation::isValid() const
{
	return timeLine.empty();
}
std::vector<glm::mat4> Animation::getBindPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4> pose(timeLine[0].poses.size());
	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
std::vector<glm::mat4> Animation::getPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4> pose(timeLine[0].poses.size());
	for (unsigned int i = 0; i < roots.size(); i++)
		computePose(pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
//

//	Protected functions
void Animation::computePose(std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy, int depth) const
{
	glm::mat4 t = glm::translate(timeLine[debugframe].poses[joint].position);
	glm::mat4 r = glm::toMat4(timeLine[debugframe].poses[joint].orientation);
	glm::mat4 s = glm::scale(timeLine[debugframe].poses[joint].scale);

	pose[joint] = parentPose * t * r * s;

	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computePose(pose, pose[joint], hierarchy[joint].sons[i], hierarchy, depth + 1);
}
void Animation::computeBindPose(std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy, int depth) const
{
	pose[joint] = hierarchy[joint].inverseLocalBindTransform;
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computeBindPose(pose, pose[joint], hierarchy[joint].sons[i], hierarchy, depth + 1);
}