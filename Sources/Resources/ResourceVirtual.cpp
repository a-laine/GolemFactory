#include "ResourceVirtual.h"

//	Static attributes
ResourceVirtual::VerboseLevel ResourceVirtual::logVerboseLevel = ResourceVirtual::NONE;
//

//  Default
ResourceVirtual::ResourceVirtual(const std::string& resourceName)
    : name(resourceName)
    , count(0)
    , state(INVALID)
{
}
ResourceVirtual::ResourceVirtual()
    : name("unknown")
    , count(0)
    , state(INVALID)
{
}
ResourceVirtual::~ResourceVirtual() {}
//

//  Public functions
bool ResourceVirtual::isValid() const
{
	return state == VALID;
}

std::string ResourceVirtual::getIdentifier() const
{
    return name;
}

void ResourceVirtual::assign(const ResourceVirtual* other)
{
    State s = other->state;
    state = s;
}
//

//	Protected functions
void ResourceVirtual::printErrorLog(const std::string& resourceName, const int& errorLine, bool& printHeader)
{
	if (printHeader && logVerboseLevel >= ResourceVirtual::WARNINGS)
		std::cerr << "WARNING : loading mesh : " << resourceName << " : wrong number of argument successfully parsed :" << std::endl;
	if (logVerboseLevel >= ResourceVirtual::WARNINGS)
		std::cerr << "   check line : " << errorLine << std::endl;
	printHeader = true;
}
//