#include "CollisionTriangle.h"
#include "CollisionPoint.h"
#include "CollisionSegment.h"
#include "CollisionUtils.h"

#include <iostream>

//	Specialized functions : triangle
bool Collision::collide_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c)
{
	//	easy but overkill
	if (collide_SegmentvsTriangle(triangle1a, triangle1b, triangle2a, triangle2b, triangle2c)) return true;
	else if (collide_SegmentvsTriangle(triangle1a, triangle1c, triangle2a, triangle2b, triangle2c)) return true;
	else if (collide_SegmentvsTriangle(triangle1b, triangle1c, triangle2a, triangle2b, triangle2c)) return true;
	else if (collide_SegmentvsTriangle(triangle2a, triangle2b, triangle1a, triangle1b, triangle1c)) return true;
	else if (collide_SegmentvsTriangle(triangle2a, triangle2c, triangle1a, triangle1b, triangle1c)) return true;
	else  return collide_SegmentvsTriangle(triangle2b, triangle2c, triangle1a, triangle1b, triangle1c);
}
bool Collision::collide_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	//	special cases
	if (triangle1 == triangle2) return collide_SegmentvsOrientedBox(triangle1, triangle3, boxTranform, boxMin, boxMax);
	else if (triangle1 == triangle3) return collide_SegmentvsOrientedBox(triangle1, triangle2, boxTranform, boxMin, boxMax);
	else if (triangle2 == triangle3) return collide_SegmentvsOrientedBox(triangle1, triangle2, boxTranform, boxMin, boxMax);

	//	special case of SAT
	glm::vec3 x = glm::vec3(boxTranform[0]);
	glm::vec3 y = glm::vec3(boxTranform[1]);
	glm::vec3 z = glm::vec3(boxTranform[2]);
	glm::vec3 edge1 = triangle2 - triangle1;
	glm::vec3 edge2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(edge1, edge2);
	glm::vec3 distance = glm::vec3(boxTranform*glm::vec4(0.5f*(boxMin + boxMax), 1.f)) - triangle1;
	glm::vec3 sb = 0.5f * glm::abs(boxMax - boxMin);

	//	first pass
	if      (std::abs(glm::dot(x, distance)) > projectHalfBox(x, sb) + projectTriangle(x, edge1, edge2)) return false;
	else if (std::abs(glm::dot(y, distance)) > projectHalfBox(y, sb) + projectTriangle(y, edge1, edge2)) return false;
	else if (std::abs(glm::dot(z, distance)) > projectHalfBox(z, sb) + projectTriangle(z, edge1, edge2)) return false;
	else if (std::abs(glm::dot(n, distance)) > projectHalfBox(n, sb)/* + projectTriangle(n, edge1, edge2)*/) return false;

	glm::vec3 edge3 = triangle3 - triangle2;
	glm::vec3 xe1 = glm::cross(x, edge1);
	glm::vec3 ye1 = glm::cross(y, edge1);
	glm::vec3 ze1 = glm::cross(z, edge1);
	glm::vec3 xe2 = glm::cross(x, edge2);
	glm::vec3 ye2 = glm::cross(y, edge2);
	glm::vec3 ze2 = glm::cross(z, edge2);
	glm::vec3 xe3 = glm::cross(x, edge3);
	glm::vec3 ye3 = glm::cross(y, edge3);
	glm::vec3 ze3 = glm::cross(z, edge3);

	//	second pass
	if      (std::abs(glm::dot(xe1, distance)) > projectHalfBox(xe1, sb) + projectTriangle(xe1, edge1, edge2)) return false;
	else if (std::abs(glm::dot(xe2, distance)) > projectHalfBox(xe2, sb) + projectTriangle(xe2, edge1, edge2)) return false;
	else if (std::abs(glm::dot(xe3, distance)) > projectHalfBox(xe3, sb) + projectTriangle(xe3, edge1, edge2)) return false;
	else if (std::abs(glm::dot(ye1, distance)) > projectHalfBox(ye1, sb) + projectTriangle(ye1, edge1, edge2)) return false;
	else if (std::abs(glm::dot(ye2, distance)) > projectHalfBox(ye2, sb) + projectTriangle(ye2, edge1, edge2)) return false;
	else if (std::abs(glm::dot(ye3, distance)) > projectHalfBox(ye3, sb) + projectTriangle(ye3, edge1, edge2)) return false;
	else if (std::abs(glm::dot(ze1, distance)) > projectHalfBox(ze1, sb) + projectTriangle(ze1, edge1, edge2)) return false;
	else if (std::abs(glm::dot(ze2, distance)) > projectHalfBox(ze2, sb) + projectTriangle(ze2, edge1, edge2)) return false;
	else if (std::abs(glm::dot(ze3, distance)) > projectHalfBox(ze3, sb) + projectTriangle(ze3, edge1, edge2)) return false;
	else return true;
}
bool Collision::collide_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return collide_TrianglevsOrientedBox(triangle1, triangle2, triangle3, glm::mat4(1.f), boxMin, boxMax);
}
bool Collision::collide_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (triangle1 == triangle2) return collide_SegmentvsSphere(triangle1, triangle3, sphereCenter, sphereRadius);
	else if (triangle1 == triangle3) return collide_SegmentvsSphere(triangle1, triangle2, sphereCenter, sphereRadius);
	else if (triangle2 == triangle3) return collide_SegmentvsSphere(triangle1, triangle2, sphereCenter, sphereRadius);

	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	glm::vec3 n = glm::normalize(glm::cross(u1, u2));
	glm::vec3 p = (sphereCenter - triangle1) - glm::dot(sphereCenter - triangle1, n) * n;

	//	getting closest point in triangle from barycentric coordinates
	float crossDot = glm::dot(u1, u2);
	float magnitute = glm::dot(u1, u1)*glm::dot(u2, u2) - crossDot*crossDot;
	glm::vec2 barry;
	barry.x = (glm::dot(u2, u2) * glm::dot(p, u1) - crossDot * glm::dot(p, u2)) / magnitute;
	barry.y = (glm::dot(u1, u1) * glm::dot(p, u2) - crossDot * glm::dot(p, u1)) / magnitute;

	if (barry.x < 0.f) barry.x = 0.f;
	if (barry.y < 0.f) barry.y = 0.f;
	if (barry.x + barry.y > 1.f) barry /= (barry.x + barry.y);

	glm::vec3 closest = triangle1 + barry.x*u1 + barry.y*u2;
	return collide_PointvsSphere(closest, sphereCenter, sphereRadius);
}
bool Collision::collide_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	/*
		1. compute intersection point between segment extended line and triangle plane
		2. with previous point compute the closest point which is on triangle
		3. with this point get the closest one which is on capsule segment
		4. simply check the shortest distance with capsule radius
	*/


	//	begin and eliminate special cases
	if (capsule1 == capsule2) return collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius);

	glm::vec3 v1 = triangle2 - triangle1;
	glm::vec3 v2 = triangle3 - triangle1;
	glm::vec3 n = glm::cross(v1, v2);
	if (n == glm::vec3(0.f)) // flat triangle
	{
		glm::vec3 v3 = triangle3 - triangle2;
		float d1 = glm::dot(v1, v1);
		float d2 = glm::dot(v2, v2);
		float d3 = glm::dot(v3, v3);

		if (d1 >= d2 && d1 >= d3) return collide_SegmentvsCapsule(triangle1, triangle2, capsule1, capsule2, capsuleRadius);
		else if (d2 >= d1 && d2 >= d3) return collide_SegmentvsCapsule(triangle1, triangle3, capsule1, capsule2, capsuleRadius);
		else return collide_SegmentvsCapsule(triangle3, triangle2, capsule1, capsule2, capsuleRadius);
	}

	//	compute intersection point between ray and plane
	n = glm::normalize(n);
	glm::vec3 s = capsule2 - capsule1;
	if (glm::dot(n, s) == 0.f) // segment parallel to triangle plane -> solve by SAT
	{
		if (std::abs(glm::dot(n, capsule1 - triangle1)) > capsuleRadius) return false; // segment too far on direction n
		glm::vec3 a = capsule1 - glm::dot(n, capsule1 - triangle1) * n;
		glm::vec3 b = capsule2 - glm::dot(n, capsule2 - triangle1) * n;

		/*
			3. if a or b inside triangle ->COLLIDE                 (ou inversement)

			   d1 = distance seg[a,b] / seg[1,2]
			   d2 = distance seg[a,b] / seg[1,3]
			   d3 = distance seg[a,b] / seg[3,2]

			   if min(d1, d2, d3) < projected radius
		*/
		
		
		std::cout << "Collision : collide_TrianglevsCapsule : special case not supported : segment parallel to triangle plane" << std::endl;
		return false;
	}
	glm::vec3 u = glm::normalize(s);
	glm::vec3 intersection = capsule1 + glm::dot(n, triangle1 - capsule1) / glm::dot(n, u)*u - triangle1;

	//	checking barycentric coordinates
	float crossDot = glm::dot(v1, v2);
	float magnitute = glm::dot(v1, v1)*glm::dot(v2, v2) - crossDot*crossDot;
	glm::vec2 barry;

	barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - crossDot * glm::dot(intersection, v2)) / magnitute;
	barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - crossDot * glm::dot(intersection, v1)) / magnitute;
	if (barry.x < 0.f) barry.x = 0.f;
	if (barry.y < 0.f) barry.y = 0.f;
	if (barry.x + barry.y > 1.f) barry /= (barry.x + barry.y);

	glm::vec3 closestTrianglePoint = triangle1 + barry.x*v1 + barry.y*v2;
	glm::vec3 closestSegmentPoint = getSegmentClosestPoint(capsule1, capsule2, closestTrianglePoint);
	return glm::length(closestSegmentPoint - closestTrianglePoint) < capsuleRadius;
}
//