#pragma once

//#include <glm/glm.hpp>
#include <vector>

#include "Math/TMath.h"


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

		virtual mat4f computeInertiaMatrix() const;

		virtual void transform(const vec4f& position, const vec4f& scale, const quatf& orientation);

		virtual vec4f support(const vec4f& direction) const;
		virtual void getFacingFace(const vec4f& direction, std::vector<vec4f>& points) const;
		//

		// debug
		const char* getTypeStr() const;
		//

		//	Attributes
		ShapeType type;
		//
};
