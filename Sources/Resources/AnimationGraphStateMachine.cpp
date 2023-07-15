#include "AnimationGraphStateMachine.h"
#include "AnimationGraphBlendTree.h"
#include "ResourceManager.h"


bool AG::StateMachine::Condition::evaluate(const AnimationParameter& _comparisonParameter) const
{
    switch (m_parameter.m_type)
    {
        case AnimationParameter::ParameterType::TRIGGER:
            return _comparisonParameter.m_value.Bool;

        case AnimationParameter::ParameterType::BOOL:
            switch (m_comparisonType)
            {
                case ComparisonType::EQUALS:
                    return _comparisonParameter.m_value.Bool == m_parameter.m_value.Bool;
                case ComparisonType::NOT_EQUALS:
                    return _comparisonParameter.m_value.Bool != m_parameter.m_value.Bool;
                default: break;
            }
            break;

        case AnimationParameter::ParameterType::INT:
            switch (m_comparisonType)
            {
                case ComparisonType::GREATER:
                    return _comparisonParameter.m_value.Int > m_parameter.m_value.Int;
                case ComparisonType::LESS:
                    return _comparisonParameter.m_value.Int < m_parameter.m_value.Int;
                case ComparisonType::EQUALS:
                    return _comparisonParameter.m_value.Int == m_parameter.m_value.Int;
                case ComparisonType::NOT_EQUALS:
                    return _comparisonParameter.m_value.Int != m_parameter.m_value.Int;
                default: 
                    break;
            }
            break;

        case AnimationParameter::ParameterType::FLOAT:
            switch (m_comparisonType)
            {
                case ComparisonType::GREATER:
                    return _comparisonParameter.m_value.Float > m_parameter.m_value.Float;
                case ComparisonType::LESS:
                    return _comparisonParameter.m_value.Float < m_parameter.m_value.Float;
                case ComparisonType::EQUALS:
                    return _comparisonParameter.m_value.Float == m_parameter.m_value.Float;
                case ComparisonType::NOT_EQUALS:
                    return _comparisonParameter.m_value.Float != m_parameter.m_value.Float;
                default: 
                    break;
            }
            break;

        default:
            break;
    }
    return false;
}


bool AG::StateMachine::setState(unsigned int _stateId, const StateMachineData& _data, StateMachineRuntime& _runtimeData) const
{
    if (_stateId >= 0 && _stateId < m_states.size())
    {
        _runtimeData.m_currentStateId = _stateId;
        _runtimeData.m_currentStateTime = 0.f;
        _runtimeData.m_currentStateLoopCount = 0;
        _runtimeData.m_transitionTime = 0.f;
        _runtimeData.m_targetStateTime = 0.f;
        _runtimeData.m_targetStateLoopCount = 0;
        _runtimeData.m_currentTransition = nullptr;

        _runtimeData.m_targetStateEvaluation.clear();
        _runtimeData.m_finalEvaluation.clear();

        if (m_states[_stateId].m_subgraphId < 0)
            initializeClipStateEvaluation(m_states[_stateId], _data.m_statesData[_stateId].m_animation, _runtimeData.m_currentStateEvaluation);
        else
        {

        }
        _runtimeData.m_finalEvaluation = _runtimeData.m_currentStateEvaluation;

        return true;
    }
    return false;
}

void AG::StateMachine::evaluate(float _elapsedTime, const StateMachineData& _data, const ParamList& _paramList, StateMachineRuntime& _runtimeData) const
{
    if (!_runtimeData.m_currentTransition)
    {
        // evaluate anyState transition first
        if (m_anyStateId >= 0)
            _runtimeData.m_currentTransition = recursiveTestTransitions(m_anyStateId, 0, _data, _paramList, _runtimeData);
        if (!_runtimeData.m_currentTransition && ( _runtimeData.m_currentStateLoopCount || 
            _runtimeData.m_currentStateTime > m_states[_runtimeData.m_currentStateId].m_exitTime))
            _runtimeData.m_currentTransition = recursiveTestTransitions(_runtimeData.m_currentStateId, 0, _data, _paramList, _runtimeData);

        if (_runtimeData.m_currentTransition)
        {
            _runtimeData.m_targetStateTime = 0.f;
            _runtimeData.m_transitionTime = 0.f;

            int targetId = _runtimeData.m_currentTransition->m_stateTargetId;
            if (m_states[targetId].m_subgraphId < 0)
            {
                initializeClipStateEvaluation(m_states[targetId], _data.m_statesData[targetId].m_animation, _runtimeData.m_targetStateEvaluation);
                initializeMatchingTable(_runtimeData.m_currentStateEvaluation, _runtimeData.m_targetStateEvaluation, _runtimeData.m_finalEvaluation);
            }
            else
            {
                int subGraphId = m_states[targetId].m_subgraphId;
                initializeSubGraph(_runtimeData.m_subGraphs[subGraphId]);
                evaluateSubGraph(_elapsedTime, _paramList, _runtimeData.m_subGraphs[subGraphId], _runtimeData.m_targetStateEvaluation);
            }
        }
        /*if (m_anyStateId >= 0)
        {
            Transition* transition = recursiveTestTransitions(m_anyStateId, 0, _data, _paramList, _runtimeData);
            if (transition)
            {
                _runtimeData.m_targetStateTime = 0.f;
                _runtimeData.m_transitionTime = 0.f;
                _runtimeData.m_currentTransition = transition;

                int targetId = transition->m_stateTargetId;
                if (m_states[targetId].m_subgraphId < 0)
                {
                    initializeClipStateEvaluation(m_states[targetId], _data.m_statesData[targetId].m_animation, _runtimeData.m_targetStateEvaluation);
                    initializeMatchingTable(_runtimeData.m_currentStateEvaluation, _runtimeData.m_targetStateEvaluation, _runtimeData.m_finalEvaluation);
                }
                else
                {
                    int subGraphId = m_states[targetId].m_subgraphId;
                    initializeSubGraph(_runtimeData.m_subGraphs[subGraphId]);
                    evaluateSubGraph(_elapsedTime, _paramList, _runtimeData.m_subGraphs[subGraphId], _runtimeData.m_targetStateEvaluation);
                }
            }
        }

        // evaluate current state transitions
        if (!_runtimeData.m_currentTransition && (_runtimeData.m_currentStateLoopCount || _runtimeData.m_currentStateTime > m_states[_runtimeData.m_currentStateId].m_exitTime))
        {
            Transition* transition = recursiveTestTransitions(_runtimeData.m_currentStateId, 0, _data, _paramList, _runtimeData);
            if (transition)
            {
                _runtimeData.m_targetStateTime = 0.f;
                _runtimeData.m_transitionTime = 0.f;
                _runtimeData.m_currentTransition = transition;

                int targetId = transition->m_stateTargetId;
                if (m_states[targetId].m_subgraphId < 0)
                {
                    initializeClipStateEvaluation(m_states[targetId], _data.m_statesData[targetId].m_animation, _runtimeData.m_targetStateEvaluation);
                    initializeMatchingTable(_runtimeData.m_currentStateEvaluation, _runtimeData.m_targetStateEvaluation, _runtimeData.m_finalEvaluation);
                }
                else
                {
                    int subGraphId = m_states[targetId].m_subgraphId;
                    initializeSubGraph(_runtimeData.m_subGraphs[subGraphId]);
                    evaluateSubGraph(_elapsedTime, _paramList, _runtimeData.m_subGraphs[subGraphId], _runtimeData.m_targetStateEvaluation);
                }
            }
        }*/
    }

    // evaluate transition curves
    else
    {
        const Transition* transition = _runtimeData.m_currentTransition;
        _runtimeData.m_transitionTime += _elapsedTime;

        // end transition
        if (_runtimeData.m_transitionTime >= transition->m_duration)
        {
            _runtimeData.m_currentStateId = transition->m_stateTargetId;
            _runtimeData.m_currentStateLoopCount = _runtimeData.m_targetStateLoopCount;
            _runtimeData.m_currentStateTime = _runtimeData.m_targetStateTime;
            _runtimeData.m_currentStateEvaluation.swap(_runtimeData.m_targetStateEvaluation);
            _runtimeData.m_targetStateEvaluation.clear();
            _runtimeData.m_currentTransition = nullptr;
            _runtimeData.m_targetStateTime = 0.f;
            _runtimeData.m_targetStateLoopCount = 0;
        }

        // evaluate sub graph
        else if (m_states[transition->m_stateTargetId].m_subgraphId >= 0)
        {
            const StateData& targetData = _data.m_statesData[transition->m_stateTargetId];
            _runtimeData.m_targetStateTime += targetData.m_speed * _elapsedTime;
            int subGraphId = m_states[transition->m_stateTargetId].m_subgraphId;
            evaluateSubGraph(_elapsedTime, _paramList, _runtimeData.m_subGraphs[subGraphId], _runtimeData.m_targetStateEvaluation);

            // initialize table only once
            if (m_states[_runtimeData.m_currentStateId].m_subgraphId < 0)
                initializeMatchingTable(_runtimeData.m_currentStateEvaluation, _runtimeData.m_targetStateEvaluation, _runtimeData.m_finalEvaluation);
        }

        // evaluate transition
        else
        {
            const StateData& targetData = _data.m_statesData[transition->m_stateTargetId];
            _runtimeData.m_targetStateTime += targetData.m_speed * _elapsedTime;
            evaluateClipState(_runtimeData.m_targetStateTime, m_states[transition->m_stateTargetId], _data, _runtimeData.m_targetStateEvaluation,
                &_runtimeData.m_targetStateLoopCount);
        }
    }

    // current state loop
    const StateData& stateData = _data.m_statesData[_runtimeData.m_currentStateId];
    const State& currentState = m_states[_runtimeData.m_currentStateId];
    if (currentState.m_subgraphId >= 0)
    {
        _runtimeData.m_currentStateTime += stateData.m_speed * _elapsedTime;
        evaluateSubGraph(_elapsedTime, _paramList, _runtimeData.m_subGraphs[currentState.m_subgraphId], _runtimeData.m_currentStateEvaluation);

        // initialize table if in transition
        if (_runtimeData.m_currentTransition)
            initializeMatchingTable(_runtimeData.m_currentStateEvaluation, _runtimeData.m_targetStateEvaluation, _runtimeData.m_finalEvaluation);
    }
    else
    {
        _runtimeData.m_currentStateTime += stateData.m_speed * _elapsedTime;
        evaluateClipState(_runtimeData.m_currentStateTime, currentState, _data, _runtimeData.m_currentStateEvaluation, &_runtimeData.m_currentStateLoopCount);
    }



    if (!_runtimeData.m_currentTransition)
    {
        _runtimeData.m_finalEvaluation = _runtimeData.m_currentStateEvaluation;
    }
    else
    {
        float t = 0.f;
        if (_runtimeData.m_currentTransition)
            t = clamp(_runtimeData.m_transitionTime / std::max(_runtimeData.m_currentTransition->m_duration, 0.001f), 0.f, 1.f);

        for (int i = 0; i < _runtimeData.m_currentStateEvaluation.size(); i++)
        {
            const BoneCurvesState& state0 = _runtimeData.m_currentStateEvaluation[i];
            int targetId = _runtimeData.m_currentStateEvaluation[i].m_skeletonBoneIndex;
            if (targetId == -1)
            {
                _runtimeData.m_finalEvaluation[i].m_position = state0.m_position;
                _runtimeData.m_finalEvaluation[i].m_rotation = state0.m_rotation;
                _runtimeData.m_finalEvaluation[i].m_scale = state0.m_scale;
            }
            else
            {
                const BoneCurvesState& state1 = _runtimeData.m_targetStateEvaluation[targetId];
                _runtimeData.m_finalEvaluation[i].m_position = vec4f::lerp(state0.m_position, state1.m_position, t);
                _runtimeData.m_finalEvaluation[i].m_rotation = quatf::slerp(state0.m_rotation, state1.m_rotation, t);
                _runtimeData.m_finalEvaluation[i].m_scale = lerp(state0.m_scale, state1.m_scale, t);
            }
        }
        for (int i = _runtimeData.m_currentStateEvaluation.size(); i < _runtimeData.m_finalEvaluation.size(); i++)
        {
            const BoneCurvesState& state1 = _runtimeData.m_targetStateEvaluation[_runtimeData.m_finalEvaluation[i].m_skeletonBoneIndex];
            _runtimeData.m_finalEvaluation[i].m_position = state1.m_position;
            _runtimeData.m_finalEvaluation[i].m_rotation = state1.m_rotation;
            _runtimeData.m_finalEvaluation[i].m_scale = state1.m_scale;
        }
    }
}

void AG::StateMachine::initializeClipStateEvaluation(const State& _state, const AnimationClip* _clip, Evaluation& _targetEvaluation) const
{
    _targetEvaluation.clear();
    if (!_clip)
        return;
    for (int i = 0; i < _clip->m_boneCurves.size(); i++)
    {
        BoneCurvesState state;
        state.m_boneName = _clip->m_boneCurves[i].m_boneName;
        state.m_posKey = state.m_rotKey = state.m_scaleKey = 0;

        state.m_position = _clip->m_boneCurves[i].m_positionCurve[0].m_value;
        const vec4f v = _clip->m_boneCurves[i].m_rotationCurve[0].m_value;
        state.m_rotation = quatf(v.w, v.x, v.y, v.z);
        state.m_scale = _clip->m_boneCurves[i].m_scaleCurve[0].m_value;

        _targetEvaluation.push_back(state);
    }
}

void AG::StateMachine::initializeSubGraph(SubGraph& subgraph) const
{
    if (subgraph.m_graph)
    {
        if (subgraph.m_type == SubGraph::SubGraphType::BLEND_TREE)
        {
            BlendTree* blendTree = (BlendTree*)subgraph.m_graph;
            BlendTree::BlendTreeData* blendTreeData = (BlendTree::BlendTreeData*)subgraph.m_graphData;
            BlendTree::BlendTreeRuntime* blendTreeRuntime = (BlendTree::BlendTreeRuntime*)subgraph.m_graphRuntime;
        }
        else
        {
            StateMachine* stateMachine = (StateMachine*)subgraph.m_graph;
            StateMachine::StateMachineData* stateMachineData = (StateMachine::StateMachineData*)subgraph.m_graphData;
            StateMachine::StateMachineRuntime* stateMachineRuntime = (StateMachine::StateMachineRuntime*)subgraph.m_graphRuntime;
            stateMachine->setState(stateMachine->m_entryStateId, *stateMachineData, *stateMachineRuntime);
        }
    }
}

AG::StateMachine::Transition* AG::StateMachine::recursiveTestTransitions(int _stateId, int _depth, const StateMachineData& _data, const ParamList& _paramList, StateMachineRuntime& _runtimeData) const
{
    if (_depth > 4)
        return nullptr;

    bool doRecursive = false;
    int targetId;
    Transition* validTransition = nullptr;

    const std::vector<Transition>& transitions = m_states[_stateId].m_transitionOut;
    for (const Transition& transition : transitions)
    {
        const std::vector<Condition>& conditions = transition.m_conditions;
        bool isValid = true;
        for (const Condition& condition : conditions)
        {
            isValid = condition.evaluate(_paramList[condition.parameterId]);
            if (!isValid)
                break;
        }
        if (isValid)
        {
            validTransition = (Transition*)&transition;
            targetId = transition.m_stateTargetId;

            // avaoid a useless transition
            if (targetId == _runtimeData.m_currentStateId)
                return nullptr;

            doRecursive = m_states[targetId].m_subgraphId < 0 && _data.m_statesData[targetId].m_animation == nullptr;
            break;
        }
    }

    if (doRecursive)
        return recursiveTestTransitions(targetId, _depth + 1, _data, _paramList, _runtimeData);
    return validTransition;
}

void AG::StateMachine::evaluateSubGraph(float _elapsedTime, const ParamList& _paramList, SubGraph& _subgraph, Evaluation& _targetEvaluation) const
{
    if (_subgraph.m_graph)
    {
        if (_subgraph.m_type == SubGraph::SubGraphType::BLEND_TREE)
        {
            BlendTree* blendTree = (BlendTree*)_subgraph.m_graph;
            BlendTree::BlendTreeData* blendTreeData = (BlendTree::BlendTreeData*)_subgraph.m_graphData;
            BlendTree::BlendTreeRuntime* blendTreeRuntime = (BlendTree::BlendTreeRuntime*)_subgraph.m_graphRuntime;
            blendTree->evaluate(_elapsedTime, *blendTreeData, _paramList, *blendTreeRuntime);
            _targetEvaluation = blendTreeRuntime->m_nodeRuntimes[blendTree->m_entryNodeId].m_evaluation;
        }
        else
        {
            StateMachine* stateMachine = (StateMachine*)_subgraph.m_graph;
            StateMachine::StateMachineData* stateMachineData = (StateMachine::StateMachineData*)_subgraph.m_graphData;
            StateMachine::StateMachineRuntime* stateMachineRuntime = (StateMachine::StateMachineRuntime*)_subgraph.m_graphRuntime;
            stateMachine->evaluate(_elapsedTime, *stateMachineData, _paramList, *stateMachineRuntime);
            _targetEvaluation = stateMachineRuntime->m_finalEvaluation;
        }
    }
}

void AG::StateMachine::evaluateClipState(float& _time, const  State& _state, const StateMachineData& _data, Evaluation& _targetEvaluation, 
    unsigned int* _loopCounter) const
{
    const StateData& stateData = _data.m_statesData[_state.m_id];
    const AnimationClip* clip = stateData.m_animation;

    if (_time >= clip->m_duration)
    {
        if (_loopCounter)
            (*_loopCounter)++;
        while (_time >= clip->m_duration)
            _time -= clip->m_duration;

        for (int i = 0; i < _targetEvaluation.size(); i++)
        {
            BoneCurvesState& state = _targetEvaluation[i];
            const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

            for (int j = 1; j < curve.m_positionCurve.size(); j++)
            {
                if (curve.m_positionCurve[j].m_time > _time)
                {
                    state.m_posKey = j - 1;
                    break;
                }
            }
            for (int j = 1; j < curve.m_scaleCurve.size(); j++)
            {
                if (curve.m_scaleCurve[j].m_time > _time)
                {
                    state.m_scaleKey = j - 1;
                    break;
                }
            }
            for (int j = 1; j < curve.m_rotationCurve.size(); j++)
            {
                if (curve.m_rotationCurve[j].m_time > _time)
                {
                    state.m_rotKey = j - 1;
                    break;
                }
            }
        }
    }

    // evaluate current state
    for (int i = 0; i < _targetEvaluation.size(); i++)
    {
        BoneCurvesState& state = _targetEvaluation[i];
        const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

        // change curve keyframes if needed
        if (_time > curve.m_positionCurve[state.m_posKey + 1].m_time)
        {
            for (int j = state.m_posKey + 2; j < curve.m_positionCurve.size(); j++)
            {
                if (curve.m_positionCurve[j].m_time > _time)
                {
                    state.m_posKey = j - 1;
                    break;
                }
            }
        }
        if (_time > curve.m_rotationCurve[state.m_rotKey + 1].m_time)
        {
            for (int j = state.m_rotKey + 2; j < curve.m_rotationCurve.size(); j++)
            {
                if (curve.m_rotationCurve[j].m_time > _time)
                {
                    state.m_rotKey = j - 1;
                    break;
                }
            }
        }
        if (_time > curve.m_scaleCurve[state.m_scaleKey + 1].m_time)
        {
            for (int j = state.m_scaleKey + 2; j < curve.m_scaleCurve.size(); j++)
            {
                if (curve.m_scaleCurve[j].m_time > _time)
                {
                    state.m_scaleKey = j - 1;
                    break;
                }
            }
        }

        // evaluate segments
        float t = (_time - curve.m_positionCurve[state.m_posKey].m_time) / (curve.m_positionCurve[state.m_posKey + 1].m_time - curve.m_positionCurve[state.m_posKey].m_time);
        state.m_position = vec4f::lerp(curve.m_positionCurve[state.m_posKey].m_value, curve.m_positionCurve[state.m_posKey + 1].m_value, t);

        t = (_time - curve.m_rotationCurve[state.m_rotKey].m_time) / (curve.m_rotationCurve[state.m_rotKey + 1].m_time - curve.m_rotationCurve[state.m_rotKey].m_time);
        state.m_rotation = quatf::slerp(*(quatf*)&curve.m_rotationCurve[state.m_rotKey].m_value, *(quatf*)&curve.m_rotationCurve[state.m_rotKey + 1].m_value, t);

        t = (_time - curve.m_scaleCurve[state.m_scaleKey].m_time) / (curve.m_scaleCurve[state.m_scaleKey + 1].m_time - curve.m_scaleCurve[state.m_scaleKey].m_time);
        state.m_scale = lerp(curve.m_scaleCurve[state.m_scaleKey].m_value, curve.m_scaleCurve[state.m_scaleKey + 1].m_value, t);
    }
}

void AG::StateMachine::initializeMatchingTable(Evaluation& _curentEvaluation, Evaluation& _targetEvaluation, Evaluation& _finalEvaluation) const
{
    int finalEvalSize = _curentEvaluation.size() + _targetEvaluation.size();
    for (int i = 0; i < _curentEvaluation.size(); i++)
        _curentEvaluation[i].m_skeletonBoneIndex = -1;
    for (int i = 0; i < _targetEvaluation.size(); i++)
        _targetEvaluation[i].m_skeletonBoneIndex = -1;

    for (int i = 0; i < _curentEvaluation.size(); i++)
    {
        if (i < _targetEvaluation.size() && _curentEvaluation[i].m_boneName == _targetEvaluation[i].m_boneName)
        {
            _targetEvaluation[i].m_skeletonBoneIndex = i;
            _curentEvaluation[i].m_skeletonBoneIndex = i;
            finalEvalSize--;
        }
        else
        {
            for (int j = 0; j < _targetEvaluation.size(); j++)
            {
                if (_curentEvaluation[i].m_boneName == _targetEvaluation[j].m_boneName)
                {
                    _targetEvaluation[j].m_skeletonBoneIndex = i;
                    _curentEvaluation[i].m_skeletonBoneIndex = j;
                    finalEvalSize--;
                    break;
                }
            }
        }
    }

    _finalEvaluation.resize(finalEvalSize);
    for (int i = 0; i < _curentEvaluation.size(); i++)
        _finalEvaluation[i] = _curentEvaluation[i];
    if (finalEvalSize != _curentEvaluation.size())
    {
        int id = _curentEvaluation.size();
        for (int i = 0; i < _targetEvaluation.size(); i++)
        {
            if (_targetEvaluation[i].m_skeletonBoneIndex == -1)
            {
                _finalEvaluation[id] = _targetEvaluation[i];
                _finalEvaluation[id].m_skeletonBoneIndex = i;
            }
        }
    }

}