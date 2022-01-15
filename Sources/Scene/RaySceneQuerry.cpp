#include "RaySceneQuerry.h"
#include <Physics/Collision.h>

RaySceneQuerry::RaySceneQuerry(const glm::vec3& pos, const glm::vec3& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}

VirtualSceneQuerry::CollisionType RaySceneQuerry::operator() (const NodeVirtual* node)
{
	//	Segment/AABB collision test
	glm::vec3 s = 1.01f * glm::vec3(node->getAllowanceSize());
	Segment ray(position, position + distance * direction);
	AxisAlignedBox box(node->getBBMin() - s, node->getBBMax() + s);

	if (Collision::collide(&ray, &box))
	{
		result.push_back(node);
		return VirtualSceneQuerry::OVERLAP;
	}
	return VirtualSceneQuerry::NONE;

}
