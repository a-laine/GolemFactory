#include "AnimationGraph.h"
#include "ResourceManager.h"
#include "Skeleton.h"

const char* AnimationGraphParameter::g_ParameterTypeCombo = "Trigger\0Bool\0Int\0Float\0\0";
char const* const AnimationGraph::directory = "Animations/";
char const* const AnimationGraph::extension = ".animGraph";
std::string AnimationGraph::defaultName;

bool AnimationGraphCondition::evaluate(const AnimationGraphParameter& _comparisonParameter) const
{
    using ptype = AnimationGraphParameter::ParameterType;
    if (_comparisonParameter.m_type != m_parameter.m_type)
        return false;

    switch (m_parameter.m_type)
    {
        case ptype::BOOL:
        case ptype::TRIGGER:
            return _comparisonParameter.m_value.Bool;
        case ptype::FLOAT:
        {

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
                default: return false;
            }
        }
        case ptype::INT:
        {
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
                default: return false;
            }
        }
        default: return false;
    }
}

AnimationGraphData::~AnimationGraphData()
{
    ResourceManager* manager = ResourceManager::getInstance();
    for (AnimationGraphStateData& data : m_statesData)
    {
        manager->release(data.m_animation);
        data.m_animation = nullptr;
    }
}

/*void AnimationGraphRuntimeData::initialize(const Skeleton* _skeleton)
{
    m_currentStateEvaluation.clear();
    m_skeletonFinalPose.clear();
    if (!_skeleton || m_currentStateEvaluation.empty())
        return;

    for (int i = 0; i < m_currentStateEvaluation.size(); i++)
    {
        m_currentStateEvaluation[i].m_skeletonBoneIndex = _skeleton ? (_skeleton->getBoneId(m_currentStateEvaluation[i].m_boneName)) : -1;

    }        
    const int boneCount = _skeleton->getBones().size();
    for (int i = 0; i < boneCount; i++)
    {
        int index = -1;
        for (int j = 0; j < m_currentStateEvaluation.size(); j++)
        {
            if (m_currentStateEvaluation[j].m_skeletonBoneIndex == i)
            {
                index = j;
                break;
            }
        }
        m_bones2curveState.push_back(index);
        m_skeletonFinalPose.push_back(mat4f::identity);
    }
        
    const std::vector<Skeleton::Bone>& bones = _skeleton->getBones();
    for (int i = 0; i < bones.size(); i++)
    {
        mat4f parent = bones[i].parent ? m_skeletonFinalPose[bones[i].parent->id] : mat4f::identity;
        mat4f trs;
        if (m_bones2curveState[i] >= 0)
        {
            const BoneCurvesState& state = m_currentStateEvaluation[m_bones2curveState[i]];
            trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(i == 0 ? 0.01 : 1.f));
        }
        else
            trs = bones[i].relativeBindTransform;
        m_skeletonFinalPose[i] = parent * trs;
    }
}*/



AnimationGraph::AnimationGraph(const std::string& _graphName)
    : ResourceVirtual(_graphName, ResourceVirtual::ResourceType::ANIMATION_GRAPH)
{
    m_entryState = 0;
}

const std::vector<AnimationGraphState>& AnimationGraph::getStates() const { return m_states; }

bool AnimationGraph::setState(unsigned int _stateId, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const
{
    if (_stateId >= 0 && _stateId < m_states.size())
    {
        _runtimeData.m_stateTime = 0.f;
        _runtimeData.m_currentStateIndex = _stateId;
        _runtimeData.m_currentStateEvaluation.clear();
        _runtimeData.m_bones2currentStateEvaluation.clear();

        _runtimeData.m_transitionTime = 0.f;
        _runtimeData.m_currentTransition = nullptr;
        _runtimeData.m_currentTargetEvaluation.clear();
        _runtimeData.m_bones2currentTargetEvaluation.clear();

        if (_runtimeData.m_skeleton)
        {
            const AnimationGraphState& state = m_states[_stateId];
            const AnimationClip* clip = _data.m_statesData[_stateId].m_animation;
            const std::vector<Skeleton::Bone>& bones = _runtimeData.m_skeleton->m_bones;

            _runtimeData.m_bones2currentStateEvaluation.assign(bones.size(), -1);
            if (_runtimeData.m_skeletonFinalPose.size() != bones.size())
                _runtimeData.m_skeletonFinalPose.assign(bones.size(), mat4f::identity);

            for (int i = 0; i < clip->m_boneCurves.size(); i++)
            {
                AnimationGraphRuntimeData::BoneCurvesState state;
                state.m_boneName = clip->m_boneCurves[i].m_boneName;
                state.m_posKey = state.m_rotKey = state.m_scaleKey = 0;

                state.m_position = clip->m_boneCurves[i].m_positionCurve[0].m_value;
                const vec4f v = clip->m_boneCurves[i].m_rotationCurve[0].m_value;
                state.m_rotation = quatf(v.w, v.x, v.y, v.z);
                state.m_scale = clip->m_boneCurves[i].m_scaleCurve[0].m_value;

                int boneId = _runtimeData.m_skeleton->getBoneId(state.m_boneName);
                state.m_skeletonBoneIndex = boneId;
                if (boneId >= 0)
                    _runtimeData.m_bones2currentStateEvaluation[boneId] = i;
                _runtimeData.m_currentStateEvaluation.push_back(state);
            }

            for (int i = 0; i < bones.size(); i++)
            {
                mat4f parent = bones[i].parent ? _runtimeData.m_skeletonFinalPose[bones[i].parent->id] : mat4f::identity;
                mat4f trs;
                int id = _runtimeData.m_bones2currentStateEvaluation[i];
                if (id >= 0)
                {
                    const AnimationGraphRuntimeData::BoneCurvesState& state = 
                        _runtimeData.m_currentStateEvaluation[id];
                    trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(state.m_scale));
                }
                else
                    trs = bones[i].relativeBindTransform;
                _runtimeData.m_skeletonFinalPose[i] = parent * trs;
            }
        }

        return true;
    }
    return false;
}

void AnimationGraph::setTransition(AnimationGraphTransition* _transition, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const
{
    _runtimeData.m_targetStateTime = 0.f;
    _runtimeData.m_transitionTime = 0.f;
    _runtimeData.m_currentTransition = _transition;
    _runtimeData.m_currentTargetEvaluation.clear();
    _runtimeData.m_bones2currentTargetEvaluation.clear();

    if (_runtimeData.m_skeleton)
    {
        const AnimationGraphState& state = *_transition->m_stateTo;
        const AnimationClip* clip = _data.m_statesData[state.id].m_animation;
        const std::vector<Skeleton::Bone>& bones = _runtimeData.m_skeleton->m_bones;

        _runtimeData.m_bones2currentTargetEvaluation.assign(bones.size(), -1);

        for (int i = 0; i < clip->m_boneCurves.size(); i++)
        {
            AnimationGraphRuntimeData::BoneCurvesState state;
            state.m_boneName = clip->m_boneCurves[i].m_boneName;
            state.m_posKey = state.m_rotKey = state.m_scaleKey = 0;

            state.m_position = clip->m_boneCurves[i].m_positionCurve[0].m_value;
            const vec4f v = clip->m_boneCurves[i].m_rotationCurve[0].m_value;
            state.m_rotation = quatf(v.w, v.x, v.y, v.z);
            state.m_scale = clip->m_boneCurves[i].m_scaleCurve[0].m_value;

            int boneId = _runtimeData.m_skeleton->getBoneId(state.m_boneName);
            state.m_skeletonBoneIndex = boneId;
            if (boneId >= 0)
                _runtimeData.m_bones2currentTargetEvaluation[boneId] = i;
            _runtimeData.m_currentTargetEvaluation.push_back(state);
        }
    }
}

bool AnimationGraph::setState(const std::string& _stateName, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const
{
    for (int i = 0; i < m_states.size(); i++)
    {
        if (m_states[i].m_name == _stateName)
            return setState(i, _data, _runtimeData);
    }
    return false;
}

AnimationGraph::~AnimationGraph()
{}

void AnimationGraph::initialize(std::vector<AnimationGraphState>& _states, std::vector<AnimationGraphTransition>& _transitions, 
    unsigned int _entryState)
{
    m_states.swap(_states);
    m_transitions.swap(_transitions);
    m_entryState = _entryState;
    state = VALID;
}

/*void AnimationGraph::addVariant(const std::string& _variantName, AnimationGraphData* _variantData)
{
    m_dataVariant[_variantName] = _variantData;
}*/
void AnimationGraph::setVariants(std::map<std::string, AnimationGraphData>& _variants)
{
    m_dataVariant.swap(_variants);
}

unsigned int AnimationGraph::getEntryState() const { return m_entryState; }
void AnimationGraph::setDefaultParameters(std::vector<AnimationGraphParameter>& _parameters) { m_defaultParameters.swap(_parameters); }
std::vector<AnimationGraphParameter> AnimationGraph::getParametersCopy() const { return m_defaultParameters; }

const AnimationGraphData* AnimationGraph::getVariant(const std::string& _name) const
{
    const auto& it = m_dataVariant.find(_name);
    if (it != m_dataVariant.end())
        return &(it->second);
    return nullptr;
}

void AnimationGraph::evaluate(float _elapsedTime, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const
{
    // evaluate transition's conditions
    if (!_runtimeData.m_currentTransition)
    {
        const std::vector<AnimationGraphTransition*>& transitions = m_states[_runtimeData.m_currentStateIndex].m_transitionOut;
        for (const AnimationGraphTransition* transition : transitions)
        {
            const std::vector<AnimationGraphCondition>& conditions = transition->m_conditions;
            bool isValid = true;
            for (const AnimationGraphCondition& condition : conditions)
            {
                isValid = condition.evaluate(_runtimeData.m_parameters[condition.parameterId]);
                if (!isValid)
                    break;
            }
            if (isValid)
            {
                setTransition((AnimationGraphTransition*) transition, _data, _runtimeData);
                break;
            }
        }
    }

    // evaluate transition curves
    else
    {
        const AnimationGraphTransition* transition = _runtimeData.m_currentTransition;
        _runtimeData.m_transitionTime += _elapsedTime;

        // end transition
        if (_runtimeData.m_transitionTime >= transition->m_duration)
        {
            _runtimeData.m_currentStateIndex = transition->m_stateTo->id;
            _runtimeData.m_stateTime = _runtimeData.m_targetStateTime;
            _runtimeData.m_bones2currentStateEvaluation.swap(_runtimeData.m_bones2currentTargetEvaluation);
            _runtimeData.m_currentStateEvaluation.swap(_runtimeData.m_currentTargetEvaluation);

            _runtimeData.m_bones2currentTargetEvaluation.clear();
            _runtimeData.m_currentTargetEvaluation.clear();
            _runtimeData.m_currentTransition = nullptr;
            _runtimeData.m_targetStateTime = 0.f;
        }

        // evaluate transition
        else
        {
            const AnimationGraphStateData& targetData = _data.m_statesData[transition->m_stateTo->id];
            const AnimationClip* clip = targetData.m_animation;

            _runtimeData.m_targetStateTime += targetData.m_speed * _elapsedTime;

            // animation loop and reset evals
            if (_runtimeData.m_targetStateTime >= clip->m_duration)
            {
                while (_runtimeData.m_targetStateTime >= clip->m_duration)
                    _runtimeData.m_targetStateTime -= clip->m_duration;

                const float time = _runtimeData.m_targetStateTime;
                for (int i = 0; i < _runtimeData.m_currentTargetEvaluation.size(); i++)
                {
                    AnimationGraphRuntimeData::BoneCurvesState& state = _runtimeData.m_currentTargetEvaluation[i];
                    const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

                    for (int j = 1; j < curve.m_positionCurve.size(); j++)
                    {
                        if (curve.m_positionCurve[j].m_time > time)
                        {
                            state.m_posKey = j - 1;
                            break;
                        }
                    }
                    for (int j = 1; j < curve.m_scaleCurve.size(); j++)
                    {
                        if (curve.m_scaleCurve[j].m_time > time)
                        {
                            state.m_scaleKey = j - 1;
                            break;
                        }
                    }
                    for (int j = 1; j < curve.m_rotationCurve.size(); j++)
                    {
                        if (curve.m_rotationCurve[j].m_time > time)
                        {
                            state.m_rotKey = j - 1;
                            break;
                        }
                    }
                }
            }

            // evaluate
            const float time = _runtimeData.m_targetStateTime;
            for (int i = 0; i < _runtimeData.m_currentTargetEvaluation.size(); i++)
            {
                AnimationGraphRuntimeData::BoneCurvesState& state = _runtimeData.m_currentTargetEvaluation[i];
                const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

                // change curve keyframes if needed
                if (time > curve.m_positionCurve[state.m_posKey + 1].m_time)
                {
                    for (int j = state.m_posKey + 2; j < curve.m_positionCurve.size(); j++)
                    {
                        if (curve.m_positionCurve[j].m_time > time)
                        {
                            state.m_posKey = j - 1;
                            break;
                        }
                    }
                }
                if (time > curve.m_rotationCurve[state.m_rotKey + 1].m_time)
                {
                    for (int j = state.m_rotKey + 2; j < curve.m_rotationCurve.size(); j++)
                    {
                        if (curve.m_rotationCurve[j].m_time > time)
                        {
                            state.m_rotKey = j - 1;
                            break;
                        }
                    }
                }
                if (time > curve.m_scaleCurve[state.m_scaleKey + 1].m_time)
                {
                    for (int j = state.m_scaleKey + 2; j < curve.m_scaleCurve.size(); j++)
                    {
                        if (curve.m_scaleCurve[j].m_time > time)
                        {
                            state.m_scaleKey = j - 1;
                            break;
                        }
                    }
                }

                // evaluate segments
                float t = (time - curve.m_positionCurve[state.m_posKey].m_time) / (curve.m_positionCurve[state.m_posKey + 1].m_time - curve.m_positionCurve[state.m_posKey].m_time);
                state.m_position = vec4f::lerp(curve.m_positionCurve[state.m_posKey].m_value, curve.m_positionCurve[state.m_posKey + 1].m_value, t);

                t = (time - curve.m_rotationCurve[state.m_rotKey].m_time) / (curve.m_rotationCurve[state.m_rotKey + 1].m_time - curve.m_rotationCurve[state.m_rotKey].m_time);
                vec4f v0 = curve.m_rotationCurve[state.m_rotKey].m_value;
                vec4f v1 = curve.m_rotationCurve[state.m_rotKey + 1].m_value;
                state.m_rotation = quatf::slerp(quatf(v0.w, v0.x, v0.y, v0.z), quatf(v1.w, v1.x, v1.y, v1.z), t);

                t = (time - curve.m_scaleCurve[state.m_scaleKey].m_time) / (curve.m_scaleCurve[state.m_scaleKey + 1].m_time - curve.m_scaleCurve[state.m_scaleKey].m_time);
                state.m_scale = lerp(curve.m_scaleCurve[state.m_scaleKey].m_value, curve.m_scaleCurve[state.m_scaleKey + 1].m_value, t);
            }
        }
    }



    // current state loop
    const AnimationGraphStateData& stateData = _data.m_statesData[_runtimeData.m_currentStateIndex];
    const AnimationClip* clip = stateData.m_animation;
    _runtimeData.m_stateTime += stateData.m_speed * _elapsedTime;
    if (_runtimeData.m_stateTime >= clip->m_duration)
    {
        while (_runtimeData.m_stateTime >= clip->m_duration)
            _runtimeData.m_stateTime -= clip->m_duration;

        const float time = _runtimeData.m_stateTime;
        for (int i = 0; i < _runtimeData.m_currentStateEvaluation.size(); i++)
        {
            AnimationGraphRuntimeData::BoneCurvesState& state = _runtimeData.m_currentStateEvaluation[i];
            const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

            for (int j = 1; j < curve.m_positionCurve.size(); j++)
            {
                if (curve.m_positionCurve[j].m_time > time)
                {
                    state.m_posKey = j - 1;
                    break;
                }
            }
            for (int j = 1; j < curve.m_scaleCurve.size(); j++)
            {
                if (curve.m_scaleCurve[j].m_time > time)
                {
                    state.m_scaleKey = j - 1;
                    break;
                }
            }
            for (int j = 1; j < curve.m_rotationCurve.size(); j++)
            {
                if (curve.m_rotationCurve[j].m_time > time)
                {
                    state.m_rotKey = j - 1;
                    break;
                }
            }
        }
    }

    // evaluate current state
    const float time = _runtimeData.m_stateTime;
    for (int i = 0; i < _runtimeData.m_currentStateEvaluation.size(); i++)
    {
        AnimationGraphRuntimeData::BoneCurvesState& state = _runtimeData.m_currentStateEvaluation[i];
        const AnimationClip::BoneCurves& curve = clip->m_boneCurves[i];

        // change curve keyframes if needed
        if (time > curve.m_positionCurve[state.m_posKey + 1].m_time)
        {
            for (int j = state.m_posKey + 2; j < curve.m_positionCurve.size(); j++)
            {
                if (curve.m_positionCurve[j].m_time > time)
                {
                    state.m_posKey = j - 1;
                    break;
                }
            }
        }
        if (time > curve.m_rotationCurve[state.m_rotKey + 1].m_time)
        {
            for (int j = state.m_rotKey + 2; j < curve.m_rotationCurve.size(); j++)
            {
                if (curve.m_rotationCurve[j].m_time > time)
                {
                    state.m_rotKey = j - 1;
                    break;
                }
            }
        }
        if (time > curve.m_scaleCurve[state.m_scaleKey + 1].m_time)
        {
            for (int j = state.m_scaleKey + 2; j < curve.m_scaleCurve.size(); j++)
            {
                if (curve.m_scaleCurve[j].m_time > time)
                {
                    state.m_scaleKey = j - 1;
                    break;
                }
            }
        }

        // evaluate segments
        float t = (time - curve.m_positionCurve[state.m_posKey].m_time) / (curve.m_positionCurve[state.m_posKey + 1].m_time - curve.m_positionCurve[state.m_posKey].m_time);
        state.m_position = vec4f::lerp(curve.m_positionCurve[state.m_posKey].m_value, curve.m_positionCurve[state.m_posKey + 1].m_value, t);

        t = (time - curve.m_rotationCurve[state.m_rotKey].m_time) / (curve.m_rotationCurve[state.m_rotKey + 1].m_time - curve.m_rotationCurve[state.m_rotKey].m_time);
        vec4f v0 = curve.m_rotationCurve[state.m_rotKey].m_value;
        vec4f v1 = curve.m_rotationCurve[state.m_rotKey + 1].m_value;
        state.m_rotation = quatf::slerp(quatf(v0.w, v0.x, v0.y, v0.z), quatf(v1.w, v1.x, v1.y, v1.z), t);

        t = (time - curve.m_scaleCurve[state.m_scaleKey].m_time) / (curve.m_scaleCurve[state.m_scaleKey + 1].m_time - curve.m_scaleCurve[state.m_scaleKey].m_time);
        state.m_scale = lerp(curve.m_scaleCurve[state.m_scaleKey].m_value, curve.m_scaleCurve[state.m_scaleKey + 1].m_value, t);
    }

    // compute matrices
    const std::vector<Skeleton::Bone>& bones = _runtimeData.m_skeleton->m_bones;
    float t = 0.f;
    if (_runtimeData.m_currentTransition)
        t = _runtimeData.m_transitionTime / _runtimeData.m_currentTransition->m_duration;

    for (int i = 0; i < bones.size(); i++)
    {
        mat4f parent = bones[i].parent ? _runtimeData.m_skeletonFinalPose[bones[i].parent->id] : mat4f::identity;
        mat4f trs;
        if (_runtimeData.m_currentTransition)
        {
            int fromid = _runtimeData.m_bones2currentStateEvaluation[i];
            int toid = _runtimeData.m_bones2currentTargetEvaluation[i];
            if (fromid >= 0)
            {
                if (toid >= 0)
                {
                    const AnimationGraphRuntimeData::BoneCurvesState& state0 = _runtimeData.m_currentStateEvaluation[fromid];
                    const AnimationGraphRuntimeData::BoneCurvesState& state1 = _runtimeData.m_currentTargetEvaluation[toid];

                    vec4f pos = vec4f::lerp(state0.m_position, state1.m_position, t);
                    quatf rot = quatf::slerp(state0.m_rotation, state1.m_rotation, t);
                    float s = lerp(state0.m_scale, state1.m_scale, t);
                    trs = mat4f::TRS(pos, rot, vec4f(s));
                }
                else
                {
                    const AnimationGraphRuntimeData::BoneCurvesState& state = _runtimeData.m_currentStateEvaluation[fromid];
                    trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(state.m_scale));
                }
            }
            else if (toid >= 0)
            {
                const AnimationGraphRuntimeData::BoneCurvesState& state = _runtimeData.m_currentTargetEvaluation[toid];
                trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(state.m_scale));
            }
            else trs = bones[i].relativeBindTransform;
        }
        else
        {
            int id = _runtimeData.m_bones2currentStateEvaluation[i];
            if (id >= 0)
            {
                const AnimationGraphRuntimeData::BoneCurvesState& state =
                    _runtimeData.m_currentStateEvaluation[id];
                trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(state.m_scale));
            }
            else
                trs = bones[i].relativeBindTransform;
        }
        _runtimeData.m_skeletonFinalPose[i] = parent * trs;
    }
}


std::string AnimationGraph::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
const std::string& AnimationGraph::getDefaultName() { return AnimationGraph::defaultName; };
void AnimationGraph::setDefaultName(const std::string& name) { AnimationGraph::defaultName = name; };
std::string AnimationGraph::getIdentifier() const { return AnimationGraph::getIdentifier(name); }
std::string AnimationGraph::getLoaderId(const std::string& resourceName) const
{
    return extension;
}

void AnimationGraph::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Graph infos");
#endif
}