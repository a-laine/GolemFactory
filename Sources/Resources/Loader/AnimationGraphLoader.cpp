#include "AnimationGraphLoader.h"

#include <iostream>
#include <Utiles/Parser/Reader.h>
#include <Utiles/ConsoleColor.h>
#include <Resources/ResourceManager.h>


std::string AnimationGraphLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += AnimationGraph::directory;
    str += fileName;
    str += AnimationGraph::extension;
    return str;
}

void AnimationGraphLoader::PrintError(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : AnimationGraphLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}

void AnimationGraphLoader::PrintWarning(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : AnimationGraphLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}




bool AnimationGraphLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    //	initialization
    Variant v; Variant* tmp = nullptr;
    try
    {
        Reader::parseFile(v, getFileName(resourceDirectory, fileName));
        tmp = &(v.getMap().begin()->second);
    }
    catch (std::exception&)
    {
        if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading animation : " << fileName << " : fail to open or parse file" << std::endl;
        return false;
    }

    Variant& graphMap = *tmp;
    if (graphMap.getType() != Variant::MAP)
    {
        PrintError(fileName.c_str(), "wrong file formating");
        return false;
    }

    // clear
    m_layers.clear();
    m_variants.clear();
    m_parameters.clear();
    paramIds.clear();

    // usefull
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = (float)it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = (float)it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    // parameters
    using ptype = AnimationParameter::ParameterType;
    auto it0 = graphMap.getMap().find("parameters");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto varray = it0->second.getArray();
        for (int i = 0; i < varray.size(); i++)
        {
            auto& v = varray[i];
            if (v.getType() == Variant::MAP)
            {
                AnimationParameter parameter;

                // name
                auto it1 = v.getMap().find("name");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::STRING)
                    parameter.m_name = it1->second.toString();
                else
                    parameter.m_name = "-param(" + std::to_string(m_parameters.size()) + ")-";

                // type
                it1 = v.getMap().find("type");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::STRING)
                {
                    std::string s = it1->second.toString();
                    if (s == "int")
                        parameter.m_type = ptype::INT;
                    else if (s == "bool")
                        parameter.m_type = ptype::BOOL;
                    else if (s == "trigger")
                        parameter.m_type = ptype::TRIGGER;
                    else if (s == "float")
                        parameter.m_type = ptype::FLOAT;
                    else
                    {
                        std::string msg = parameter.m_name + " : has an invalid type (-> fallback to int)";
                        PrintWarning(fileName.c_str(), msg.c_str());
                        parameter.m_type = ptype::INT;
                    }
                }

                // value
                it1 = v.getMap().find("value");
                if (it1 != v.getMap().end())
                {
                    switch (parameter.m_type)
                    {
                        case ptype::BOOL: 
                        case ptype::TRIGGER:
                            {
                                if (it1->second.getType() == Variant::BOOL)
                                    parameter.m_value.Bool = it1->second.toBool();
                                else
                                {
                                    std::string msg = parameter.m_name + " : has an invalid value";
                                    PrintWarning(fileName.c_str(), msg.c_str());
                                    parameter.m_value.Bool = false;
                                }
                            }
                            break;
                        case ptype::INT:
                            {
                                if (it1->second.getType() == Variant::INT)
                                    parameter.m_value.Int = it1->second.toInt();
                                else
                                {
                                    std::string msg = parameter.m_name + " : has an invalid value";
                                    PrintWarning(fileName.c_str(), msg.c_str());
                                    parameter.m_value.Int = 0;
                                }
                            }
                            break;
                        case ptype::FLOAT:
                            {
                                if (it1->second.getType() == Variant::FLOAT)
                                    parameter.m_value.Float = it1->second.toFloat();
                                else if (it1->second.getType() == Variant::DOUBLE)
                                    parameter.m_value.Float = (float)it1->second.toDouble();
                                else if (it1->second.getType() == Variant::INT)
                                    parameter.m_value.Float = (float)it1->second.toInt();
                                else
                                {
                                    std::string msg = parameter.m_name + " : has an invalid value";
                                    PrintWarning(fileName.c_str(), msg.c_str());
                                    parameter.m_value.Float = 0.f;
                                }
                            }
                            break;
                        default: 
                            parameter.m_value.Int = 0;
                            break;
                    }
                }
                else
                {
                    switch (parameter.m_type)
                    {
                        case ptype::BOOL: 
                        case ptype::TRIGGER: 
                            parameter.m_value.Bool = false; 
                            break;
                        case ptype::FLOAT:
                            parameter.m_value.Float = 0.f;
                            break;
                        default: 
                            parameter.m_value.Int = 0; 
                            break;
                    }
                    parameter.m_value.Int = 0;
                }

                paramIds[parameter.m_name] = (int)m_parameters.size();
                m_parameters.push_back(parameter);
            }
        }
    }

    // layers
    it0 = graphMap.getMap().find("layers");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto varray0 = it0->second.getArray();
        for (int i = 0; i < varray0.size(); i++)
        {
            auto& v0 = varray0[i];
            if (v0.getType() == Variant::MAP)
            {
                AnimationGraph::Layer layer;
                layer.m_blendTreeEntryId = -1;
                layer.stateMachineEntryId = -1;

                // name
                auto it1 = v0.getMap().find("name");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                    layer.m_name = it1->second.toString();
                else
                    layer.m_name = "layer " + std::to_string(m_parameters.size());

                // entry names
                std::string entryName;
                it1 = v0.getMap().find("entryName");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                    entryName = it1->second.toString();

                // state machines
                it1 = v0.getMap().find("stateMachines");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::MAP)
                        {
                            loadStateMachine(fileName, v1, layer.m_stateMachines, layer.m_subGraphNames);
                            if (layer.m_stateMachines.back().m_name == entryName)
                            {
                                layer.stateMachineEntryId = (int)layer.m_stateMachines.size() - 1;
                                layer.m_blendTreeEntryId = -1;
                            }
                        }
                    }
                }

                // blend trees
                it1 = v0.getMap().find("blendTrees");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::MAP)
                        {
                            loadBlendTree(fileName, v1, layer.m_blendTrees, layer.m_subGraphNames);
                            if (layer.m_blendTrees.back().m_name == entryName)
                            {
                                layer.stateMachineEntryId = -1;
                                layer.m_blendTreeEntryId = (int)layer.m_blendTrees.size() - 1;
                            }
                        }
                    }
                }

                // push
                m_layers.push_back(layer);
            }
        }
    }

    // layers variants
    it0 = graphMap.getMap().find("variants");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto varray0 = it0->second.getArray();
        for (int i = 0; i < varray0.size(); i++)
        {
            auto& v0 = varray0[i];
            if (v0.getType() == Variant::MAP)
            {
                std::vector<AnimationGraph::LayerData> allLayersData;
                std::string variantName;

                auto it1 = v0.getMap().find("name");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                    variantName = it1->second.toString();

                it1 = v0.getMap().find("layersData");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    AnimationGraph::LayerData layerData;
                    auto varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::MAP)
                        {
                            auto it2 = v1.getMap().find("blendTreeData");
                            if (it2 != v1.getMap().end() && it2->second.getType() == Variant::ARRAY)
                            {
                                auto varray2 = it2->second.getArray();
                                for (int k = 0; k < varray2.size(); k++)
                                {
                                    auto& v3 = varray2[k];
                                    if (v3.getType() == Variant::MAP)
                                        loadBlendTreeData(fileName, v3, layerData.m_blendTreesData);
                                }
                            }

                            it2 = v1.getMap().find("stateMachineData");
                            if (it2 != v1.getMap().end() && it2->second.getType() == Variant::ARRAY)
                            {
                                auto varray2 = it2->second.getArray();
                                for (int k = 0; k < varray2.size(); k++)
                                {
                                    auto& v3 = varray2[k];
                                    if (v3.getType() == Variant::MAP)
                                        loadStateMachineData(fileName, v3, layerData.m_stateMachinesData);
                                }
                            }
                        }
                    }
                    allLayersData.push_back(layerData);
                }

                m_variants[variantName] = allLayersData;
            }
        }
    }

    // layer loading verification
    bool ok = true;
    for (const auto& it0 : m_layers)
    {
        if (it0.m_blendTreeEntryId < 0 && it0.stateMachineEntryId < 0)
        {
            std::string msg = "layer '" + it0.m_name + "' has no valid entry state";
            PrintError(fileName.c_str(), msg.c_str());
            ok = false;
        }
        if (it0.m_blendTrees.empty() && it0.m_stateMachines.empty())
        {
            std::string msg = "layer '" + it0.m_name + "' is empty";
            PrintError(fileName.c_str(), msg.c_str());
            ok = false;
        }

        for (const auto& it1 : it0.m_blendTrees)
        {
            if (it1.m_nodes.empty())
            {
                std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "' is empty";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }
            if (it1.m_entryNodeId == -1)
            {
                std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "' has no valid entry state";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }

            for (const auto& it2 : it1.m_nodes)
            {
                if (!it2.m_childrenId.empty())
                {
                    if (it2.m_parameterIds.empty())
                    {
                        std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "', node '" + it2.m_name + "' has no valid parameter array";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }
                    if (it2.m_childrenId.size() != it2.m_childrenInfluence.size())
                    {
                        std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "', node '" + it2.m_name + "' influence array size mismatch children count";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }
                    if (it2.m_childrenId.size() != it2.m_childrenPoint.size())
                    {
                        std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "', node " + it2.m_name + "' blend point array size mismatch children count";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }

                    bool invalidId = false;
                    for (const auto& it3 : it2.m_childrenId)
                        if (it3 < 0 || it3 == it2.m_id || it3 >= it1.m_nodes.size())
                        {
                            invalidId = true;
                            break;
                        }
                    if (invalidId)
                    {
                        std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "', node " + it2.m_name + "' contain invalid children ids";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }

                    invalidId = false;
                    for (const auto& it3 : it2.m_parameterIds)
                        if (it3 < 0 || it3 >= m_parameters.size())
                        {
                            invalidId = true;
                            break;
                        }
                    if (invalidId)
                    {
                        std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "', node '" + it2.m_name + "' contain invalid parameters names";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }
                }
                else
                {
                    if (it2.m_parentId == -1)
                    {
                        std::string msg = "layer '" + it0.m_name + "', blend tree '" + it1.m_name + "', node " + it2.m_name + "' has no valid parent id";
                        PrintWarning(fileName.c_str(), msg.c_str());
                    }
                }
            }
        }

        for (const auto& it1 : it0.m_stateMachines)
        {
            if (it1.m_states.empty())
            {
                std::string msg = "layer '" + it0.m_name + "', state machine '" + it1.m_name + "' is empty";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }
            if (it1.m_entryStateId == -1)
            {
                std::string msg = "layer '" + it0.m_name + "', state machine '" + it1.m_name + "' has no valid entryState";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }
            if (it1.m_anyStateId >= (int)it1.m_states.size())
            {
                std::string msg = "layer '" + it0.m_name + "', state machine '" + it1.m_name + "' has no valid anyState";
                PrintWarning(fileName.c_str(), msg.c_str());
            }

            for (const auto& it2 : it1.m_states)
            {
                if (it2.m_transitionOut.empty())
                {
                    std::string msg = "layer '" + it0.m_name + "', state machine '" + it1.m_name + "', state '" + it2.m_name + "' has no exit transition";
                    PrintWarning(fileName.c_str(), msg.c_str());
                }

                int transitionIndex = 0;
                for (const auto& it3 : it2.m_transitionOut)
                {
                    if (it3.m_stateTargetId < 0 || it3.m_stateTargetId >= it1.m_states.size())
                    {
                        std::string msg = "layer '" + it0.m_name + "', state machine '" + it1.m_name + "', state '" + it2.m_name + "', transition " + 
                            std::to_string(transitionIndex) + " has invalid target state id";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }

                    int conditionIndex = 0;
                    for (const auto& it4 : it3.m_conditions)
                    {
                        if (it4.parameterId < 0 || it4.parameterId >= m_parameters.size())
                        {
                            std::string msg = "layer '" + it0.m_name + "', state machine '" + it1.m_name + "', state " + it2.m_name + "', transition " +
                                std::to_string(transitionIndex) + ", condition " + std::to_string(conditionIndex) + " has invalid parameter id";
                        }
                        conditionIndex++;
                    }
                    transitionIndex++;
                }
            }
        }

        for (const auto& it1 : it0.m_subGraphNames)
        {
            bool found = false;
            for (const auto& it2 : it0.m_blendTrees)
            {
                if (it1 == it2.m_name)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                for (const auto& it2 : it0.m_stateMachines)
                {
                    if (it1 == it2.m_name)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                std::string msg = "layer '" + it0.m_name + "' , unknown subgraph '" + it1 + "'";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }
        }
    }

    // variant verification
    if (m_variants.empty())
    {
        PrintError(fileName.c_str(), "no data variant found !");
        ok = false;
    }
    for (auto& it0 : m_variants)
    {
        if (it0.second.size() != m_layers.size())
        {
            std::string msg = "variant '" + it0.first + "' mismatch layer count";
            PrintError(fileName.c_str(), msg.c_str());
            ok = false;
            continue;
        }

        for (int i = 0; i < m_layers.size(); i++)
        {
            if (m_layers[i].m_stateMachines.size() != it0.second[i].m_stateMachinesData.size())
            {
                std::string msg = "variant '" + it0.first + "', layer '" + m_layers[i].m_name + "' mismatch state machine count";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }
            else
            {
                for (int j = 0; j < m_layers[i].m_stateMachines.size(); j++)
                {
                    if (m_layers[i].m_stateMachines[j].m_states.size() != it0.second[i].m_stateMachinesData[j].m_statesData.size())
                    {
                        std::string msg = "variant '" + it0.first + "', layer '" + m_layers[i].m_name + "', state machine '" + m_layers[i].m_stateMachines[j].m_name + 
                            "' mismatch state count";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }
                }
            }

            if (m_layers[i].m_blendTrees.size() != it0.second[i].m_blendTreesData.size())
            {
                std::string msg = "variant '" + it0.first + "', layer '" + m_layers[i].m_name + "' mismatch blend trees count";
                PrintError(fileName.c_str(), msg.c_str());
                ok = false;
            }
            else
            {
                for (int j = 0; j < m_layers[i].m_blendTrees.size(); j++)
                {
                    if (m_layers[i].m_blendTrees[j].m_nodes.size() != it0.second[i].m_blendTreesData[j].m_nodeData.size())
                    {
                        std::string msg = "variant '" + it0.first + "', layer '" + m_layers[i].m_name + "', blend tree '" + m_layers[i].m_blendTrees[j].m_name + "' mismatch node count";
                        PrintError(fileName.c_str(), msg.c_str());
                        ok = false;
                    }
                }
            }
        }
    }

    return ok;
}

void AnimationGraphLoader::initialize(ResourceVirtual* resource)
{
    AnimationGraph* graph = static_cast<AnimationGraph*>(resource);
    graph->initialize(m_layers);
    graph->setParameters(m_parameters);
    graph->setVariants(m_variants);
}


void AnimationGraphLoader::loadStateMachine(const std::string& fileName, Variant& variant, std::vector<AG::StateMachine>& targetArray,
    std::vector<std::string>& _subGrapTarget)
{
    using AGSM = AG::StateMachine;
    using ptype = AnimationParameter::ParameterType;
    using ctype = AGSM::Condition::ComparisonType;

    // usefull
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = (float)it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = (float)it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    // name
    AG::StateMachine stateMachine;
    stateMachine.m_entryStateId = -1;
    stateMachine.m_anyStateId = -1;
    auto it0 = variant.getMap().find("name");
    if (it0 != variant.getMap().end() && it0->second.getType() == Variant::STRING)
        stateMachine.m_name = it0->second.toString();

    // entry state
    std::string entryStateName;
    it0 = variant.getMap().find("entryState");
    if (it0 != variant.getMap().end() && it0->second.getType() == Variant::STRING)
        entryStateName = it0->second.toString();

    // states
    std::map<std::string, int> stateIds;
    it0 = variant.getMap().find("states");
    if (it0 != variant.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto& varray0 = it0->second.getArray();
        for (int i = 0; i < varray0.size(); i++)
        {
            auto& v0 = varray0[i];
            if (v0.getType() == Variant::MAP)
            {
                AGSM::State state;
                state.m_id = (int)stateMachine.m_states.size();
                state.m_subgraphId = -1;
                state.m_exitTime = -1.f;

                // name
                auto it1 = v0.getMap().find("name");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                    state.m_name = it1->second.toString();
                else
                    state.m_name = "-state(" + std::to_string(state.m_id) + ")-";
                stateIds[state.m_name] = state.m_id;

                // subgraph
                it1 = v0.getMap().find("subgraphName");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                {
                    state.m_subgraphId = (int)_subGrapTarget.size(); 
                    _subGrapTarget.push_back(it1->second.toString());
                }

                // exit time
                TryLoadAsFloat(v0, "exitTime", state.m_exitTime);

                // transitions
                it1 = v0.getMap().find("transitions");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::MAP)
                        {
                            AGSM::Transition transition;
                            transition.m_stateTargetId = -1;

                            // states
                            auto it2 = v1.getMap().find("to");
                            if (it2 != v1.getMap().end() && it2->second.getType() == Variant::INT)
                                transition.m_stateTargetId = it2->second.toInt();

                            // duration
                            if (!TryLoadAsFloat(v1, "duration", transition.m_duration))
                            {
                                transition.m_duration = 0.f;
                                PrintWarning(fileName.c_str(), "a transition had no duration");
                            }

                            // conditions
                            it2 = v1.getMap().find("conditions");
                            if (it2 != v1.getMap().end() && it2->second.getType() == Variant::ARRAY)
                            {
                                auto& varray2 = it2->second.getArray();
                                for (int k = 0; k < varray2.size(); k++)
                                {
                                    auto& v2 = varray2[k];
                                    if (v2.getType() == Variant::MAP)
                                    {
                                        // name
                                        AGSM::Condition condition;
                                        condition.parameterId = -1;
                                        auto it3 = v2.getMap().find("paramName");
                                        if (it3 != v2.getMap().end() && it3->second.getType() == Variant::STRING)
                                        {
                                            auto it4 = paramIds.find(it3->second.toString());
                                            if (it4 != paramIds.end())
                                            {
                                                condition.parameterId = it4->second;
                                                condition.m_parameter = m_parameters[condition.parameterId];
                                            }
                                        }

                                        // comparison
                                        it3 = v2.getMap().find("comparison");
                                        if (it3 != v2.getMap().end() && it3->second.getType() == Variant::STRING)
                                        {
                                            std::string s = it3->second.toString();
                                            if (s == "greater")
                                                condition.m_comparisonType = ctype::GREATER;
                                            else if (s == "less")
                                                condition.m_comparisonType = ctype::LESS;
                                            else if (s == "equals")
                                                condition.m_comparisonType = ctype::EQUALS;
                                            else if (s == "notEquals")
                                                condition.m_comparisonType = ctype::NOT_EQUALS;
                                            else
                                            {
                                                PrintWarning(fileName.c_str(), "a condition has an invalid comparison type (was set to equals)");
                                                condition.m_comparisonType = ctype::EQUALS;
                                            }
                                        }

                                        // value
                                        it3 = v2.getMap().find("value");
                                        if (it3 != v2.getMap().end())
                                        {
                                            switch (condition.m_parameter.m_type)
                                            {
                                                case ptype::BOOL:
                                                case ptype::TRIGGER:
                                                {
                                                    if (it3->second.getType() == Variant::BOOL)
                                                        condition.m_parameter.m_value.Bool = it3->second.toBool();
                                                    else
                                                    {
                                                        std::string msg = "condition on " + condition.m_parameter.m_name + " : has an invalid test value";
                                                        PrintWarning(fileName.c_str(), msg.c_str());
                                                    }
                                                }
                                                break;
                                                case ptype::INT:
                                                {
                                                    if (it3->second.getType() == Variant::INT)
                                                        condition.m_parameter.m_value.Int = it3->second.toInt();
                                                    else
                                                    {
                                                        std::string msg = "condition on " + condition.m_parameter.m_name + " : has an invalid test value";
                                                        PrintWarning(fileName.c_str(), msg.c_str());
                                                    }
                                                }
                                                break;
                                                case ptype::FLOAT:
                                                {
                                                    if (it3->second.getType() == Variant::FLOAT)
                                                        condition.m_parameter.m_value.Float = it3->second.toFloat();
                                                    else if (it3->second.getType() == Variant::DOUBLE)
                                                        condition.m_parameter.m_value.Float = (float)it3->second.toDouble();
                                                    else
                                                    {
                                                        std::string msg = "condition on " + condition.m_parameter.m_name + " : has an invalid test value";
                                                        PrintWarning(fileName.c_str(), msg.c_str());
                                                    }
                                                }
                                                break;
                                                default:
                                                    break;
                                            }
                                        }

                                        transition.m_conditions.push_back(condition);
                                    }
                                }
                            }

                            // end
                            state.m_transitionOut.push_back(transition);
                        }
                    }
                }

                if (state.m_name == entryStateName)
                    stateMachine.m_entryStateId = state.m_id;
                if (state.m_name == "_anyState")
                    stateMachine.m_anyStateId = state.m_id;
                stateMachine.m_states.push_back(state);
            }
        }
    }
    targetArray.push_back(stateMachine);
}

void AnimationGraphLoader::loadStateMachineData(const std::string& fileName, Variant& variant, std::vector<AG::StateMachine::StateMachineData>& targetData)
{
    using AGSM = AG::StateMachine;
    AGSM::StateMachineData stateMachineData;
    ResourceManager* manager = ResourceManager::getInstance();

    // usefull
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = (float)it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = (float)it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    auto it1 = variant.getMap().find("statesData");
    if (it1 != variant.getMap().end() && it1->second.getType() == Variant::ARRAY)
    {
        auto& vdarray = it1->second.getArray();
        for (int j = 0; j < vdarray.size(); j++)
        {
            auto& vd = vdarray[j];
            if (vd.getType() == Variant::MAP)
            {
                AGSM::StateData stateData;
                stateData.m_animation = nullptr;
                stateData.m_speed = 1.f;

                auto it2 = vd.getMap().find("clip");
                if (it2 != vd.getMap().end() && it2->second.getType() == Variant::STRING)
                    stateData.m_animation = manager->getResource<AnimationClip>(it2->second.toString());

                TryLoadAsFloat(vd, "speed", stateData.m_speed);

                stateMachineData.m_statesData.push_back(stateData);
            }
        }
    }
    targetData.push_back(stateMachineData);
}

void AnimationGraphLoader::loadBlendTree(const std::string& fileName, Variant& variant, std::vector<AG::BlendTree>& targetArray,
    std::vector<std::string>& _subGrapTarget)
{
    using AGBT = AG::BlendTree;
    using ptype = AnimationParameter::ParameterType;

    // usefull
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = (float)it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = (float)it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };
    const auto TryLoadAsVec4f = [](Variant& variant, vec4f& destination)
    {
        int sucessfullyParsed = 0;
        if (variant.getType() == Variant::ARRAY)
        {
            auto varray = variant.getArray();
            vec4f parsed = destination;
            for (int i = 0; i < 4 && i < varray.size(); i++)
            {
                auto& element = varray[i];
                if (element.getType() == Variant::FLOAT)
                {
                    parsed[i] = element.toFloat();
                    sucessfullyParsed++;
                }
                else if (element.getType() == Variant::DOUBLE)
                {
                    parsed[i] = (float)element.toDouble();
                    sucessfullyParsed++;
                }
                else if (element.getType() == Variant::INT)
                {
                    parsed[i] = (float)element.toInt();
                    sucessfullyParsed++;
                }
            }
            destination = parsed;
        }
        return sucessfullyParsed;
    };

    // name
    AG::BlendTree blendTree;
    blendTree.m_entryNodeId = -1;
    auto it0 = variant.getMap().find("name");
    if (it0 != variant.getMap().end() && it0->second.getType() == Variant::STRING)
        blendTree.m_name = it0->second.toString();

    // entry state
    std::string entryNodeName;
    it0 = variant.getMap().find("entryNode");
    if (it0 != variant.getMap().end() && it0->second.getType() == Variant::STRING)
        entryNodeName = it0->second.toString();

    // nodes
    std::map<std::string, int> nodeIds;
    it0 = variant.getMap().find("nodes");
    if (it0 != variant.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto& varray0 = it0->second.getArray();
        for (int i = 0; i < varray0.size(); i++)
        {
            auto& v0 = varray0[i];
            if (v0.getType() == Variant::MAP)
            {
                AGBT::Node node;
                node.m_id = (int)blendTree.m_nodes.size();
                node.m_globalInfluence = 1.f;
                node.m_subgraphId = -1;

                // name
                auto it1 = v0.getMap().find("name");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                    node.m_name = it1->second.toString();
                if (node.m_name == entryNodeName)
                    blendTree.m_entryNodeId = node.m_id;

                // subgraph
                it1 = v0.getMap().find("subgraphName");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::STRING)
                {
                    node.m_subgraphId = (int)_subGrapTarget.size();
                    _subGrapTarget.push_back(it1->second.toString());
                }

                // parent id
                it1 = v0.getMap().find("parentId");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::INT)
                    node.m_parentId = it1->second.toInt();
                else node.m_parentId = -1;

                // children ids
                it1 = v0.getMap().find("childrenIds");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::INT)
                            node.m_childrenId.push_back(v1.toInt());
                        else
                            node.m_childrenId.push_back(-1);
                    }
                }

                // global influence
                TryLoadAsFloat(v0, "globalInfluence", node.m_globalInfluence);

                // parameters
                it1 = v0.getMap().find("blendParameters");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::STRING)
                        {
                            auto it2 = paramIds.find(v1.toString());
                            if (it2 != paramIds.end())
                                node.m_parameterIds.push_back(it2->second);
                            else
                                node.m_parameterIds.push_back(-1);
                        }
                    }
                }

                // blend point
                it1 = v0.getMap().find("blendPoints");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        if (v1.getType() == Variant::ARRAY)
                        {
                            vec4f point = vec4f::zero;
                            int dim = TryLoadAsVec4f(v1, point);
                            if (dim != node.m_parameterIds.size())
                                PrintWarning(fileName.c_str(), "blendPoints wrong dimension");
                            node.m_childrenPoint.push_back(point);
                        }
                    }
                }

                // blend influence
                it1 = v0.getMap().find("blendInfluences");
                if (it1 != v0.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& varray1 = it1->second.getArray();
                    for (int j = 0; j < varray1.size(); j++)
                    {
                        auto& v1 = varray1[j];
                        float influence = 1.f;
                        if (v1.getType() == Variant::FLOAT)
                            influence = v1.toFloat();
                        else if (v1.getType() == Variant::DOUBLE)
                            influence = (float)v1.toDouble();
                        else if (v1.getType() == Variant::INT)
                            influence = (float)v1.toInt();

                        node.m_childrenInfluence.push_back(influence);
                    }
                }

                blendTree.m_nodes.push_back(node);
            }
        }
    }
    targetArray.push_back(blendTree);
}

void AnimationGraphLoader::loadBlendTreeData(const std::string& fileName, Variant& variant, std::vector<AG::BlendTree::BlendTreeData>& targetData)
{
    using AGBT = AG::BlendTree;

    // usefull
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = (float)it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = (float)it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    AGBT::BlendTreeData treeData;
    ResourceManager* manager = ResourceManager::getInstance();
    if (variant.getType() == Variant::MAP)
    {
        auto it1 = variant.getMap().find("nodesData");
        if (it1 != variant.getMap().end() && it1->second.getType() == Variant::ARRAY)
        {
            auto& varray1 = it1->second.getArray();
            for (int j = 0; j < varray1.size(); j++)
            {
                auto& v1 = varray1[j];
                if (v1.getType() == Variant::MAP)
                {
                    AGBT::NodeData data;

                    data.m_animation = nullptr;
                    data.m_speed = 1.f;

                    auto it2 = v1.getMap().find("clip");
                    if (it2 != v1.getMap().end() && it2->second.getType() == Variant::STRING)
                        data.m_animation = manager->getResource<AnimationClip>(it2->second.toString());

                    bool foundSpeed = TryLoadAsFloat(v1, "speed", data.m_speed);

                    treeData.m_nodeData.push_back(data);
                }
            }
        }
    }
    targetData.push_back(treeData);
}