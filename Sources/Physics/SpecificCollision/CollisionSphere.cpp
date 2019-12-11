#include "CollisionSphere.h"
#include "CollisionUtils.h"
#include "Physics/SpecificCollision/CollisionTriangle.h"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>

//	Specialized functions : sphere
bool Collision::collide_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius)
{
	return glm::length2(sphere2Center - sphere1Center) <= (sphere1Radius + sphere2Radius)*(sphere1Radius + sphere2Radius);
}
bool Collision::collide_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	return glm::length2(sphereCenter - getSegmentClosestPoint(capsule1, capsule2, sphereCenter)) < (sphereRadius + capsuleRadius)*(sphereRadius + capsuleRadius);
}
bool Collision::collide_SpherevsHull(const glm::vec3& sphereCenter, const float& sphereRadius, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase)
{
	struct Triangle
	{
		Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) : p1(a), p2(b), p3(c) {};
		glm::vec3 p1, p2, p3;
	};

	//	search for all faces front to sphere center
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * glm::vec4(sphereCenter, 1.f));
	std::vector<Triangle> frontFaces;
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		if (glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]) >= 0)
			frontFaces.push_back(Triangle(hullPoints[hullFaces[3 * i]], hullPoints[hullFaces[3 * i + 1]], hullPoints[hullFaces[3 * i + 2]]));
	}

	if(frontFaces.empty())
		return true;

	//	test if each front faces collide
	for (unsigned int i = 0; i < frontFaces.size(); i++)
	{
		if (collide_TrianglevsSphere(frontFaces[i].p1, frontFaces[i].p2, frontFaces[i].p3, p, sphereRadius))
			return true;
	}
	return false;
}
//