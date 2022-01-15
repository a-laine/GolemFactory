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
		glm::vec3 computeClosingVelocity() const;

		void createFromReport(CollisionReport& report, const int& pointIndex, const float& deltaTime);
		//


		// Attributes
		RigidBody *body1, *body2;
		Entity* entity1, *entity2;
		glm::vec3 worldPoint;
		float depth;
		glm::vec3 localPoint1, localPoint2;
		
		int axisCount;
		glm::vec3 targetLinearVelocity;
		//glm::vec3 targetAngularVelocity;
		glm::vec3 accumulationLinear;

		bool frictionLimit;
		float friction;
		glm::vec3 accumulationLinearMin;
		glm::vec3 accumulationLinearMax;
		glm::vec3 velocityChangePerAxis;

		glm::vec3 axis[3];
		glm::vec3 rotationPerUnitImpulse1[3];
		glm::vec3 rotationPerUnitImpulse2[3];
		//
};

