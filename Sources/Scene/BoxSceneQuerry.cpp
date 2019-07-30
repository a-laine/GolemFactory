#include "BoxSceneQuerry.h"
#include "Physics/Collision.h"


BoxSceneQuerry::BoxSceneQuerry(const glm::vec3& cornerMin, const glm::vec3& cornerMax) : bbMin(cornerMin), bbMax(cornerMax)
{}
VirtualSceneQuerry::CollisionType BoxSceneQuerry::operator() (const NodeVirtual* node)
{
	glm::vec3 s = 1.01f * glm::vec3(node->getAllowanceSize());
	if (Collision::collide_AxisAlignedBoxvsAxisAlignedBox(bbMin, bbMax, node->getBBMin() - s, node->getBBMax() + s))
	{
		result.push_back(node);
		return VirtualSceneQuerry::OVERLAP;
	}
	return VirtualSceneQuerry::NONE;
}

