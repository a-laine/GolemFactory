#pragma once

#include "AnimationClip.h"
#include "AnimationGraphStructs.h"


namespace AG
{
	class StateMachine
	{
		public:
			using Evaluation = std::vector<BoneCurvesState>;
			using ParamList = std::vector<AnimationParameter>;

			struct State;
			struct Condition
			{
				int parameterId;
				enum ComparisonType { GREATER, LESS, EQUALS, NOT_EQUALS };
				ComparisonType m_comparisonType;
				AnimationParameter m_parameter;

				bool evaluate(const AnimationParameter& _comparisonParameter) const;
			};
			struct Transition
			{
				int m_stateTargetId;
				float m_duration;
				std::vector<Condition> m_conditions;
			};
			struct State
			{
				std::string m_name;
				int m_id;
				int m_subgraphId;
				float m_exitTime;
				std::vector<Transition> m_transitionOut;
			};
			struct StateData
			{
				AnimationClip* m_animation;
				float m_speed;
			};
			struct StateMachineData
			{
				std::vector<StateData> m_statesData;
			};
			struct StateMachineRuntime
			{
				Transition* m_currentTransition;
				int m_currentStateId;
				float m_transitionTime;

				float m_currentStateTime;
				unsigned int m_currentStateLoopCount;
				Evaluation m_currentStateEvaluation;
				float m_targetStateTime;
				unsigned int m_targetStateLoopCount;
				Evaluation m_targetStateEvaluation;

				Evaluation m_finalEvaluation;

				std::vector<SubGraph> m_subGraphs;
			};

			bool setState(unsigned int _stateId, const StateMachineData& _data, StateMachineRuntime& _runtimeData) const;
			void evaluate(float _elapsedTime, const StateMachineData& _data, const ParamList& _paramList, StateMachineRuntime& _runtimeData) const;

			std::string m_name;
			int m_entryStateId;
			int m_anyStateId;
			std::vector<State> m_states;

		private:
			void evaluateSubGraph(float _elapsedTime, const ParamList& _paramList, SubGraph& _subgraph, Evaluation& _targetEvaluation) const;
			void evaluateClipState(float& _time, const  State& _state, const StateMachineData& _data, Evaluation& _targetEvaluation, unsigned int* _loopCounter = nullptr) const;
			void initializeMatchingTable(Evaluation& _curentEvaluation, Evaluation& _targetEvaluation, Evaluation& _finalEvaluation) const;
			void initializeClipStateEvaluation(const State& _state, const AnimationClip* _clip, Evaluation& _targetEvaluation) const;
			void initializeSubGraph(SubGraph& subgraph) const;
			Transition* recursiveTestTransitions(int stateId, int depth, const StateMachineData& _data, const ParamList& _paramList, StateMachineRuntime& _runtimeData) const;
	};
}


