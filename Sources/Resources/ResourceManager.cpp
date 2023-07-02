#include "ResourceManager.h"
#include "Shader.h"
#include "Loader/ShaderLoader.h"

#include <Utiles/ToolBox.h>
#include <World/World.h>
#include <Utiles/ProfilerConfig.h>

#ifdef USE_IMGUI
bool ResourcesWindowEnable = true;
#endif

//  Default
ResourceManager::ResourceManager(const std::string& path)
{
    setRepository(path);
}
ResourceManager::~ResourceManager()
{
    for (auto& element : resources)
        delete element.second;
    resources.clear();

    for (auto& element : loaders)
        delete element.second;
    loaders.clear();

    clearGarbage();
}
//

//  Public functions
void ResourceManager::release(ResourceVirtual* resource)
{
    //	prevent fail & decrement resources user counter
    if(!resource)  return;
	resource->count--;

	if (resource->count <= 0)
    {
		//	remove resource from avalaible ressources lists
        mutexList.lock();
        resources.erase(resource->getIdentifier());
        mutexList.unlock();

		//	add resources to garbage for delayed deletion
        mutexGarbage.lock();
        garbage.push_back(resource);
        mutexGarbage.unlock();
    }
}
void ResourceManager::clearGarbage()
{
	//	instanciate the dummy list
    std::vector<ResourceVirtual*> garbageCopy;

	//	swap dummy list with garbage
    mutexGarbage.lock();
	garbage.swap(garbageCopy);
    mutexGarbage.unlock();

	//	delete element in garbage list
	for (unsigned int i = 0; i < garbageCopy.size(); i++)
		if (garbageCopy[i]) delete garbageCopy[i];
    garbageCopy.clear();
}


void ResourceManager::addResource(ResourceVirtual* resource)
{
    GF_ASSERT(resource);
    bool inserted = addResource_internal(resource);
    GF_ASSERT(!inserted, "Resource with same name doesn't correspond");
    resource->count++;
}

void ResourceManager::addNewResourceLoader(const std::string& id, IResourceLoader* loader)
{
    GF_ASSERT(loader);
    auto it = loaders.find(id);
    if(it != loaders.end())
    {
        delete it->second;
        it->second = loader;
    }
    else
    {
        loaders.emplace(id, loader);
    }
}
//

//  Set/get functions
void ResourceManager::setRepository(const std::string& path)
{
    repository = path;
}
std::string ResourceManager::getRepository() const
{
    return repository;
}
unsigned int ResourceManager::getNumberOfRessources() const
{
    return (int)resources.size();
}
//

//  Private functions
void ResourceManager::loadResource_internal(ResourceVirtual* resource, const std::string& fileName, const std::string& loaderId)
{
    if(fileName.empty())
        return;

    IResourceLoader* loader = findLoader_internal(loaderId);
    if(!loader || !loader->load(getRepository(), fileName))
        return;

    loader->initialize(resource);

    std::vector<ResourceVirtual*> extraResources;
    loader->getResourcesToRegister(extraResources);
    for(ResourceVirtual* res : extraResources)
    {
        if(!addResource_internal(res))
            delete res;
    }
}
bool ResourceManager::addResource_internal(ResourceVirtual* resource)
{
    mutexList.lock();
    auto result = resources.emplace(resource->getIdentifier(), resource);
    bool ok = result.second || result.first->second == resource;
    mutexList.unlock();
    return ok;
}
ResourceVirtual* ResourceManager::findResource_internal(const std::string& identifier)
{
    ResourceVirtual* result = nullptr;
    mutexList.lock();
    auto it = resources.find(identifier);
    if(it != resources.end())
        result = it->second;
    mutexList.unlock();
    return result;
}
IResourceLoader* ResourceManager::findLoader_internal(const std::string& loaderId)
{
    auto it = loaders.find(loaderId);
    if(it != loaders.end())
        return it->second;
    else
        return nullptr;
}
//

//
std::vector<std::string> ResourceManager::getAllResourceName(ResourceVirtual::ResourceType type)
{
    std::vector<std::string> result;
    for (auto it = resources.begin(); it != resources.end(); it++)
    {
        if (it->second->getType() != type)
            continue;
        result.push_back(it->second->name);
    }
    return result;
}
void ResourceManager::drawImGui(World& world)
{
#ifdef USE_IMGUI
    SCOPED_CPU_MARKER("ResourceManager");

    mutexList.lock();
    ImGui::Begin("Resource manager");
    ImGui::PushID(this);
    const ImVec4 sectionColor = ImVec4(1, 1, 0, 1);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Filters");
    m_nameFilter.Draw();

    ImGui::Spacing(); ImGui::Spacing();

    const char* TabNames[] = { "Meshes", "Materials",  "Textures" ,"Shaders", "Skeletons", "Animations", "Fonts" };
    ResourceVirtual::ResourceType TabTypes[] = { ResourceVirtual::ResourceType::MESH, ResourceVirtual::ResourceType::MATERIAL,
        ResourceVirtual::ResourceType::TEXTURE, ResourceVirtual::ResourceType::SHADER, ResourceVirtual::ResourceType::SKELETON,
        ResourceVirtual::ResourceType::ANIMATION, ResourceVirtual::ResourceType::FONT };
    
    if (ImGui::BeginTabBar("ResourceTabBar", ImGuiTabBarFlags_None))
    {
        int TabLength = sizeof(TabTypes) / sizeof(ResourceVirtual::ResourceType);

        for (int i = 0; i < TabLength; i++)
        {
            if (ImGui::BeginTabItem(TabNames[i]))
            {
                auto selectedIt = selectedResources.find(TabTypes[i]);
                ResourceVirtual* selectedRes = selectedIt != selectedResources.end() ? selectedIt->second : nullptr;

                ImGui::BeginChild("ResourceList", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
                {
                    bool noResourceLoaded = true;
                    for (const auto& it : resources)
                    {
                        if (!it.second || it.second->getType() != TabTypes[i])
                            continue;
                        if (!m_nameFilter.PassFilter(it.second->getIdentifier().c_str()))
                            continue;

                        noResourceLoaded = false;
                        bool b = selectedRes == it.second;

                        bool popColor = false;
                        if (TabTypes[i] == ResourceVirtual::ResourceType::TEXTURE)
                        {
                            Texture* tex = (Texture*)it.second;
                            if (tex->isEnginePrivate)
                            {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.5, 0, 1));
                                popColor = true;
                            }
                        }

                        if (ImGui::Selectable(it.second->name.c_str(), &b))
                            selectedResources[TabTypes[i]] = it.second;
                        if (popColor)
                            ImGui::PopStyleColor();
                    }

                    if (noResourceLoaded)
                    {
                        ImGui::TextDisabled("(no loaded resource of this type, or none had pass the name filter)");
                    }

                    ImGui::EndChild();
                }

                ImGui::SameLine();
                ImGui::BeginChild("ResourceInfos", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
                {
                    if (selectedRes)
                        selectedRes->onDrawImGui();
                    else
                        ImGui::TextDisabled("(select an item)");
                    ImGui::EndChild();
                }

                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }


    ImGui::PopID();
    ImGui::End();
    mutexList.unlock();
#endif // USE_IMGUI
}
//