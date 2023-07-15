#pragma once

#include "ResourceVirtual.h"
#include "AnimationClip.h"
#include "AnimationGraphStructs.h"
#include <Utiles/Parser/Variant.h>


namespace AG
{
	class BlendTree
	{
		public:
			using Evaluation = std::vector<BoneCurvesState>;
			using ParamList = std::vector<AnimationParameter>;

			struct Node
			{
				std::string m_name;
				int m_id;
				int m_subgraphId;
				int m_parentId;

				std::vector<int> m_parameterIds;
				float m_globalInfluence;

				std::vector<int> m_childrenId;
				std::vector<float> m_childrenInfluence;
				std::vector<vec4f> m_childrenPoint;
			};

			struct NodeData
			{
				AnimationClip* m_animation;
				float m_speed;
			};
			struct BlendTreeData
			{
				std::vector<NodeData> m_nodeData;
			};

			struct NodeRuntimeData
			{
				float m_time;
				vec4f m_blendPoint;
				std::vector<float> m_childrenWeights;
				Evaluation m_evaluation;
				float m_weight;
				float m_duration;
			};
			struct BlendTreeRuntime
			{
				std::vector<NodeRuntimeData> m_nodeRuntimes;
				std::map<std::string, std::vector<int>> m_bone2curveTable;
				std::vector<SubGraph> m_subGraphs;
			};

			void evaluate(float _elapsedTime, const BlendTreeData& _data, const ParamList& _paramList, BlendTreeRuntime& _runtimeData) const;

			void initializeRuntime(const BlendTreeData& _data, BlendTreeRuntime& _runtimeData) const;

			std::string m_name;
			int m_entryNodeId;
			std::vector<Node> m_nodes;

		private:
			void evaluateNodeWeights(const Node& _node, const BlendTreeData& _data, const ParamList& _paramList, BlendTreeRuntime& _runtimeData) const;
			void evaluateNode(float _time, const Node& _node, const BlendTreeData& _data, BlendTreeRuntime& _runtimeData) const;

			void evaluateClipState(float& _time, const Node& _state, const BlendTreeData& _data, BlendTreeRuntime& _runtimeData) const;
			void initializeClipStateEvaluation(const Node& _state, const AnimationClip* _clip, Evaluation& _targetEvaluation) const;
	};
}


