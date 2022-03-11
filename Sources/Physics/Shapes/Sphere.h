#pragma once

#include "Shape.h"

class Sphere : public Shape
{
	public:
		//	Default
		Sphere(const glm::vec4& position = glm::vec4(0.f), const float& r = 0.f);
		//

		//	Public functions
		Sphere toSphere() const override;
		AxisAlignedBox toAxisAlignedBox() const override;
		Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation) override;

		virtual glm::vec4 support(const glm::vec4& direction) const override;
		virtual void getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const override;
		//

		//	Attributes
		glm::vec4 center;
		float radius;
		//
};