#include "KeyMapping.h"


char const * const KeyMapping::directory = "Config/";
char const * const KeyMapping::extension = ".json";
std::string KeyMapping::defaultName;

KeyMapping::KeyMapping(const std::string& fileName)
    : ResourceVirtual(fileName)
{
}

const std::multimap<std::string, Event*>& KeyMapping::getList() const
{
    return mapping;
}

void KeyMapping::initialize(const std::multimap<std::string, Event*>& list)
{
    mapping = list;
}

void KeyMapping::initialize(std::multimap<std::string, Event*>&& list)
{
    mapping = std::move(list);
}

std::string KeyMapping::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}

std::string KeyMapping::getIdentifier() const
{
    return getIdentifier(name);
}

std::string KeyMapping::getLoaderId(const std::string& resourceName) const
{
    return "keyMapping";
}

const std::string& KeyMapping::getDefaultName()
{
    return defaultName;
}

void KeyMapping::setDefaultName(const std::string& name)
{
    defaultName = name;
}
