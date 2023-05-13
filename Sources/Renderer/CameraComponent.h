#pragma once

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/quaternion.hpp>

#include <EntityComponent/Component.hpp>
#include "Math/TMath.h"


class CameraComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(AnimationComponent, Component)

public:
	explicit CameraComponent(bool freeRotations);

	void onAddToEntity(Entity* entity) override;

	mat4f getViewMatrix() const;
	mat4f getModelMatrix() const;
	vec4f getForward() const;
	vec4f getRight() const;
	vec4f getUp() const;
	vec4f getPosition() const;
	quatf getOrientation() const;
	//float getFieldOfView() const;
	float getVerticalFieldOfView() const;
	void getFrustrum(vec4f& position, vec4f& forward, vec4f& right, vec4f& up) const;

	void setPosition(const vec4f& position);
	void setOrientation(const quatf& orientation);
	void setVerticalFieldOfView(float fov);
	void setDirection(vec4f direction);

	void translate(const vec4f& direction);
	void rotate(const quatf& rotation);
	void rotate(float pitch, float yaw);
	void rotateAround(const vec4f& target, float pitch, float yaw);
	void rotateAround(const vec4f& target, float pitch, float yaw, float distance);
	void lookAt(const vec4f& target);
	void lookAt(const vec4f& target, float distance);

	void onDrawImGui() override;
	void drawDebug(float viewportRatio, float farDistance = 10.f, float nearDistance = 1.f, vec4f color = vec4f(1.f)) const;

private:
	float m_verticalFov;

#ifdef USE_IMGUI
	bool  m_drawFrustrum = false;
	vec2f m_nearFarDistance = vec2f(1, 10);
#endif
};
