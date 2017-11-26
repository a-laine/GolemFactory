#pragma once

#include <glm/gtx/matrix_interpolation.hpp>

#include "InstanceDrawable.h"

class InstanceAnimatable : public InstanceDrawable
{
	friend struct AnimationTrack;

	public:
		//	Miscellaneous
		enum AnimationConfigurationFlags
		{
			PLAY = 1 << 0,
			LOOPED = 1 << 1
		};
		//

		//  Default
		InstanceAnimatable(const std::string& meshName, const std::string& shaderName);
		virtual ~InstanceAnimatable();
		//

		//	Public functions
		void animate(float step);
		void launchAnimation(const std::string& labelName, const bool& flaged = false);
		void stopAnimation(const std::string& labelName);
		//

		//	Set/get functions
		bool isAnimationRunning(const std::string& animationName);

		void setSkeleton(std::string skeletonName);
		void setSkeleton(Skeleton* s);
		void setAnimation(std::string animationName);
		void setAnimation(Animation* a);

		Skeleton* getSkeleton() const;
		Animation* getAnimation() const;
		std::vector<glm::mat4> getPose() const;
		glm::vec3 getJointPosition(const std::string& jointName);
		//

	protected:
		//	Protected functions
		void computePose(std::vector<glm::mat4>& result, const std::vector<JointPose>& input, const glm::mat4& parentPose, unsigned int joint);
		//
		
		//	Miscellaneous
		struct AnimationTrack
		{
			AnimationTrack(const unsigned int& poseSize, const std::string& n = "");
			bool animate(const float& step, const InstanceAnimatable* const parent);

			std::string name;
			int start, stop, exit, previous, next;
			float time, uselessTime;
			bool loop, flag;
			std::vector<JointPose> pose;
			unsigned int jointCounter;
		};
		//

		// Attributes
		Skeleton* skeleton;		//!< Skeleton resource pointer
		Animation* animation;	//!< Animation resource

		Mutex locker;
		std::vector<glm::mat4> pose;
		std::list<AnimationTrack> currentAnimations;
		//
};
