#pragma once

#include <string>


#include <EntityComponent/Component.hpp>
#include <Resources/AnimationGraph.h>


class Skeleton;
class AnimationClip;
class Mesh;
class SkeletonComponent;

class Animator : public Component
{
	public:
		explicit Animator();
		virtual ~Animator() override;

		bool load(Variant& jsonObject, const std::string& objectName) override;
		void save(Variant& jsonObject) override;
		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;

		bool isValid() const;
		void update(float elapsedTime);

		const std::vector<mat4f>& getSkeletonPose() const;

	private:
		SkeletonComponent* m_skeletonComponent;
		Skeleton* m_skeleton;
		AnimationGraph* m_graph;
		AnimationGraphData* m_graphData;
		AnimationGraphRuntimeData m_runtimeData;
		bool m_immutableData;
};

