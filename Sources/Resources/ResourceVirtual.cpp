#include "ResourceVirtual.h"

//	Static attributes
ResourceVirtual::VerboseLevel ResourceVirtual::logVerboseLevel = ResourceVirtual::VerboseLevel::NONE;
//

//  Default
ResourceVirtual::ResourceVirtual(const std::string& resourceName, ResourceType resourceType)
    : name(resourceName)
    , count(0)
    , state(INVALID)
    , type(resourceType)
{
}
ResourceVirtual::ResourceVirtual()
    : name("unknown")
    , count(0)
    , state(INVALID)
    , type(ResourceType::NONE)
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

void ResourceVirtual::onDrawImGui()
{
#ifdef USE_IMGUI
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Sharable infos");
    ImGui::Text("Name : %s", name.c_str());
    ImGui::Text("Reference count : %d", count.load());
    ImGui::Spacing();
#endif
}

//	Protected functions
void ResourceVirtual::printErrorLog(const std::string& resourceName, const int& errorLine, bool& printHeader)
{
	if (printHeader && logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
		std::cerr << "WARNING : loading resource : " << resourceName << " : wrong number of argument successfully parsed :" << std::endl;
	if (logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
		std::cerr << "   check line : " << errorLine << std::endl;
	printHeader = true;
}
//