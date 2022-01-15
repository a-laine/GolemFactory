#include <Physics/Collision.h>
#include "CollisionUtils.h"

//#include "CollisionAxisAlignedBox.h"


//#include <Physics/SpecificIntersection/IntersectionSegment.h>

//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/component_wise.hpp>
//#include <glm/gtx/norm.hpp>


//	Specialized functions : axis aligned box
bool Collision::collide_AxisAlignedBoxvsAxisAlignedBox(const glm::vec3& box1Min, const glm::vec3& box1Max, const glm::vec3& box2Min, const glm::vec3& box2Max, CollisionReport* report)
{
	if (glm::any(glm::greaterThan(box1Min, box2Max)) || glm::any(glm::greaterThan(box2Min, box1Max)))
		return false;
	else
	{
		if (report)
		{
			report->collision = true;

			glm::vec3 center1 = 0.5f * (box1Max + box1Min);
			glm::vec3 center2 = 0.5f * (box2Max + box2Min);
			glm::vec3 size1 = 0.5f * (box1Max - box1Min);
			glm::vec3 size2 = 0.5f * (box2Max - box2Min);
			glm::vec3 v = center1 - center2;
			glm::vec3 delta = glm::abs(v) - size1 - size2;

			report->depths.push_back(std::numeric_limits<float>::max());
			if (delta.x < 0.f && -delta.x < report->depths.back())
			{
				report->depths.back() = delta.x;
				report->normal = glm::vec3(v.x > 0.f ? 1 : -1, 0, 0);
			}
			if (delta.y < 0.f && -delta.y < report->depths.back())
			{
				report->depths.back() = delta.y;
				report->normal = glm::vec3(0, v.y > 0.f ? 1 : -1, 0);
			}
			if (delta.z < 0.f && -delta.z < report->depths.back())
			{
				report->depths.back() = delta.z;
				report->normal = glm::vec3(0, 0, v.z > 0.f ? 1 : -1);
			}
			report->points.push_back(center1 + report->normal * glm::dot(report->normal, size1));
		}
		return true;
	}
}
//