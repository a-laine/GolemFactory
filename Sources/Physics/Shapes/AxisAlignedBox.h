#pragma once

#include "Shape.h"

class AxisAlignedBox : public Shape
{
	public:
		//	Default
		AxisAlignedBox(const vec4f& cornerMin = vec4f(0.f), const vec4f& cornerMax = vec4f(0.f));
		//

		//	Override
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual mat4f computeInertiaMatrix() const override;

		virtual void transform(const vec4f& position, const vec4f& scale, const quatf& orientation) override;

		virtual vec4f support(const vec4f& direction) const override;
		virtual void getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const override;
		//

		//	AABB specific
		void add(const AxisAlignedBox& _other);
		//

		//	Attributes
		vec4f min, max;
		//
};
