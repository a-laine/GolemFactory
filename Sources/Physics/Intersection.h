#pragma once

#include <utility>

#include "Shape.h"


namespace Intersection
{
	bool intersect(const Shape& a, const Shape& b);

	//	Specialized functions
		//	point vs all other
		inline bool intersect_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2);
		inline bool intersect_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2);
		inline bool intersect_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
		inline bool intersect_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
		inline bool intersect_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax);
		inline bool intersect_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius);
		inline bool intersect_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);

		//	Segment vs all other (except point)
		inline bool intersect_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b);
		inline bool intersect_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
		inline bool intersect_SegmentvsOrientedBox(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
		inline bool intersect_SegmentvsAxisAlignedBox(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& boxMin, const glm::vec3& boxMax);
		inline bool intersect_SegmentvsSphere(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& sphereCenter, const float& sphereRadius);
		inline bool intersect_SegmentvsCapsule(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);

		//	Triangle vs all other (...)
		inline bool intersect_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c);
		inline bool intersect_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
		inline bool intersect_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax);
		inline bool intersect_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius);
		inline bool intersect_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	
		//	Oriented Box vs all other (...)
		inline bool intersect_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max);
		inline bool intersect_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
		inline bool intersect_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius);
		inline bool intersect_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	
		//	Axis Aligned Box vs all other (...)
		inline bool intersect_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
		inline bool intersect_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius);
		inline bool intersect_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
		
		//	Sphere vs all other (...)
		inline bool intersect_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius);
		inline bool intersect_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);

		//	Capsule
		inline bool intersect_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius);
	//
};
