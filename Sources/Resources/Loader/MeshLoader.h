#pragma once

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include "../Joint.h"

class MeshLoader
{
    public:
		//  Default
		MeshLoader();
		~MeshLoader();
		//

        //  Public functions
        int loadMesh(std::string file);
        //

		//	Attributes
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> colors;
		std::vector<glm::ivec3> bones;
		std::vector<glm::vec3> weights;
		std::vector<unsigned int> faces;

		std::map<std::string, int> boneMap;
		std::vector<unsigned int> roots;
		std::vector<Joint> joints;

		std::vector<KeyFrame> animations;

	private:
		//	Private functions
		void clear();
		void readSceneHierarchy(const aiNode* node, int depth = 0);
		int searchKeyFrameIndex(const float& keyTime);
		void updateKeyFramePosition(const float& keyTime, const int& joint, const glm::vec3& p);
		void updateKeyFrameOrientation(const float& keyTime, const int& joint, const glm::fquat& q);
		void updateKeyFrameScale(const float& keyTime, const int& joint, const glm::vec3& s);
		//

		//	Debug
		void printJoint(unsigned int joint, int depth);
		//
};
