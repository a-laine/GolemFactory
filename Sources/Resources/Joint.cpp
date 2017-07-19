#include "Joint.h"

Joint::Joint(std::string n, glm::mat4 offsetMatrix) : parent(-1), name(n), inverseLocalBindTransform(offsetMatrix)
{
	relativeBindTransform = glm::mat4(1.f);
}


JointPose::JointPose()
{
	position = glm::vec3(0.f, 0.f, 0.f);
	orientation = glm::fquat(1.f, 0.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);
}

