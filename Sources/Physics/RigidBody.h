#pragma once

//#include <glm/glm.hpp>
//#include <glm/gtx/quaternion.hpp>

#include <vector>

#include "EntityComponent/Component.hpp"
#include "Physics/Shapes/AxisAlignedBox.h"

//class Cluster;
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

		bool load(Variant& jsonObject, const std::string& objectName) override;

		void initialize(bool explicitMass, float _mass);
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
		RigidBodyType m_type;
		SolverType m_solver;

		float m_mass, m_inverseMass, m_volumicMass;
		float m_gravityFactor;
		float m_bouncyness;
		float m_friction;
		float m_damping;

		mat4f m_inertia;
		mat4f m_inverseInertia;

		vec4f m_externalForces;
		vec4f m_externalTorques;
		vec4f m_linearAcceleration;
		vec4f m_angularAcceleration;
		vec4f m_linearVelocity;
		vec4f m_angularVelocity;
		//

	private:
		//	Internal (used by Physics engine)
		AxisAlignedBox m_sweptBox;
		vec4f m_previousPosition;
		quatf m_previousOrientation;

		std::vector<Component*> m_colliders;
		std::vector<Shape*> m_worldShapes;

		int m_clusterIndex = -1;

#ifdef USE_IMGUI
		bool m_drawColliders = false;
		int m_drawClusterCollisionCache = 0;
		vec4f m_cacheColor = vec4f(1, 0.5f, 0, 0);
#endif
};
