#include "FrustrumSceneQuerry.h"
#include <Physics/Collision.h>

#include <iostream>

#define FRUSTRUM_COEFF	2.f

FrustrumSceneQuerry::FrustrumSceneQuerry(const vec4f& position, const vec4f& direction, const vec4f& verticalDir, const vec4f& leftDir, 
	float verticalAngle, float contextRatio)
{
	Set(position, direction, verticalDir, leftDir, verticalAngle, contextRatio);
}

void FrustrumSceneQuerry::Set(const vec4f& position, const vec4f& direction, const vec4f& verticalDir, const vec4f& leftDir, float verticalAngle, float contextRatio, float farDistance)
{
	float a1 = 0.5f * verticalAngle;
	float ratio = contextRatio;
	float ca1 = cos(a1);
	float sa1 = sin(a1);
	float sa2 = ratio * sa1;

	frustrumPlaneNormals[0] = -direction; // near
	frustrumPlaneNormals[1] = direction; // far
	frustrumPlaneNormals[2] = ca1 * leftDir - sa2 * direction; // left
	frustrumPlaneNormals[3] = -ca1 * leftDir - sa2 * direction; // right
	frustrumPlaneNormals[4] = ca1 * verticalDir - sa1 * direction; // up
	frustrumPlaneNormals[5] = -ca1 * verticalDir - sa1 * direction; // down

	for (int i = 2; i < 6; i++)
		frustrumPlaneNormals[i].normalize();

	for (int i = 0; i < 6; i++)
		frustrumPlaneNormals[i].w = -vec4f::dot(frustrumPlaneNormals[i], position);
	frustrumPlaneNormals[1].w -= farDistance;

	// compute far corners directions
	vec4f tdir = ca1 * direction;
	frustrumCorners[1] = tdir + sa2 * leftDir + sa1 * verticalDir;
	frustrumCorners[2] = tdir - sa2 * leftDir + sa1 * verticalDir;
	frustrumCorners[3] = tdir + sa2 * leftDir - sa1 * verticalDir;
	frustrumCorners[4] = tdir - sa2 * leftDir - sa1 * verticalDir;

	float invDot = 1.f / vec4f::dot(frustrumCorners[1], direction);
	for (int i = 1; i < 4; i++)
		frustrumCorners[i] = position + (farDistance * invDot) * frustrumCorners[i];
	frustrumCorners[0] = position;
}

VirtualSceneQuerry::CollisionType FrustrumSceneQuerry::operator() (const NodeVirtual* node)
{
	// implement proper SAT !!!!

	if (node->getDepth() > maxDepth)
		return VirtualSceneQuerry::CollisionType::NONE;

	vec4f center = node->getCenter();
	vec4f halfSize = node->getInflatedHalfSize();
	vec4f s; vec4b mask;
	vec4f sizeArray[6];

	// box against frustrum planes
	for (int i = 0; i < 6; i++)
	{
		mask = vec4f::lessThan(frustrumPlaneNormals[i], vec4f::zero);
		s = vec4f::maskSelect(mask, -halfSize, halfSize);
		if (vec4f::dot(frustrumPlaneNormals[i], center - s) > 0.f)
			return VirtualSceneQuerry::CollisionType::NONE;
		sizeArray[i] = s;
	}

	// frustrum corners against box planes
	vec4f axis, p1, p2;
	for (int i = 0; i < 3; i++)
	{
		axis = vec4f::zero;
		axis[i] = 1.f;
		s = vec4f::dot(halfSize, axis) * axis;
		p1 = center + s;
		p2 = center - s;

		int failCount = 0;
		for (int j = 0; j < 5; j++)
		{
			if (vec4f::dot(frustrumCorners[j] - p1, axis) > 0.f)
				failCount++;
			else break;
		}
		if (failCount == 5)
			return VirtualSceneQuerry::CollisionType::NONE;

		failCount = 0;
		for (int j = 0; j < 5; j++)
		{
			if (vec4f::dot(frustrumCorners[j] - p1, axis) > 0.f)
				failCount++;
			else break;
		}
		if (failCount == 5)
			return VirtualSceneQuerry::CollisionType::NONE;
	}

	// test if box is full inside frustrum
	int insideTest = 0;
	for (int i = 0; i < 6; i++)
	{
		if (vec4f::dot(frustrumPlaneNormals[i], center + sizeArray[i]) < 0.f)
			insideTest++;
		else break;
	}

	result.push_back(node);
	return insideTest == 6 ? VirtualSceneQuerry::CollisionType::INSIDE : VirtualSceneQuerry::CollisionType::OVERLAP;
}
std::vector<const NodeVirtual*>& FrustrumSceneQuerry::getResult() { return result; }




bool FrustrumSceneQuerry::TestSphere(vec4f center, float radius)
{
	// sphere against frustrum planes
	for (int i = 0; i < 6; i++)
	{
		if (vec4f::dot(frustrumPlaneNormals[i], center) > radius)
			return false;
	}
	return true;
}
bool FrustrumSceneQuerry::TestAABB(vec4f min, vec4f max)
{
	vec4f center = 0.5f * (max + min);
	vec4f halfSize = 0.5f * (max - min);
	vec4f s; vec4b mask;

	// AABB against frustrum planes
	for (int i = 0; i < 6; i++)
	{
		mask = vec4f::lessThan(frustrumPlaneNormals[i], vec4f::zero);
		s = vec4f::maskSelect(mask, -halfSize, halfSize);
		if (vec4f::dot(frustrumPlaneNormals[i], center - s) > 0.f)
			return false;
	}

	// frustrum corners against box planes
	vec4f axis, p1, p2;
	for (int i = 0; i < 3; i++)
	{
		axis = vec4f::zero;
		axis[i] = 1.f;
		s = vec4f::dot(halfSize, axis) * axis;
		p1 = center + s;
		p2 = center - s;

		int failCount = 0;
		for (int j = 0; j < 5; j++)
		{
			if (vec4f::dot(frustrumCorners[j] - p1, axis) > 0.f)
				failCount++;
			else break;
		}
		if (failCount == 5)
			return false;

		failCount = 0;
		for (int j = 0; j < 5; j++)
		{
			if (vec4f::dot(frustrumCorners[j] - p1, axis) > 0.f)
				failCount++;
			else break;
		}
		if (failCount == 5)
			return false;
	}
	return true;
}