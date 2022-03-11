#include "RaySceneQuerry.h"
#include "Physics/Collision.h"

RaySceneQuerry::RaySceneQuerry(const glm::vec4& pos, const glm::vec4& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}

VirtualSceneQuerry::CollisionType RaySceneQuerry::operator() (const NodeVirtual* node)
{
	//	Segment/AABB collision test
	const float s = 1.01f * node->getAllowanceSize();
	glm::vec4 s4 = glm::vec4(s, s, s, 0);
	Segment ray(position, position + distance * direction);
	AxisAlignedBox box(node->getBBMin() - s4, node->getBBMax() + s4);

	if (Collision::collide(&ray, &box))
	{
		result.push_back(node);
		return VirtualSceneQuerry::OVERLAP;
	}
	return VirtualSceneQuerry::NONE;

}
