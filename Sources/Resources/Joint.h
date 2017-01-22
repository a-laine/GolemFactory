#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


struct Joint
{
	Joint() { parent = -1; name = "unknown joint"; };
	unsigned int parent;
	std::vector<unsigned int> sons;
	std::string name;
};

struct JointPose
{
	glm::vec3 position;
	glm::fquat orientation;
	glm::vec3 scale;
};

struct KeyFrame
{
	float time;
	std::vector<JointPose> poses;
};
