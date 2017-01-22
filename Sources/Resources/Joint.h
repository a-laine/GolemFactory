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
	glm::vec3 position;
	glm::fquat orientation;

	unsigned int parent;
	std::vector<unsigned int> sons;
	std::string name;
};
