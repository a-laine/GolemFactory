#pragma once

#include "Shape.h"

#include <vector>

class Hull : public Shape
{
	public:
		//	Default
		Hull(const std::vector<glm::vec3>& v, const std::vector<unsigned short>& f);
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual void operator=(const Shape& s) override;
		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;
		virtual Shape* duplicate() const override;
		virtual glm::vec3 GJKsupport(const glm::vec3& direction) const override;
		//

		//	Attributes
		std::vector<glm::vec3> vertices;
		std::vector<unsigned short> faces;
		//
};