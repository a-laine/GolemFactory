#include "Animator.h"

#include <EntityComponent/Entity.hpp>
#include <Animation/AnimationComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Resources/ResourceManager.h>
#include <Scene/SceneManager.h>
#include <World/World.h>
#include <Utiles/ProfilerConfig.h>

std::vector<Animator*> g_allAnimator;


Animator::Animator()
{
	m_skeleton = nullptr;
	m_skeletonComponent = nullptr;
	m_graph = nullptr;
	m_graphData = nullptr;
	m_runtimeData.m_currentTransition = nullptr;
	m_runtimeData.m_stateTime = 0.f;
	m_runtimeData.m_transitionTime = 0.f;
	m_runtimeData.m_currentStateIndex = 0;
	m_immutableData = true;
}

Animator::~Animator()
{
	ResourceManager::getInstance()->release(m_graph);
	if (!m_immutableData)
		delete m_graphData;
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
				m_graphData = (AnimationGraphData*)m_graph->getVariant(variantName);
			if (m_graph && m_graphData)
			{
				unsigned int state0 = m_graph->getEntryState();
				m_graph->setState(state0, *m_graphData, m_runtimeData);
				m_runtimeData.m_parameters = m_graph->getParametersCopy();
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
			m_runtimeData.m_skeleton = m_skeletonComponent->getSkeleton();
		g_allAnimator.push_back(this);

		if (m_runtimeData.m_skeleton && isValid())
			m_graph->setState(m_runtimeData.m_currentStateIndex, *m_graphData, m_runtimeData);
	}
}

bool Animator::isValid() const
{
	if (!m_graph) return false;
	if (!m_graphData) return false;
	if (!m_runtimeData.m_skeleton) return false;
	if (!m_skeletonComponent) return false;
	//if (!m_runtimeData.m_currentState) return false;
	//if (m_runtimeData.m_skeletonFinalPose.empty()) return false;
	return true;
}
void Animator::update(float elapsedTime)
{
	SCOPED_CPU_MARKER("Animator");

	m_graph->evaluate(elapsedTime, *m_graphData, m_runtimeData);
	for (int i = 0; i < m_runtimeData.m_parameters.size(); i++)
	{
		if (m_runtimeData.m_parameters[i].m_type == AnimationGraphParameter::ParameterType::TRIGGER)
			m_runtimeData.m_parameters[i].m_value.Bool = false;
	}

	m_skeletonComponent->setPose(m_runtimeData.m_skeletonFinalPose);
	m_skeletonComponent->recomputeBoundingBox();
	if (getParentEntity())
	{
		getParentEntity()->recomputeBoundingBox();
		getParentEntity()->getParentWorld()->getSceneManager().updateObject(getParentEntity());
	}
}

const std::vector<mat4f>& Animator::getSkeletonPose() const
{
	return m_runtimeData.m_skeletonFinalPose;
}

void Animator::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.7, 0.7, 0.5, 1);
	std::ostringstream unicName;
	unicName << "Animation component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Animator");
		ImGui::Indent();
		ImGui::Text("Graph name : %s", m_graph ? m_graph->name : "null");
		ImGui::Text("Graph name : %s", m_graphData ? m_graphData->m_name : "null");

		const auto& states = m_graph->getStates();
		const AnimationClip* clip = m_graphData->m_statesData[m_runtimeData.m_currentStateIndex].m_animation;
		ImGui::Text("Current state : %s", states[m_runtimeData.m_currentStateIndex].m_name.c_str());
		ImGui::SliderFloat("CurrentTime", &m_runtimeData.m_stateTime, 0.f, clip->getDuration(), "%.2f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Unindent();

		ImGui::TextColored(componentColor, "Parameters");
		ImGui::Indent();
		using ptype = AnimationGraphParameter::ParameterType;
		for (int i = 0; i < m_runtimeData.m_parameters.size(); i++)
		{
			AnimationGraphParameter& parameter = m_runtimeData.m_parameters[i];
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
					ImGui::DragFloat(parameter.m_name.c_str(), &parameter.m_value.Float);
					break;
				default: break;
			}
		}
		ImGui::Unindent();

		AnimationGraphTransition* transition = m_runtimeData.m_currentTransition;
		if (transition)
		{
			ImGui::TextColored(componentColor, "Transition");
			ImGui::Indent();
			ImGui::Text("From state : %s", transition->m_stateFrom->m_name.c_str());
			ImGui::Text("To state : %s", transition->m_stateTo->m_name.c_str());
			ImGui::SliderFloat("Time", &m_runtimeData.m_transitionTime, 0.f, transition->m_duration, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::Unindent();
		}

		ImGui::TreePop();
	}
#endif
}