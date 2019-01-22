#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "EntityComponent/Component.hpp"

class RigidBody : public Component
{
	GF_DECLARE_COMPONENT_CLASS(RigidBody, Component)

	friend class Physics;

	public:
		//	Miscelleneous
		enum RigidBodyType
		{
			DYNAMIC,
			STATIC,
			KINEMATICS
		};
		//

		//	Default
		RigidBody(const RigidBodyType& type = DYNAMIC);
		virtual ~RigidBody() override;
		//

		//	Set / get / test
		void setType(const RigidBodyType& t);
		void setMass(const float& m);
		void setGravityFactor(const float& f);
		void setAcceleration(const glm::vec3& a);
		void setVelocity(const glm::vec3& v);

		RigidBodyType getType() const;
		float getMass() const;
		float getGravityFactor() const;
		glm::vec3 getAcceleration() const;
		glm::vec3 getVelocity() const;

		void addForce(const glm::vec3& force);
		void addForce(const glm::vec3& force, const glm::vec3& contactPoint);
		void addTorque(const glm::vec3& torque);

		bool isResting() const;
		//

	protected:
		//	Attributes
		RigidBodyType type;

		float mass;
		float gravityFactor;
		glm::mat3 inertia;

		glm::vec3 acceleration;
		glm::vec3 velocity;

		glm::vec3 angularAcceleration;
		glm::vec3 angularVelocity;

		std::vector<glm::vec3> forces;
		std::vector<glm::vec3> torques;
		//

	private:
		//	Internal (used by Physics engine)
		glm::vec3 savedPosition;
		glm::fquat savedRotation;
		//
};
