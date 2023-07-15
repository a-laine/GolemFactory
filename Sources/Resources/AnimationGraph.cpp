#include "AnimationGraph.h"
#include "ResourceManager.h"
#include "Skeleton.h"


char const* const AnimationGraph::directory = "Animations/";
char const* const AnimationGraph::extension = ".animGraph";
std::string AnimationGraph::defaultName;

AnimationGraph::AnimationGraph(const std::string& _graphName)
    : ResourceVirtual(_graphName, ResourceVirtual::ResourceType::ANIMATION_GRAPH)
{

}

AnimationGraph::~AnimationGraph()
{
    ResourceManager* manager = ResourceManager::getInstance();
    for (auto& it0 : m_variants)
        for (auto& it1 : it0.second)
        {
            for (auto& it2 : it1.m_stateMachinesData)
                for (auto& it3 : it2.m_statesData)
                    manager->release(it3.m_animation);

            for (auto& it2 : it1.m_blendTreesData)
                for (auto& it3 : it2.m_nodeData)
                    manager->release(it3.m_animation);
        }
}

// initialization
void AnimationGraph::initialize(std::vector<Layer>& _layers)
{
    m_layers.swap(_layers);
    state = VALID;
}
void AnimationGraph::setParameters(std::vector<AnimationParameter>& _parameters)
{
    m_defaultParameters.swap(_parameters);
}
void AnimationGraph::setVariants(std::map<std::string, std::vector<LayerData>>& _variants)
{
    m_variants.swap(_variants);
}
void AnimationGraph::getRuntime(const std::vector<LayerData>& _data, std::vector<LayerRuntime>& _targetRuntime) const
{
    std::vector<LayerRuntime> layersRuntime;
    layersRuntime.resize(m_layers.size());
    layersRuntime.shrink_to_fit();

    for (int i = 0; i < m_layers.size(); i++)
    {
        layersRuntime[i].m_stateMachinesRuntime.resize(m_layers[i].m_stateMachines.size());
        layersRuntime[i].m_stateMachinesRuntime.shrink_to_fit();

        for (int j = 0; j < m_layers[i].m_stateMachines.size(); j++)
        {
            const auto& stateMachine = m_layers[i].m_stateMachines[j];
            auto& runtime = layersRuntime[i].m_stateMachinesRuntime[j];
            const auto& data = _data[i].m_stateMachinesData[j];
            stateMachine.setState(stateMachine.m_entryStateId, data, runtime);
        }

        layersRuntime[i].m_blendTreesRuntime.resize(m_layers[i].m_blendTrees.size());
        layersRuntime[i].m_blendTreesRuntime.shrink_to_fit();

        for (int j = 0; j < m_layers[i].m_blendTrees.size(); j++)
        {
            const auto& blendTree = m_layers[i].m_blendTrees[j];
            auto& runtime = layersRuntime[i].m_blendTreesRuntime[j];
            const auto& data = _data[i].m_blendTreesData[j];
            blendTree.initializeRuntime(data, runtime);
        }

        std::vector<SubGraph> subgraphs;
        for (int j = 0; j < m_layers[i].m_subGraphNames.size(); j++)
        {
            bool found = false;
            for (int k = 0; k < m_layers[i].m_blendTrees.size(); k++)
            {
                if (m_layers[i].m_blendTrees[k].m_name == m_layers[i].m_subGraphNames[j])
                {
                    SubGraph subgraph;
                    subgraph.m_type = SubGraph::SubGraphType::BLEND_TREE;
                    subgraph.m_graph = (void*)&m_layers[i].m_blendTrees[k];
                    subgraph.m_graphData = (void*)&_data[i].m_blendTreesData[k];
                    subgraph.m_graphRuntime = &layersRuntime[i].m_blendTreesRuntime[k];
                    subgraphs.push_back(subgraph);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                for (int k = 0; k < m_layers[i].m_blendTrees.size(); k++)
                {
                    if (m_layers[i].m_blendTrees[k].m_name == m_layers[i].m_subGraphNames[j])
                    {
                        SubGraph subgraph;
                        subgraph.m_type = SubGraph::SubGraphType::STATE_MACHINE;
                        subgraph.m_graph = (void*)&m_layers[i].m_stateMachines[k];
                        subgraph.m_graphData = (void*)&_data[i].m_stateMachinesData[k];
                        subgraph.m_graphRuntime = &layersRuntime[i].m_stateMachinesRuntime[k];
                        subgraphs.push_back(subgraph);
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                SubGraph subgraph;
                subgraph.m_type = SubGraph::SubGraphType::BLEND_TREE;
                subgraph.m_graph = nullptr;
                subgraph.m_graphData = nullptr;
                subgraph.m_graphRuntime = nullptr;
                subgraphs.push_back(subgraph);
            }
        }

        for (int j = 0; j < m_layers[i].m_stateMachines.size(); j++)
        {
            auto& runtime = layersRuntime[i].m_stateMachinesRuntime[j];
            runtime.m_subGraphs = subgraphs;
        }
        for (int j = 0; j < m_layers[i].m_blendTrees.size(); j++)
        {
            auto& runtime = layersRuntime[i].m_blendTreesRuntime[j];
            runtime.m_subGraphs = subgraphs;
        }
    }

    _targetRuntime.swap(layersRuntime);
}

// instance copy functions
std::vector<AnimationParameter> AnimationGraph::getParametersCopy() const
{
    return m_defaultParameters;
}
const std::vector<AnimationGraph::LayerData>* AnimationGraph::getVariant(const std::string& _name) const
{
    auto it = m_variants.find(_name);
    if (it != m_variants.end())
        return &it->second;
    return nullptr;
}
const std::vector<AnimationGraph::Layer>& AnimationGraph::getLayers() const
{
    return m_layers;
}

// evaluation of the graph
void AnimationGraph::evaluate(float _elapsedTime, const ParamList& _paramList, const std::vector<LayerData>& _data, std::vector<LayerRuntime>& _runtime, Evaluation& _result) const
{
    std::vector<Evaluation*> layersResult;
    for (int i = 0; i < m_layers.size(); i++)
    {
        const Layer& layer = m_layers[i];
        if (layer.m_blendTreeEntryId >= 0)
        {
            int id = layer.m_blendTreeEntryId;
            const AG::BlendTree& graph = layer.m_blendTrees[id];
            const AG::BlendTree::BlendTreeData& data = _data[i].m_blendTreesData[id];
            AG::BlendTree::BlendTreeRuntime& runtime = _runtime[i].m_blendTreesRuntime[id];

            graph.evaluate(_elapsedTime, data, _paramList, runtime);
            layersResult.push_back(&runtime.m_nodeRuntimes[graph.m_entryNodeId].m_evaluation);
        }
        else if (layer.stateMachineEntryId >= 0)
        {
            int id = layer.stateMachineEntryId;
            const AG::StateMachine& graph = layer.m_stateMachines[id];
            const AG::StateMachine::StateMachineData& data = _data[i].m_stateMachinesData[id];
            AG::StateMachine::StateMachineRuntime& runtime = _runtime[i].m_stateMachinesRuntime[id];

            graph.evaluate(_elapsedTime, data, _paramList, runtime);
            layersResult.push_back(&runtime.m_finalEvaluation);
        }
    }

    if (layersResult.size() >= 1)
        _result = *layersResult[0];

    for (int i = 1; i < layersResult.size(); i++)
    {

    }
}

// resource stuff
std::string AnimationGraph::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
const std::string& AnimationGraph::getDefaultName()
{
    return defaultName;
}
void AnimationGraph::setDefaultName(const std::string& name)
{
    defaultName = name;
}
std::string AnimationGraph::getIdentifier() const
{
    return getIdentifier(name);
}
std::string AnimationGraph::getLoaderId(const std::string& resourceName) const
{
    return extension;
}

// debug
void AnimationGraph::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    if (!m_defaultParameters.empty())
    {
        ImGui::Spacing();
        using ptype = AnimationParameter::ParameterType;
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Parameters");
        for (int i = 0; i < m_defaultParameters.size(); i++)
        {
            switch (m_defaultParameters[i].m_type)
            {
                case ptype::TRIGGER:
                    ImGui::BulletText("Trigger %s", m_defaultParameters[i].m_name.c_str());
                    break;
                case ptype::BOOL: 
                    ImGui::BulletText("Bool %s : default %s", m_defaultParameters[i].m_name.c_str(), m_defaultParameters[i].m_value.Bool ? "TRUE" : "false");
                    break;
                case ptype::INT:
                    ImGui::BulletText("Int %s : default %d", m_defaultParameters[i].m_name.c_str(), m_defaultParameters[i].m_value.Int);
                    break;
                case ptype::FLOAT:
                    ImGui::BulletText("Float %s : default %f", m_defaultParameters[i].m_name.c_str(), m_defaultParameters[i].m_value.Float);
                    break;
                default:
                    ImGui::BulletText("-unknown- %s", m_defaultParameters[i].m_name.c_str());
                    break;
            }
        }
    }

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Variants");
    for (const auto& it : m_variants)
        ImGui::BulletText(it.first.c_str());

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Layers");
    for (const auto& it : m_layers)
    {
        ImGui::BulletText(it.m_name.c_str());

        ImGui::Indent();
        if (it.stateMachineEntryId >= 0)
            ImGui::Text("Entry state machine : \"%s\"", it.m_stateMachines[it.stateMachineEntryId].m_name.c_str());
        else if (it.m_blendTreeEntryId >= 0)
            ImGui::Text("Entry blend tree : \"%s\"", it.m_blendTrees[it.m_blendTreeEntryId].m_name.c_str());
        else
            ImGui::Text("Unknown entry state or node");
        ImGui::Text("State machines count : %d", it.m_stateMachines.size());
        ImGui::Text("Blend trees count : %d", it.m_blendTrees.size());
        ImGui::Unindent();
    }

#endif
}