#include "AnimationGraphBlendTree.h"
#include "ResourceManager.h"


void AG::BlendTree::evaluate(float _elapsedTime, const BlendTreeData& _data, const ParamList& _paramList, BlendTreeRuntime& _runtimeData) const
{
    NodeRuntimeData& runtime = _runtimeData.m_nodeRuntimes[m_entryNodeId];
    runtime.m_weight = 1.f;
    evaluateNodeWeights(m_nodes[m_entryNodeId], _data, _paramList, _runtimeData);

    runtime.m_time += _elapsedTime;
    while (runtime.m_time > runtime.m_duration)
        runtime.m_time -= runtime.m_duration;

    evaluateNode(runtime.m_time / runtime.m_duration, m_nodes[m_entryNodeId], _data, _runtimeData);
}

void AG::BlendTree::initializeRuntime(const BlendTreeData& _data, BlendTreeRuntime& _runtimeData) const
{
    auto& matchingTable = _runtimeData.m_bone2curveTable;
    _runtimeData.m_nodeRuntimes.resize(m_nodes.size());
    _runtimeData.m_nodeRuntimes.shrink_to_fit();

    for (const Node& node : m_nodes)
    {
        NodeRuntimeData& runtime = _runtimeData.m_nodeRuntimes[node.m_id];
        runtime.m_time = 0.f;
        runtime.m_weight = node.m_id == m_entryNodeId ? 1.f : 0.f;
        runtime.m_blendPoint = vec4f::zero;
        runtime.m_childrenWeights.assign(node.m_childrenId.size(), 0.f);
        runtime.m_childrenWeights.shrink_to_fit();
        runtime.m_evaluation.clear();

        const AnimationClip* clip = _data.m_nodeData[node.m_id].m_animation;
        if (!clip)
            continue;

        runtime.m_duration = clip->m_duration;
        for (int i = 0; i < clip->m_boneCurves.size(); i++)
        {
            BoneCurvesState state;
            state.m_boneName = clip->m_boneCurves[i].m_boneName;
            state.m_posKey = state.m_rotKey = state.m_scaleKey = 0;

            state.m_position = clip->m_boneCurves[i].m_positionCurve[0].m_value;
            const vec4f v = clip->m_boneCurves[i].m_rotationCurve[0].m_value;
            state.m_rotation = quatf(v.w, v.x, v.y, v.z);
            state.m_scale = clip->m_boneCurves[i].m_scaleCurve[0].m_value;

            runtime.m_evaluation.push_back(state);

            auto it = matchingTable.find(state.m_boneName);
            if (it == matchingTable.end())
            {
                auto it2 = matchingTable.emplace(state.m_boneName, std::vector<int>());
                it2.first->second.assign(m_nodes.size(), -1);
                it2.first->second[node.m_id] = i;
            }
            else
            {
                it->second[node.m_id] = i;
            }
        }
    }
}

void AG::BlendTree::evaluateClipState(float& _time01, const Node& _node, const BlendTreeData& _data, BlendTreeRuntime& _runtimeData) const
{
    const AnimationClip* clip = _data.m_nodeData[_node.m_id].m_animation;
    Evaluation& evaluation = _runtimeData.m_nodeRuntimes[_node.m_id].m_evaluation;
    float time = _time01 * clip->m_duration;

    // evaluate current state
    for (int i = 0; i < evaluation.size(); i++)
    {
        BoneCurvesState& state = evaluation[i];
        const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

        // change curve keyframes if needed
        for (int k = 1; k < curve.m_positionCurve.size(); k++)
        {
            if (curve.m_positionCurve[k].m_time > time)
            {
                state.m_posKey = k - 1;
                break;
            }
        }
        for (int k = 1; k < curve.m_scaleCurve.size(); k++)
        {
            if (curve.m_scaleCurve[k].m_time > time)
            {
                state.m_scaleKey = k - 1;
                break;
            }
        }
        for (int k = 1; k < curve.m_rotationCurve.size(); k++)
        {
            if (curve.m_rotationCurve[k].m_time > time)
            {
                state.m_rotKey = k - 1;
                break;
            }
        }

        // evaluate segments
        float t = (time - curve.m_positionCurve[state.m_posKey].m_time) / (curve.m_positionCurve[state.m_posKey + 1].m_time - curve.m_positionCurve[state.m_posKey].m_time);
        state.m_position = vec4f::lerp(curve.m_positionCurve[state.m_posKey].m_value, curve.m_positionCurve[state.m_posKey + 1].m_value, t);

        t = (time - curve.m_rotationCurve[state.m_rotKey].m_time) / (curve.m_rotationCurve[state.m_rotKey + 1].m_time - curve.m_rotationCurve[state.m_rotKey].m_time);
        state.m_rotation = quatf::slerp(*(quatf*)&curve.m_rotationCurve[state.m_rotKey].m_value, *(quatf*)&curve.m_rotationCurve[state.m_rotKey + 1].m_value, t);

        t = (time - curve.m_scaleCurve[state.m_scaleKey].m_time) / (curve.m_scaleCurve[state.m_scaleKey + 1].m_time - curve.m_scaleCurve[state.m_scaleKey].m_time);
        state.m_scale = lerp(curve.m_scaleCurve[state.m_scaleKey].m_value, curve.m_scaleCurve[state.m_scaleKey + 1].m_value, t);
    }
}

void AG::BlendTree::initializeClipStateEvaluation(const Node& _state, const AnimationClip* _clip, Evaluation& _targetEvaluation) const
{
    _targetEvaluation.clear();
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

void AG::BlendTree::evaluateNodeWeights(const Node& _node, const BlendTreeData& _data, const ParamList& _paramList, BlendTreeRuntime& _runtimeData) const
{
    if (_node.m_childrenId.empty())
        return;

    NodeRuntimeData& runtime = _runtimeData.m_nodeRuntimes[_node.m_id];
    if (runtime.m_weight == 0.f)
    {
        runtime.m_duration = 0.f;
        for (int i = 0; i < _node.m_childrenId.size(); i++)
        {
            int childId = _node.m_childrenId[i];
            _runtimeData.m_nodeRuntimes[childId].m_weight = 0.f;
            evaluateNodeWeights(m_nodes[childId], _data, _paramList, _runtimeData);
        }
        return;
    }

    // compute eval position
    runtime.m_blendPoint = vec4f::zero;
    for (int i = 0; i < _node.m_parameterIds.size(); i++)
    {
        if (_node.m_parameterIds[i] < 0)
            continue;

        const AnimationParameter& param = _paramList[_node.m_parameterIds[i]];
        if (param.m_type != AnimationParameter::ParameterType::FLOAT)
            continue;

        runtime.m_blendPoint[i] = param.m_value.Float;
    }

    // compute kernel weights
    std::vector<std::pair<float, int>> kernel;
    kernel.assign(_node.m_childrenPoint.size(), { 0.f, 0 });
    for (int i = 0; i < _node.m_childrenPoint.size(); i++)
    {
        float influence = _node.m_childrenInfluence[i] * _node.m_globalInfluence;
        float w = (_node.m_childrenPoint[i] - runtime.m_blendPoint).getNorm2() / (influence * influence);
        kernel[i].first = exp(-w);
        kernel[i].second = i;
    }

    std::sort(kernel.begin(), kernel.end(), [](std::pair<float, int>& a, std::pair<float, int>& b) { return a.first > b.first; });
    int blendClipCount = 1;
    vec4f blendClipWeights = vec4f::zero;
    blendClipWeights[0] = kernel[0].first;
    float weightSum = kernel[0].first;

    for (int i = 1; i < kernel.size() && i < 4; i++, blendClipCount++)
    {
        float w = kernel[i].first;
        if (w / weightSum > 0.002f)
        {
            weightSum += w;
            blendClipWeights[i] = w;
        }
        else break;
    }
    blendClipWeights /= weightSum;
    runtime.m_duration = 0.f;

    for (int i = 0; i < kernel.size(); i++)
    {
        const int childIndex = kernel[i].second;
        const int childId = _node.m_childrenId[childIndex];
        float weight = i < blendClipCount ? blendClipWeights[i] : 0.f;
        _runtimeData.m_nodeRuntimes[childId].m_weight = weight;
        evaluateNodeWeights(m_nodes[childId], _data, _paramList, _runtimeData);

        runtime.m_duration += weight * _runtimeData.m_nodeRuntimes[childId].m_duration;
    }
}

void AG::BlendTree::evaluateNode(float _time01, const Node& _node, const BlendTreeData& _data, BlendTreeRuntime& _runtimeData) const
{
    if (_node.m_childrenId.empty())
        return;

    NodeRuntimeData& runtime = _runtimeData.m_nodeRuntimes[_node.m_id];
    std::vector<int> blendChildrenId;

    for (int i = 0; i < _node.m_childrenId.size(); i++)
    {
        const int childId = _node.m_childrenId[i];
        NodeRuntimeData& childRuntime = _runtimeData.m_nodeRuntimes[childId];
        if (childRuntime.m_weight == 0.f)
            continue;

        blendChildrenId.push_back(childId);
        if (m_nodes[childId].m_childrenId.empty())
            evaluateClipState(_time01, m_nodes[childId], _data, _runtimeData);
        else
            evaluateNode(_time01, m_nodes[childId], _data, _runtimeData);
    }

    if (blendChildrenId.size() == 1)
    {
        const int childId = blendChildrenId[0];
        NodeRuntimeData& childRuntime = _runtimeData.m_nodeRuntimes[childId];
        runtime.m_evaluation = childRuntime.m_evaluation;
        return;
    }

    std::vector<vec4f> tmpArray;
    tmpArray.resize(blendChildrenId.size());
    auto& matchingTable = _runtimeData.m_bone2curveTable;
    runtime.m_evaluation.resize(matchingTable.size());

    int i = 0;
    for (auto& it : matchingTable)
    {
        BoneCurvesState& targetCurve = runtime.m_evaluation[i];
        targetCurve.m_boneName = it.first;
        targetCurve.m_position = vec4f::zero;
        targetCurve.m_scale = 0.f;
        vec4f vn_1 = vec4f::zero;

        bool hasData = false;
        for (int j = 0; j < blendChildrenId.size(); j++)
        {
            const int childId = blendChildrenId[j];
            NodeRuntimeData& childRuntime = _runtimeData.m_nodeRuntimes[childId];
            int curveId = it.second[childId];
            targetCurve.m_position += childRuntime.m_weight * childRuntime.m_evaluation[curveId].m_position;
            targetCurve.m_scale += childRuntime.m_weight * childRuntime.m_evaluation[curveId].m_scale;

            quatf q = childRuntime.m_evaluation[curveId].m_rotation;
            tmpArray[j] = childRuntime.m_weight * vec4f(q.x, q.y, q.z, q.w);
            vn_1 += tmpArray[j];
            hasData = true;
        }
        if (hasData)
            targetCurve.m_boneName = it.first;
        else continue;

        if (blendChildrenId.size() == 2)
        {
            const int child0 = blendChildrenId[0];
            NodeRuntimeData& childRuntime0 = _runtimeData.m_nodeRuntimes[child0];
            quatf q0 = childRuntime0.m_evaluation[it.second[child0]].m_rotation;

            const int child1 = blendChildrenId[1];
            NodeRuntimeData& childRuntime1 = _runtimeData.m_nodeRuntimes[child1];
            quatf q1 = childRuntime1.m_evaluation[it.second[child1]].m_rotation;

            targetCurve.m_rotation = quatf::slerp(q1, q0, childRuntime0.m_weight / (childRuntime0.m_weight + childRuntime1.m_weight));
        }
        else
        {
            mat4f QQt = mat4f(0.f);
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    for (int l = 0; l < tmpArray.size(); l++)
                        QQt[j][k] += tmpArray[l][k] * tmpArray[l][j];

            if (vn_1.getNorm2() < 0.1f)
                vn_1 = tmpArray[0];
            vn_1.normalize();
            vec4f vn;
            for (int j = 0; j < 50; j++)
            {
                vn = QQt * vn_1;
                vn.normalize();
                if ((vn - vn_1).getNorm2() < 1E-06f)
                    break;
                vn_1 = vn;
            }

            targetCurve.m_rotation = quatf(vn.w, vn.x, vn.y, vn.z);
        }

        i++;
    }
    runtime.m_evaluation.resize(i);
}