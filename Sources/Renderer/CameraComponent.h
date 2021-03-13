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
	glm::vec3 getForward() const;
	glm::vec3 getRight() const;
	glm::vec3 getUp() const;
	glm::vec3 getPosition() const;
	glm::quat getOrientation() const;
	glm::mat4 getGlobalViewMatrix() const;
	glm::vec3 getGlobalPosition() const;
	float getFieldOfView() const;
	float getVerticalFieldOfView(float aspectRatio) const;
	bool getFreeRotations() const;
	void getFrustrum(glm::vec3& position, glm::vec3& forward, glm::vec3& right, glm::vec3& up) const;

	void setPosition(const glm::vec3& position);
	void setOrientation(const glm::quat& orientation);
	void setFieldOfView(float fov);
	void setFreeRotations(bool freeRotations);
	void setDirection(const glm::vec3& direction);

	void translate(const glm::vec3& direction);
	void rotate(const glm::quat& rotation);
	void rotate(float pitch, float yaw);
	void rotateAround(const glm::vec3& target, float pitch, float yaw);
	void rotateAround(const glm::vec3& target, float pitch, float yaw, float distance);
	void lookAt(const glm::vec3& target);
	void lookAt(const glm::vec3& target, float distance);

private:
	glm::quat m_orientation;
	glm::vec3 m_position;
	float m_fov;
	bool m_freeRotations;
};
