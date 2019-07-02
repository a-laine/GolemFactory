#include "VirtualEntityCollector.h"

bool VirtualEntityCollector::operator() (Entity* entity)
{
	result.push_back(entity);
	return true;
}
std::vector<Entity*>& VirtualEntityCollector::getResult()
{
	return result;
}