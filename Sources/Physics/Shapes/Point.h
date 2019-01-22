#pragma once

#include "Shape.h"

class Point : public Shape
{
	public:
		//	Default
		Point(const glm::vec3& position = glm::vec3(0.f));
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual void operator=(const Shape& s) override;
		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;
		virtual Shape* duplicate() const override;
		//

		//	Attributes
		glm::vec3 p;
		//
};



