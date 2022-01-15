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
		explicit Hull(Mesh* m, glm::mat4 transform = glm::mat4(1.f));
		~Hull();
		//

		//	Public functions
		virtual Sphere toSphere() const override;
		virtual AxisAlignedBox toAxisAlignedBox() const override;
		virtual Shape& operator=(const Shape& s) override;
		virtual Shape* duplicate() const override;

		//virtual glm::mat3 computeInertiaMatrix() const override;

		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation) override;

		virtual glm::vec3 support(const glm::vec3& direction) const override;
		virtual void getFacingFace(const glm::vec3& direction, std::vector<glm::vec3>& points) const override;
		//

		//	Attributes
		Mesh* mesh;
		glm::mat4 base;
		//

};