#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "EntityComponent/Component.hpp"
#include "Physics/Shapes/AxisAlignedBox.h"

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
		void computeWorldShapes();
		//

		//	Set / get / test
		void setType(const RigidBodyType& t);
		void setMass(const float& m);
		void setGravityFactor(const float& f);

		void setExternalForces(const glm::vec4& f);
		void setExternalTorques(const glm::vec4& t);
		void setLinearAcceleration(const glm::vec4& a);
		void setAngularAcceleration(const glm::vec4& a);
		void setLinearVelocity(const glm::vec4& v);
		void setAngularVelocity(const glm::vec4& v);
		void setPosition(const glm::vec4& p);
		void setOrientation(const glm::fquat& q);

		void setBouncyness(const float& b);
		void setFriction(const float& f);
		void setDamping(const float& f);



		RigidBodyType getType() const;
		float getMass() const;
		float getInverseMass() const;
		const glm::mat3& getInertia() const;
		const glm::mat3& getInverseInertia() const;
		float getGravityFactor() const;
		float getFriction() const;
		float getDamping() const;

		glm::vec4 getExternalForces() const;
		glm::vec4 getExternalTorques() const;
		glm::vec4 getLinearAcceleration() const;
		glm::vec4 getAngularAcceleration() const;
		glm::vec4 getLinearVelocity() const;
		glm::vec4 getAngularVelocity() const;
		glm::vec4 getPosition() const;
		glm::fquat getOrientation() const;
		//

		//
		bool isResting() const;
		glm::vec4 computeLocalPoint(glm::vec4 worldPoint) const;
		glm::vec4 computeLocalDirection(glm::vec4 worldDirection) const;
		glm::vec4 computeWorldDirection(glm::vec4 localDirection) const;
		//

	protected:
		//	Attributes
		RigidBodyType type;
		SolverType solver;

		float inverseMass;
		float gravityFactor;
		float bouncyness;
		float friction;
		float damping;

		float mass;
		glm::mat3 inertia;
		glm::mat3 inverseInertia;

		glm::vec4 externalForces;
		glm::vec4 externalTorques;
		glm::vec4 linearAcceleration;
		glm::vec4 angularAcceleration;
		glm::vec4 linearVelocity;
		glm::vec4 angularVelocity;
		//

	private:
		//	Internal (used by Physics engine)
		AxisAlignedBox sweptBox;
		glm::vec4 previousPosition;
		glm::fquat previousOrientation;

		std::vector<Component*> colliders;
		std::vector<Shape*> worldShapes;
};
