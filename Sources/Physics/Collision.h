#pragma once

#include "BoundingVolume.h"
#include "CollisionReport.h"
#include "GJK.h"

/*#include "SpecificCollision/CollisionPoint.h"
#include "SpecificCollision/CollisionSegment.h"
#include "SpecificCollision/CollisionTriangle.h"
#include "SpecificCollision/CollisionOrientedBox.h"
#include "SpecificCollision/CollisionAxisAlignedBox.h"
#include "SpecificCollision/CollisionSphere.h"
#include "SpecificCollision/CollisionCapsule.h"*/


class Collision
{
	public:
		static void DispatchMatrixInit();
		static bool collide(const Shape* a, const Shape* b, CollisionReport* report = nullptr);

	private:
		using CollisionTest = bool (*)(const Shape*, const Shape*, CollisionReport*);
		static CollisionTest dispatchMatrix[8][8];

		static bool _GJKCollision(const Shape* a, const Shape* b, CollisionReport* report);

		static bool _PointvsPoint(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsSegment(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsTriangle(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsSphere(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsAxisAlignedBox(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsOrientedBox(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsCapsule(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _PointvsHull(const Shape* a, const Shape* b, CollisionReport* report);

		static bool _SegmentvsPoint(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SegmentvsSegment(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SegmentvsTriangle(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SegmentvsSphere(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SegmentvsCapsule(const Shape* a, const Shape* b, CollisionReport* report);

		static bool _SpherevsPoint(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SpherevsSegment(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SpherevsSphere(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SpherevsAxisAlignedBox(const Shape* a, const Shape* b, CollisionReport* report);
		//static bool _SpherevsOrientedBox(const Shape* a, const Shape* b, CollisionReport* report);
		static bool _SpherevsCapsule(const Shape* a, const Shape* b, CollisionReport* report);


	public:
		//	Specialized functions : point vs all other
		static bool collide_PointvsPoint(const glm::vec4& point1, const glm::vec4& point2, CollisionReport* report = nullptr);

		static bool collide_PointvsSegment(const glm::vec4& point, const glm::vec4& segment1, const glm::vec4& segment2, CollisionReport* report = nullptr);
		static bool collide_SegmentvsPoint(const glm::vec4& point, const glm::vec4& segment1, const glm::vec4& segment2, CollisionReport* report = nullptr);

		static bool collide_PointvsTriangle(const glm::vec4& point, const glm::vec4& triangle1, const glm::vec4& triangle2, const glm::vec4& triangle3, CollisionReport* report = nullptr);
		static bool collide_TrianglevsPoint(const glm::vec4& point, const glm::vec4& triangle1, const glm::vec4& triangle2, const glm::vec4& triangle3, CollisionReport* report = nullptr);

		static bool collide_PointvsSphere(const glm::vec4& point, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_SpherevsPoint(const glm::vec4& point, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		static bool collide_PointvsAxisAlignedBox(const glm::vec4& point, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report = nullptr);
		static bool collide_AxisAlignedBoxvsPoint(const glm::vec4& point, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report = nullptr);

		static bool collide_PointvsOrientedBox(const glm::vec4& point, const glm::mat4& boxTranform, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report = nullptr);
		static bool collide_OrientedBoxvsPoint(const glm::vec4& point, const glm::mat4& boxTranform, const glm::vec4& boxMin, const glm::vec4& boxMax, CollisionReport* report = nullptr);

		static bool collide_PointvsCapsule(const glm::vec4& point, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
		static bool collide_CapsulevsPoint(const glm::vec4& point, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);

		static bool collide_PointvsHull(const glm::vec4& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report = nullptr);
		static bool collide_HullvsPoint(const glm::vec4& point, const std::vector<glm::vec3>& hullPoints, const std::vector<glm::vec3>& hullNormals, const std::vector<unsigned short>& hullFaces, const glm::mat4& hullBase, CollisionReport* report = nullptr);


		//	Specialized functions : Segment vs all other (except previous Shape)
		static bool collide_SegmentvsSegment(const glm::vec4& segment1a, const glm::vec4& segment1b, const glm::vec4& segment2a, const glm::vec4& segment2b, CollisionReport* report = nullptr);

		static bool collide_SegmentvsTriangle(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& triangle1, const glm::vec4& triangle2, const glm::vec4& triangle3, CollisionReport* report = nullptr);
		static bool collide_TrianglevsSegment(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& triangle1, const glm::vec4& triangle2, const glm::vec4& triangle3, CollisionReport* report = nullptr);

		static bool collide_SegmentvsSphere(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_SpherevsSegment(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		static bool collide_SegmentvsCapsule(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
		static bool collide_CapsulevsSegment(const glm::vec4& segment1, const glm::vec4& segment2, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);


		//	Specialized functions : Sphere vs all other (except previous Shape)
		static bool collide_SpherevsSphere(const glm::vec4& sphere1Center, const float& sphere1Radius, const glm::vec4& sphere2Center, const float& sphere2Radius, CollisionReport* report = nullptr);

		static bool collide_SpherevsCapsule(const glm::vec4& sphereCenter, const float& sphereRadius, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
		static bool collide_CapsulevsSphere(const glm::vec4& sphereCenter, const float& sphereRadius, const glm::vec4& capsule1, const glm::vec4& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);

		static bool collide_SpherevsAxisAlignedBox(const glm::vec4& boxMin, const glm::vec4& boxMax, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_AxisAlignedBoxvsSphere(const glm::vec4& boxMin, const glm::vec4& boxMax, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		static bool collide_SpherevsOrientedBox(const glm::vec4& boxMin, const glm::vec4& boxMax, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_OrientedBoxvsSphere(const glm::vec4& boxMin, const glm::vec4& boxMax, const glm::vec4& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		//	Specialized functions : Axis Aligned Box vs all other (except previous Shape)
		//static bool collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec4& box1Min, const glm::vec4& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
		static bool collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec4& box1Min, const glm::vec4& box1Max, const glm::vec4& box2Min, const glm::vec4& box2Max, CollisionReport* report = nullptr);

};
