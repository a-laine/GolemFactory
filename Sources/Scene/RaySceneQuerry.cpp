#include "RaySceneQuerry.h"
#include "Physics/Collision.h"

RaySceneQuerry::RaySceneQuerry(const glm::vec3& pos, const glm::vec3& dir, float maxDist) : position(pos), direction(dir), distance(maxDist)
{}

SceneManager::CollisionType RaySceneQuerry::operator() (const NodeVirtual* node)
{
	//	Segment/AABB collision test
	glm::vec3 s = 1.01f * glm::vec3(node->getAllowanceSize());
	if (Collision::collide_SegmentvsAxisAlignedBox(position, position + distance*direction, node->getBBMin() - s, node->getBBMax() + s))
	{
		result.push_back(node);
		return SceneManager::OVERLAP;
	}
	return SceneManager::NONE;

}
std::vector<const NodeVirtual*>& RaySceneQuerry::getResult() { return result; }
