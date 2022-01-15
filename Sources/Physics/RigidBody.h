#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "EntityComponent/Component.hpp"

class RigidBody : public Component
{
	GF_DECLARE_COMPONENT_CLASS(RigidBody, Component)

	friend class Physics;
	friend class Swept;
	friend class Constraint;

	public:
		//	Miscelleneous
		enum RigidBodyType
		{
			DYNAMIC,
			STATIC,
			KINEMATICS
		};
		enum SolverType
		{
			DISCRETE,
			CONTINUOUS,
			SUPERSAMPLING
		};
		//

		//	Default
		RigidBody(const RigidBodyType& type = DYNAMIC, const SolverType& solver = DISCRETE);
		virtual ~RigidBody() override;

		void initialize(const float& _mass);
		//

		//	Set / get / test
		void setType(const RigidBodyType& t);
		void setMass(const float& m);
		void setGravityFactor(const float& f);

		void setExternalForces(const glm::vec3& f);
		void setExternalTorques(const glm::vec3& t);
		void setLinearAcceleration(const glm::vec3& a);
		void setAngularAcceleration(const glm::vec3& a);
		void setLinearVelocity(const glm::vec3& v);
		void setAngularVelocity(const glm::vec3& v);
		void setPosition(const glm::vec3& p);
		void setOrientation(const glm::fquat& q);

		void setBouncyness(const float& b);
		void setFriction(const float& f);




		RigidBodyType getType() const;
		float getMass() const;
		float getInverseMass() const;
		const glm::mat3& getInertia() const;
		const glm::mat3& getInverseInertia() const;
		float getGravityFactor() const;
		float getFriction() const;

		glm::vec3 getExternalForces() const;
		glm::vec3 getExternalTorques() const;
		glm::vec3 getLinearAcceleration() const;
		glm::vec3 getAngularAcceleration() const;
		glm::vec3 getLinearVelocity() const;
		glm::vec3 getAngularVelocity() const;
		glm::vec3 getPosition() const;
		glm::fquat getOrientation() const;
		//

		//
		bool isResting() const;
		glm::vec3 computeLocalPoint(glm::vec3 worldPoint) const;
		glm::vec3 computeLocalDirection(glm::vec3 worldDirection) const;
		glm::vec3 computeWorldDirection(glm::vec3 localDirection) const;
		//

	protected:
		//	Attributes
		RigidBodyType type;
		SolverType solver;

		float inverseMass;
		float gravityFactor;
		float bouncyness;
		float friction;
		float drag;

		float mass;
		glm::mat3 inertia;
		glm::mat3 inverseInertia;

		glm::vec3 externalForces;
		glm::vec3 externalTorques;
		glm::vec3 linearAcceleration;
		glm::vec3 angularAcceleration;
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocity;
		//

	private:
		//	Internal (used by Physics engine)
		glm::vec3 predictedPosition;
		glm::fquat predictedOrientation;
		glm::vec3 beforeStepPosition;
		glm::fquat beforeStepOrientation;
};
