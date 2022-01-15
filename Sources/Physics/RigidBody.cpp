#include "RigidBody.h"
#include <EntityComponent/Entity.hpp>

#include <iostream>

//	Default
RigidBody::RigidBody(const RigidBodyType& type, const SolverType& solver) : 
	type(type), solver(solver), 
	gravityFactor(1.f), bouncyness(0.5f), friction(0.1f), drag(0.f),
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

	const Shape* shape = getParentEntity()->getLocalBoundingShape();
	if (shape)
	{
		inertia = mass * shape->computeInertiaMatrix();
		inverseInertia = glm::inverse(inertia);
	}
	else
	{
		inertia = glm::mat3(mass);
		inverseInertia = glm::mat3(inverseMass);
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

void RigidBody::setExternalForces(const glm::vec3& f) { externalForces = f; }
void RigidBody::setExternalTorques(const glm::vec3& t) { externalTorques = t; }
void RigidBody::setLinearAcceleration(const glm::vec3& a) { linearAcceleration = a; }
void RigidBody::setAngularAcceleration(const glm::vec3& a) { angularAcceleration = a; }
void RigidBody::setLinearVelocity(const glm::vec3& v) { linearVelocity = v; }
void RigidBody::setAngularVelocity(const glm::vec3& v) { angularVelocity = v; }
void RigidBody::setPosition(const glm::vec3& p)
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

glm::vec3 RigidBody::getExternalForces() const { return externalForces; }
glm::vec3 RigidBody::getExternalTorques() const { return externalTorques; }
glm::vec3 RigidBody::getLinearAcceleration() const { return linearAcceleration; }
glm::vec3 RigidBody::getAngularAcceleration() const { return angularAcceleration; }
glm::vec3 RigidBody::getLinearVelocity() const { return linearVelocity; }
glm::vec3 RigidBody::getAngularVelocity() const { return angularVelocity; }
glm::vec3 RigidBody::getPosition() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getPosition();
	else
		return glm::vec3(0.f);
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
glm::vec3 RigidBody::computeLocalPoint(glm::vec3 worldPoint) const
{
	return glm::vec3(getParentEntity()->getInverseTransformMatrix() * glm::vec4(worldPoint, 1.f));
}
glm::vec3 RigidBody::computeLocalDirection(glm::vec3 worldDirection) const
{
	return glm::vec3(getParentEntity()->getInverseTransformMatrix() * glm::vec4(worldDirection, 0.f));
}
glm::vec3 RigidBody::computeWorldDirection(glm::vec3 localDirection) const
{
	return glm::vec3(getParentEntity()->getTransformMatrix() * glm::vec4(localDirection, 0.f));
}
//
