#include "Joint.h"

Joint::Joint() : parent(-1), name("unknown joint")
{
}

Joint::Joint(std::string n) : parent(-1), name(n)
{
}


JointPose::JointPose()
{
	position = glm::vec3(0.f, 0.f, 0.f);
	orientation = glm::fquat(1.f, 0.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);
}

