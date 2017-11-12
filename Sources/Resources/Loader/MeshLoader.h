#pragma once

#include <vector>
#include <map>
#include <list>
#include <tuple>
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
		//  Miscellaneous
		/*!
		*	\enum VerboseLevel
		*	\brief The verbose level for logs
		*/
		enum VerboseLevel
		{
			NONE = 0,		//!< No log printed
			ERRORS = 1,		//!< Just print errors in log
			WARNINGS = 2,	//!< Print errors and logs
			ALL = 3			//!< Print all logs (errors, warning and optionnal informations)
		};
		//

		//  Default
		MeshLoader();
		~MeshLoader();
		//

        //  Public functions
        int loadMesh(std::string file);
        //

		//	Attributes
		static VerboseLevel logVerboseLevel;

		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normales;
		std::vector<glm::vec3> colors;
		std::vector<glm::ivec3> bones;
		std::vector<glm::vec3> weights;
		std::vector<unsigned int> faces;

		std::map<std::string, int> boneMap;

		glm::mat4 globalMatrix;
		std::vector<unsigned int> roots;
		std::vector<Joint> joints;

		std::vector<KeyFrame> animations;

	private:
		//	Temporary loading structures
		typedef std::tuple<float, std::string, glm::vec3>  tupleVec3;
		typedef std::tuple<float, std::string, glm::fquat> tupleQuat;
		typedef std::list<tupleVec3> BidirectionnalVectorMap;
		typedef std::list<tupleQuat> BidirectionnalQuaternionMap;
		//

		//	Private functions
		void clear();
		void readSceneHierarchy(const aiNode* node, int depth = 0);
		//

		//	Debug
		void printJoint(unsigned int joint, int depth);
		//
};
