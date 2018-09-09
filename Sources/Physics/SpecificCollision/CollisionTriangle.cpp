#include "CollisionTriangle.h"
#include "CollisionPoint.h"
#include "CollisionSegment.h"
#include "CollisionUtils.h"


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
	else if (glm::dot(e1, distance) > projectHalfBox(e1, sb) + projectTriangle(e1, edge1, edge2)) return false;
	else if (glm::dot(e2, distance) > projectHalfBox(e2, sb) + projectTriangle(e2, edge1, edge2)) return false;

	glm::vec3 e3 = glm::normalize(triangle3 - triangle2);
	glm::vec3 xe1 = glm::normalize(glm::cross(x, e1));
	glm::vec3 ye1 = glm::normalize(glm::cross(y, e1));
	glm::vec3 ze1 = glm::normalize(glm::cross(z, e1));
	glm::vec3 xe2 = glm::normalize(glm::cross(x, e2));
	glm::vec3 ye2 = glm::normalize(glm::cross(y, e2));
	glm::vec3 ze2 = glm::normalize(glm::cross(z, e2));
	glm::vec3 xe3 = glm::normalize(glm::cross(x, e3));
	glm::vec3 ye3 = glm::normalize(glm::cross(y, e3));
	glm::vec3 ze3 = glm::normalize(glm::cross(z, e3));

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
	glm::vec2 bary = getBarycentricCoordinates(u1, u2, p);
	if (bary.x < 0.f) bary.x = 0.f;
	if (bary.y < 0.f) bary.y = 0.f;
	if (bary.x + bary.y > 1.f) bary /= (bary.x + bary.y);

	glm::vec3 closest = triangle1 + bary.x*u1 + bary.y*u2;
	return collide_PointvsSphere(closest, sphereCenter, sphereRadius);
}
bool Collision::collide_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
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
	if (glm::dot(n, s) == 0.f) // segment parallel to triangle plane
	{
		float S = glm::dot(n, capsule1 - triangle1);						// distance [capsule seg, triangle plane]
		if (std::abs(S) > capsuleRadius) return false;						// segment too far on direction n
		glm::vec3 a = capsule1 - glm::dot(n, capsule1 - triangle1) * n;		// capsule seg projection on plane
		glm::vec3 b = capsule2 - glm::dot(n, capsule2 - triangle1) * n;

		// test if one of extremity of projection segment is inside triangle
		glm::vec2 bary = getBarycentricCoordinates(v1, v2, a);
		if (bary.x <= 1.f && bary.y <= 1.f && bary.x + bary.y <= 1.f) return true;
		bary = getBarycentricCoordinates(v1, v2, b);
		if (bary.x <= 1.f && bary.y <= 1.f && bary.x + bary.y <= 1.f) return true;

		// test projection segment against triangle
		std::pair<glm::vec3, glm::vec3> s1 = getSegmentsClosestSegment(a, b, triangle1, triangle2);
		std::pair<glm::vec3, glm::vec3> s2 = getSegmentsClosestSegment(a, b, triangle1, triangle3);
		std::pair<glm::vec3, glm::vec3> s3 = getSegmentsClosestSegment(a, b, triangle3, triangle2);

		float d1 = glm::length(s1.first - s1.second);						// distance seg[a,b] / seg[1,2]
		float d2 = glm::length(s2.first - s2.second);						// distance seg[a,b] / seg[1,3]
		float d3 = glm::length(s3.first - s3.second);						// distance seg[a,b] / seg[3,2]
		
		float size = sqrt(capsuleRadius * capsuleRadius - S * S) + COLLISION_EPSILON;	//	r²sin² = r² - r²cos² = r² - S²
		return d1<size || d2<size || d3<size;								//	seg[a,b] is far enough to triangle
	}
	else // standard case
	{
		glm::vec3 u = glm::normalize(s);
		glm::vec3 intersection = capsule1 + glm::dot(n, triangle1 - capsule1) / glm::dot(n, u)*u -triangle1;

		//	checking barycentric coordinates
		glm::vec2 bary = getBarycentricCoordinates(v1, v2, intersection);
		if (bary.x < 0.f) bary.x = 0.f;
		if (bary.y < 0.f) bary.y = 0.f;
		if (bary.x + bary.y > 1.f) bary /= (bary.x + bary.y);

		// compute and compare minimal distance to capsule radius
		glm::vec3 closestTrianglePoint = triangle1 + bary.x*v1 + bary.y*v2;
		glm::vec3 closestSegmentPoint = getSegmentClosestPoint(capsule1, capsule2, closestTrianglePoint);
		return glm::length(closestSegmentPoint - closestTrianglePoint) <= capsuleRadius;
	}
}
//