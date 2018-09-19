#pragma once

#include <glm/glm.hpp>

class Sphere;
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

		//	Public functions
		virtual Sphere toSphere() const;
		//

		//	Attributes
		ShapeType type;
		//
};


class Point : public Shape
{
	public:
		//	Default
		Point(const glm::vec3& position = glm::vec3(0.f));
		//

		//	Public functions
		Sphere toSphere() const;
		//

		//	Attributes
		glm::vec3 p;
		//
};
class Segment : public Shape
{
	public:
		//	Default
		Segment(const glm::vec3& a = glm::vec3(0.f), const glm::vec3& b = glm::vec3(0.f));
		//

		//	Public functions
		Sphere toSphere() const;
		//

		//	Attributes
		glm::vec3 p1, p2;
		//
};
class Triangle : public Shape
{
	public:
		//	Default
		Triangle(const glm::vec3& a = glm::vec3(0.f), const glm::vec3& b = glm::vec3(0.f), const glm::vec3& c = glm::vec3(0.f));
		//

		//	Public functions
		Sphere toSphere() const;
		//

		//	Attributes
		glm::vec3 p1, p2, p3;
		//
};
class OrientedBox : public Shape
{
	public:
		//	Default
		OrientedBox(const glm::mat4& transformationMatrix = glm::mat4(1.f), const glm::vec3& localMin = glm::vec3(0.f), const glm::vec3& localMax = glm::vec3(0.f));
		//

		//	Public functions
		Sphere toSphere() const;
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
		AxisAlignedBox(const glm::vec3& cornerMin = glm::vec3(0.f), const glm::vec3& cornerMax = glm::vec3(0.f));
		//

		//	Public functions
		Sphere toSphere() const;
		//

		//	Attributes
		glm::vec3 min, max;
		//
};
class Sphere : public Shape
{
	public:
		//	Default
		Sphere(const glm::vec3& position = glm::vec3(0.f), const float& r = 0.f);
		//

		//	Public functions
		Sphere toSphere() const;
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
		Capsule(const glm::vec3& a = glm::vec3(0.f), const glm::vec3& b = glm::vec3(0.f), const float& r = 0.f);
		//

		//	Public functions
		Sphere toSphere() const;
		//

		//	Attributes
		glm::vec3 p1, p2;
		float radius;
		//
};
