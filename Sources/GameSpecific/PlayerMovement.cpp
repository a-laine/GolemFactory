#include "PlayerMovement.h"
#include <EntityComponent/ComponentUpdater.h>
#include "Utiles/Assert.hpp"
#include <Utiles/Debug.h>

#include <Events/EventHandler.h>
#include <HUD/WidgetManager.h>
#include <Events/EventEnum.h>




PlayerMovement::PlayerMovement()
{
	m_camera = nullptr;
	m_animator = nullptr;
	m_smoothedVelocity = vec4f::zero;
	m_immobileDuration = 0.f;
	m_acceleration = 3.f;
	m_groundedCastRadius = 0.35f;
	m_groundedOffset = 0.2f;
	m_jumpImpulse = 8.f;
	m_grounded = true;
}

void PlayerMovement::setCamera(CameraComponent* _camera)
{
	m_camera = _camera;
}

void PlayerMovementUpdate(void* _This, float dt)
{
	PlayerMovement* This = (PlayerMovement*)_This;
	This->update(dt);
}

void PlayerMovement::update(float _dt)
{
	if (!m_camera || !m_animator)
		return;

	// aliases
	Entity* entity = getParentEntity();
	vec4f direction = vec4f::zero;
	vec4f forward = m_camera->getForward();
	forward.y = 0.f;
	forward.normalize();
	vec4f right = m_camera->getRight();
	vec4f playerFwd = (entity->getWorldOrientation() * vec4f(0, 0, 1, 0)).getNormal();
	vec4f playerRight = (entity->getWorldOrientation() * vec4f(1, 0, 0, 0)).getNormal();
	vec4f playerPosition = entity->getWorldPosition();

	// inputs
	float speed = 2.f;
	if (EventHandler::getInstance()->isActivated(EventEnum::FORWARD)) direction += forward;
	if (EventHandler::getInstance()->isActivated(BACKWARD)) direction -= forward;
	if (EventHandler::getInstance()->isActivated(LEFT)) direction -= right;
	if (EventHandler::getInstance()->isActivated(RIGHT)) direction += right;
	if (EventHandler::getInstance()->isActivated(SNEAKY)) speed = 1.f;

	if (std::abs(direction.x) > 0.001f || std::abs(direction.z) > 0.001f)
	{
		direction.normalize();
		m_immobileDuration = 0.f;
		float dot = vec4f::dot(playerFwd, direction);

		if (EventHandler::getInstance()->isActivated(RUN))
		{
			float tolerance = 0.8f;
			speed += std::max((dot - tolerance) / (1.f - tolerance), 0.f);
		}

		float angle = acos(clamp(dot, -1.f, 1.f));
		float sign = playerFwd.x * direction.z - playerFwd.z * direction.x;
		if (sign > 0.f)
			angle = -angle;

		const float angleChange = speed * _dt;
		if (std::abs(angle) > angleChange)
			angle *= angleChange / std::abs(angle);

		quatf dq = quatf(vec3f(0, angle, 0));
		entity->setWorldOrientation(dq * entity->getWorldOrientation());
	}
	else
	{
		speed = 0.f;
		m_immobileDuration += _dt;
	}

	m_grounded = true;

	World* world = entity->getParentWorld();
	if (world)
	{
		Sphere sphere;
		sphere.center = playerPosition + vec4f(0, m_groundedOffset, 0, 0);
		sphere.radius = m_groundedCastRadius;
		m_grounded = world->getPhysics().collisionTest(sphere, &world->getSceneManager(), (uint64_t)Entity::Flags::Fl_Collision,
			(uint64_t)Entity::Flags::Fl_Player);
	}

	if (m_grounded)
	{
		if (EventHandler::getInstance()->isActivated(JUMP))
		{
			m_grounded = false;
			m_smoothedVelocity.y = m_jumpImpulse;
			m_immobileDuration = 0.f;
			m_animator->setParameter("jump", true);
			vec4f v = m_rigidbody->getLinearVelocity() + vec4f(0, m_jumpImpulse, 0, 0);
			m_rigidbody->setLinearVelocity(m_smoothedVelocity);
		}
		else m_smoothedVelocity.y = 0.f;
	}

	// change velocity
	const float velocityChange = m_acceleration * _dt;
	m_velocity = speed * direction;
	vec4f delta = m_velocity - m_smoothedVelocity;
	delta.y = 0;
	float deltaMag = delta.getNorm();
	if (deltaMag > velocityChange)
		delta *= velocityChange / deltaMag;
	m_smoothedVelocity += delta;

	// send to animator
	m_animator->setParameter("moveX", vec4f::dot(m_smoothedVelocity, playerRight));
	m_animator->setParameter("moveZ", vec4f::dot(m_smoothedVelocity, playerFwd));
	m_animator->setParameter("immobileDuration", m_immobileDuration);
	m_animator->setParameter("grounded", m_grounded);

	// integrate velocity
	float pow = sqrt(m_smoothedVelocity.x * m_smoothedVelocity.x + m_smoothedVelocity.z * m_smoothedVelocity.z);
	m_rigidbody->setLinearVelocity(vec4f(pow * m_smoothedVelocity.x, m_rigidbody->getLinearVelocity().y, pow * m_smoothedVelocity.z, 0));
}

void PlayerMovement::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	ComponentUpdater::getInstance()->add(Component::ePlayer, &PlayerMovementUpdate, this);

	m_animator = entity->getComponent<Animator>();
	m_rigidbody = entity->getComponent<RigidBody>();
}

void PlayerMovement::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.7, 0.0, 0.0, 1);
	std::ostringstream unicName;
	unicName << "Player movement##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::DragFloat("jumpImpulse", &m_jumpImpulse, 0.1f);
		ImGui::DragFloat("acceleration", &m_acceleration, 0.1f);
		ImGui::DragFloat("grounded cast radius", &m_groundedCastRadius, 0.1f);
		ImGui::DragFloat("grounded offset", &m_groundedOffset, 0.1f);

		ImGui::TreePop();
	}

	vec4f groundedCastCenter = getParentEntity()->getWorldPosition() + vec4f(0, m_groundedOffset, 0, 0);
	Debug::color = m_grounded ? Debug::orange : Debug::yellow;
	Debug::drawWiredSphere(groundedCastCenter, m_groundedCastRadius);
#endif
}

