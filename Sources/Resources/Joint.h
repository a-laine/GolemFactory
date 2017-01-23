#pragma once

#include <fstream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>


struct Joint
{
	Joint();

	unsigned int parent;
	std::vector<unsigned int> sons;

	glm::vec3 offsetPosition;
	glm::fquat offsetOrientation;
	glm::vec3 offsetScale;

	std::string name;
};

struct JointPose
{
	JointPose();

	glm::vec3 position;
	glm::fquat orientation;
	glm::vec3 scale;
};

struct KeyFrame
{
	float time;
	std::vector<JointPose> poses;
};
