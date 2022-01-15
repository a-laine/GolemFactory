#pragma once

#include "Shape.h"

class AxisAlignedBox : public Shape
{
	public:
		//	Default
		AxisAlignedBox(const glm::vec3& cornerMin = glm::vec3(0.f), const glm::vec3& cornerMax = glm::vec3(0.f));
		//

		//	Override
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;

		virtual glm::vec3 support(const glm::vec3& direction) const override;
		virtual void getFacingFace(const glm::vec3& direction, std::vector<glm::vec3>& points) const override;
		//

		//	AABB specific
		void add(const AxisAlignedBox& _other);
		//

		//	Attributes
		glm::vec3 min, max;
		//
};
