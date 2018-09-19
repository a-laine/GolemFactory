#pragma once

#include "Component.hpp"
#include "Resources/ResourceManager.h"
#include "Physics/Shape.h"


class AnimationEngine : public Component
{
	GF_DECLARE_COMPONENT_CLASS(AnimationEngine, Component)
	public:
		//  Default
		AnimationEngine(const std::string& skeletonName, const std::string& animationName);
		virtual ~AnimationEngine() override;
		//

		//	Public functions
		void animate(float step);
		void launchAnimation(const std::string& labelName, const bool& flaged = false);
		void stopAnimation(const std::string& labelName);

		void computeCapsules(const Mesh* mesh);
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
		
		struct ShortCapsule
		{
			ShortCapsule() : index1(0), index2(0), radius(0.f) {};
			unsigned short index1;
			unsigned short index2;
			float radius;
		};
		std::vector<ShortCapsule> getCapsules() const;
		//

	protected:
		//	Protected functions
		void computePose(std::vector<glm::mat4>& result, const std::vector<JointPose>& input, const glm::mat4& parentPose, unsigned int joint);
		//

		//	Miscellaneous
		struct AnimationTrack
		{
			//  Default
			AnimationTrack(const unsigned int& poseSize, const std::string& n = "");
			//
			
			//	Facility functions
			bool animate(const float& step, const AnimationEngine* const parent);
			//

			//	Attributes
			std::string name;
			int start, stop, exit, previous, next;
			float time, uselessTime;
			bool loop, flag;
			std::vector<JointPose> pose;
			unsigned int jointCounter;
			//
		};
		//

		// Attributes
		Skeleton* skeleton;
		Animation* animation;

		std::vector<glm::mat4> pose;
		std::list<AnimationTrack> currentAnimations;
		std::vector<ShortCapsule> capsules;
		//
};

