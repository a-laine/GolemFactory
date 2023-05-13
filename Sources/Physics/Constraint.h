#pragma once
#include <EntityComponent/Entity.hpp>
#include <Physics/RigidBody.h>
#include <Physics/CollisionReport.h>


class Constraint
{
	public:
		//
		Constraint();
		//

		//
		vec4f computeClosingVelocity() const;

		void createFromReport(CollisionReport& report, const int& pointIndex, const float& deltaTime);
		//


		// Attributes
		RigidBody *body1, *body2;
		Entity* entity1, *entity2;
		vec4f worldPoint;
		float depth;
		vec4f localPoint1, localPoint2;
		
		int axisCount;
		vec4f targetLinearVelocity;
		//glm::vec3 targetAngularVelocity;
		vec4f accumulationLinear;

		bool frictionLimit;
		float friction;
		vec4f accumulationLinearMin;
		vec4f accumulationLinearMax;
		vec4f velocityChangePerAxis;

		vec4f axis[3];
		vec4f rotationPerUnitImpulse1[3];
		vec4f rotationPerUnitImpulse2[3];
		//
};

