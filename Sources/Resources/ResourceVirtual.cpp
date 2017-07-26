#include "ResourceVirtual.h"

//	Static attributes
int ResourceVirtual::logVerboseLevel = 0;
//

//  Default
ResourceVirtual::ResourceVirtual(const std::string& resourceName, ResourceType resourceType) : name(resourceName), type(resourceType)
{
    count = 0;
}
ResourceVirtual::ResourceVirtual(ResourceType resourceType) : name("unknown"), type(resourceType)
{
	count = 0;
}
ResourceVirtual::~ResourceVirtual(){}
//

//  Public functions
bool ResourceVirtual::isValid() const
{
	return true;
}
//
