#pragma once
#include <EntityComponent/Entity.hpp>
#include <Physics/RigidBody.h>
#include <Physics/CollisionReport.h>


class Constraint
{
	public:
		//
		vec4f computeClosingVelocity() const;

		void createFromReport(CollisionReport& report, const int& pointIndex, const float& deltaTime);
		//


		// Attributes
		RigidBody* body1 = nullptr;
		RigidBody* body2 = nullptr;
		Entity* entity1 = nullptr;
		Entity* entity2 = nullptr;
		vec4f worldPoint = vec4f::zero;
		vec4f localPoint1 = vec4f::zero;
		vec4f localPoint2 = vec4f::zero;
		
		vec4f targetLinearVelocity = vec4f::zero;
		vec4f accumulationLinear = vec4f::zero;

		float depth = 0.f;
		int axisCount = 3;
		float friction = 0.f;
		bool frictionLimit = true;

		vec4f accumulationLinearMin = vec4f(std::numeric_limits<float>::min());
		vec4f accumulationLinearMax = vec4f(std::numeric_limits<float>::max());
		vec4f velocityChangePerAxis = vec4f::one;

		vec4f axis[3] = { vec4f(1, 0, 0, 0), vec4f(0, 1, 0, 0), vec4f(0, 0, 1, 0) };
		vec4f rotationPerUnitImpulse1[3];
		vec4f rotationPerUnitImpulse2[3];
		//
};

