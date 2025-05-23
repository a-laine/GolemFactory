#include "BoxSceneQuerry.h"
#include <Physics/Collision.h>


BoxSceneQuerry::BoxSceneQuerry() : bbMin(0), bbMax(0)
{}

BoxSceneQuerry::BoxSceneQuerry(const vec4f& cornerMin, const vec4f& cornerMax) : bbMin(cornerMin), bbMax(cornerMax)
{}

void BoxSceneQuerry::Set(const vec4f& cornerMin, const vec4f& cornerMax)
{
	bbMin = cornerMin;
	bbMax = cornerMax;
}

bool BoxSceneQuerry::TestAABB(vec4f min, vec4f max)
{
	if (vec4b::any(vec4f::greaterThan(bbMin, max)) || vec4b::any(vec4f::greaterThan(min, bbMax)))
		return false;
	return true;
}

VirtualSceneQuerry::CollisionType BoxSceneQuerry::operator() (const NodeVirtual* node)
{
	const float s = 1.01f * node->getAllowanceSize();
	const vec4f s4 = vec4f(s, s, s, 0);
	if (Collision::collide_AxisAlignedBoxvsAxisAlignedBox(bbMin, bbMax, node->getBBMin() - s4, node->getBBMax() + s4))
	{
		result.push_back(node);
		return VirtualSceneQuerry::CollisionType::OVERLAP;
	}
	return VirtualSceneQuerry::CollisionType::NONE;
}

