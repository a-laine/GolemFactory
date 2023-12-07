#pragma once

#include <EntityComponent/Component.hpp>
#include <Renderer/CameraComponent.h>
#include <Math/TMath.h>
#include <Animation/Animator.h>
#include <Physics/RigidBody.h>

class PlayerMovement : public Component
{
	GF_DECLARE_COMPONENT_CLASS(PlayerMovement, Component)

	public:
		PlayerMovement();

		void setCamera(CameraComponent* _camera);

		void update(float _dt);

		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;

	protected:
		CameraComponent* m_camera;
		Animator* m_animator;
		RigidBody* m_rigidbody;
		vec4f m_velocity;
		vec4f m_smoothedVelocity;
		float m_acceleration;
		float m_speed;
		float m_immobileDuration;
		float m_groundedCastRadius;
		float m_jumpImpulse;
		float m_groundedOffset;
		bool m_grounded;
};


