#include "CharacterController.h"
#include <EntityComponent/ComponentUpdater.h>
#include "Utiles/Assert.hpp"
#include <Utiles/Debug.h>

#include <Events/EventHandler.h>
#include <HUD/WidgetManager.h>
#include <Events/EventEnum.h>
#include <ConsoleColor.h>




CharacterController::CharacterController()
{
	m_camera = nullptr;
	m_animator = nullptr;
	m_collider = nullptr;

	m_characterState = CharacterState::eMoveIdle;
	m_isGrounded = true;
}

void CharacterController::setCamera(CameraComponent* _camera)
{
	m_camera = _camera;
}

void CharacterController::update(Component::UpdatePass updatePass, float _dt)
{
	SCOPED_CPU_MARKER("CharacterController");
	if (!m_camera || !m_animator)
		return;

	// update called in fixed update
	if (updatePass == Component::UpdatePass::ePhysics)
	{
		m_rigidbody->setLinearVelocity(vec4f(m_previousMovement.x, m_rigidbody->getLinearVelocity().y, m_previousMovement.z, 0));
		return;
	}

	// aliases
	Entity* entity = getParentEntity();
	vec4f direction = vec4f::zero;
	vec4f cameraForward = m_camera->getForward();
	cameraForward.y = 0.f;
	cameraForward.normalize();
	vec4f left = m_camera->getLeft();
	EventHandler* eventMgr = EventHandler::getInstance();

	bool isWalking, isSprinting, wannaJump, doAction, doSlide;
	float horizontal, vertical, move_x, move_z, localSlope, groundDistance, groundnearby;
	horizontal = vertical = move_x = move_z = 0.f;
	doSlide = false;
	vec4f currentForward = entity->getWorldOrientation() * vec4f(0, 0, 1, 0);
	vec4f currentLeft = entity->getWorldOrientation() * vec4f(1, 0, 0, 0);
	vec4f targetForwardDirection = currentForward;

	const auto PrintWarning = [&](const std::string& msg)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
		{
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : " << entity->getName() <<
				" : CharacterController : " << msg << std::flush;
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	};
	const auto PrintError = [&](const std::string& msg)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
		{
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR : " << entity->getName() <<
				" : CharacterController : " << msg << std::flush;
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	};

	// inputs
	doAction = !m_companion && eventMgr->isActivated(ACTION);
	if (m_controllable)
	{
		if (eventMgr->isActivated(LEFT)) horizontal += 1.f;
		if (eventMgr->isActivated(RIGHT)) horizontal -= 1.f;
		if (eventMgr->isActivated(EventEnum::FORWARD)) vertical += 1.f;
		if (eventMgr->isActivated(BACKWARD)) vertical -= 1.f;
		isSprinting = eventMgr->isActivated(RUN);
		if (isSprinting)
			m_isSneaking = false;
		if (eventMgr->isActivated(SNEAKY, Event::EventType::ACTIVATED_BUTTON_DOWN))
		{
			doSlide = isSprinting && !m_isSneaking;
			m_isSneaking = !m_isSneaking;
		}
		isWalking = m_isSneaking;
		wannaJump = eventMgr->isActivated(JUMP);
		targetForwardDirection = cameraForward;
	}
	else
	{
		isWalking = isSprinting = wannaJump = false;
		horizontal = vertical = 0.f;
		currentForward.y = 0.f;
		currentForward.normalize();
		targetForwardDirection = currentForward;
	}

	// ground check
	localSlope = 0.f;
	groundDistance = 2.f;
	groundnearby = false;
	World* world = entity->getParentWorld();
	if (world)
	{
		m_collisionCache.m_aabb = entity->getBoundingBox();
		m_collisionCache.m_aabb.min -= vec4f(2, 2, 2, 0);
		m_collisionCache.m_aabb.max += vec4f(2, 2, 2, 0);

		world->getPhysics().getCollisionCache(&world->getSceneManager(), m_collisionCache, (uint64_t)Entity::Flags::Fl_Collision,
			(uint64_t)Entity::Flags::Fl_Player);

		Segment ray(entity->getWorldPosition() + vec4f(0, m_raycastGroundDistance, 0, 0), entity->getWorldPosition() - vec4f(0, 5 * m_raycastGroundDistance, 0, 0));
		m_isGrounded = world->getPhysics().raycastInCache(m_collisionCache, ray, &world->getSceneManager(),
			(uint64_t)Entity::Flags::Fl_Collision, 
			(uint64_t)Entity::Flags::Fl_Player,
			&m_groundResult);
		groundnearby = m_isGrounded;

		if (m_isGrounded)
		{
			vec4f playerFwd = (entity->getWorldOrientation() * vec4f(0, 0, 1, 0)).getNormal();
			m_isGrounded = m_groundResult.m_normal.y > std::cos(m_slopeLimit * DEG2RAD) && m_groundResult.m_distance < m_raycastGroundDistance;
			localSlope = -std::acos(m_groundResult.m_normal.y) * RAD2DEG * glm::sign(vec4f::dot(playerFwd, m_groundResult.m_normal));
			groundDistance = m_groundResult.m_distance;
		}
		if (!m_isGrounded)
		{
			Sphere sphere;
			sphere.radius = m_extraGroundSphereRadius;
			sphere.center = entity->getWorldPosition();
			m_isGrounded = world->getPhysics().collisionInCache(m_collisionCache, &sphere, &world->getSceneManager(), (uint64_t)Entity::Flags::Fl_Collision,
				(uint64_t)Entity::Flags::Fl_Player);
		}
	}

	m_localSlopeAngle = (1.f - m_slopeSmoothGain) * m_localSlopeAngle + m_slopeSmoothGain * localSlope;
	m_smoothedHorizontal = (1.f - m_smoothGain) * m_smoothedHorizontal + m_smoothGain * horizontal;
	m_smoothedVertical = (1.f - m_smoothGain) * m_smoothedVertical + m_smoothGain * vertical;
	bool inputMovement = std::abs(horizontal) > 0.001f || std::abs(vertical) > 0.001f;
	direction = vec4f(horizontal, 0, vertical, 0);
	if (inputMovement)
		direction.normalize();

	switch (m_characterState)
	{
		case CharacterState::eMoveIdle:
			{
				// falling
				if (!m_isGrounded)
				{
					m_characterState = CharacterState::eFalling;
					move_x = 0.f;
					move_z = 0.f;
					m_run = 0.f;
				}

				// movement
				else if (inputMovement)
				{
					currentForward.y = 0.f;
					currentForward.normalize();
					float curentAimAngle = -std::acos(clamp(vec4f::dot(currentForward, targetForwardDirection), -1.f, 1.f));
					if (vec4f::dot(currentForward, left) < 0)
						curentAimAngle *= -1.f;
					float angle = 0.1f * curentAimAngle;// smoothDamped(curentAimAngle, 0.f, m_aimingSmoothVelocity, m_aimingSmoothTime, _dt);
					quatf dq = quatf(std::cos(0.5f * angle), 0, std::sin(0.5f * angle), 0);
					//vec4f newForward = quatf(std::cos(0.5f * angle), 0, std::sin(0.5f * angle), 0) * currentForward;
					//quatf q = quatf(vec3f(0, 0, 1), newForward.xyz());
					entity->setWorldOrientation(dq * entity->getWorldOrientation() * (-dq));

					currentForward = entity->getWorldOrientation() * vec4f(0, 0, 1, 0);
					currentLeft = entity->getWorldOrientation() * vec4f(1, 0, 0, 0);
					float speed = computeSpeed(isWalking && !doSlide, isSprinting, direction.z, _dt);
					vec4f movementDirection = m_smoothedVertical * currentForward + m_smoothedHorizontal * currentLeft;
					if (movementDirection.getNorm2() > 0.0001f)
						movementDirection.normalize();
					vec4f newSpeed = speed * movementDirection;
					float curvingAngle = m_run * vec4f::dot(newSpeed - m_previousMovement, currentLeft);
					m_leanValue = (1.f - m_leanSmoothGain) * m_leanValue + m_leanSmoothGain * curvingAngle;

					m_previousMovement = newSpeed;
					move_x = (isSprinting && vertical != 0.f) ? 0.f : m_run * m_smoothedHorizontal;
					move_z = m_run * m_smoothedVertical;

					if (doSlide)
					{
						m_jumpTimer = m_slideDuration;
						m_run = 3.f;
						move_z = 3.f;
						m_animator->setParameter("slide", true);
						m_characterState = CharacterState::eSliding;
					}
					else if (wannaJump)
					{
						m_characterState = CharacterState::eJumping;
						m_jumpTimer = 0.1f;
						m_animator->setParameter("jump", true);
					}
				}

				// idle
				else
				{
					m_run = moveTowards(m_run, 0.f, 10 * _dt);
					move_x = m_run * m_smoothedHorizontal;
					move_z = m_run * m_smoothedVertical;
					m_previousMovement = vec4f::zero;
					m_leanValue = (1.f - m_leanSmoothGain) * m_leanValue;

					if (wannaJump)
					{
						m_characterState = CharacterState::eJumping;
						m_jumpTimer = m_jumpDelay;
						m_animator->setParameter("jump", true);
					}
				}

				m_animator->setParameter("moveX", move_x);
				m_animator->setParameter("moveZ", move_z);
				m_animator->setParameter("locomotionFloat", move_z * m_localSlopeAngle);
			}
			break;

		case CharacterState::eSliding:
			{
				// falling
				if (!m_isGrounded)
				{
					m_characterState = CharacterState::eFalling;
					move_x = 0.f;
					move_z = 0.f;
					m_run = 0.f;
					m_animator->setParameter("fall", true);
					m_jumpTimer = 0.f;
				}

				// movement
				else if (m_jumpTimer > 0.f)
				{
					m_jumpTimer -= _dt;
					m_previousMovement *= 0.99f;
				}
				else
				{
					m_characterState = CharacterState::eMoveIdle;
				}

				m_animator->setParameter("moveX", move_x);
				m_animator->setParameter("moveZ", move_z);
				m_animator->setParameter("locomotionFloat", move_z* m_localSlopeAngle);
				m_leanValue = (1.f - m_leanSmoothGain) * m_leanValue;
			}
			break;

		case CharacterState::eJumping:
			{
				m_isSneaking = false;
				if (m_jumpTimer > 0.f)
				{
					m_jumpTimer -= _dt;
					if (m_jumpTimer <= 0.f)
					{
						vec4f vel = m_rigidbody->getLinearVelocity();
						vel.y += m_jumpForce;
						m_rigidbody->setLinearVelocity(vel);
					}
				}
				else
				{
					m_jumpTimer -= _dt;
					if (m_jumpTimer < -m_jumpOutDelay)
						m_characterState = CharacterState::eFalling;
				}

				m_animator->setParameter("moveX", 0.f);
				m_animator->setParameter("moveZ", m_run);
				m_animator->setParameter("locomotionFloat", groundDistance);
				m_leanValue = (1.f - m_leanSmoothGain) * m_leanValue;
			}
			break;

		case CharacterState::eFalling:
			{
				m_isSneaking = false;
				if (m_isGrounded)
				{
					m_characterState = CharacterState::eLanding;
					m_landedTimer = m_landedDelay;
				}
				else
				{
					if (groundnearby)
					{
						vec4f n = m_groundResult.m_normal;
						n.x += 0.00001f;
						n.y = 0.f;
						m_previousMovement += m_slopeDeviationForce * n;
					}
				}

				m_animator->setParameter("moveX", 0.f);
				m_animator->setParameter("moveZ", m_run);
				m_animator->setParameter("locomotionFloat", groundDistance);
				m_leanValue = (1.f - m_leanSmoothGain) * m_leanValue;
			}
			break;

		case CharacterState::eLanding:
			{
				if (m_landedTimer > 0.f)
				{
					m_landedTimer -= _dt;
					if (m_landedTimer <= 0.f)
					{
						m_characterState = CharacterState::eMoveIdle;
					}
				}

				if (m_previousMovement.getNorm2() > 0.1f)
				{
					float speed = computeSpeed(isWalking || m_isSneaking, isSprinting, direction.z, _dt);
					vec4f movementDirection = m_smoothedVertical * currentForward + m_smoothedHorizontal * currentLeft;
					if (movementDirection.getNorm2() > 0.0001f)
						movementDirection.normalize();
					m_previousMovement = speed * movementDirection;

					move_x = (isSprinting && vertical != 0.f) ? 0.f : m_run * m_smoothedHorizontal;
					move_z = m_run * m_smoothedVertical;

					m_animator->setParameter("moveX", move_x);
					m_animator->setParameter("moveZ", move_z);
					m_animator->setParameter("locomotionFloat", groundDistance);
				}
				else
				{
					m_run = 0.f;
				}

				m_leanValue = (1.f - m_leanSmoothGain) * m_leanValue;
			}
			break;

		// WTF ?!
		default:
			{
				PrintError("Unhandled state !" + getCharacterStateString(m_characterState));
				m_characterState = CharacterState::eMoveIdle;
				m_run = 0.f;
				m_animator->setParameter("moveX", 0);
				m_animator->setParameter("moveZ", 0);
			}
			break;
	}

	m_animator->setParameter("grounded", m_isGrounded);

	m_rigidbody->setGravityFactor((m_isGrounded && m_characterState != CharacterState::eJumping) ? 6.f : 1.f);
	m_rigidbody->setLinearVelocity(vec4f(m_previousMovement.x, m_rigidbody->getLinearVelocity().y, m_previousMovement.z, 0));
}

float CharacterController::computeSpeed(bool isWalking, bool isSprinting, float forwardDirection, float dt)
{
	float speed;
	if (isWalking)
	{
		speed = m_walkSpeed;
		if (m_run > 1.f)
			m_run = moveTowards(m_run, 1.f, 15.f * dt);
		else
			m_run = moveTowards(m_run, 1.f, 5.f * dt);
	}
	else if (isSprinting && forwardDirection > 0.71f)
	{
		speed = m_sprintSpeed;
		if (m_run < 2.f)
			m_run = moveTowards(m_run, 3.f, 15.f * dt);
		else
			m_run = moveTowards(m_run, 3.f, 5.f * dt);
	}
	else
	{
		float fwdboost = forwardDirection > 0.f ? std::sqrt(forwardDirection) : 0.f;
		speed = m_runSpeed + fwdboost;
		if (m_run < 1.f)
			m_run = moveTowards(m_run, 2.f, 10.f * dt);
		else
			m_run = moveTowards(m_run, 2.f, 5.f * dt);
	}
	return speed;
}


std::string CharacterController::getCharacterStateString(CharacterState state)
{
	switch (state)
	{
		case CharacterController::eMoveIdle: return "eMoveIdle";
		case CharacterController::eJumping: return "eJumping";
		case CharacterController::eSliding: return "eSliding";
		case CharacterController::eFalling: return "eFalling";
		case CharacterController::eLanding: return "eLanding";
		case CharacterController::eAction: return "eAction";
		case CharacterController::eCarControl: return "eCarControl";
		case CharacterController::eInteractionWithOther: return "eInteractionWithOther";
		default: return "???";
	}
	return "???!!!???";
}

void CharacterController::setControlable(bool enableControl) { m_controllable = enableControl; }
void CharacterController::setCompanion(bool enableCompanion) { m_companion = enableCompanion; }
void CharacterController::setMale(bool enableMale) { m_isMale = enableMale; }
void CharacterController::setSpeeds(float walk, float run, float sprint)
{
	m_walkSpeed = walk;
	m_runSpeed = run;
	m_sprintSpeed = sprint;
}
void CharacterController::setJumpForce(float force)
{
	m_jumpForce = force;
}


bool CharacterController::isControlable() const { return m_controllable; }
bool CharacterController::isCompanion() const { return m_companion; }
bool CharacterController::isMale() const { return m_isMale; }


void CharacterController::onAddToEntity(Entity* entity)
{
	using uPass = Component::UpdatePass;
	Component::onAddToEntity(entity);

	m_animator = entity->getComponent<Animator>();
	m_rigidbody = entity->getComponent<RigidBody>();
	m_collider = entity->getComponent<Collider>();

	ComponentUpdater::getInstance()->add(uPass::eBeginFrame, [this](uPass updatePass, float _dt) {update(updatePass, _dt);}, this, entity);
	m_rigidbody->suscribeFixedUpdate([this](uPass updatePass, float _dt) {update(updatePass, _dt); }, this);

	//ComponentUpdater::getInstance()->add(uPass::ePhysics, [this](uPass updatePass, float _dt) {update(updatePass, _dt);}, this);


}

void CharacterController::onDrawImGui()
{
#ifdef USE_IMGUI
	static vec4f g_cacheColor = Debug::blue;

	const ImVec4 componentColor = ImVec4(0.7f, 0.f, 0.f, 1.f);
	std::ostringstream unicName;
	unicName << "Player movement##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox("Is Controllable", &m_controllable);
		ImGui::Checkbox("Is Companion", &m_companion);
		ImGui::DragFloat("Walk speed", &m_walkSpeed);
		ImGui::DragFloat("Run speed", &m_runSpeed);
		ImGui::DragFloat("Sprint speed", &m_sprintSpeed);
		ImGui::DragFloat("Jump force", &m_jumpForce);

		ImGui::Checkbox("Draw collision cache", &m_drawCollisionCache);
		ImGui::ColorEdit4("Cache color", &g_cacheColor.x);
		ImGui::Checkbox("m_drawGroundTestShapes", &m_drawGroundTestShapes);

		ImGui::TreePop();
	}

	if (m_drawCollisionCache)
	{
		if (g_cacheColor.w > 0.001f)
			m_collisionCache.debugDraw(false, g_cacheColor);
		if (g_cacheColor.w < 0.9f)
			m_collisionCache.debugDraw(true, g_cacheColor);
	}
	if (m_drawGroundTestShapes)
	{
		Debug::color = m_isGrounded ? Debug::green : Debug::red;
		Debug::drawLineSphere(getParentEntity()->getWorldPosition(), m_extraGroundSphereRadius);
	}
#endif
}

