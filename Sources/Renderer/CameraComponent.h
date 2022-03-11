#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <EntityComponent/Component.hpp>


class CameraComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(AnimationComponent, Component)

public:
	explicit CameraComponent(bool freeRotations);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getModelMatrix() const;
	glm::vec4 getForward() const;
	glm::vec4 getRight() const;
	glm::vec4 getUp() const;
	glm::vec4 getPosition() const;
	glm::quat getOrientation() const;
	glm::mat4 getGlobalViewMatrix() const;
	glm::vec4 getGlobalPosition() const;
	float getFieldOfView() const;
	float getVerticalFieldOfView(float aspectRatio) const;
	bool getFreeRotations() const;
	void getFrustrum(glm::vec4& position, glm::vec4& forward, glm::vec4& right, glm::vec4& up) const;

	void setPosition(const glm::vec4& position);
	void setOrientation(const glm::quat& orientation);
	void setFieldOfView(float fov);
	void setFreeRotations(bool freeRotations);
	void setDirection(const glm::vec4& direction);

	void translate(const glm::vec4& direction);
	void rotate(const glm::quat& rotation);
	void rotate(float pitch, float yaw);
	void rotateAround(const glm::vec4& target, float pitch, float yaw);
	void rotateAround(const glm::vec4& target, float pitch, float yaw, float distance);
	void lookAt(const glm::vec4& target);
	void lookAt(const glm::vec4& target, float distance);

private:
	glm::quat m_orientation;
	glm::vec4 m_position;
	float m_fov;
	bool m_freeRotations;
};
