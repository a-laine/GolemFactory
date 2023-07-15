#pragma once

#include "ResourceVirtual.h"
#include "AnimationClip.h"
#include "AnimationGraphStructs.h"
#include <Utiles/Parser/Variant.h>

#include "AnimationGraphStateMachine.h"
#include "AnimationGraphBlendTree.h"


class Skeleton;
class AnimationGraph : public ResourceVirtual
{
	public:
		using Evaluation = std::vector<BoneCurvesState>;
		using ParamList = std::vector<AnimationParameter>;
		static char const* const directory;
		static char const* const extension;

		struct Layer
		{
			std::string m_name;
			std::vector<AG::StateMachine> m_stateMachines;
			std::vector<AG::BlendTree> m_blendTrees;
			std::vector<std::string> m_subGraphNames;
			int stateMachineEntryId;
			int m_blendTreeEntryId;
		};
		struct LayerData
		{
			std::vector<AG::StateMachine::StateMachineData> m_stateMachinesData;
			std::vector<AG::BlendTree::BlendTreeData> m_blendTreesData;
		};
		struct LayerRuntime
		{
			std::vector<AG::StateMachine::StateMachineRuntime> m_stateMachinesRuntime;
			std::vector<AG::BlendTree::BlendTreeRuntime> m_blendTreesRuntime;
		};



		// constructor / destructor
		AnimationGraph(const std::string& _graphName = "unknown");
		~AnimationGraph();

		// initialization
		void initialize(std::vector<Layer>& _layers);
		void setParameters(std::vector<AnimationParameter>& _parameters);
		void setVariants(std::map<std::string, std::vector<LayerData>>& _variants);
		void getRuntime(const std::vector<LayerData>& _data, std::vector<LayerRuntime>& _targetRuntime) const;

		// set / get
		const std::vector<Layer>& getLayers() const;

		// instance copy functions
		std::vector<AnimationParameter> getParametersCopy() const;
		const std::vector<LayerData>* getVariant(const std::string& _name) const;

		// evaluation of the graph
		void evaluate(float _elapsedTime, const ParamList& _paramList, const std::vector<LayerData>& _data, std::vector<LayerRuntime>& _runtime, Evaluation& _result) const;

		// resource stuff
		static std::string getIdentifier(const std::string& resourceName);
		static const std::string& getDefaultName();
		static void setDefaultName(const std::string& name);
		std::string getIdentifier() const override;
		std::string getLoaderId(const std::string& resourceName) const;

		// debug
		void onDrawImGui() override;

	protected:
		static std::string defaultName;

		std::vector<Layer> m_layers;
		std::map<std::string, std::vector<LayerData>> m_variants;
		std::vector<AnimationParameter> m_defaultParameters;
};