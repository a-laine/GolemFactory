#include "Shape.h"

//	Default
Shape::Shape(const ShapeType& shapeType) : type(shapeType) {}

Point::Point(const glm::vec3& position) : Shape(POINT), p(position){}

Segment::Segment(const glm::vec3& a, const glm::vec3& b) : Shape(SEGMENT), p1(a), p2(b) {}

Triangle::Triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) : Shape(TRIANGLE), p1(a), p2(b), p3(c) {}

OrientedBox::OrientedBox(const glm::mat4& transformationMatrix, const glm::vec3& localMin, const glm::vec3& localMax)
	: Shape(ORIENTED_BOX), transform(transformationMatrix), min(localMin), max(localMax) {}

AxisAlignedBox::AxisAlignedBox(const glm::vec3& cornerMin, const glm::vec3& cornerMax)
	: Shape(AXIS_ALIGNED_BOX), min(cornerMin), max(cornerMax) {}

Sphere::Sphere(const glm::vec3& position, const float& r) : Shape(SPHERE), center(position), radius(r) {}

Capsule::Capsule(const glm::vec3& a, const glm::vec3& b, const float& r) : Shape(CAPSULE), p1(a), p2(b), radius(r) {}
//

