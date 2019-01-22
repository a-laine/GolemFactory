#include "Shape.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"



Shape::Shape(const ShapeType& ShapeType) : type(ShapeType) {}
Sphere Shape::toSphere() const { return Sphere(glm::vec3(0.f), 0.f); }
AxisAlignedBox Shape::toAxisAlignedBox() const { return AxisAlignedBox(glm::vec3(0.f), glm::vec3(0.f)); }
void Shape::operator=(const Shape& s) {}
void Shape::transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) {}
Shape* Shape::duplicate() const { return new Shape(type); }
