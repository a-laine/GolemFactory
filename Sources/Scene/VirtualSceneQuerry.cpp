#include "VirtualSceneQuerry.h"


VirtualSceneQuerry::CollisionType VirtualSceneQuerry::operator() (const NodeVirtual* node)
{
	result.push_back(node);
	return VirtualSceneQuerry::OVERLAP;
};
std::vector<const NodeVirtual*>& VirtualSceneQuerry::getResult()
{
	return result;
}
