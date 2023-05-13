#pragma once

#include "Shape.h"

#include <vector>
#include <list>
#include <map>

class Mesh;

class Hull : public Shape
{
	public:
		//	Default
		explicit Hull(Mesh* m, mat4f transform = mat4f::identity);
		~Hull();
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
		Mesh* mesh;
		mat4f base;
		//

};