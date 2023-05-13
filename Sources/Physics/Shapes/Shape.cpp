#include "Shape.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"



Shape::Shape(const ShapeType& ShapeType) : type(ShapeType) {}
Sphere Shape::toSphere() const { return Sphere(vec4f(0.f), 0.f); }
AxisAlignedBox Shape::toAxisAlignedBox() const { return AxisAlignedBox(vec4f(0, 0, 0, 1), vec4f(0, 0, 0, 1)); }
Shape& Shape::operator=(const Shape& s) { type = s.type; return *this; }
Shape* Shape::duplicate() const { return new Shape(type); }

mat4f Shape::computeInertiaMatrix() const { return mat4f::identity; }

void Shape::transform(const vec4f& position, const vec4f& scale, const quatf& orientation) {}

vec4f Shape::support(const vec4f& direction) const { return vec4f(0, 0, 0, 1); }
void Shape::getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const {}

const char* Shape::getTypeStr() const
{
	switch (type)
	{
		case Shape::ShapeType::NONE:			return "Unknown";
		case Shape::ShapeType::POINT:			return "Point";
		case Shape::ShapeType::SEGMENT:			return "Segment";
		case Shape::ShapeType::TRIANGLE:		return "Triangle";
		case Shape::ShapeType::SPHERE:			return "Sphere";
		case Shape::ShapeType::AXIS_ALIGNED_BOX:return "AxisAlignedBox";
		case Shape::ShapeType::ORIENTED_BOX:	return "OrientedBox";
		case Shape::ShapeType::CAPSULE:			return "Capsule";
		case Shape::ShapeType::HULL:			return "ConvexHull";
		default: return "unknown";
	}
}