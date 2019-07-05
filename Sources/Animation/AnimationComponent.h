#pragma once

#include <string>
#include <vector>
#include <list>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <EntityComponent/Component.hpp>
#include <Resources/Joint.h>
#include <Utiles/Mutex.h>


class Skeleton;
class Animation;
class Mesh;

class AnimationComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(AnimationComponent, Component)

	friend struct AnimationTrack;

	public:
		explicit AnimationComponent(const std::string& animationName = "unknown");
		virtual ~AnimationComponent() override;

		void setAnimation(std::string animationName);
		void setAnimation(Animation* animation);
		Animation* getAnimation() const;

        bool isValid() const;

		void launchAnimation(const std::string& labelName, unsigned int nbPoses, const bool& flaged = false);
		void stopAnimation(const std::string& labelName);
		bool isAnimationRunning();
		bool isAnimationRunning(const std::string& animationName);

		void updateAnimations(float step);
		void blendAnimations(std::vector<JointPose>& result);
		void cleanAnimationTracks(float step);


	private:
		struct AnimationTrack
		{
			explicit AnimationTrack(const unsigned int& poseSize, const std::string& animation = "");
			bool animate(const float& step, const AnimationComponent* const parent);

			std::string animationName;
			int start, stop, exit, previous, next;
			float time, uselessTime;
			bool loop, flag;
			std::vector<JointPose> pose;
			unsigned int jointCounter;
		};


		//Skeleton* m_skeleton;		//!< Skeleton resource pointer
		Animation* m_animation;	//!< Animation resource

		std::list<AnimationTrack> currentAnimations;
};

