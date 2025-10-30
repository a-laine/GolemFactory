#include "Constraint.h"


vec4f Constraint::computeClosingVelocity() const
{
	vec4f v = body1->m_linearVelocity;
	v += vec4f::cross(body1->m_angularVelocity, body1->m_orientation * localPoint1);

	if (body2)
	{
		v -= body2->m_linearVelocity;
		v -= vec4f::cross(body2->m_angularVelocity, body2->m_orientation * localPoint2);
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
	depth = std::min(report.depths[pointIndex], 0.1f);

	axisCount = 3;
	axis[0] = report.normal.getNormal();
	axis[1] = (std::abs(axis[0].x) > std::abs(axis[0].z) ? vec4f(-axis[0].y, axis[0].x, 0, 0) : vec4f(0, -axis[0].z, axis[0].y, 0)).getNormal();
	axis[2] = vec4f::cross(axis[0], axis[1]);

	worldPoint = report.points[pointIndex] + report.depths[pointIndex] * axis[0];

	quatf iquat1 = conjugate(body1->m_orientation);
	quatf iquat2 = body2 ? conjugate(body2->m_orientation) : quatf::identity;

	localPoint1 = iquat1 * (report.points[pointIndex] - body1->m_position);
	if (body2)
		localPoint2 = iquat2 * (worldPoint - body2->m_position);

	float bouncyness = body2 ? 0.5f * (body1->m_bouncyness + body2->m_bouncyness) : body1->m_bouncyness;
	float closingVelocity = std::min(vec4f::dot(computeClosingVelocity(), axis[0]), 0.f);
	targetLinearVelocity = vec4f(30*depth - bouncyness * closingVelocity, 0.f, 0.f, 0.f);

	frictionLimit = true;
	friction = body2 ? std::min(body1->m_friction, body2->m_friction) : body1->m_friction;
	accumulationLinearMin = vec4f(0.f, -100000.f, -100000.f, 0.f);
	accumulationLinearMax = vec4f(100000.f, 100000.f, 100000.f, 0.f);

	for (int i = 0; i < 3; i++)
	{
		velocityChangePerAxis[i] = body1->m_inverseMass;
		mat4f M = mat4f(body1->m_orientation);
		mat4f iM = mat4f::transpose(M);

		if (body1->getType() == RigidBody::RigidBodyType::DYNAMIC)
		{
			vec4f r1 = body1->m_orientation * localPoint1;
			vec4f torquePerUnitImpulse = vec4f::cross(r1, axis[i]);
			//rotationPerUnitImpulse1[i] = body1->m_orientation * (body1->m_inverseInertia * (iquat1 * torquePerUnitImpulse));
			rotationPerUnitImpulse1[i] = M * body1->getInverseInertia() * iM * torquePerUnitImpulse;
			rotationPerUnitImpulse1[i].w = 0;
			vec4f velocityPerUnitImpulse = vec4f::cross(rotationPerUnitImpulse1[i], r1);
			velocityChangePerAxis[i] += vec4f::dot(velocityPerUnitImpulse, axis[i]);
		}
		else
		{
			rotationPerUnitImpulse1[i] = vec4f::zero;
		}

		if (body2)
		{
			velocityChangePerAxis[i] += body2->m_inverseMass;
			M = mat4f(body2->m_orientation);
			iM = mat4f::transpose(M);

			if (body2->getType() == RigidBody::RigidBodyType::DYNAMIC)
			{
				vec4f r2 = body2->m_orientation * localPoint2;
				vec4f torquePerUnitImpulse = vec4f::cross(r2, axis[i]);
				//rotationPerUnitImpulse2[i] = body2->m_orientation * (body2->m_inverseInertia * (iquat2 * torquePerUnitImpulse));
				rotationPerUnitImpulse2[i] = M * body2->getInverseInertia() * iM * torquePerUnitImpulse;
				rotationPerUnitImpulse2[i].w = 0;
				vec4f velocityPerUnitImpulse = vec4f::cross(rotationPerUnitImpulse2[i], r2);
				velocityChangePerAxis[i] += vec4f::dot(velocityPerUnitImpulse, axis[i]);
			}
			else
			{
				rotationPerUnitImpulse2[i] = vec4f::zero;
			}
		}
	}
}
