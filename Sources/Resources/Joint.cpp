#include "Joint.h"

Joint::Joint() : parent(-1), name("unknown joint")
{
	offsetPosition = glm::vec3(0.f, 0.f, 0.f);
	offsetOrientation = glm::fquat(0.f, 0.f, 0.f, 1.f);
	offsetScale = glm::vec3(1.f, 1.f, 1.f);
};

JointPose::JointPose()
{
	position = glm::vec3(0.f, 0.f, 0.f);
	orientation = glm::fquat(0.f, 0.f, 0.f, 1.f);
	scale = glm::vec3(1.f, 1.f, 1.f);
}

