#include "Constraint.h"

Constraint::Constraint() :
	body1(nullptr), body2(nullptr), entity1(nullptr), entity2(nullptr),
	worldPoint(0.f), depth(0.f),
	targetLinearVelocity(0.f),
	frictionLimit(false), friction(0.f),
	accumulationLinear(0.f), axisCount(3)
{
	axis[0] = glm::vec3(1, 0, 0);
	axis[1] = glm::vec3(0, 1, 0);
	axis[2] = glm::vec3(0, 0, 1);

	accumulationLinearMin = glm::vec3(std::numeric_limits<float>::min());
	accumulationLinearMax = glm::vec3(std::numeric_limits<float>::max());
}

glm::vec3 Constraint::computeClosingVelocity() const
{
	glm::vec3 v = body1->getLinearVelocity();
	v += glm::cross(body1->getAngularVelocity(), localPoint1);

	if (body2)
	{
		v -= body2->getLinearVelocity();
		v -= glm::cross(body2->getAngularVelocity(), localPoint2);
	}
	return v;
}

void Constraint::createFromReport(CollisionReport& report, const int& pointIndex, const float& deltaTime)
{
	entity1 = report.entity1;
	entity2 = report.entity2;
	body1 = report.body1;
	body2 = report.body2;
	worldPoint = report.points[pointIndex];
	depth = report.depths[pointIndex];

	axisCount = 3;

	axis[0] = -glm::normalize(report.normal);
	axis[1] = glm::normalize(std::abs(axis[0].x) > std::abs(axis[0].z) ? glm::vec3(-axis[0].y, axis[0].x, 0.0) : glm::vec3(0.0, -axis[0].z, axis[0].y));
	axis[2] = glm::cross(axis[0], axis[1]);
	localPoint1 = worldPoint - body1->getPosition();
	targetLinearVelocity = glm::vec3(depth * 20.f, 0.f, 0.f);; 

	frictionLimit = false;
	friction = body2 ? std::min(body1->friction, body2->friction) : body1->friction;
	accumulationLinearMin = glm::vec3(0.f, 0.f, 0.f);
	accumulationLinearMax = glm::vec3(100000.f, 0.f, 0.f);

	for (int i = 0; i < 3; i++)
	{
		velocityChangePerAxis[i] = body1->getInverseMass();
		glm::mat3 M = glm::mat3(body1->getParentEntity()->getOrientation());
		glm::mat3 iM = glm::transpose(M);

		glm::vec3 torquePerUnitImpulse = glm::cross(localPoint1, axis[i]);
		rotationPerUnitImpulse1[i] = M * body1->getInverseInertia() * iM * torquePerUnitImpulse;
		glm::vec3 velocityPerUnitImpulse = glm::cross(rotationPerUnitImpulse1[i], localPoint1);
		velocityChangePerAxis[i] += glm::dot(velocityPerUnitImpulse, axis[i]);

		if (body2)
		{
			localPoint2 = worldPoint - body2->getPosition();

			velocityChangePerAxis[i] += body2->getInverseMass();
			M = glm::mat3(body2->getParentEntity()->getOrientation());
			iM = glm::transpose(M);

			torquePerUnitImpulse = glm::cross(localPoint2, axis[i]);
			rotationPerUnitImpulse2[i] = M * body2->getInverseInertia() * iM * torquePerUnitImpulse;
			velocityPerUnitImpulse = glm::cross(rotationPerUnitImpulse2[i], localPoint2);
			velocityChangePerAxis[i] += glm::dot(velocityPerUnitImpulse, axis[i]);
		}
	}
}
