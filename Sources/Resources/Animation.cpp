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
std::vector<glm::mat4> Animation::getBindPose(int frame, const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	//std::cout << frame << std::endl;

	std::vector<glm::mat4> pose(timeLine[0].poses.size());
	for (unsigned int i = 0; i < roots.size(); i++)
		computeBindPose(frame, pose, glm::mat4(1.f), roots[i], hierarchy);
	return pose;
}
//

//	Protected functions
void Animation::computeBindPose(int frame, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy, int depth) const
{
	glm::mat4 t = glm::translate(timeLine[0].poses[joint].position);
	glm::mat4 o = glm::toMat4(timeLine[frame].poses[joint].orientation);
	glm::mat4 s = glm::scale(timeLine[frame].poses[joint].scale);
	pose[joint] = parentPose * t * o * s;
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < depth; j++)
			std::cout << "  ";
		std::cout << pose[joint][0][i] << ' ' << pose[joint][1][i] << ' ' << pose[joint][2][i] << ' ' << pose[joint][3][i] << std::endl;
	}
	std::cout << std::endl;
	
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computeBindPose(frame, pose, pose[joint], hierarchy[joint].sons[i], hierarchy, depth+1);
}
//
