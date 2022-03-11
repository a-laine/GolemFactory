#include "BoxSceneQuerry.h"
#include <Physics/Collision.h>


BoxSceneQuerry::BoxSceneQuerry(const glm::vec4& cornerMin, const glm::vec4& cornerMax) : bbMin(cornerMin), bbMax(cornerMax)
{}
VirtualSceneQuerry::CollisionType BoxSceneQuerry::operator() (const NodeVirtual* node)
{
	const float s = 1.01f * node->getAllowanceSize();
	const glm::vec4 s4 = glm::vec4(s, s, s, 0);
	if (Collision::collide_AxisAlignedBoxvsAxisAlignedBox(bbMin, bbMax, node->getBBMin() - s4, node->getBBMax() + s4))
	{
		result.push_back(node);
		return VirtualSceneQuerry::OVERLAP;
	}
	return VirtualSceneQuerry::NONE;
}

