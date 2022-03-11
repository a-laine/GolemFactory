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
		glm::vec4 computeClosingVelocity() const;

		void createFromReport(CollisionReport& report, const int& pointIndex, const float& deltaTime);
		//


		// Attributes
		RigidBody *body1, *body2;
		Entity* entity1, *entity2;
		glm::vec4 worldPoint;
		float depth;
		glm::vec4 localPoint1, localPoint2;
		
		int axisCount;
		glm::vec4 targetLinearVelocity;
		//glm::vec3 targetAngularVelocity;
		glm::vec4 accumulationLinear;

		bool frictionLimit;
		float friction;
		glm::vec4 accumulationLinearMin;
		glm::vec4 accumulationLinearMax;
		glm::vec4 velocityChangePerAxis;

		glm::vec4 axis[3];
		glm::vec4 rotationPerUnitImpulse1[3];
		glm::vec4 rotationPerUnitImpulse2[3];
		//
};

