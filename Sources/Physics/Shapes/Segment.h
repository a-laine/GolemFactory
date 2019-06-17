#pragma once

#include "Shape.h"

class Segment : public Shape
{
	public:
		//	Default
		Segment(const glm::vec3& a = glm::vec3(0.f), const glm::vec3& b = glm::vec3(0.f));
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
		glm::vec3 p1, p2;
		//
};
