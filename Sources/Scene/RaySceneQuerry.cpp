#include "RaySceneQuerry.h"
#include "Physics/Collision.h"

RaySceneQuerry::RaySceneQuerry(const vec4f& pos, const vec4f& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}

VirtualSceneQuerry::CollisionType RaySceneQuerry::operator() (const NodeVirtual* node)
{
	//	Segment/AABB collision test
	const float s = 1.01f * node->getAllowanceSize();
	vec4f s4 = vec4f(s, s, s, 0);
	Segment ray(position, position + distance * direction);
	AxisAlignedBox box(node->getBBMin() - s4, node->getBBMax() + s4);

	if (Collision::collide(&ray, &box))
	{
		result.push_back(node);
		return VirtualSceneQuerry::CollisionType::OVERLAP;
	}
	return VirtualSceneQuerry::CollisionType::NONE;

}
