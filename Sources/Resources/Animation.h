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
	friend class AnimationEngine;
    public:
        //  Default
		Animation(const std::string& animationName, const std::vector<KeyFrame>& keyFrames);
		Animation(const std::string& path, const std::string& animationName);
        ~Animation();
		//

		//	Public functions
        bool isValid() const;
		//

		//	Set/get functions
		std::vector<glm::mat4x4> getKeyPose(const unsigned int& keyFramePose ,const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const;
		std::pair<int, int> getBoundingKeyFrameIndex(float time) const;
		std::vector<KeyFrame> getTimeLine() const;
		std::map<std::string, KeyLabel> getLabels() const;
        //

		//	Attributes
		static std::string extension;   //!< Default extension
		//

    protected:
		//	Protected functions
		void computePose(const unsigned int& keyFrame, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const;
		//

        //	Attributes
		std::vector<KeyFrame> timeLine;
		std::map<std::string, KeyLabel> labels;
		std::vector<glm::mat4> inverseBindPose;
		//
};
