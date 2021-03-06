#include "CameraComponent.h"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/constants.hpp>

#include <EntityComponent/Entity.hpp>


CameraComponent::CameraComponent(bool freeRotations)
	: m_orientation(1.f, 0.f, 0.f, 0.f)
	, m_position(0.f)
	, m_fov(90)
	, m_freeRotations(freeRotations)
{
}

glm::mat4 CameraComponent::getViewMatrix() const
{
	glm::mat4 m;
	glm::quat rot = glm::inverse(m_orientation);
	m = glm::toMat4(rot);
	m[3] = glm::rotate(rot, glm::vec4(-m_position, 1.f));
	return m;
}

glm::mat4 CameraComponent::getGlobalViewMatrix() const
{
	glm::mat4 m = glm::toMat4(m_orientation);
	m[3] = glm::vec4(m_position, 1.f);
	if (m_freeRotations)
	{
		m[3] *= glm::vec4(getParentEntity()->getScale(), 1.f);
		m[3] += glm::vec4(getParentEntity()->getPosition(), 0.f);
	}
	else
	{
		m = m * getParentEntity()->getMatrix();
	}
	return glm::inverse(m);
}

glm::mat4 CameraComponent::getModelMatrix() const
{
	glm::mat4 m = glm::toMat4(m_orientation);
	m[3] = glm::vec4(m_position, 1.f);
	return m;
}

glm::vec3 CameraComponent::getForward() const
{
	return glm::rotate(m_orientation, glm::vec3(0, 0, -1));
}

glm::vec3 CameraComponent::getRight() const
{
	return glm::rotate(m_orientation, glm::vec3(1, 0, 0));
}

glm::vec3 CameraComponent::getUp() const
{
	return glm::rotate(m_orientation, glm::vec3(0, 1, 0));
}

glm::vec3 CameraComponent::getPosition() const
{
	return m_position;
}

glm::vec3 CameraComponent::getGlobalPosition() const
{
	return m_position * getParentEntity()->getScale() + getParentEntity()->getPosition();
}

void CameraComponent::getFrustrum(glm::vec3& position, glm::vec3& forward, glm::vec3& right, glm::vec3& up) const
{
	if (m_freeRotations)
	{
		position = m_position * getParentEntity()->getScale() + getParentEntity()->getPosition();
		forward = glm::rotate(m_orientation, glm::vec3(0, 0, -1));
		right = glm::rotate(m_orientation, glm::vec3(1, 0, 0));
		up = glm::rotate(m_orientation, glm::vec3(0, 1, 0));
	}
	else
	{
		position = glm::vec3(getParentEntity()->getMatrix() * glm::vec4(m_position, 1.f));
		forward = glm::vec3(getParentEntity()->getMatrix() * glm::rotate(m_orientation, glm::vec4(0, 0, -1, 0)));
		right = glm::vec3(getParentEntity()->getMatrix() * glm::rotate(m_orientation, glm::vec4(1, 0, 0, 0)));
		up = glm::vec3(getParentEntity()->getMatrix() * glm::rotate(m_orientation, glm::vec4(0, 1, 0, 0)));
	}
}

glm::quat CameraComponent::getOrientation() const
{
	return m_orientation;
}

float CameraComponent::getFieldOfView() const
{
	return m_fov;
}

float CameraComponent::getVerticalFieldOfView(float aspectRatio) const
{
	return m_fov / aspectRatio;
}

bool CameraComponent::getFreeRotations() const
{
	return m_freeRotations;
}

void CameraComponent::setPosition(const glm::vec3& position)
{
	m_position = position;
}

void CameraComponent::setOrientation(const glm::quat& orientation)
{
	m_orientation = orientation;
}

void CameraComponent::setFieldOfView(float fov)
{
	m_fov = fov;
}

void CameraComponent::setFreeRotations(bool freeRotations)
{
	m_freeRotations = freeRotations;
}

void CameraComponent::setDirection(const glm::vec3& direction)
{
	glm::vec3 euler = glm::eulerAngles(m_orientation);

	if (!(direction.x == 0 && direction.y == 0 && direction.z == 0))
		euler.x = atan2(sqrt(direction.x*direction.x + direction.y*direction.y), -direction.z);

	if (direction.x != 0 || direction.y != 0)
		euler.z = -atan2(direction.x, direction.y);

	m_orientation = glm::quat(euler);
}

void CameraComponent::translate(const glm::vec3& direction)
{
	m_position += direction;
}

void CameraComponent::rotate(const glm::quat& rotation)
{
	m_orientation *= rotation;
}

void CameraComponent::rotate(float pitch, float yaw)
{
	glm::vec3 euler = glm::eulerAngles(m_orientation) + glm::vec3(pitch, 0.f, yaw);

	float _Pi = glm::pi<float>();
	float twoPi = 2.f*glm::pi<float>();

	if (euler.x > _Pi - 0.01f) euler.x = _Pi - 0.01f;
	else if (euler.x < 0.01f) euler.x = 0.01f;
	if (euler.z > _Pi) euler.z -= twoPi;
	else if (euler.z < -_Pi) euler.z += twoPi;

	m_orientation = glm::quat(euler);
}

void CameraComponent::rotateAround(const glm::vec3& target, float pitch, float yaw)
{
	rotate(pitch, yaw);

	float d = glm::distance(m_position, target);
	m_position = target - d * getForward();
}

void CameraComponent::rotateAround(const glm::vec3& target, float pitch, float yaw, float distance)
{
	rotate(pitch, yaw);
	m_position = target - distance * getForward();
}

void CameraComponent::lookAt(const glm::vec3& target)
{
	setDirection(target - m_position);
}

void CameraComponent::lookAt(const glm::vec3& target, float distance)
{
	setDirection(target - m_position);
	m_position = target - distance * getForward();
}
