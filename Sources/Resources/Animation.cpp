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
	std::cerr << "ERROR : loading animation : " << animationName << "\n" << extension << "format not yet implemented" << std::endl;
}
Animation::~Animation() {}
//

//	Public functions
bool Animation::isValid() const
{
	return !timeLine.empty();
}

std::vector<glm::mat4> Animation::getKeyPose(const unsigned int& keyFrame, const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4> pose(timeLine[0].poses.size(), glm::mat4(1.f));
	for (unsigned int i = 0; i < roots.size() && keyFrame < timeLine.size(); i++)
		computePose(keyFrame, pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
std::pair<int, int> Animation::getBoundingKeyFrameIndex(float time) const
{
	int previous = -1;
	int next = -1;
	for (unsigned int i = 0; i < timeLine.size(); i++)
	{
		if (timeLine[i].time >= time)
		{
			previous = i - 1;
			next = i;
			break;
		}
	}
	return std::pair<int, int>(previous, next);
}
//

//	Protected functions
void Animation::computePose(const unsigned int& keyFrame, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const
{
	glm::mat4 t = glm::translate(timeLine[keyFrame].poses[joint].position);
	glm::mat4 r = glm::toMat4(timeLine[keyFrame].poses[joint].rotation);
	glm::mat4 s = glm::scale(timeLine[keyFrame].poses[joint].scale);

	pose[joint] = parentPose *t * r * s;
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computePose(keyFrame, pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}
//