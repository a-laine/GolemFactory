#pragma once

#include "Shape.h"

class Sphere : public Shape
{
	public:
		//	Default
		Sphere(const glm::vec3& position = glm::vec3(0.f), const float& r = 0.f);
		//

		//	Public functions
		Sphere toSphere() const override;
		AxisAlignedBox toAxisAlignedBox() const override;
		Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;

		virtual glm::vec3 support(const glm::vec3& direction) const override;
		virtual void getFacingFace(const glm::vec3& direction, std::vector<glm::vec3>& points) const override;
		//

		//	Attributes
		glm::vec3 center;
		float radius;
		//
};