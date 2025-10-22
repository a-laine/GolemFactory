#pragma once


#include <EntityComponent/Component.hpp>
#include <Renderer/CameraComponent.h>
#include <Math/TMath.h>
#include <Animation/Animator.h>
#include <Physics/Physics.h>
#include <Physics/CollisionReport.h>
#include <Physics/Shapes/Collider.h>

class CharacterController : public Component
{
	GF_DECLARE_COMPONENT_CLASS(CharacterController, Component)

	public:
		CharacterController();

		void setCamera(CameraComponent* _camera);

		void update(Component::UpdatePass updatePass, float _dt) override;

		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;

		void setControlable(bool enableControl);
		void setCompanion(bool enableCompanion);
		void setMale(bool enableMale);
		void setSpeeds(float walk, float run, float sprint);
		void setJumpForce(float force);
		
		bool isControlable() const;
		bool isCompanion() const;
		bool isMale() const;


		enum CharacterState
		{
			eMoveIdle = 0,
			eJumping,
			eSliding,
			eFalling,
			eLanding,
			eAction,
			eCarControl,
			eInteractionWithOther
		};

		static std::string getCharacterStateString(CharacterState state);

	protected:
		CameraComponent* m_camera;
		Animator* m_animator;
		RigidBody* m_rigidbody;
		Collider* m_collider;

		Physics::CollisionCache m_collisionCache;

		bool m_controllable = true;
		bool m_companion = false;
		bool m_isMale = true;

#pragma region Movement
		float m_walkSpeed = 1.5f;
		float m_runSpeed = 4.f;
		float m_sprintSpeed = 8.f;
		float m_jumpForce = 8.f;
		float m_aimingSmoothTime = 0.003f;
		float m_smoothGain = 0.1f;
		float m_slopeSmoothGain = 0.1f;
		float m_aimingSmoothVelocity;
		float m_smoothedHorizontal = 0.f;
		float m_smoothedVertical = 0.f;
		float m_localSlopeAngle = 0.f;
		float m_run = 0.f;
		vec4f m_previousMovement = vec4f::zero;
		float m_leanValue = 0.f;
		float m_leanSmoothGain = 0.1f;
		float m_headAimingSpeed = 100.f;
		float m_standingControllerHeight = 1.8f;
#pragma endregion

#pragma region Grounded+jump
		RaycastReport m_groundResult;
		float m_raycastGroundDistance = 0.3f;
		float m_extraGroundSphereRadius = 0.3f;
		float m_jumpDelay = 0.4f;
		float m_jumpOutDelay = 0.3f;
		float m_jumpTimer = 0.f;
		float m_slideDuration = 1.f;
		float m_landedDelay = 0.4f;
		float m_landedTimer = 0.f;
		float m_slopeDeviationForce = 0.5f;
		float m_slopeLimit = 30.f;

		bool m_isGrounded = false;
		bool m_isSneaking = false;
#pragma endregion

#pragma region State+vehicle+interact
		CharacterState m_characterState;

#pragma endregion

	#ifdef USE_IMGUI
		bool m_drawCollisionCache = false;
		bool m_drawGroundTestShapes = false;
	#endif

		float computeSpeed(bool isWalking, bool isSprinting, float forwardDirection, float dt);

		/*vec4f m_velocity;
		vec4f m_smoothedVelocity;
		float m_acceleration;
		float m_speed;
		float m_immobileDuration;
		float m_groundedCastRadius;
		float m_jumpImpulse;
		float m_groundedOffset;
		bool m_grounded;*/
};


