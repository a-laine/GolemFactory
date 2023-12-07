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

		bool setParameter(const std::string& _name, float _value);
		bool setParameter(const std::string& _name, bool _value);
		bool setParameter(const std::string& _name, int _value);

	private:
		using GraphData = std::vector<AnimationGraph::LayerData>;

		void drawImGui(int _stateId, const AG::StateMachine& _graph, const AG::StateMachine::StateMachineData& _data, 
			AG::StateMachine::StateMachineRuntime& _runtime, bool isSubgraph);
		void drawImGui(int _nodeId, const AG::BlendTree& _tree, const AG::BlendTree::BlendTreeData& _data,
			AG::BlendTree::BlendTreeRuntime& _runtime, bool isSubgraph);
		void drawFullGraphWindow();

		SkeletonComponent* m_skeletonComponent;
		Skeleton* m_skeleton;
		AnimationGraph* m_graph;
		std::string m_variantName;
		GraphData* m_data;
		std::vector<AnimationGraph::LayerRuntime> m_runtime;
		std::vector<AnimationParameter> m_graphParameters;
		bool m_immutableData;
		std::vector<BoneCurvesState> m_evaluation;

#ifdef USE_IMGUI
		bool m_fullGraphWindow = false;
#endif
};

