#include "RigidBody.h"

#include <iostream>

//	Default
RigidBody::RigidBody(const RigidBodyType& type, const SolverType& solver) : type(type), solver(solver), mass(1.f), gravityFactor(1.f), bouncyness(0.1f), friction(0.1f),
	inertia(1.f), velocity(0.f), acceleration(0.f), angularVelocity(0.f), angularAcceleration(0.f)
{
	inverseMass = 1.f / mass;
	inverseInertia = glm::inverse(inertia);
}
RigidBody::~RigidBody()
{}
//

//	Set / get / test
void RigidBody::setType(const RigidBodyType& t) { type = t; }
void RigidBody::setMass(const float& m) { mass = m; inverseMass = 1.f / mass; }
void RigidBody::setGravityFactor(const float& f) { gravityFactor = f; }
void RigidBody::setAcceleration(const glm::vec3& a) { acceleration = a; }
void RigidBody::setVelocity(const glm::vec3& v) { velocity = v; }
void RigidBody::setAngularAcceleration(const glm::vec3& a) { angularAcceleration = a; }
void RigidBody::setAngularVelocity(const glm::vec3& v) { angularVelocity = v; }


RigidBody::RigidBodyType RigidBody::getType() const { return type; }
float RigidBody::getMass() const { return mass; }
float RigidBody::getGravityFactor() const { return gravityFactor; }
glm::vec3 RigidBody::getAcceleration() const { return acceleration; }
glm::vec3 RigidBody::getVelocity() const { return velocity; }
glm::vec3 RigidBody::getDeltaPosition() const { return deltaPosition; }
glm::fquat RigidBody::getDeltaRotation() const { return deltaRotation; }


void RigidBody::addForce(const glm::vec3& force) { forces.push_back(force); }
void RigidBody::addForce(const glm::vec3& force, const glm::vec3& contactPoint)
{
	forces.push_back(force);
	torques.push_back(glm::cross(contactPoint, force));
}
void RigidBody::addTorque(const glm::vec3& torque) { torques.push_back(torque); }


bool RigidBody::isResting() const
{
	return velocity == glm::vec3(0.f) && angularVelocity == glm::vec3(0.f)&& forces.empty() && torques.empty() && acceleration == glm::vec3(0.f) && angularAcceleration == glm::vec3(0.f);
}
//
