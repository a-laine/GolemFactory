#pragma once

#include <vector>
#include <map>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"


class Animation : public ResourceVirtual
{
	friend class AnimationComponent;
    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        //  Default
		Animation(const std::string& animationName = "unknown");
        ~Animation();
		//

		//	Public functions
        void initialize(const std::vector<KeyFrame>& animations, const std::map<std::string, KeyLabel>& names);
        void initialize(std::vector<KeyFrame>&& animations, std::map<std::string, KeyLabel>&& names);
        void initialize(const std::vector<KeyFrame>& animations);
        void initialize(std::vector<KeyFrame>&& animations);
        void clear();
		//

		//	Set/get functions
		std::vector<glm::mat4x4> getKeyPose(const unsigned int& keyFramePose ,const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const;
		std::pair<int, int> getBoundingKeyFrameIndex(float time) const;
		const std::vector<KeyFrame>& getTimeLine() const;
		const std::map<std::string, KeyLabel>& getLabels() const;
        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        //

    protected:
        static std::string defaultName;

		//	Protected functions
		void computePose(const unsigned int& keyFrame, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const;
		//

        //	Attributes
		std::vector<KeyFrame> timeLine;
		std::map<std::string, KeyLabel> labels;
		//
};
