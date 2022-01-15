#include "CollisionTriangle.h"
#include "CollisionPoint.h"
#include "CollisionSegment.h"
#include "CollisionUtils.h"

#include <iostream>


//	Specialized functions : triangle
bool Collision2::collide_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c)
{
	//	easy but overkill
	if (collide_SegmentvsTriangle(triangle1a, triangle1b, triangle2a, triangle2b, triangle2c)) return true;
	else if (collide_SegmentvsTriangle(triangle1a, triangle1c, triangle2a, triangle2b, triangle2c)) return true;
	else if (collide_SegmentvsTriangle(triangle1b, triangle1c, triangle2a, triangle2b, triangle2c)) return true;
	else if (collide_SegmentvsTriangle(triangle2a, triangle2b, triangle1a, triangle1b, triangle1c)) return true;
	else if (collide_SegmentvsTriangle(triangle2a, triangle2c, triangle1a, triangle1b, triangle1c)) return true;
	else  return collide_SegmentvsTriangle(triangle2b, triangle2c, triangle1a, triangle1b, triangle1c);
}
/*bool Collision::collide_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	//	flat triangle
	if (triangle1 == triangle2) return collide_SegmentvsOrientedBox(triangle1, triangle3, boxTranform, boxMin, boxMax);
	else if (triangle1 == triangle3) return collide_SegmentvsOrientedBox(triangle1, triangle2, boxTranform, boxMin, boxMax);
	else if (triangle2 == triangle3) return collide_SegmentvsOrientedBox(triangle1, triangle2, boxTranform, boxMin, boxMax);

	//	special case of SAT
	glm::vec3 x = glm::vec3(boxTranform[0]);
	glm::vec3 y = glm::vec3(boxTranform[1]);
	glm::vec3 z = glm::vec3(boxTranform[2]);
	glm::vec3 edge1 = triangle2 - triangle1;
	glm::vec3 edge2 = triangle3 - triangle1;
	glm::vec3 e1 = glm::normalize(edge1);
	glm::vec3 e2 = glm::normalize(edge2);
	glm::vec3 n = glm::normalize(glm::cross(edge1, edge2));
	glm::vec3 distance = glm::vec3(boxTranform*glm::vec4(0.5f*(boxMin + boxMax), 1.f)) - triangle1;
	glm::vec3 sb = 0.5f * glm::abs(boxMax - boxMin);

	//	first pass
	if      (glm::dot(x, distance) > projectHalfBox(x, sb) + projectTriangle(x, edge1, edge2)) return false;
	else if (glm::dot(y, distance) > projectHalfBox(y, sb) + projectTriangle(y, edge1, edge2)) return false;
	else if (glm::dot(z, distance) > projectHalfBox(z, sb) + projectTriangle(z, edge1, edge2)) return false;
	else if (glm::dot(n, distance) > projectHalfBox(n, sb)) return false;

	glm::vec3 e3 = glm::normalize(triangle3 - triangle2);
	glm::vec3 xe1 = glm::cross(x, e1); if (glm::dot(xe1, xe1) > COLLISION_EPSILON) xe1 = glm::normalize(xe1); else xe1 = glm::vec3(1, 0, 0);
	glm::vec3 ye1 = glm::cross(y, e1); if (glm::dot(ye1, ye1) > COLLISION_EPSILON) ye1 = glm::normalize(ye1); else ye1 = glm::vec3(1, 0, 0);
	glm::vec3 ze1 = glm::cross(z, e1); if (glm::dot(ze1, ze1) > COLLISION_EPSILON) ze1 = glm::normalize(ze1); else ze1 = glm::vec3(1, 0, 0);
	glm::vec3 xe2 = glm::cross(x, e2); if (glm::dot(xe2, xe2) > COLLISION_EPSILON) xe2 = glm::normalize(xe2); else xe2 = glm::vec3(1, 0, 0);
	glm::vec3 ye2 = glm::cross(y, e2); if (glm::dot(ye2, ye2) > COLLISION_EPSILON) ye2 = glm::normalize(ye2); else ye2 = glm::vec3(1, 0, 0);
	glm::vec3 ze2 = glm::cross(z, e2); if (glm::dot(ze2, ze2) > COLLISION_EPSILON) ze2 = glm::normalize(ze2); else ze2 = glm::vec3(1, 0, 0);
	glm::vec3 xe3 = glm::cross(x, e3); if (glm::dot(xe3, xe3) > COLLISION_EPSILON) xe3 = glm::normalize(xe3); else xe3 = glm::vec3(1, 0, 0);
	glm::vec3 ye3 = glm::cross(y, e3); if (glm::dot(ye3, ye3) > COLLISION_EPSILON) ye3 = glm::normalize(ye3); else ye3 = glm::vec3(1, 0, 0);
	glm::vec3 ze3 = glm::cross(z, e3); if (glm::dot(ze3, ze3) > COLLISION_EPSILON) ze3 = glm::normalize(ze3); else ze3 = glm::vec3(1, 0, 0);

	//	second pass
	if      (glm::dot(xe1, distance) > projectHalfBox(xe1, sb) + projectTriangle(xe1, edge1, edge2)) return false;
	else if (glm::dot(xe2, distance) > projectHalfBox(xe2, sb) + projectTriangle(xe2, edge1, edge2)) return false;
	else if (glm::dot(xe3, distance) > projectHalfBox(xe3, sb) + projectTriangle(xe3, edge1, edge2)) return false;
	else if (glm::dot(ye1, distance) > projectHalfBox(ye1, sb) + projectTriangle(ye1, edge1, edge2)) return false;
	else if (glm::dot(ye2, distance) > projectHalfBox(ye2, sb) + projectTriangle(ye2, edge1, edge2)) return false;
	else if (glm::dot(ye3, distance) > projectHalfBox(ye3, sb) + projectTriangle(ye3, edge1, edge2)) return false;
	else if (glm::dot(ze1, distance) > projectHalfBox(ze1, sb) + projectTriangle(ze1, edge1, edge2)) return false;
	else if (glm::dot(ze2, distance) > projectHalfBox(ze2, sb) + projectTriangle(ze2, edge1, edge2)) return false;
	else if (glm::dot(ze3, distance) > projectHalfBox(ze3, sb) + projectTriangle(ze3, edge1, edge2)) return false;
	else return true;
}*/
bool Collision2::collide_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	return collide_TrianglevsOrientedBox(triangle1, triangle2, triangle3, glm::mat4(1.f), boxMin, boxMax);
}
bool Collision2::collide_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	if (triangle1 == triangle2) return collide_SegmentvsSphere(triangle1, triangle3, sphereCenter, sphereRadius);
	else if (triangle1 == triangle3) return collide_SegmentvsSphere(triangle1, triangle2, sphereCenter, sphereRadius);
	else if (triangle2 == triangle3) return collide_SegmentvsSphere(triangle1, triangle2, sphereCenter, sphereRadius);

	glm::vec3 u1 = triangle2 - triangle1;
	glm::vec3 u2 = triangle3 - triangle1;
	glm::vec3 n = glm::normalize(glm::cross(u1, u2));
	glm::vec3 p = (sphereCenter - triangle1) - glm::dot(sphereCenter - triangle1, n) * n;

	//	getting closest point in triangle from barycentric coordinates
	glm::vec2 bary = CollisionUtils::getBarycentricCoordinates(u1, u2, p);
	if (bary.x < 0.f) bary.x = 0.f;
	if (bary.y < 0.f) bary.y = 0.f;
	if (bary.x + bary.y > 1.f) bary /= (bary.x + bary.y);

	glm::vec3 closest = triangle1 + bary.x*u1 + bary.y*u2;
	return false;// collide_PointvsSphere(closest, sphereCenter, sphereRadius);
}
bool Collision2::collide_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	if (capsule1 == capsule2) return collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius);
	if (collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule1, capsuleRadius)) return true;
	if (collide_TrianglevsSphere(triangle1, triangle2, triangle3, capsule2, capsuleRadius)) return true;

	if (collide_SegmentvsCapsule(triangle1, triangle2, capsule1, capsule2, capsuleRadius)) return true;
	if (collide_SegmentvsCapsule(triangle2, triangle3, capsule1, capsule2, capsuleRadius)) return true;
	if (collide_SegmentvsCapsule(triangle3, triangle1, capsule1, capsule2, capsuleRadius)) return true;

	return false;
}
//