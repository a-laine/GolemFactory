#include "Animator.h"

#include <EntityComponent/Entity.hpp>
#include <Animation/AnimationComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Resources/Skeleton.h>
#include <Resources/ResourceManager.h>
#include <Scene/SceneManager.h>
#include <World/World.h>
#include <Utiles/ProfilerConfig.h>
#include <EntityComponent/ComponentUpdater.h>

#ifdef USE_IMGUI
	#define IMGUI_DEFINE_MATH_OPERATORS
	#include <imgui_internal.h>
#endif

extern std::map<Component::UpdatePass, std::vector<Component::ComponentUpdateData>> gComponentUpdateList;

void AnimatorUpdate(void* componentPtr, float dt)
{
	Animator* animator = (Animator * )componentPtr;
	animator->update(dt);
}

Animator::Animator()
{
	m_skeleton = nullptr;
	m_skeletonComponent = nullptr;
	m_graph = nullptr;
	//m_graphData = nullptr;
	//m_runtimeData.m_currentTransition = nullptr;
	//m_runtimeData.m_stateTime = 0.f;
	//m_runtimeData.m_transitionTime = 0.f;
	//m_runtimeData.m_currentStateIndex = 0;
	m_immutableData = true;
}

Animator::~Animator()
{
	ResourceManager::getInstance()->release(m_graph);
	if (!m_immutableData)
		delete m_data;
}

bool Animator::load(Variant& jsonObject, const std::string& objectName)
{
	if (jsonObject.getType() == Variant::MAP)
	{
		std::string graphName, variantName;
		auto it1 = jsonObject.getMap().find("graphName");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
			graphName = it1->second.toString();
		it1 = jsonObject.getMap().find("variant");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
			variantName = it1->second.toString();

		if (!graphName.empty() && !variantName.empty())
		{
			m_graph = ResourceManager::getInstance()->getResource<AnimationGraph>(graphName);
			if (m_graph)
				m_data = (GraphData*)m_graph->getVariant(variantName);
			if (m_graph && m_data)
			{
				m_variantName = variantName;
				m_graph->getRuntime(*m_data, m_runtime);
				m_graphParameters = m_graph->getParametersCopy();
				return true;
			}
		}
	}
	return false;
}

void Animator::save(Variant& jsonObject)
{

}

void Animator::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	if (entity)
	{
		m_skeletonComponent = entity->getComponent<SkeletonComponent>();
		if (m_skeletonComponent)
			m_skeleton = m_skeletonComponent->getSkeleton();

		ComponentUpdater::getInstance()->add(Component::eCommon, &AnimatorUpdate, this);
	}
}

bool Animator::isValid() const
{
	if (!m_graph) return false;
	if (!m_data) return false;
	if (!m_skeletonComponent) return false;
	if (!m_skeleton) return false;
	return true;
}


bool Animator::setParameter(const std::string& _name, float _value)
{
	for (auto& param : m_graphParameters)
	{
		if (param.m_name == _name)
		{
			if (param.m_type != AnimationParameter::ParameterType::FLOAT)
				return false;
			param.m_value.Float = _value;
			return true;
		}
	}
	return false;
}
bool Animator::setParameter(const std::string& _name, bool _value)
{
	for (auto& param : m_graphParameters)
	{
		if (param.m_name == _name)
		{
			if (param.m_type != AnimationParameter::ParameterType::BOOL && 
				param.m_type != AnimationParameter::ParameterType::TRIGGER)
				return false;
			param.m_value.Bool = _value;
			return true;
		}
	}
	return false;
}
bool Animator::setParameter(const std::string& _name, int _value)
{
	for (auto& param : m_graphParameters)
	{
		if (param.m_name == _name)
		{
			if (param.m_type != AnimationParameter::ParameterType::INT)
				return false;
			param.m_value.Int = _value;
			return true;
		}
	}
	return false;
}

void Animator::update(float elapsedTime)
{
	SCOPED_CPU_MARKER("Animator");

	m_graph->evaluate(elapsedTime, m_graphParameters, *m_data, m_runtime, m_evaluation);
	for (int i = 0; i < m_graphParameters.size(); i++)
	{
		if (m_graphParameters[i].m_type == AnimationParameter::ParameterType::TRIGGER)
			m_graphParameters[i].m_value.Bool = false;
	}

	std::vector<mat4f> pose = m_skeleton->getBindPose();
	const std::vector<Skeleton::Bone>& bones = m_skeleton->getBones();
	std::vector<int> bones2curve;
	if (m_evaluation.size() > 0)
	{
		bones2curve.assign(bones.size(), -1);
		for (int i = 0; i < m_evaluation.size(); i++)
		{
			int id = m_skeleton->getBoneId(m_evaluation[i].m_boneName);
			if (id >= 0)
				bones2curve[id] = i;
		}
	}

	if (!bones2curve.empty() && m_evaluation.size() > 0)
	{
		for (int i = 0; i < bones.size(); i++)
		{
			mat4f parent = bones[i].parent ? pose[bones[i].parent->id] : mat4f::identity;
			mat4f trs;
			int id = bones2curve[i];
			if (id >= 0)
			{
				const BoneCurvesState& state = m_evaluation[id];
				trs = mat4f::TRS(state.m_position, state.m_rotation, vec4f(state.m_scale));
			}
			else
				trs = bones[i].relativeBindTransform;
			pose[i] = parent * trs;
		}
	}

	m_skeletonComponent->swapPose(pose);
	m_skeletonComponent->recomputeBoundingBox();
	if (getParentEntity())
	{
		getParentEntity()->recomputeBoundingBox();
		getParentEntity()->getParentWorld()->getSceneManager().updateObject(getParentEntity());
	}
}


void Animator::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.7f, 0.7f, 0.5f, 1.f);
	std::ostringstream unicName;
	unicName << "Animator##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Animation graph");
		ImGui::Indent();
		ImGui::Text("Graph name : %s", m_graph ? m_graph->name.c_str() : "null");
		ImGui::Text("Variant name : %s", m_variantName.c_str());
		ImGui::Checkbox("Show Full graph evaluation", &m_fullGraphWindow);

		ImGui::Unindent();

		ImGui::TextColored(componentColor, "Parameters");
		ImGui::Indent();
		using ptype = AnimationParameter::ParameterType;
		for (int i = 0; i < m_graphParameters.size(); i++)
		{
			AnimationParameter& parameter = m_graphParameters[i];
			switch (parameter.m_type)
			{
				case ptype::BOOL:
				case ptype::TRIGGER:
					ImGui::Checkbox(parameter.m_name.c_str(), &parameter.m_value.Bool);
					break;
				case ptype::INT:
					ImGui::DragInt(parameter.m_name.c_str(), &parameter.m_value.Int);
					break;
				case ptype::FLOAT:
					ImGui::DragFloat(parameter.m_name.c_str(), &parameter.m_value.Float, 0.01f);
					break;
				default: break;
			}
		}
		ImGui::Unindent();


		ImGui::TextColored(componentColor, "Layers");
		const auto& layers = m_graph->getLayers();
		for (int i = 0; i < layers.size(); i++)
		{
			const AnimationGraph::Layer& layer = layers[i];
			if (ImGui::TreeNode((void*)&layer, "Layer : %s", layer.m_name.c_str()))
			{
				if (layer.stateMachineEntryId >= 0)
				{
					const AG::StateMachine& stateMachine = layer.m_stateMachines[layer.stateMachineEntryId];
					AG::StateMachine::StateMachineRuntime& smruntime = m_runtime[i].m_stateMachinesRuntime[layer.stateMachineEntryId];
					const AG::StateMachine::State& state = stateMachine.m_states[smruntime.m_currentStateId];
					const AG::StateMachine::StateMachineData& smdata = (*m_data)[i].m_stateMachinesData[layer.stateMachineEntryId];
					drawImGui(smruntime.m_currentStateId, stateMachine, smdata, smruntime, false);

					if (state.m_subgraphId >= 0)
					{
						SubGraph& subgraph = smruntime.m_subGraphs[state.m_subgraphId];
						AG::BlendTree* blendTree = (AG::BlendTree*)subgraph.m_graph;
						AG::BlendTree::BlendTreeData* btdata = (AG::BlendTree::BlendTreeData*)subgraph.m_graphData;
						AG::BlendTree::BlendTreeRuntime* btruntime = (AG::BlendTree::BlendTreeRuntime*)subgraph.m_graphRuntime;
						drawImGui(blendTree->m_entryNodeId, *blendTree, *btdata, *btruntime, true);
					}
				}
				else if (layer.m_blendTreeEntryId >= 0)
				{
					const AG::BlendTree& blendTree = layer.m_blendTrees[layer.m_blendTreeEntryId];
					AG::BlendTree::BlendTreeRuntime& btruntime = m_runtime[i].m_blendTreesRuntime[layer.m_blendTreeEntryId];
					const AG::BlendTree::Node& node = blendTree.m_nodes[blendTree.m_entryNodeId];
					const AG::BlendTree::BlendTreeData& btdata = (*m_data)[i].m_blendTreesData[layer.m_blendTreeEntryId];
					drawImGui(blendTree.m_entryNodeId, blendTree, btdata, btruntime, false);

					if (node.m_subgraphId >= 0)
					{
						SubGraph& subgraph = btruntime.m_subGraphs[node.m_subgraphId];
						AG::StateMachine* stateMachine = (AG::StateMachine*)subgraph.m_graph;
						AG::StateMachine::StateMachineData* smdata = (AG::StateMachine::StateMachineData*)subgraph.m_graphData;
						AG::StateMachine::StateMachineRuntime* smruntime = (AG::StateMachine::StateMachineRuntime*)subgraph.m_graphRuntime;
						drawImGui(smruntime->m_currentStateId, *stateMachine, *smdata, *smruntime, true);
					}
				}
				else
				{
					ImGui::Text("Cannot draw current state");
				}
				layer.stateMachineEntryId;

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	if (m_fullGraphWindow && m_graph)
		drawFullGraphWindow();
#endif
}

void Animator::drawImGui(int _stateId, const AG::StateMachine& _graph, const AG::StateMachine::StateMachineData& _data,
	AG::StateMachine::StateMachineRuntime& _runtime, bool isSubgraph)
{
#ifdef USE_IMGUI
	using ptype = AnimationParameter::ParameterType;
	using ctype = AG::StateMachine::Condition::ComparisonType;
	const ImVec4 componentColor = isSubgraph ? ImVec4(0.4f, 0.75f, 0.2f, 1.f) : ImVec4(0.75f, 0.4f, 0.f, 1.f);
	std::string header = isSubgraph ? "Sub state Machine overview" : "State Machine overview";
	ImGui::TextColored(componentColor, header.c_str());

	// Current state overview
	const AG::StateMachine::State& state = _graph.m_states[_stateId];
	const AG::StateMachine::StateData& data = _data.m_statesData[_stateId];
	ImGui::Text("Current state : %s", state.m_name.c_str());
	ImGui::Text("State clip : %s", data.m_animation ? data.m_animation->name.c_str() : "-no clip-");
	if (state.m_exitTime >= 0.f)
		ImGui::Text("Exit time : %f", state.m_exitTime);
	if(data.m_animation)
		ImGui::SliderFloat("Current time", &_runtime.m_currentStateTime, 0.f, data.m_animation->m_duration, "%.2f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::Text("Transitions");
	for (int i = 0; i < state.m_transitionOut.size(); i++)
	{
		const AG::StateMachine::Transition& transition = state.m_transitionOut[i];
		ImGui::BulletText("To %s (duration : %f) :", _graph.m_states[transition.m_stateTargetId].m_name.c_str(), transition.m_duration);
		ImGui::Indent();
		for (int j = 0; j < transition.m_conditions.size(); j++)
		{
			const std::string& name = transition.m_conditions[j].m_parameter.m_name;
			const AnimationParameter::Param& value = transition.m_conditions[j].m_parameter.m_value;

			switch (transition.m_conditions[j].m_parameter.m_type)
			{
				case ptype::TRIGGER: ImGui::Text("-Trigger %s is TRUE", name.c_str()); break;

				case ptype::BOOL:
					switch (transition.m_conditions[j].m_comparisonType)
					{
						case ctype::EQUALS:     ImGui::Text("-Bool %s == %s", name.c_str(), value.Bool ? "TRUE" : "false"); break;
						case ctype::NOT_EQUALS: ImGui::Text("-Bool %s != %s", name.c_str(), value.Bool ? "TRUE" : "false"); break;
						default:                ImGui::Text("-ERROR bool-"); break;
					}
					break;

				case ptype::INT:
					switch (transition.m_conditions[j].m_comparisonType)
					{
						case ctype::EQUALS:     ImGui::Text("-Int %s == %d", name.c_str(), value.Int); break;
						case ctype::NOT_EQUALS: ImGui::Text("-Int %s != %d", name.c_str(), value.Int); break;
						case ctype::GREATER:    ImGui::Text("-Int %s > %d", name.c_str(), value.Int); break;
						case ctype::LESS:       ImGui::Text("-Int %s < %d", name.c_str(), value.Int); break;
						default:                ImGui::Text("-ERROR int-"); break;
					}
					break;

				case ptype::FLOAT:
					switch (transition.m_conditions[j].m_comparisonType)
					{
						case ctype::EQUALS:     ImGui::Text("-Float %s == %f", name.c_str(), value.Float); break;
						case ctype::NOT_EQUALS: ImGui::Text("-Float %s != %f", name.c_str(), value.Float); break;
						case ctype::GREATER:    ImGui::Text("-Float %s > %f", name.c_str(), value.Float); break;
						case ctype::LESS:       ImGui::Text("-Float %s < %f", name.c_str(), value.Float); break;
						default:                ImGui::Text("-ERROR float-"); break;
					}
					break;

				default: ImGui::Text("-ERROR type-"); break;
			}
		}
		ImGui::Unindent();
	}

	// Current transition overview
	AG::StateMachine::Transition* transition = _runtime.m_currentTransition;
	if (transition)
	{
		ImGui::TextColored(componentColor, "Transition");
		ImGui::Indent();
		ImGui::Text("To state : %s", _graph.m_states[transition->m_stateTargetId].m_name.c_str());
		ImGui::SliderFloat("Time", &_runtime.m_transitionTime, 0.f, transition->m_duration, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Unindent();
	}
#endif
}

void Animator::drawImGui(int _nodeId, const AG::BlendTree& _tree, const AG::BlendTree::BlendTreeData& _data,
	AG::BlendTree::BlendTreeRuntime& _runtime, bool isSubgraph)
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = isSubgraph ? ImVec4(0.4f, 0.75f, 0.2f, 1.f) : ImVec4(0.75f, 0.4f, 0.f, 1.f);
	std::string header = isSubgraph ? "Sub blend tree overview" : "Blend tree overview";
	ImGui::TextColored(componentColor, header.c_str());
	ImGui::Indent();

	// aliases
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const float availableSizeX = ImGui::GetContentRegionAvail().x - 5;
	const ImVec2 cp = window->DC.CursorPos;
	const ImVec2 pointHalfSize = ImVec2(3, 3);
	const float weightRadius = 0.5f;
	const ImVec2 center = cp + ImVec2(0.5f * availableSizeX, 0.5f * availableSizeX);
	const ImVec2 mouse = ImGui::GetIO().MousePos;
	ImDrawList* drawList = window->DrawList;
	ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.4f, 1.f, 1.f));
	ImU32 selectedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 1.f, 0.4f, 1.f));

	const AG::BlendTree::Node& node = _tree.m_nodes[_nodeId];
	AG::BlendTree::NodeRuntimeData& runtime = _runtime.m_nodeRuntimes[_nodeId];
	float maxD = -std::numeric_limits<float>::max();
	for (int i = 0; i < node.m_childrenPoint.size(); i++)
		for (int j = 0; j < node.m_parameterIds.size(); j++)
			maxD = std::max(maxD, node.m_childrenPoint[i][j]);
	maxD += weightRadius;
	float ratio = 0.5f * availableSizeX / maxD;

	// draw graph
	ImGui::RenderFrame(cp, cp + ImVec2(availableSizeX, availableSizeX), 0xFF404040, true, false);
	for (int i = 0; i < node.m_childrenPoint.size(); i++)
	{
		int childNodeId = node.m_childrenId[i];
		float w = _runtime.m_nodeRuntimes[childNodeId].m_weight;
		ImVec2 p = center + ImVec2(node.m_childrenPoint[i].x * ratio, -node.m_childrenPoint[i].y * ratio);
		ImVec2 c0 = p - pointHalfSize;
		ImVec2 c1 = p + pointHalfSize;
		if (w == 0.f)
			drawList->AddRectFilled(c0, c1, color);
		else
		{
			drawList->AddRectFilled(c0, c1, selectedColor);
			drawList->AddCircle(p, w * weightRadius * ratio, selectedColor, 16);
		}

		if (mouse.x >= c0.x && mouse.y >= c0.y && mouse.x <= c1.x && mouse.y <= c1.y)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			const AnimationClip* clip = _data.m_nodeData[childNodeId].m_animation;
			ImGui::TextColored(componentColor, clip ? clip->name.c_str() : "-no clip-");
			for (int j = 0; j < node.m_parameterIds.size(); j++)
			{
				const AnimationParameter& parameter = m_graphParameters[node.m_parameterIds[j]];
				ImGui::Text("%s : %f", parameter.m_name.c_str(), node.m_childrenPoint[i][j]);
			}
			ImGui::Text("weight : %f", w);
			ImGui::Text("sigma : %f", node.m_childrenInfluence[i]);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	ImVec2 p = center + ImVec2(runtime.m_blendPoint.x * ratio, -runtime.m_blendPoint.y * ratio);
	drawList->AddCircleFilled(p, pointHalfSize.x, ImGui::ColorConvertFloat4ToU32(ImVec4(0.7f, 0.f, 0.f, 1.f)), 16);
	ImGui::Dummy(ImVec2(0, availableSizeX));

	ImGui::PushItemWidth(200);
	ImGui::DragFloat("Global influence", (float*)&node.m_globalInfluence, 0.001f, 0.00001f, 2.f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::Unindent();
#endif
}

void Animator::drawFullGraphWindow()
{
#ifdef USE_IMGUI
	std::string windowName = "AnimationGraph : " + m_graph->name + "##" + std::to_string((uintptr_t)this);
	ImGui::Begin(windowName.c_str(), &m_fullGraphWindow);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const float availableSizeX = 100;
	const ImVec2 cp = window->DC.CursorPos;
	ImGui::RenderFrame(cp, cp + ImVec2(availableSizeX, availableSizeX), 0xFF404040, true, false);
	ImGui::Dummy(ImVec2(0, availableSizeX));

	ImGui::Text("TODO : using https://github.com/thedmd/imgui-node-editor");
	ImGui::End();
#endif
}