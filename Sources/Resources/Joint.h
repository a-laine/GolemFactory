#pragma once

#include <fstream>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


struct Joint
{
	glm::vec3 position;
	glm::fquat orientation;

	unsigned int parent;
	std::vector<unsigned int> sons;
};
