#include "CameraComponent.h"
#include "EntityComponent/Entity.hpp"
#include "Utiles/Assert.hpp"
#include <Utiles/Debug.h>


CameraComponent::CameraComponent(bool freeRotations) : m_verticalFov(0.88)
{
}

void CameraComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	setDirection(vec4f(0, 0, 1, 0));
}

mat4f CameraComponent::getViewMatrix() const
{
	return getParentEntity()->getInverseWorldTransformMatrix();
}


mat4f CameraComponent::getModelMatrix() const
{
	return getParentEntity()->getWorldTransformMatrix();
}

vec4f CameraComponent::getRight() const
{
	return getParentEntity()->getWorldOrientation() * vec4f(1, 0, 0, 0);
}

vec4f CameraComponent::getUp() const
{
	return getParentEntity()->getWorldOrientation() * vec4f(0, 1, 0, 0);
}

vec4f CameraComponent::getForward() const
{
	return getParentEntity()->getWorldOrientation() * vec4f(0, 0, -1, 0);
}

vec4f CameraComponent::getPosition() const
{
	return getParentEntity()->getWorldPosition();
}

void CameraComponent::getFrustrum(vec4f& position, vec4f& forward, vec4f& right, vec4f& up) const
{
	position = getPosition();
	forward = getForward();
	right = getRight();
	up = getUp();
}

quatf CameraComponent::getOrientation() const
{
	return getParentEntity()->getWorldOrientation();
}

/*float CameraComponent::getFieldOfView() const
{
	return m_fov;
}*/

float CameraComponent::getVerticalFieldOfView() const
{
	return m_verticalFov;
}


void CameraComponent::setPosition(const vec4f& position)
{
	getParentEntity()->setWorldPosition(position);
}

void CameraComponent::setOrientation(const quatf& orientation)
{
	getParentEntity()->setWorldOrientation(orientation);
}

void CameraComponent::setVerticalFieldOfView(float fov)
{
	m_verticalFov = fov;
}

void CameraComponent::setDirection(vec4f direction)
{
	direction.w = 0.f;
	if (direction.getNorm2() < EPSILON)
		direction = vec4f(0, 0, 1, 0);

	direction.normalize();
	
	GF_ASSERT((std::abs(direction.y) < 1.f - (float)EPSILON), "direction too close to vertical !");

	vec4f right = vec4f::cross(direction, vec4f(0, 1, 0, 0)).getNormal();
	vec4f up = vec4f::cross(right, direction);

	mat4f view(direction, up, right, vec4f(0, 0, 0, 1));
	quatf q = quatf(view);
	q.normalize();
	//getParentEntity()->setWorldTransformation(getParentEntity()->getWorldPosition(), getParentEntity()->getWorldScale(), q);
	getParentEntity()->setWorldOrientation(q);
}

void CameraComponent::translate(const vec4f& direction)
{
	getParentEntity()->setWorldPosition(getParentEntity()->getWorldPosition() + direction);
}

void CameraComponent::rotate(const quatf& rotation)
{
	getParentEntity()->setWorldOrientation(rotation * getParentEntity()->getWorldOrientation());
}

void CameraComponent::rotate(float verticalDelta, float horizontalDelta)
{
	quatf orientation = getParentEntity()->getWorldOrientation();
	vec4f front = orientation * vec4f(0, 0, 1, 0);
	vec4f up = orientation * vec4f(0, 1, 0, 0);
	vec4f right = orientation * vec4f(1, 0, 0, 0);

	front = front + horizontalDelta * right;
	vec4f newfront = front - verticalDelta * up;
	if (std::abs(newfront.y) < 0.97f)
		front = newfront;

	front.normalize();
	right = vec4f::cross(vec4f(0, 1, 0, 0), front).getNormal();
	up = vec4f::cross(front, right);

	mat4f view(right, up, front, vec4f(0, 0, 0, 1));
	quatf q = quatf(view);
	q.normalize();
	//getParentEntity()->setWorldTransformation(getParentEntity()->getWorldPosition(), getParentEntity()->getWorldScale(), q);
	getParentEntity()->setWorldOrientation(q);
}

void CameraComponent::rotateAround(const vec4f& target, float pitch, float yaw)
{
	rotate(pitch, yaw);
	float d =(getParentEntity()->getWorldPosition() - target).getNorm();
	getParentEntity()->setWorldPosition(target - d * getForward());
}

void CameraComponent::rotateAround(const vec4f& target, float pitch, float yaw, float distance)
{
	rotate(pitch, yaw);
	vec4f fwd = getForward();
	getParentEntity()->setWorldPosition(target - distance * getForward());
}

void CameraComponent::lookAt(const vec4f& target)
{
	setDirection(target - getParentEntity()->getWorldPosition());
}

void CameraComponent::lookAt(const vec4f& target, float distance)
{
	setDirection(target - getParentEntity()->getWorldPosition());
	getParentEntity()->setWorldPosition(target - distance * getForward());
}

void CameraComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.5, 0.5, 0.7, 1);
	std::ostringstream unicName;
	unicName << "Camera component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		internalImGuiDraw();
		ImGui::TreePop();
	}

	if (m_drawFrustrum)
	{
		drawDebug(Debug::viewportRatio, m_nearFarDistance.x, m_nearFarDistance.y, 
			vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w));
	}
#endif // USE_IMGUI
}

void CameraComponent::internalImGuiDraw()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.5, 0.5, 0.7, 1);
	ImGui::TextColored(componentColor, "Camera parameters");
	ImGui::Indent();
	constexpr float rangeMin = (float)RAD2DEG * 0.05f;
	constexpr float rangeMax = (float)RAD2DEG * 1.5f;
	float fov = (float)RAD2DEG * m_verticalFov;
	if (ImGui::SliderFloat("Vertical Fov", &fov, rangeMin, rangeMax, "%.3frad"))
		m_verticalFov = (float)RAD2DEG * fov;
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::TextColored(componentColor, "Camera gizmos");
	ImGui::Indent();
	ImGui::Checkbox("Draw frustrum shape", &m_drawFrustrum);
	ImGui::DragFloat2("Debug range", &m_nearFarDistance[0], 0.1f, 0.f, 1000.f, "%.3f");
	ImGui::Unindent();
#endif // USE_IMGUI
}

void CameraComponent::drawDebug(float viewportRatio, float farDistance, float nearDistance, vec4f color) const
{
	// aliases
	vec4f p = getPosition();
	vec4f dir = getForward();
	vec4f left = -getRight();
	vec4f up = getUp();
	float a1 = 0.5f * m_verticalFov;
	float ca1 = cos(a1);
	float sa1 = sin(a1);
	float sa2 = viewportRatio * sa1;
	vec4f tdir = ca1 * dir;
	vec4f u1 = tdir + sa2 * left + sa1 * up;
	vec4f u2 = tdir - sa2 * left + sa1 * up;
	vec4f u3 = tdir + sa2 * left - sa1 * up;
	vec4f u4 = tdir - sa2 * left - sa1 * up;

	// boundaries
	Debug::color = color;
	Debug::drawLine(p + nearDistance * u1, p + farDistance * u1);
	Debug::drawLine(p + nearDistance * u2, p + farDistance * u2);
	Debug::drawLine(p + nearDistance * u3, p + farDistance * u3);
	Debug::drawLine(p + nearDistance * u4, p + farDistance * u4);

	Debug::drawLine(p + nearDistance * u1, p + nearDistance * u2);
	Debug::drawLine(p + nearDistance * u3, p + nearDistance * u4);
	Debug::drawLine(p + nearDistance * u1, p + nearDistance * u3);
	Debug::drawLine(p + nearDistance * u2, p + nearDistance * u4);

	Debug::drawLine(p + farDistance * u1, p + farDistance * u2);
	Debug::drawLine(p + farDistance * u3, p + farDistance * u4);
	Debug::drawLine(p + farDistance * u1, p + farDistance * u3);
	Debug::drawLine(p + farDistance * u2, p + farDistance * u4);

	// plane normals
	float d = (0.25f * (farDistance + nearDistance));
	vec4f nup = ca1 * up - sa1 * dir;
	vec4f cup = p + d * (u1 + u2);
	Debug::drawPoint(cup);
	Debug::drawLine(cup, cup + nup);

	vec4f ndown = -ca1 * up - sa1 * dir;
	vec4f cdown = p + d * (u3 + u4);
	Debug::drawPoint(cdown);
	Debug::drawLine(cdown, cdown + ndown);

	vec4f nleft = ca1 * left - sa2 * dir;
	vec4f cleft = p + d * (u1 + u3);
	Debug::drawPoint(cleft);
	Debug::drawLine(cleft, cleft + nleft);

	vec4f nright = -ca1 * left - sa2 * dir;
	vec4f cright = p + d * (u2 + u4);
	Debug::drawPoint(cright);
	Debug::drawLine(cright, cright + nright);

	vec4f cfar = p + farDistance * tdir;
	vec4f cnear = p + nearDistance * tdir;
	Debug::drawPoint(cfar);
	Debug::drawLine(cfar, cfar + dir);
	Debug::drawPoint(cnear);
	Debug::drawLine(cnear, cnear - dir);
}