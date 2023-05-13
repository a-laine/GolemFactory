#include "RigidBody.h"
#include "Shapes/Collider.h"
#include <EntityComponent/Entity.hpp>

#include <iostream>
#include <sstream>
#include <Utiles/Debug.h>

//	Default
RigidBody::RigidBody(const RigidBodyType& type, const SolverType& solver) : 
	type(type), solver(solver), 
	gravityFactor(1.f), bouncyness(0.5f), friction(0.1f), damping(0.f),
	mass(1.f), inverseMass(1.f), inertia(1.f), inverseInertia(1.f),
	externalForces(0.f), externalTorques(0.f), linearVelocity(0.f), linearAcceleration(0.f), angularVelocity(0.f), angularAcceleration(0.f)
{

}
RigidBody::~RigidBody()
{}

void RigidBody::initialize(const float& _mass)
{
	mass = _mass;
	inverseMass = 1.f / _mass;

	colliders.clear();
	getParentEntity()->getAllComponents<Collider>(colliders);

	if (!colliders.empty())
	{
		inertia = mat4f(0.f);
		for (int i = 0; i < colliders.size(); i++)
		{
			Collider* collider = static_cast<Collider*>(colliders[i]);
			inertia += collider->m_shape->computeInertiaMatrix();
		}
		inertia *= mass;
		inverseInertia = mat4f::inverse(inertia);
	}
	else
	{
		inertia = mat4f(mass);
		inverseInertia = mat4f(inverseMass);
	}
}
void RigidBody::computeWorldShapes()
{
	for (Shape* shape : worldShapes)
		delete shape;
	worldShapes.clear();

	vec4f position = getPosition();
	vec4f scale(getParentEntity()->getWorldScale());
	quatf orientation = getOrientation();

	for (Component* colliderComponent : colliders)
	{
		Collider* collider = static_cast<Collider*>(colliderComponent);
		worldShapes.push_back(collider->m_shape->duplicate());
		worldShapes.back()->transform(position, scale, orientation);
	}
}
//

//	Set / get / test
void RigidBody::setType(const RigidBodyType& t) { type = t; }
void RigidBody::setMass(const float& m)
{
	inertia *= m / mass;
	inverseInertia *= mass / m;
	mass = m;
	inverseMass = 1.f / mass;
}
void RigidBody::setGravityFactor(const float& f) { gravityFactor = f; }
void RigidBody::setBouncyness(const float& b) { bouncyness = b; }
void RigidBody::setFriction(const float& f) { friction = f; }
void RigidBody::setDamping(const float& f) { damping = f; }

void RigidBody::setExternalForces(const vec4f& f) { externalForces = f; }
void RigidBody::setExternalTorques(const vec4f& t) { externalTorques = t; }
void RigidBody::setLinearAcceleration(const vec4f& a) { linearAcceleration = a; }
void RigidBody::setAngularAcceleration(const vec4f& a) { angularAcceleration = a; }
void RigidBody::setLinearVelocity(const vec4f& v) { linearVelocity = v; }
void RigidBody::setAngularVelocity(const vec4f& v) { angularVelocity = v; }
void RigidBody::setPosition(const vec4f& p)
{
	Entity* parent = getParentEntity();
	if (parent)
		parent->setWorldPosition(p);
}
void RigidBody::setOrientation(const quatf& q)
{
	Entity* parent = getParentEntity();
	if (parent)
		parent->setWorldOrientation(q);
}


RigidBody::RigidBodyType RigidBody::getType() const { return type; }
float RigidBody::getMass() const { return mass; }
float RigidBody::getInverseMass() const { return inverseMass; }
const mat4f& RigidBody::getInertia() const { return inertia; }
const mat4f& RigidBody::getInverseInertia() const { return inverseInertia; }
float RigidBody::getGravityFactor() const { return gravityFactor; }
float RigidBody::getFriction() const { return friction; }
float RigidBody::getDamping() const { return damping; }

vec4f RigidBody::getExternalForces() const { return externalForces; }
vec4f RigidBody::getExternalTorques() const { return externalTorques; }
vec4f RigidBody::getLinearAcceleration() const { return linearAcceleration; }
vec4f RigidBody::getAngularAcceleration() const { return angularAcceleration; }
vec4f RigidBody::getLinearVelocity() const { return linearVelocity; }
vec4f RigidBody::getAngularVelocity() const { return angularVelocity; }
vec4f RigidBody::getPosition() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getWorldPosition();
	else
		return vec4f(0.f);
}
quatf RigidBody::getOrientation() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getWorldOrientation();
	else
		return quatf::identity;
}



/*glm::vec3 RigidBody::getDeltaPosition() const { return deltaPosition; }
glm::fquat RigidBody::getDeltaRotation() const { return deltaRotation; }*/


/*void RigidBody::addForce(const glm::vec3& force) { forces.push_back(force); }
void RigidBody::addForce(const glm::vec3& force, const glm::vec3& contactPoint)
{
	forces.push_back(force);
	torques.push_back(glm::cross(contactPoint, force));
}
void RigidBody::addTorque(const glm::vec3& torque) { torques.push_back(torque); }*/


bool RigidBody::isResting() const
{
	/*return velocity == glm::vec3(0.f) &&
		angularVelocity == glm::vec3(0.f) &&
		forces.empty() && torques.empty() &&
		acceleration == glm::vec3(0.f) &&
		angularAcceleration == glm::vec3(0.f);*/
	return false;
}
vec4f RigidBody::computeLocalPoint(vec4f worldPoint)
{
	return getParentEntity()->getInverseWorldTransformMatrix() * worldPoint;
}
vec4f RigidBody::computeLocalDirection(vec4f worldDirection)
{
	return getParentEntity()->getInverseWorldTransformMatrix() * worldDirection;
}
vec4f RigidBody::computeWorldDirection(vec4f localDirection)
{
	return getParentEntity()->getWorldTransformMatrix() * localDirection;
}
//


void RigidBody::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Physics);
}
void RigidBody::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0, 1, 0, 1);
	std::ostringstream unicName;
	unicName << "RigidBody component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw colliders", &m_drawColliders);
		ImGui::Unindent();

		ImGui::TreePop();
	}

	if (m_drawColliders)
	{
		drawColliders(vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w));
	}
#endif // USE_IMGUI
}
void RigidBody::drawColliders(vec4f color) const
{
	for (const Component* comp : colliders)
	{
		const Collider* collider = static_cast<const Collider*>(comp);
		collider->drawDebug(color);
	}
}