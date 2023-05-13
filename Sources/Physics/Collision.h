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
		static bool collide_PointvsPoint(const vec4f& point1, const vec4f& point2, CollisionReport* report = nullptr);

		static bool collide_PointvsSegment(const vec4f& point, const vec4f& segment1, const vec4f& segment2, CollisionReport* report = nullptr);
		static bool collide_SegmentvsPoint(const vec4f& point, const vec4f& segment1, const vec4f& segment2, CollisionReport* report = nullptr);

		static bool collide_PointvsTriangle(const vec4f& point, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report = nullptr);
		static bool collide_TrianglevsPoint(const vec4f& point, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report = nullptr);

		static bool collide_PointvsSphere(const vec4f& point, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_SpherevsPoint(const vec4f& point, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		static bool collide_PointvsAxisAlignedBox(const vec4f& point, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report = nullptr);
		static bool collide_AxisAlignedBoxvsPoint(const vec4f& point, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report = nullptr);

		static bool collide_PointvsOrientedBox(const vec4f& point, const mat4f& boxTranform, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report = nullptr);
		static bool collide_OrientedBoxvsPoint(const vec4f& point, const mat4f& boxTranform, const vec4f& boxMin, const vec4f& boxMax, CollisionReport* report = nullptr);

		static bool collide_PointvsCapsule(const vec4f& point, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
		static bool collide_CapsulevsPoint(const vec4f& point, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);

		static bool collide_PointvsHull(const vec4f& point, const std::vector<vec4f>& hullPoints, const std::vector<vec4f>& hullNormals, const std::vector<unsigned short>& hullFaces, const mat4f& hullBase, CollisionReport* report = nullptr);
		static bool collide_HullvsPoint(const vec4f& point, const std::vector<vec4f>& hullPoints, const std::vector<vec4f>& hullNormals, const std::vector<unsigned short>& hullFaces, const mat4f& hullBase, CollisionReport* report = nullptr);


		//	Specialized functions : Segment vs all other (except previous Shape)
		static bool collide_SegmentvsSegment(const vec4f& segment1a, const vec4f& segment1b, const vec4f& segment2a, const vec4f& segment2b, CollisionReport* report = nullptr);

		static bool collide_SegmentvsTriangle(const vec4f& segment1, const vec4f& segment2, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report = nullptr);
		static bool collide_TrianglevsSegment(const vec4f& segment1, const vec4f& segment2, const vec4f& triangle1, const vec4f& triangle2, const vec4f& triangle3, CollisionReport* report = nullptr);

		static bool collide_SegmentvsSphere(const vec4f& segment1, const vec4f& segment2, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_SpherevsSegment(const vec4f& segment1, const vec4f& segment2, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		static bool collide_SegmentvsCapsule(const vec4f& segment1, const vec4f& segment2, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
		static bool collide_CapsulevsSegment(const vec4f& segment1, const vec4f& segment2, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);


		//	Specialized functions : Sphere vs all other (except previous Shape)
		static bool collide_SpherevsSphere(const vec4f& sphere1Center, const float& sphere1Radius, const vec4f& sphere2Center, const float& sphere2Radius, CollisionReport* report = nullptr);

		static bool collide_SpherevsCapsule(const vec4f& sphereCenter, const float& sphereRadius, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);
		static bool collide_CapsulevsSphere(const vec4f& sphereCenter, const float& sphereRadius, const vec4f& capsule1, const vec4f& capsule2, const float& capsuleRadius, CollisionReport* report = nullptr);

		static bool collide_SpherevsAxisAlignedBox(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_AxisAlignedBoxvsSphere(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		static bool collide_SpherevsOrientedBox(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);
		static bool collide_OrientedBoxvsSphere(const vec4f& boxMin, const vec4f& boxMax, const vec4f& sphereCenter, const float& sphereRadius, CollisionReport* report = nullptr);

		//	Specialized functions : Axis Aligned Box vs all other (except previous Shape)
		//static bool collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max);
		static bool collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max, CollisionReport* report = nullptr);

};
