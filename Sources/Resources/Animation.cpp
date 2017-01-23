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
Animation::~Animation()
{

}
//

//	Public functions
bool Animation::isValid() const
{
	return timeLine.empty();
}
std::vector<glm::mat4x4> Animation::getBindPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<glm::mat4x4> pose(timeLine[0].poses.size());
	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(pose, glm::mat4x4(1.f), roots[i], hierarchy);
	return std::vector<glm::mat4x4>();
}
//

//	Protected functions
void Animation::computeBindPose(std::vector<glm::mat4x4>& pose, const glm::mat4x4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const
{
	glm::mat4 t = glm::translate(timeLine[0].poses[joint].position);
	glm::mat4 s = glm::scale(timeLine[0].poses[joint].scale);
	glm::mat4 o = glm::toMat4(timeLine[0].poses[joint].orientation);
	pose[joint] = parentPose * s * t * o;
	
	//for (int i = 0; i < 4; i++)
	//	std::cout << pose[joint][0][i] << ' ' << pose[joint][1][i] << ' ' << pose[joint][2][i] << ' ' << pose[joint][3][i] << std::endl;


	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computeBindPose(pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}
//
