#include "ResourceVirtual.h"

//	Static attributes
ResourceVirtual::VerboseLevel ResourceVirtual::logVerboseLevel = (ResourceVirtual::VerboseLevel) ResourceVirtual::NONE;
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

//	Protected functions
void ResourceVirtual::printErrorLog(std::string resourceName, int errorLine, bool& printHeader)
{
	if (printHeader && logVerboseLevel >= ResourceVirtual::WARNINGS)
		std::cerr << "WARNING : loading mesh : " << resourceName << " : wrong number of argument successfully parsed :" << std::endl;
	if (logVerboseLevel >= ResourceVirtual::WARNINGS)
		std::cerr << "   check line : " << errorLine << std::endl;
	printHeader = true;
}
//