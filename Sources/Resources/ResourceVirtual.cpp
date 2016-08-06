#include "ResourceVirtual.h"

//  Default
ResourceVirtual::ResourceVirtual(std::string path,std::string resourceName,ResourceType resourceType) :
    name(resourceName),type(resourceType)
{
    count = 0;
}
ResourceVirtual::ResourceVirtual(ResourceType resourceType) :type(resourceType)
{
	count = 0;
}
ResourceVirtual::~ResourceVirtual(){}
//

//  Public functions
bool ResourceVirtual::isValid() const{return true;}
//
