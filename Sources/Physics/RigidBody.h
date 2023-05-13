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

		void setExternalForces(const vec4f& f);
		void setExternalTorques(const vec4f& t);
		void setLinearAcceleration(const vec4f& a);
		void setAngularAcceleration(const vec4f& a);
		void setLinearVelocity(const vec4f& v);
		void setAngularVelocity(const vec4f& v);
		void setPosition(const vec4f& p);
		void setOrientation(const quatf& q);

		void setBouncyness(const float& b);
		void setFriction(const float& f);
		void setDamping(const float& f);



		RigidBodyType getType() const;
		float getMass() const;
		float getInverseMass() const;
		const mat4f& getInertia() const;
		const mat4f& getInverseInertia() const;
		float getGravityFactor() const;
		float getFriction() const;
		float getDamping() const;

		vec4f getExternalForces() const;
		vec4f getExternalTorques() const;
		vec4f getLinearAcceleration() const;
		vec4f getAngularAcceleration() const;
		vec4f getLinearVelocity() const;
		vec4f getAngularVelocity() const;
		vec4f getPosition() const;
		quatf getOrientation() const;
		//

		//
		bool isResting() const;
		vec4f computeLocalPoint(vec4f worldPoint);
		vec4f computeLocalDirection(vec4f worldDirection);
		vec4f computeWorldDirection(vec4f localDirection);
		//

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;
		void drawColliders(vec4f color) const;

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
		mat4f inertia;
		mat4f inverseInertia;

		vec4f externalForces;
		vec4f externalTorques;
		vec4f linearAcceleration;
		vec4f angularAcceleration;
		vec4f linearVelocity;
		vec4f angularVelocity;
		//

	private:
		//	Internal (used by Physics engine)
		AxisAlignedBox sweptBox;
		vec4f previousPosition;
		quatf previousOrientation;

		std::vector<Component*> colliders;
		std::vector<Shape*> worldShapes;

#ifdef USE_IMGUI
		bool m_drawColliders = false;
#endif
};
