#include "IntersectionPoint.h"

#include "Physics/SpecificCollision/CollisionUtils.h"
#include "Physics/SpecificCollision/CollisionPoint.h"

#include <glm/gtx/norm.hpp>


Intersection::Contact Intersection::intersect_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2)
{
	glm::vec3 n = glm::normalize(point2 - point1);
	return Intersection::Contact(point1, point2, n, -n);
}
Intersection::Contact Intersection::intersect_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2)
{
	Contact contact;
	contact.contactPointA = point;
	contact.contactPointB = getSegmentClosestPoint(segment1, segment2, point);
	if (contact.contactPointB == segment1)
		contact.normalB = glm::normalize(point - segment1);
	else if (contact.contactPointB == segment2)
		contact.normalB = glm::normalize(point - segment2);
	else
		contact.normalB = glm::normalize(point - contact.contactPointB);
	contact.normalA = -contact.normalB;
	return contact;
}
Intersection::Contact Intersection::intersect_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3)
{
	// compute variables and get barrycentric coordinates of projected point
	glm::vec3 n = glm::normalize(glm::cross(triangle2 - triangle1, triangle3 - triangle1));
	glm::vec3 p = point - triangle1;
	if (glm::dot(p, n) < 0.f)
		n *= -1.f;
	glm::vec3 u = p - n * glm::dot(p, n);
	glm::vec2 barry = getBarycentricCoordinates(triangle2 - triangle1, triangle3 - triangle1, u);
	
	if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) // projection out of triangle
	{
		// compute closest point on each edges
		glm::vec3 p1 = getSegmentClosestPoint(triangle1, triangle2, point);
		glm::vec3 p2 = getSegmentClosestPoint(triangle3, triangle2, point);
		glm::vec3 p3 = getSegmentClosestPoint(triangle3, triangle1, point);
		
		// compute distance for each of these point
		float d1 = glm::length2(point - p1);
		float d2 = glm::length2(point - p2);
		float d3 = glm::length2(point - p3);

		// choose the best candidate
		if (d1 <= d2 && d1 <= d3) p = p1;
		else if (d2 <= d1 && d2 <= d3) p = p2;
		else p = p3;

		// compute contact result and exit
		Contact contact;
		contact.contactPointA = point;
		contact.contactPointB = p;
		contact.normalA = glm::normalize(p - point);
		contact.normalB = -contact.normalA;
		return contact;
	}
	else // projected point is inside triangle
	{
		Contact contact;
		contact.contactPointA = point;
		contact.contactPointB = triangle1 + u;
		contact.normalA = -n;
		contact.normalB = n;
		return contact;
	}
}
Intersection::Contact Intersection::intersect_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	// init
	glm::vec3 p = glm::vec3(glm::inverse(boxTranform) * glm::vec4(point, 1.f));
	Contact contact = intersect_PointvsAxisAlignedBox(p, boxMin, boxMax);

	// end
	contact.contactPointA = glm::vec3(boxTranform * glm::vec4(contact.contactPointA, 1.f));
	contact.contactPointB = glm::vec3(boxTranform * glm::vec4(contact.contactPointB, 1.f));
	contact.normalA = glm::vec3(boxTranform * glm::vec4(contact.normalA, 0.f));
	contact.normalB = glm::vec3(boxTranform * glm::vec4(contact.normalB, 0.f));
	return contact;
}
Intersection::Contact Intersection::intersect_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax)
{
	//	special case of obb/sphere
	glm::vec3 bcenter = 0.5f * (boxMax + boxMin);
	glm::vec3 bsize = 0.5f * glm::abs(boxMax - boxMin);
	glm::vec3 p = point - bcenter;
	glm::vec3 n = glm::vec3(0.f);

	if (Collision::collide_PointvsAxisAlignedBox(point, boxMin, boxMax))
	{
		const float dx = bsize.x - std::abs(p.x);
		const float dy = bsize.y - std::abs(p.y);
		const float dz = bsize.z - std::abs(p.z);

		// inside point is closer to an x face
		if (dx <= dy && dx <= dz)
		{
			if (p.x > 0)
			{
				n = glm::vec3(1, 0, 0);
				p.x = bsize.x;
			}
			else
			{
				n = glm::vec3(-1, 0, 0);
				p.x = -bsize.x;
			}
		}

		// inside point is closer to a y face
		else if (dy <= dx && dy <= dz)
		{
			if (p.y > 0)
			{
				n = glm::vec3(0, 1, 0);
				p.y = bsize.y;
			}
			else
			{
				n = glm::vec3(0, -1, 0);
				p.y = -bsize.y;
			}
		}

		// inside point is closer to a z face
		else
		{
			if (p.z > 0)
			{
				n = glm::vec3(0, 0, 1);
				p.z = bsize.z;
			}
			else
			{
				n = glm::vec3(0, 0, -1);
				p.z = -bsize.z;
			}
		}

		// compute result and end
		Contact contact;
		contact.contactPointA = point;
		contact.contactPointB = p + bcenter;
		contact.normalA = -n;
		contact.normalB = n;
		return contact;
	}
	else
	{
		// tmp variables
		bool inExtrudeX = false;
		bool inExtrudeY = false;
		bool inExtrudeZ = false;

		// compute closest point on box
		if (p.x > bsize.x)
			p.x = bsize.x;
		else if (p.x < -bsize.x)
			p.x = -bsize.x;
		else inExtrudeX = true;
		if (p.y > bsize.y)
			p.y = bsize.y;
		else if (p.y < -bsize.y)
			p.y = -bsize.y;
		else inExtrudeY = true;
		if (p.z > bsize.z)
			p.z = bsize.z;
		else if (p.z < -bsize.z)
			p.z = -bsize.z;
		else inExtrudeZ = true;
		p += bcenter;

		// compute box normal depending on point closest face
		if (inExtrudeX && inExtrudeY && inExtrudeZ) // p is on a corner
			n = glm::normalize(point - p);
		else if (inExtrudeX && inExtrudeY) // p is on a Z edge
			n = glm::normalize(point - glm::vec3(p.x, p.y, 0.f));
		else if (inExtrudeX && inExtrudeZ) // p is on a Y edge
			n = glm::normalize(point - glm::vec3(p.x, 0.f, p.z));
		else if (inExtrudeZ && inExtrudeY) // p is on a X edge
			n = glm::normalize(point - glm::vec3(0.f, p.y, p.z));
		else if(inExtrudeX)
			n = glm::normalize(point - glm::vec3(p.x, 0.f, 0.f));
		else if (inExtrudeY)
			n = glm::normalize(point - glm::vec3(0.f, p.y, 0.f));
		else 
			n = glm::normalize(point - glm::vec3(0.f, 0.f, p.z));

		// compute result and end
		Contact contact;
		contact.contactPointA = point;
		contact.contactPointB = p;
		contact.normalA = -n;
		contact.normalB = n;
		return contact;
	}
}
Intersection::Contact Intersection::intersect_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius)
{
	Contact contact;
	contact.normalB = -glm::normalize(sphereCenter - point);
	contact.normalA = contact.normalB;
	contact.contactPointA = point;
	contact.contactPointB = sphereCenter + sphereRadius * contact.normalA;
	return contact;
}
Intersection::Contact Intersection::intersect_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius)
{
	Contact contact;
	contact.contactPointA = point;
	contact.contactPointB = getSegmentClosestPoint(capsule1, capsule2, point);
	if (contact.contactPointB == capsule1)
	{
		contact.normalB = glm::normalize(point - capsule1);
		contact.contactPointB = capsule1 + capsuleRadius * contact.normalB;
	}
	else if (contact.contactPointB == capsule2)
	{
		contact.normalB = glm::normalize(point - capsule2);
		contact.contactPointB = capsule2 + capsuleRadius * contact.normalB;
	}
	else
	{
		contact.normalB = glm::normalize(point - contact.contactPointB);
		contact.contactPointB = contact.contactPointB + capsuleRadius * contact.normalB;
	}
	contact.normalA = -contact.normalB;
	return contact;
}
Intersection::Contact Intersection::intersect_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase)
{
	// init
	Contact contact;
	float dmin = std::numeric_limits<float>::max();
	glm::vec3 p = glm::vec3(glm::inverse(hullBase) * glm::vec4(point, 1.f));

	// point is inside hull
	if (Collision::collide_PointvsHull(point, hullPoints, hullNormals, hullFaces, hullBase))
	{
		// search closest face
		for (unsigned int i = 0; i < hullNormals.size(); i++)
		{
			glm::vec3 n = glm::normalize(hullNormals[i]);
			float d = std::abs(glm::dot(n, p - hullPoints[hullFaces[3 * i]]));
			if (d < dmin)
			{
				dmin = d;
				contact.normalB = n;
			}
		}

		// compute result
		contact.contactPointA = p;
		contact.contactPointB = p + contact.normalB * dmin;
		contact.normalA = -contact.normalB;
	}

	// point is outside
	else
	{
		// search closest front face
		for (unsigned int i = 0; i < hullNormals.size(); i++)
		{
			if (glm::dot(hullNormals[i], p - hullPoints[hullFaces[3 * i]]) >= 0)
			{
				Contact c = intersect_PointvsTriangle(p, hullPoints[hullFaces[3 * i]], hullPoints[hullFaces[3 * i + 1]], hullPoints[hullFaces[3 * i + 2]]);
				float d = glm::length2(p - c.contactPointB);
				if (d < dmin)
				{
					dmin = d;
					contact = c;
				}
			}
		}
	}

	// end
	contact.contactPointA = glm::vec3(hullBase * glm::vec4(contact.contactPointA, 1.f));
	contact.contactPointB = glm::vec3(hullBase * glm::vec4(contact.contactPointB, 1.f));
	contact.normalA = glm::vec3(hullBase * glm::vec4(contact.normalA, 0.f));
	contact.normalB = glm::vec3(hullBase * glm::vec4(contact.normalB, 0.f));
	return contact;
}