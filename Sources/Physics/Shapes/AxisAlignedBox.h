#pragma once

#include "Shape.h"

class AxisAlignedBox : public Shape
{
	public:
		//	Default
		AxisAlignedBox(const glm::vec4& cornerMin = glm::vec4(0.f), const glm::vec4& cornerMax = glm::vec4(0.f));
		//

		//	Override
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation) override;

		virtual glm::vec4 support(const glm::vec4& direction) const override;
		virtual void getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const override;
		//

		//	AABB specific
		void add(const AxisAlignedBox& _other);
		//

		//	Attributes
		glm::vec4 min, max;
		//
};
