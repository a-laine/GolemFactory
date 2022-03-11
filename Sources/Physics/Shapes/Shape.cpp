#include "Shape.h"
#include "Sphere.h"
#include "AxisAlignedBox.h"



Shape::Shape(const ShapeType& ShapeType) : type(ShapeType) {}
Sphere Shape::toSphere() const { return Sphere(glm::vec4(0.f), 0.f); }
AxisAlignedBox Shape::toAxisAlignedBox() const { return AxisAlignedBox(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 1)); }
Shape& Shape::operator=(const Shape& s) { type = s.type; return *this; }
Shape* Shape::duplicate() const { return new Shape(type); }

glm::mat3 Shape::computeInertiaMatrix() const { return glm::mat3(1.f); }

void Shape::transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation) {}

glm::vec4 Shape::support(const glm::vec4& direction) const { return glm::vec4(0, 0, 0, 1); }
void Shape::getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const {}
