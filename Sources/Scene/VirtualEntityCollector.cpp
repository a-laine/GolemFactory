#include "VirtualEntityCollector.h"

bool VirtualEntityCollector::operator() (Entity* entity)
{
	bool ok = entity->getFlags() & m_flags;
	bool nok = entity->getFlags() & m_exclusionFlags;
	if (ok && !nok)
		result.push_back(entity);
	return true;
}
std::vector<Entity*>& VirtualEntityCollector::getResult()
{
	return result;
}