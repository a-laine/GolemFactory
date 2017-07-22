#pragma once

#include <glm/gtx/matrix_interpolation.hpp>

#include "InstanceDrawable.h"

class InstanceAnimatable : public InstanceDrawable
{
	public:
		//	Miscellaneous
		enum AnimationConfigurationFlags
		{
			PLAY = 1 << 0,
			LOOPED = 1 << 1
		};
		//

		//  Default
		InstanceAnimatable(std::string meshName = "default", std::string shaderName = "default");
		virtual ~InstanceAnimatable();
		//

		//	Public functions
		void animate(float step);
		//

		//	Set/get functions
		void setAnimation(std::string animationName);
		void setAnimation(Animation* a);

		Skeleton* getSkeleton() const;
		Animation* getAnimation() const;
		std::vector<glm::mat4> getPose();
		//

	protected:
		// Attributes
		Skeleton* skeleton;		//!< Skeleton resource pointer
		Animation* animation;	//!< Animation resource

		Mutex locker;
		std::vector<glm::mat4> pose;

		uint8_t animationConfiguration;
		float animationTime;
		float animationSpeed;
		int previousKeyFrame;
		int nextKeyFrame;
		int startKeyFrame;
		int stopKeyFrame;
		//

		//	Protected functions
		void interpolatePose(const int& key1, const int& key2, const float& p, std::vector<glm::mat4>& pose, const glm::mat4& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy);
		//
};
