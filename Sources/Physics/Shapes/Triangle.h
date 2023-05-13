#pragma once

#include "Shape.h"


class Triangle : public Shape
{
	public:
		//	Default
		Triangle(const vec4f& a = vec4f(0.f), const vec4f& b = vec4f(0.f), const vec4f& c = vec4f(0.f));
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		//virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const vec4f& position, const vec4f& scale, const quatf& orientation) override;

		virtual vec4f support(const vec4f& direction) const override;
		virtual void getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const override;
		//

		//	Attributes
		vec4f p1, p2, p3;
		//
};