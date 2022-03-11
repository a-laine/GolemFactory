#include "RigidBody.h"
#include "Shapes/Collider.h"
#include <EntityComponent/Entity.hpp>

#include <iostream>

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
		inertia = glm::mat3(0.f);
		for (int i = 0; i < colliders.size(); i++)
		{
			Collider* collider = static_cast<Collider*>(colliders[i]);
			inertia += collider->m_shape->computeInertiaMatrix();
		}
		inertia *= mass;
		inverseInertia = glm::inverse(inertia);
	}
	else
	{
		inertia = glm::mat3(mass);
		inverseInertia = glm::mat3(inverseMass);
	}
}
void RigidBody::computeWorldShapes()
{
	for (Shape* shape : worldShapes)
		delete shape;
	worldShapes.clear();

	glm::vec4 position = getPosition();
	glm::vec3 scale = getParentEntity()->getScale();
	glm::quat orientation = getOrientation();

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

void RigidBody::setExternalForces(const glm::vec4& f) { externalForces = f; }
void RigidBody::setExternalTorques(const glm::vec4& t) { externalTorques = t; }
void RigidBody::setLinearAcceleration(const glm::vec4& a) { linearAcceleration = a; }
void RigidBody::setAngularAcceleration(const glm::vec4& a) { angularAcceleration = a; }
void RigidBody::setLinearVelocity(const glm::vec4& v) { linearVelocity = v; }
void RigidBody::setAngularVelocity(const glm::vec4& v) { angularVelocity = v; }
void RigidBody::setPosition(const glm::vec4& p)
{
	Entity* parent = getParentEntity();
	if (parent)
		parent->setPosition(p);
}
void RigidBody::setOrientation(const glm::fquat& q)
{
	Entity* parent = getParentEntity();
	if (parent)
		parent->setOrientation(q);
}


RigidBody::RigidBodyType RigidBody::getType() const { return type; }
float RigidBody::getMass() const { return mass; }
float RigidBody::getInverseMass() const { return inverseMass; }
const glm::mat3& RigidBody::getInertia() const { return inertia; }
const glm::mat3& RigidBody::getInverseInertia() const { return inverseInertia; }
float RigidBody::getGravityFactor() const { return gravityFactor; }
float RigidBody::getFriction() const { return friction; }
float RigidBody::getDamping() const { return damping; }

glm::vec4 RigidBody::getExternalForces() const { return externalForces; }
glm::vec4 RigidBody::getExternalTorques() const { return externalTorques; }
glm::vec4 RigidBody::getLinearAcceleration() const { return linearAcceleration; }
glm::vec4 RigidBody::getAngularAcceleration() const { return angularAcceleration; }
glm::vec4 RigidBody::getLinearVelocity() const { return linearVelocity; }
glm::vec4 RigidBody::getAngularVelocity() const { return angularVelocity; }
glm::vec4 RigidBody::getPosition() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getPosition();
	else
		return glm::vec4(0.f);
}
glm::fquat RigidBody::getOrientation() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getOrientation();
	else
		return glm::fquat(0, 0, 0, 1);
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
glm::vec4 RigidBody::computeLocalPoint(glm::vec4 worldPoint) const
{
	return getParentEntity()->getInverseTransformMatrix() * worldPoint;
}
glm::vec4 RigidBody::computeLocalDirection(glm::vec4 worldDirection) const
{
	return getParentEntity()->getInverseTransformMatrix() * worldDirection;
}
glm::vec4 RigidBody::computeWorldDirection(glm::vec4 localDirection) const
{
	return getParentEntity()->getTransformMatrix() * localDirection;
}
//
