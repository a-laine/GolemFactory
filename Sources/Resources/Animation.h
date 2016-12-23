#pragma once

#include <vector>
#include <fstream>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ResourceVirtual.h"


class Animation : public ResourceVirtual
{
    friend class ResourceManager;

    public:
		//  Miscellaneous
		struct Joint
		{
			glm::vec3 position;
			glm::fquat orientation;
		};
		struct KeyPose
		{
			float timeKey;
			std::vector<Joint> jointList;
		};
		//

        //  Default
		Animation(std::string path, std::string animationName);
        ~Animation();

        bool isValid() const;
        //

		//	Set / get functions
		
		//

		//  Attributes
		static std::string extension;
		//

    protected:
        //	Attributes
		std::vector<bool> blending;
		std::vector<KeyPose> poses;
		//
};
