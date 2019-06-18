#include "CollisionPoint.h"
#include "CollisionUtils.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>


//	Specialized functions : point
bool Collision::collide_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2)
{
	return point1 == point2;
}
bool Collision::collide_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2)
{
	if (segment1 == segment2) return point == segment1;
	else
	{
		glm::vec3 s = segment2 - segment1;
		glm::vec3 u = glm::normalize(s);
		glm::vec3 u2 = point - segment1;
		glm::vec3 u3 = u2 - glm::dot(u, u2) * u; // distance of point to segment
		return glm::dot(u3, u3) <= COLLISION_EPSILON && glm::dot(u, u2) <= glm::length(s) && glm::dot(u, u2) >= COLLISION_EPSILON;
	}
}
bool Collision::collide_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	//	check if point is coplanar to triangle
	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(u1, u2);
	glm::vec3 p = point - triangle1;

	if (n == glm::vec3(0.f)) // flat triangle
	{
		glm::vec3 u3 = triangle3 - triangle2;
		float d1 = glm::dot(u1, u1);
		float d2 = glm::dot(u2, u2);
		float d3 = glm::dot(u3, u3);

		if (d1 >= d2 && d1 >= d3) return collide_PointvsSegment(point, triangle1, triangle2);
		else if (d2 >= d1 && d2 >= d3) return collide_PointvsSegment(point, triangle1, triangle3);
		else return collide_PointvsSegment(point, triangle3, triangle2);
	}
	else if (glm::dot(p, n) <= COLLISION_EPSILON)
	{
		glm::normalize(n);

		//	checking barycentric coordinates
		float crossDot = glm::dot(u1, u2);
		float magnitute = glm::dot(u1, u1)*glm::dot(u2, u2) - crossDot*crossDot;
		glm::vec2 barry;
		barry.x = (glm::dot(u2, u2) * glm::dot(p, u1) - crossDot * glm::dot(p, u2)) / magnitute;
		barry.y = (glm::dot(u1, u1) * glm::dot(p, u2) - crossDot * glm::dot(p, u1)) / magnitute;
		return !(barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f);
	}
	else return false;
}
bool Collision::collide_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	glm::vec3 bmin = glm::vec3(boxTranform*glm::vec4(boxMin, 1.f));
	glm::vec3 bdiag = glm::vec3(boxTranform*glm::vec4(boxMax, 1.f)) - bmin;
	glm::vec3 bx = glm::vec3(boxTranform[0]);
	glm::vec3 by = glm::vec3(boxTranform[1]);
	glm::vec3 bz = glm::vec3(boxTranform[2]);

	glm::vec3 p = point - bmin;
	if (glm::dot(p, bx) < -COLLISION_EPSILON || glm::dot(p, by) < -COLLISION_EPSILON || glm::dot(p, bz) < -COLLISION_EPSILON) return false;
	else if (glm::dot(p, bx) > glm::dot(bdiag, bx) + COLLISION_EPSILON || glm::dot(p, by) > glm::dot(bdiag, by) + COLLISION_EPSILON || glm::dot(p, bz) > glm::dot(bdiag, bz) + COLLISION_EPSILON) return false;
	else return true;
}
bool Collision::collide_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	if (point.x < boxMin.x || point.y < boxMin.y || point.z < boxMin.z) return false;
	else if (point.x > boxMax.x || point.y > boxMax.y || point.z > boxMax.z) return false;
	else return true;
}
bool Collision::collide_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	return glm::length(point - sphereCenter) <= std::max(sphereRadius, COLLISION_EPSILON);
}
bool Collision::collide_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (capsule2 == capsule1) return glm::length(point - capsule1) <= std::max(capsuleRadius, COLLISION_EPSILON);
	else
	{
		glm::vec3 s = capsule2 - capsule1;
		glm::vec3 u = glm::normalize(capsule2 - capsule1);
		glm::vec3 u2 = point - capsule1;
		glm::vec3 u3 = u2 - glm::dot(u, u2) * u; // distance of point to segment
		return glm::dot(u3, u3) <= std::max(capsuleRadius, COLLISION_EPSILON) && glm::dot(u, u2) <= glm::length(s) + capsuleRadius && glm::dot(u, u2) >= -capsuleRadius;
	}
}
bool Collision::collide_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase)
{
	//std::cout << ".............TOTO................" << std::endl;
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * glm::vec4(point, 1.f));
	for (unsigned int i = 0; i < hullNormals.size(); i++)
	{
		/*std::cout << glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]) << "\t[" << hullNormals[i].x << "," << hullNormals[i].y << "," << hullNormals[i].z << "] , [" <<
			hullPoints[hullFaces[3 * i]].x << "," << hullPoints[hullFaces[3 * i]].y << "," << hullPoints[hullFaces[3 * i]].z << "]" << std::endl;*/
		if (glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]) >= 0)
		{
			//std::cout << "outside" << std::endl;
			return false;
		}
	}
	//std::cout << "point inside" << std::endl;
	return true;
}
//
