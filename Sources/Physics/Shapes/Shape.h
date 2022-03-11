#pragma once

#include <glm/glm.hpp>
#include <vector>


class AxisAlignedBox;
class Sphere;
class Shape
{
	public:
		//  Miscellaneous
		enum class ShapeType
		{
			NONE = -1,
			POINT,
			SEGMENT,
			TRIANGLE,
			SPHERE,
			AXIS_ALIGNED_BOX,
			ORIENTED_BOX,
			CAPSULE,
			HULL
		};
		//

		//	Default
		Shape(const ShapeType& ShapeType = ShapeType::NONE);
		//

		//	Public functions
		virtual Sphere toSphere() const;
		virtual AxisAlignedBox toAxisAlignedBox() const;
		virtual Shape& operator=(const Shape& s);
		virtual Shape* duplicate() const;

		virtual glm::mat3 computeInertiaMatrix() const;

		virtual void transform(const glm::vec4& position, const glm::vec3& scale, const glm::fquat& orientation);

		virtual glm::vec4 support(const glm::vec4& direction) const;
		virtual void getFacingFace(const glm::vec4& direction, std::vector<glm::vec4>& points) const;
		//

		//	Attributes
		ShapeType type;
		//
};
