#include "LightComponent.h"
#include <EntityComponent/Entity.hpp>
#include <Utiles/Debug.h>
#include <Renderer/Renderer.h>


LightComponent::LightComponent()
{
	m_isPointLight = true;
	m_range = 20;

	m_color = vec4f(1.f);
	m_intensity = 1.f;
	m_innerCutoffAngle = RAD2DEG * 28.0;
	m_outerCutoffAngle = RAD2DEG * 35.0;

	Renderer::getInstance()->addLight(this);
}

void LightComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
}

// setter geter
void LightComponent::setPointLight(bool _isPointLight) { m_isPointLight = _isPointLight; m_isUniformBufferDirty = true; }
void LightComponent::setRange(float _range) { m_range = _range; m_isUniformBufferDirty = true; }
void LightComponent::setColor(vec4f _color) { m_color = _color; m_isUniformBufferDirty = true; }
void LightComponent::setIntensity(float _intensity) { m_intensity = _intensity; m_isUniformBufferDirty = true; }
void LightComponent::setInnerCutOffAngle(float _angleRad) { m_innerCutoffAngle = _angleRad; m_isUniformBufferDirty = true; }
void LightComponent::setOuterCutOffAngle(float _angleRad) { m_outerCutoffAngle = _angleRad; m_isUniformBufferDirty = true; }


vec4f LightComponent::getDirection()
{
	return getParentEntity()->getWorldOrientation() * vec4f(0, 0, 1, 0);
}
vec4f LightComponent::getPosition()
{
	return getParentEntity()->getWorldPosition();
}
bool LightComponent::isPointLight() const { return m_isPointLight; }
float LightComponent::getRange() const { return m_range; }
float LightComponent::getIntensity() const { return m_color.w; }
float LightComponent::getInnerCutOffAngle() const { return m_innerCutoffAngle; }
float LightComponent::getOuterCutOffAngle() const { return m_outerCutoffAngle; }
//

//	Debug
void LightComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	static vec4f lastDirection = getDirection();
	bool hasChanged = (getDirection() - lastDirection).getNorm2() > 0.001f;
	lastDirection = getDirection();

	const ImVec4 componentColor = ImVec4(0.7, 0.7, 0.5, 1);
	std::ostringstream unicName;
	unicName << "Light component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Parameters");
		ImGui::Indent();

		static int typeCurrent = (m_isPointLight ? 0 : 1);
		hasChanged |= ImGui::Combo("Type", &typeCurrent, "Point\0Spot\0\0");
		m_isPointLight = (typeCurrent == 0);

		hasChanged |= ImGui::DragFloat("Range", &m_range, 0.01f, 0.f, 300.f, "%.3fm");
		if (!m_isPointLight)
			hasChanged |= ImGui::DragFloatRange2("CutOff angles", &m_innerCutoffAngle, &m_outerCutoffAngle, 0.1f, 0.f, 180.f, "%.3fdeg");
		hasChanged |= ImGui::ColorEdit3("Color", &m_color[0]);
		hasChanged |= ImGui::DragFloat("Intensity", &m_intensity, 0.01, 0.0001f, 100.f);
		
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw range", &m_drawRange);
		ImGui::Unindent();

		ImGui::TreePop();
	}
	
	m_isUniformBufferDirty |= hasChanged;

	if (m_drawRange)
	{
		Debug::color = m_color;
		Debug::drawWiredSphere(getPosition(), m_range);
	}
#endif // USE_IMGUI
}
//





