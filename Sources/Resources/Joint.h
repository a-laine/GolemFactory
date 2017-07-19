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
	Joint(std::string n = "unknown joint", glm::mat4 offsetMatrix = glm::mat4(1.f));

	unsigned int parent;
	std::vector<unsigned int> sons;

	glm::mat4 relativeBindTransform;
	glm::mat4 inverseLocalBindTransform;
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
