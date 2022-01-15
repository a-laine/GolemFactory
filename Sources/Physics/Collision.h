#pragma once

#include "BoundingVolume.h"
#include "SpecificCollision/SpecificCollision.h"

/*#include "SpecificCollision/CollisionPoint.h"
#include "SpecificCollision/CollisionSegment.h"
#include "SpecificCollision/CollisionTriangle.h"
#include "SpecificCollision/CollisionOrientedBox.h"
#include "SpecificCollision/CollisionAxisAlignedBox.h"
#include "SpecificCollision/CollisionSphere.h"
#include "SpecificCollision/CollisionCapsule.h"*/


namespace Collision
{
	//  Default (GJK)
	template<typename A, typename B>
	bool collide(const A* a, const B* b, CollisionReport* report = nullptr);












	//	Specialized functions : point vs all other
	bool collide_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2, CollisionReport* report = nullptr);

	bool collide_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2, CollisionReport* report = nullptr);
	bool collide_SegmentvsPoint(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2, CollisionReport* report = nullptr);

	bool collide_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report = nullptr);
	bool collide_TrianglevsPoint(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report = nullptr);

	bool collide_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
	bool collide_SpherevsPoint(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

	bool collide_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax, CollisionReport* report = nullptr);
	bool collide_AxisAlignedBoxvsPoint(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax, CollisionReport* report = nullptr);

	bool collide_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, CollisionReport* report = nullptr);
	bool collide_OrientedBoxvsPoint(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, CollisionReport* report = nullptr);

	bool collide_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
	bool collide_CapsulevsPoint(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);

	bool collide_PointvsHull(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report = nullptr);
	bool collide_HullvsPoint(const glm::vec3& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report = nullptr);


	//	Specialized functions : Segment vs all other (except previous Shape)
	bool collide_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b, CollisionReport* report = nullptr);

	bool collide_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report = nullptr);
	bool collide_TrianglevsSegment(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, CollisionReport* report = nullptr);

	bool collide_SegmentvsSphere(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
	bool collide_SpherevsSegment(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

	bool collide_SegmentvsCapsule(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
	bool collide_CapsulevsSegment(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);


	//	Specialized functions : Sphere vs all other (except previous Shape)
	bool collide_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius, CollisionReport* report = nullptr);

	bool collide_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
	bool collide_CapsulevsSphere(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);

	bool collide_SpherevsAxisAlignedBox(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
	bool collide_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

	bool collide_SpherevsOrientedBox(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
	bool collide_OrientedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

	//	Specialized functions : Axis Aligned Box vs all other (except previous Shape)
	bool collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max, CollisionReport* report = nullptr);

};

#include "Collision.hpp"