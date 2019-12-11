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
		void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;
		Shape* duplicate() const override;
		glm::vec3 GJKsupport(const glm::vec3& direction) const override;
		//

		//	Attributes
		glm::vec3 center;
		float radius;
		//
};