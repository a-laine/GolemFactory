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
    m_entryState = -1;
    m_states.clear();
    m_transitions.clear();
    m_dataVariant.clear();
    m_parameters.clear();

    // usefull
    const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
    {
        auto it0 = variant.getMap().find(label);
        if (it0 != variant.getMap().end())
        {
            if (it0->second.getType() == Variant::FLOAT)
                destination = it0->second.toFloat();
            else if (it0->second.getType() == Variant::DOUBLE)
                destination = it0->second.toDouble();
            else if (it0->second.getType() == Variant::INT)
                destination = it0->second.toInt();
            else return false;
            return true;
        }
        return false;
    };

    // entry state
    std::string entryStateName;
    auto it0 = graphMap.getMap().find("entryState");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::STRING)
        entryStateName = it0->second.toString();

    // states
    std::map<std::string, int> stateIds;
    it0 = graphMap.getMap().find("states");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto varray = it0->second.getArray();
        for (int i = 0; i < varray.size(); i++)
        {
            auto& v = varray[i];
            if (v.getType() == Variant::MAP)
            {
                AnimationGraphState state;
                state.id = m_states.size();
                auto it1 = v.getMap().find("name");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::STRING)
                    state.m_name = it1->second.toString();
                else
                    state.m_name = "-state(" + std::to_string(state.id) + ")-";
                stateIds[state.m_name] = state.id;
                m_states.push_back(state);

                if (state.m_name == entryStateName)
                    m_entryState = state.id;
            }
        }
    }

    // parameters
    using ptype = AnimationGraphParameter::ParameterType;
    std::map<std::string, int> paramIds;
    it0 = graphMap.getMap().find("parameters");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto varray = it0->second.getArray();
        for (int i = 0; i < varray.size(); i++)
        {
            auto& v = varray[i];
            if (v.getType() == Variant::MAP)
            {
                AnimationGraphParameter parameter;

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
                                    parameter.m_value.Float = it1->second.toDouble();
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

                paramIds[parameter.m_name] = m_parameters.size();
                m_parameters.push_back(parameter);
            }
        }
    }

    // transitions
    using ctype = AnimationGraphCondition::ComparisonType;
    it0 = graphMap.getMap().find("transitions");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        auto& varray = it0->second.getArray();
        for (int i = 0; i < varray.size(); i++)
        {
            auto& v = varray[i];
            if (v.getType() == Variant::MAP)
            {
                AnimationGraphTransition transition;

                // states
                int from = -1;
                int to = -1;
                auto it1 = v.getMap().find("from");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::STRING)
                {
                    auto it2 = stateIds.find(it1->second.toString());
                    if (it2 != stateIds.end())
                        from = it2->second;
                }
                it1 = v.getMap().find("to");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::STRING)
                {
                    auto it2 = stateIds.find(it1->second.toString());
                    if (it2 != stateIds.end())
                        to = it2->second;
                }

                if (from < 0 || to < 0)
                    PrintWarning(fileName.c_str(), "a transition points to invalid states (was discarded)");
                else
                {
                    transition.m_stateFrom = &m_states[from];
                    transition.m_stateTo = &m_states[to];
                }

                // duration
                if (!TryLoadAsFloat(v, "duration", transition.m_duration))
                {
                    transition.m_duration = 0.f;
                    PrintWarning(fileName.c_str(), "a transition had no duration");
                }

                // conditions
                it1 = v.getMap().find("conditions");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& vcarray = it1->second.getArray();
                    for (int j = 0; j < vcarray.size(); j++)
                    {
                        auto& vc = vcarray[j];
                        if (vc.getType() == Variant::MAP)
                        {
                            AnimationGraphCondition condition;
                            int paramId = -1;
                            auto it2 = vc.getMap().find("paramName");
                            if (it2 != vc.getMap().end() && it2->second.getType() == Variant::STRING)
                            {
                                auto it3 = paramIds.find(it2->second.toString());
                                if (it3 != paramIds.end())
                                {
                                    paramId = it3->second;
                                    condition.parameterId = it3->second;
                                    condition.m_parameter = m_parameters[paramId];
                                }
                            }

                            it2 = vc.getMap().find("comparison");
                            if (it2 != vc.getMap().end() && it2->second.getType() == Variant::STRING)
                            {
                                std::string s = it2->second.toString();
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

                            bool valueFound = false;
                            it2 = vc.getMap().find("value");
                            if (it2 != vc.getMap().end())
                            {
                                switch (condition.m_parameter.m_type)
                                {
                                    case ptype::BOOL:
                                    case ptype::TRIGGER:
                                        {
                                            if (it2->second.getType() == Variant::BOOL)
                                            {
                                                valueFound = true;
                                                condition.m_parameter.m_value.Bool = it2->second.toBool();
                                            }
                                            else
                                            {
                                                std::string msg = "condition on " + condition.m_parameter.m_name + " : has an invalid test value";
                                                PrintWarning(fileName.c_str(), msg.c_str());
                                            }
                                        }
                                        break;
                                    case ptype::INT:
                                        {
                                            if (it2->second.getType() == Variant::INT)
                                            {
                                                valueFound = true;
                                                condition.m_parameter.m_value.Int = it2->second.toInt();
                                            }
                                            else
                                            {
                                                std::string msg = "condition on " + condition.m_parameter.m_name + " : has an invalid test value";
                                                PrintWarning(fileName.c_str(), msg.c_str());
                                            }
                                        }
                                        break;
                                    case ptype::FLOAT:
                                        {
                                            if (it2->second.getType() == Variant::FLOAT)
                                            {
                                                valueFound = true;
                                                condition.m_parameter.m_value.Float = it2->second.toFloat();
                                            }
                                            else if (it2->second.getType() == Variant::DOUBLE)
                                            {
                                                valueFound = true;
                                                condition.m_parameter.m_value.Float = it2->second.toDouble();
                                            }
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

                            if (paramId < 0)
                                PrintWarning(fileName.c_str(), "a condition refer to a wrong parameter name");
                            else if (!valueFound)
                                PrintWarning(fileName.c_str(), "a condition has no test value");
                            else
                                transition.m_conditions.push_back(condition);
                        }
                    }
                }
                if (transition.m_conditions.empty())
                    PrintWarning(fileName.c_str(), "a transition had no duration (was discarded)");

                // push to list
                if (transition.m_stateFrom && transition.m_stateTo && !transition.m_conditions.empty())
                    m_transitions.push_back(transition);
            }
        }
    }

    // link transitions
    for (int i = 0; i < m_transitions.size(); i++)
    {
        AnimationGraphState* state = m_transitions[i].m_stateFrom;
        if (state)
            state->m_transitionOut.push_back(&m_transitions[i]);
    }

    // variant
    it0 = graphMap.getMap().find("variants");
    if (it0 != graphMap.getMap().end() && it0->second.getType() == Variant::ARRAY)
    {
        ResourceManager* manager = ResourceManager::getInstance();
        auto& varray = it0->second.getArray();
        for (int i = 0; i < varray.size(); i++)
        {
            auto& v = varray[i];
            if (v.getType() == Variant::MAP)
            {
                // name
                std::string name;
                auto it1 = v.getMap().find("name");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::STRING)
                    name = it1->second.toString();
                if (name.empty())
                {
                    PrintWarning(fileName.c_str(), "a variant has no name");
                    continue;
                }

                // datas
                auto insertion = m_dataVariant.emplace(name, AnimationGraphData());
                AnimationGraphData& tmpData = insertion.first->second;
                if (!insertion.second)
                {
                    tmpData.m_statesData.clear();
                    PrintWarning(fileName.c_str(), "two variant has the same name !");
                }

                tmpData.m_name = name;
                it1 = v.getMap().find("statesData");
                if (it1 != v.getMap().end() && it1->second.getType() == Variant::ARRAY)
                {
                    auto& vdarray = it1->second.getArray();
                    for (int j = 0; j < vdarray.size(); j++)
                    {
                        auto& vd = vdarray[j];
                        if (vd.getType() == Variant::MAP)
                        {
                            AnimationGraphStateData stateData;
                            stateData.m_animation = nullptr;

                            auto it2 = vd.getMap().find("clip");
                            if (it2 != vd.getMap().end() && it2->second.getType() == Variant::STRING)
                                stateData.m_animation = manager->getResource<AnimationClip>(it2->second.toString());

                            bool foundSpeed = TryLoadAsFloat(vd, "speed", stateData.m_speed);

                            if (!foundSpeed || stateData.m_animation == nullptr)
                                PrintWarning(fileName.c_str(), "a variant data has no animation clip name or no speed");

                            tmpData.m_statesData.push_back(stateData);
                        }
                    }
                }

                bool goodLength = tmpData.m_statesData.size() == m_states.size();
                if (goodLength && !name.empty())
                    m_dataVariant[name] = tmpData;
                else if (!goodLength)
                {
                    std::string msg = "variant " + name + " : data length does not match state length (variant was discarded)";
                    PrintWarning(fileName.c_str(), msg.c_str());
                }
                else if (name.empty())
                    PrintWarning(fileName.c_str(), "a variant has no name");
            }
        }
    }

    // end
    bool hasState = !m_states.empty();
    bool hasEntryState = m_entryState >= 0 && m_entryState < m_states.size();
    bool hasVariant = !m_dataVariant.empty();
    if (hasState && hasEntryState && hasVariant)
        return true;

    if (!hasState)
        PrintError(fileName.c_str(), "graph has no valid state");
    if (!hasEntryState)
        PrintError(fileName.c_str(), "graph has no valid entry state");
    if (!hasVariant)
        PrintError(fileName.c_str(), "graph has no valid variant");

    return false;
}

void AnimationGraphLoader::initialize(ResourceVirtual* resource)
{
    AnimationGraph* graph = static_cast<AnimationGraph*>(resource);
    graph->initialize(m_states, m_transitions, m_entryState);
    graph->setDefaultParameters(m_parameters);
    graph->setVariants(m_dataVariant);
}

void AnimationGraphLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

