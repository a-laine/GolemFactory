#pragma once

#include <glm/glm.hpp>


class AxisAlignedBox;
class Sphere;
class Shape
{
	public:
		//  Miscellaneous
		enum ShapeType
		{
			NONE = -1,
			POINT,
			SEGMENT,
			TRIANGLE,
			ORIENTED_BOX,
			AXIS_ALIGNED_BOX,
			SPHERE,
			CAPSULE,
			HULL
		};
		//

		//	Default
		Shape(const ShapeType& ShapeType = NONE);
		//

		//	Public functions
		virtual Sphere toSphere() const;
		virtual AxisAlignedBox toAxisAlignedBox() const;
		virtual Shape& operator=(const Shape& s);
		virtual void transform(const glm::vec3& position, const glm::vec3& scale, const glm::fquat& orientation);
		virtual Shape* duplicate() const;
		virtual glm::vec3 GJKsupport(const glm::vec3& direction) const;
		//

		//	Attributes
		ShapeType type;
		//
};
