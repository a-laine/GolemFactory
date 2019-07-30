#pragma once

#include "Shape.h"

class OrientedBox : public Shape
{
	public:
		//	Default
		OrientedBox(const glm::mat4& transformationMatrix = glm::mat4(1.f), const glm::vec3& localMin = glm::vec3(0.f), const glm::vec3& localMax = glm::vec3(0.f));
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;
		virtual Shape* duplicate() const override;
		virtual glm::vec3 GJKsupport(const glm::vec3& direction) const override;
		//

		//	Attributes
		glm::mat4 base;
		glm::vec3 min, max;
		//
};