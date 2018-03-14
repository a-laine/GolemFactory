#pragma once

#include <utility>
#include <algorithm>

#include "Shape.h"


namespace Collision
{
	bool collide(const Shape& a, const Shape& b);
	void debugUnitaryTest(const int& verboseLevel = 0);

	//	Specialized functions
		//	point vs all other
		bool collide_PointvsPoint(const glm::vec3& point1, const glm::vec3& point2);
		bool collide_PointvsSegment(const glm::vec3& point, const glm::vec3& segment1, const glm::vec3& segment2);
		bool collide_PointvsTriangle(const glm::vec3& point, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
		bool collide_PointvsOrientedBox(const glm::vec3& point, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
		bool collide_PointvsAxisAlignedBox(const glm::vec3& point, const glm::vec3& boxMin, const glm::vec3& boxMax);
		bool collide_PointvsSphere(const glm::vec3& point, const glm::vec3& sphereCenter, const float& sphereRadius);
		bool collide_PointvsCapsule(const glm::vec3& point, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);

		//	Segment vs all other (except point)
		bool collide_SegmentvsSegment(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& segment2a, const glm::vec3& segment2b);
		bool collide_SegmentvsTriangle(const glm::vec3& segment1, const glm::vec3& segment2, const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3);
		bool collide_SegmentvsOrientedBox(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
		bool collide_SegmentvsAxisAlignedBox(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& boxMin, const glm::vec3& boxMax);
		bool collide_SegmentvsSphere(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& sphereCenter, const float& sphereRadius);
		bool collide_SegmentvsCapsule(const glm::vec3& segment1a, const glm::vec3& segment1b, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);

		//	Triangle vs all other (...)
		bool collide_TrianglevsTriangle(const glm::vec3& triangle1a, const glm::vec3&triangle1b, const glm::vec3& triangle1c, const glm::vec3& triangle2a, const glm::vec3& triangle2b, const glm::vec3& triangle2c);
		bool collide_TrianglevsOrientedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax);
		bool collide_TrianglevsAxisAlignedBox(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& boxMin, const glm::vec3& boxMax);
		bool collide_TrianglevsSphere(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& sphereCenter, const float& sphereRadius);
		bool collide_TrianglevsCapsule(const glm::vec3& triangle1, const glm::vec3& triangle2, const glm::vec3& triangle3, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	
		//	Oriented Box vs all other (...)
		bool collide_OrientedBoxvsOrientedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::mat4& box2Tranform, const glm::vec3& box2Min, const glm::vec3& box2Max);
		bool collide_OrientedBoxvsAxisAlignedBox(const glm::mat4& box1Tranform, const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
		bool collide_OrientedBoxvsSphere(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius);
		bool collide_OrientedBoxvsCapsule(const glm::mat4& boxTranform, const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
	
		//	Axis Aligned Box vs all other (...)
		bool collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max);
		bool collide_AxisAlignedBoxvsSphere(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& sphereCenter, const float& sphereRadius);
		bool collide_AxisAlignedBoxvsCapsule(const glm::vec3& boxMin, const glm::vec3& boxMax, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);
		
		//	Sphere vs all other (...)
		bool collide_SpherevsSphere(const glm::vec3& sphere1Center, const float& sphere1Radius, const glm::vec3& sphere2Center, const float& sphere2Radius);
		bool collide_SpherevsCapsule(const glm::vec3& sphereCenter, const float& sphereRadius, const glm::vec3& capsule1, const glm::vec3& capsule2, const float& capsuleRadius);

		//	Capsule
		bool collide_CapsulevsCapsule(const glm::vec3& capsule1a, const glm::vec3& capsule1b, const float& capsule1Radius, const glm::vec3& capsule2a, const glm::vec3& capsule2b, const float& capsule2Radius);
	//
};
