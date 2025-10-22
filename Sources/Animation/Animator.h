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
	GF_DECLARE_COMPONENT_CLASS(Animator, Component)

	public:
		explicit Animator();
		virtual ~Animator() override;

		bool load(Variant& jsonObject, const std::string& objectName) override;
		bool load(const std::string& graphName, const std::string& variantName);
		void save(Variant& jsonObject) override;
		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;

		bool isValid() const;
		void update(Component::UpdatePass updatePass, float elapsedTime) override;

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

		SkeletonComponent* m_skeletonComponent = nullptr;
		Skeleton* m_skeleton = nullptr;
		AnimationGraph* m_graph = nullptr;
		std::string m_variantName;
		GraphData* m_data = nullptr;
		std::vector<AnimationGraph::LayerRuntime> m_runtime;
		std::vector<AnimationParameter> m_graphParameters;
		std::vector<BoneCurvesState> m_evaluation;
		bool m_immutableData = true;

#ifdef USE_IMGUI
		bool m_fullGraphWindow = false;
#endif
};

