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
	Joint(const std::string& jointName = "unknown joint");

	int parent;
	std::vector<unsigned int> sons;

	glm::mat4 relativeBindTransform;
	std::string name;
};

struct JointPose
{
	JointPose();

	float priority;
	glm::vec3 position;
	glm::fquat rotation;
	glm::vec3 scale;
};

struct KeyFrame
{
	float time;
	std::vector<JointPose> poses;
};

struct KeyLabel
{
	unsigned int start;
	unsigned int stop;
	unsigned int entry_key;
	unsigned int exit_key;
	bool loop;
};
