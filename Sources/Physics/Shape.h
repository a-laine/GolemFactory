#pragma once

#include <glm/glm.hpp>


class Shape
{
	public:
		//  Miscellaneous
		enum ShapeType
		{
			NONE = -1,
			POINT,
			SEGMENT,
			TRIANGLE,
			ORIENTED_BOX,
			AXIS_ALIGNED_BOX,
			SPHERE,
			CAPSULE
		};
		//

		//	Default
		Shape(const ShapeType& shapeType = NONE);
		//

		//	Attributes
		ShapeType type;
		//
};


class Point : public Shape
{
	public:
		//	Default
		Point(const glm::vec3& position);
		//

		//	Attributes
		glm::vec3 p;
		//
};
class Segment : public Shape
{
	public:
		//	Default
		Segment(const glm::vec3& a, const glm::vec3& b);
		//

		//	Attributes
		glm::vec3 p1, p2;
		//
};
class Triangle : public Shape
{
	public:
		//	Default
		Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
		//

		//	Attributes
		glm::vec3 p1, p2, p3;
		//
};
class OrientedBox : public Shape
{
	public:
		//	Default
		OrientedBox(const glm::mat4& transformationMatrix, const glm::vec3& localMin, const glm::vec3& localMax);
		//

		//	Attributes
		glm::mat4 transform;
		glm::vec3 min, max;
		//
};
class AxisAlignedBox : public Shape
{
	public:
		//	Default
		AxisAlignedBox(const glm::vec3& cornerMin, const glm::vec3& cornerMax);
		//

		//	Attributes
		glm::vec3 min, max;
		//
};
class Sphere : public Shape
{
	public:
		//	Default
		Sphere(const glm::vec3& position, const float& r);
		//

		//	Attributes
		glm::vec3 center;
		float radius;
		//
};
class Capsule : public Shape
{
	public:
		//	Default
		Capsule(const glm::vec3& a, const glm::vec3& b, const float& r);
		//

		//	Attributes
		glm::vec3 p1, p2;
		float radius;
		//
};
