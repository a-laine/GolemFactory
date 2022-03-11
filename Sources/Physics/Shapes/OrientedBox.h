#pragma once

#include "Shape.h"

class OrientedBox : public Shape
{
	public:
		//	Default
		OrientedBox(const glm::mat4& transformationMatrix = glm::mat4(1.f), const glm::vec4& localMin = glm::vec4(0.f), const glm::vec4& localMax = glm::vec4(0.f));
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation) override;

		virtual glm::vec4 support(const glm::vec4& direction) const override;
		virtual void getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const override;
		//

		//	Attributes
		glm::mat4 base;
		glm::vec4 min, max;
		//
};