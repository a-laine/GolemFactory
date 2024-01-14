#pragma once

#include <string>
#include <vector>
#include <list>
//#include <glm/glm.hpp>
#include <GL/glew.h>

#include <EntityComponent/Component.hpp>
#include <Resources/Joint.h>
#include <Utiles/Mutex.h>


class Skeleton;
class AnimationClip;
class Mesh;
class SkeletonComponent;

class AnimationComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(AnimationComponent, Component)

	public:
		explicit AnimationComponent();
		virtual ~AnimationComponent() override;

		void setAnimation(std::string animationName);
		void setAnimation(AnimationClip* animation);
		AnimationClip* getCurrentAnimation() const;

        bool isValid() const;

		void startAnimation(float speed, bool loop = true);
		void stopAnimation();
		void resumeAnimation();
		void update(float elapsedTime);
		bool isAnimationRunning();

		const std::vector<mat4f>& getSkeletonPose() const;
		bool hasSkeletonAnimation() const;

		bool load(Variant& jsonObject, const std::string& objectName) override;
		void save(Variant& jsonObject) override;
		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;


	private:
		//
		void TryInitSkeletonPose();
		void computePoseMatrices();
		//


		bool m_running;
		bool m_looped;
		float m_currentTime;
		float m_speed;
		
		SkeletonComponent* m_skeletonComponent;
		Skeleton* m_skeleton;
		AnimationClip* m_animation;

		struct BoneCurvesState
		{
			std::string m_boneName;
			int m_skeletonBoneIndex;

			int m_scaleKey;
			int m_rotKey;
			int m_posKey;

			float m_scale;
			vec4f m_position;
			quatf m_rotation;
		};
		std::vector<BoneCurvesState> m_states;

		std::vector<int> m_bones2state;
		std::vector<mat4f> m_skeletonFinalPose;
};