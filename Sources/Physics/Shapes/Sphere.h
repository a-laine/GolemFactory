#pragma once

#include "Shape.h"

class Sphere : public Shape
{
	public:
		//	Default
		Sphere(const vec4f& position = vec4f(0.f), const float& r = 0.f);
		//

		//	Public functions
		Sphere toSphere() const override;
		AxisAlignedBox toAxisAlignedBox() const override;
		Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual mat4f computeInertiaMatrix() const override;

		virtual void transform(const vec4f& position, const vec4f& scale, const quatf& orientation) override;

		virtual vec4f support(const vec4f& direction) const override;
		virtual void getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const override;
		//

		//	Attributes
		vec4f center;
		float radius;
		//
};