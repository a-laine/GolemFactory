#include "Constraint.h"


Constraint::Constraint() :
	body1(nullptr), body2(nullptr), entity1(nullptr), entity2(nullptr),
	worldPoint(0.f), depth(0.f),
	targetLinearVelocity(0.f),
	frictionLimit(false), friction(0.f),
	accumulationLinear(0.f), axisCount(3)
{
	axis[0] = vec4f(1, 0, 0, 0);
	axis[1] = vec4f(0, 1, 0, 0);
	axis[2] = vec4f(0, 0, 1, 0);

	accumulationLinearMin = vec4f(std::numeric_limits<float>::min());
	accumulationLinearMax = vec4f(std::numeric_limits<float>::max());
}

vec4f Constraint::computeClosingVelocity() const
{
	vec4f v = body1->getLinearVelocity();
	v += vec4f::cross(body1->getAngularVelocity(), localPoint1);

	if (body2)
	{
		v -= body2->getLinearVelocity();
		v -= vec4f::cross(body2->getAngularVelocity(), localPoint2);
	}

	v.w = 0.f;
	return v;
}

void Constraint::createFromReport(CollisionReport& report, const int& pointIndex, const float& deltaTime)
{
	entity1 = report.entity1;
	entity2 = report.entity2;
	body1 = report.body1;
	body2 = report.body2;
	worldPoint = report.points[pointIndex];
	depth = std::min(report.depths[pointIndex], 0.02f);

	axisCount = 3;

	axis[0] = -report.normal.getNormal();
	axis[1] = (std::abs(axis[0].x) > std::abs(axis[0].z) ? vec4f(-axis[0].y, axis[0].x, 0, 0) : vec4f(0, -axis[0].z, axis[0].y, 0)).getNormal();
	axis[2] = vec4f::cross(axis[0], axis[1]);

	localPoint1 = worldPoint - body1->getPosition();
	if (body2)
		localPoint2 = worldPoint - body2->getPosition();

	float bouncyness = body2 ? 0.5f * (body1->bouncyness + body2->bouncyness) : body1->bouncyness;
	float closingVelocity = vec4f::dot(computeClosingVelocity(), axis[0]);
	targetLinearVelocity = vec4f(0.75f * depth / deltaTime - bouncyness * closingVelocity, 0.f, 0.f, 0.f);

	frictionLimit = true;
	friction = body2 ? std::min(body1->friction, body2->friction) : body1->friction;
	accumulationLinearMin = vec4f(0.f, -100000.f, -100000.f, 0.f);
	accumulationLinearMax = vec4f(100000.f, 100000.f, 100000.f, 0.f);

	for (int i = 0; i < 3; i++)
	{
		velocityChangePerAxis[i] = body1->getInverseMass();
		mat4f M = mat4f(body1->getParentEntity()->getWorldOrientation());
		mat4f iM = mat4f::transpose(M);

		vec4f torquePerUnitImpulse = vec4f::cross(localPoint1, axis[i]);
		rotationPerUnitImpulse1[i] = M * body1->getInverseInertia() * iM * torquePerUnitImpulse;
		rotationPerUnitImpulse1[i].w = 0;
		vec4f velocityPerUnitImpulse = vec4f::cross(rotationPerUnitImpulse1[i], localPoint1);
		velocityChangePerAxis[i] += vec4f::dot(velocityPerUnitImpulse, axis[i]);

		if (body2)
		{
			velocityChangePerAxis[i] += body2->getInverseMass();
			M = mat4f(body2->getParentEntity()->getWorldOrientation());
			iM = mat4f::transpose(M);

			torquePerUnitImpulse = vec4f::cross(localPoint2, axis[i]);
			rotationPerUnitImpulse2[i] = M * body2->getInverseInertia() * iM * torquePerUnitImpulse;
			rotationPerUnitImpulse2[i].w = 0;
			velocityPerUnitImpulse = vec4f::cross(rotationPerUnitImpulse2[i], localPoint2);
			velocityChangePerAxis[i] += vec4f::dot(velocityPerUnitImpulse, axis[i]);
		}
	}
}
