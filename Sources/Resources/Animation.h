#pragma once

#include <vector>
#include <fstream>
#include <map>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"


class Animation : public ResourceVirtual
{
    friend class ResourceManager;
	friend class Renderer;
    public:
        //  Default
		Animation(const std::string& animationName, const std::vector<KeyFrame>& keyFrames);
		Animation(const std::string& path, const std::string& animationName);
        ~Animation();
		//

		//	Public functions
        bool isValid() const;
		std::vector<glm::mat4x4> getBindPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const;
		std::vector<glm::mat4x4> getPose(const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const;
        //

		//	Attributes
		static std::string extension;   //!< Default extension
		
		///
		int debugframe;
		//

    protected:
		//	Protected functions
		void computeInverseBindPose(std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const;
		void computePose(std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const;
		//

        //	Attributes
		std::vector<KeyFrame> timeLine;
		//
};
