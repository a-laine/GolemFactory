#include "VirtualSceneQuerry.h"


VirtualSceneQuerry::CollisionType VirtualSceneQuerry::operator() (const NodeVirtual* node)
{
	result.push_back(node);
	return VirtualSceneQuerry::CollisionType::OVERLAP;
};
std::vector<const NodeVirtual*>& VirtualSceneQuerry::getResult()
{
	return result;
}
void VirtualSceneQuerry::addNodeToResult(const NodeVirtual* _node)
{
	result.push_back(_node);
}
