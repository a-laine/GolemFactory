#include "Physics/Collision.h"
#include "CollisionUtils.h"

//#include "CollisionAxisAlignedBox.h"


//#include <Physics/SpecificIntersection/IntersectionSegment.h>

//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/component_wise.hpp>
//#include <glm/gtx/norm.hpp>


//	Specialized functions : axis aligned box
/*bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max)
{
	if (glm::any(glm::greaterThan(box1Min, box2Max)) || glm::any(glm::greaterThan(box2Min, box1Max)))
		return false;
	return true;
}*/
bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const vec4f& box1Min, const vec4f& box1Max, const vec4f& box2Min, const vec4f& box2Max, CollisionReport* report)
{
	if (vec4b::any(vec4f::greaterThan(box1Min, box2Max)) || vec4b::any(vec4f::greaterThan(box2Min, box1Max)))
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			vec4f center1 = 0.5f * (box1Max + box1Min);
			vec4f center2 = 0.5f * (box2Max + box2Min);
			vec4f size1 = 0.5f * (box1Max - box1Min);
			vec4f size2 = 0.5f * (box2Max - box2Min);
			vec4f v = center1 - center2;
			vec4f delta = vec4f::abs(v) - size1 - size2;

			report->depths.push_back(std::numeric_limits<float>::max());
			if (delta.x < 0.f && -delta.x < report->depths.back())
			{
				report->depths.back() = delta.x;
				report->normal = vec4f(v.x > 0.f ? 1 : -1, 0, 0, 1);
			}
			if (delta.y < 0.f && -delta.y < report->depths.back())
			{
				report->depths.back() = delta.y;
				report->normal = vec4f(0, v.y > 0.f ? 1 : -1, 0, 1);
			}
			if (delta.z < 0.f && -delta.z < report->depths.back())
			{
				report->depths.back() = delta.z;
				report->normal = vec4f(0, 0, v.z > 0.f ? 1 : -1, 1);
			}
			report->points.push_back(center1 + report->normal * vec4f::dot(report->normal, size1));
		}
		return true;
	}
}
//