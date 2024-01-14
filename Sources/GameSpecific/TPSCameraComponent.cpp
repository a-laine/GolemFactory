#include <GameSpecific/TPSCameraComponent.h>
#include <EntityComponent/ComponentUpdater.h>
#include "Utiles/Assert.hpp"
#include <Utiles/Debug.h>

#include <Events/EventHandler.h>
#include "HUD/WidgetManager.h"


void TPSCameraComponentUpdate(void* componentPtr, float dt)
{
	TPSCameraComponent* camera = (TPSCameraComponent*)componentPtr;
	camera->update(dt);
}

TPSCameraComponent::TPSCameraComponent() : CameraComponent(true)
{
	m_targetCharacter = nullptr;
	m_targetOffset = vec4f(-0.5f, 1.f, 0, 0);
	m_radius = 3.f;

#ifdef USE_IMGUI
	m_nearFarDistance.y = 2.f;
#endif
}

void TPSCameraComponent::setTargetCharacter(Entity* _targetCharacter)
{
	m_targetCharacter = _targetCharacter;
}

void TPSCameraComponent::update(float _dt)
{
	if (!m_targetCharacter)
		return;

	if (EventHandler::getInstance()->getCursorMode() || !WidgetManager::getInstance()->getBoolean("syncCamera"))
		return;

	float sensitivity = 0.2f;
	vec2f relCursorPos = EventHandler::getInstance()->getCursorPositionRelative();
	vec2f relScroll = EventHandler::getInstance()->getScrollingRelative();
	float yaw = -(float)DEG2RAD * sensitivity * relCursorPos.x;
	float pitch = -(float)DEG2RAD * sensitivity * relCursorPos.y;
	vec4f fwd = getForward();
	if (fwd.y > 0.f)
		pitch = std::min(pitch, 0.f);

	m_radius -= sensitivity * relScroll.y;
	m_radius = clamp(m_radius, 0.5f, 10.f);

	vec4f target = m_targetCharacter->getWorldPosition() + m_targetCharacter->getWorldOrientation() * m_targetOffset;
	rotateAround(target, pitch, yaw, m_radius);
}

void TPSCameraComponent::onAddToEntity(Entity* entity)
{
	CameraComponent::onAddToEntity(entity);
	ComponentUpdater::getInstance()->add(Component::ePlayer, &TPSCameraComponentUpdate, this);
}

void TPSCameraComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.5f, 0.5f, 0.7f, 1.f);
	std::ostringstream unicName;
	unicName << "TPS Camera component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		internalImGuiDraw();

		ImGui::TextColored(componentColor, "TPS cam parameters");
		ImGui::Indent();
		ImGui::DragFloat3("Target offset", &m_targetOffset.x, 0.1f);
		ImGui::SliderFloat("Radius", &m_radius, 0.f, 10.f);
		ImGui::Unindent();


		ImGui::TextColored(componentColor, "TPS cam gizmos");

		ImGui::TreePop();
	}

	if (m_drawFrustrum)
	{
		drawDebug(Debug::viewportRatio, m_nearFarDistance.x, m_nearFarDistance.y,
			vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w));
	}
	if (m_drawDebug)
	{
		const float l = 0.15f;
		const float sinA = 0.34f * l;

		vec4f o = m_targetCharacter->getWorldPosition();
		vec4f v = m_targetCharacter->getWorldOrientation() * m_targetOffset;
		vec4f vn = (std::abs(v.x) > std::abs(v.z) ? vec4f(-v.y, v.x, 0, 0) : vec4f(0, -v.z, v.y, 0)).getNormal();
		vec4f u = v.getNormal();
		vec4f target = o + v;

		Debug::color = m_targetColor;
		Debug::drawLine(o, target);
		Debug::drawLine(target, target - l * u + sinA * vn);
		Debug::drawLine(target, target - l * u - sinA * vn);
		Debug::drawSphere(target, 0.3f * l);
	}
#endif // USE_IMGUI
}