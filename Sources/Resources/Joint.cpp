#include "Joint.h"

Joint::Joint(std::string jointName) : parent(-1), name(jointName)
{
	relativeBindTransform = glm::mat4(1.f);
}


JointPose::JointPose()
{
	position = glm::vec3(0.f, 0.f, 0.f);
	rotation = glm::fquat(1.f, 0.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);
}

