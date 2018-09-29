#include "ResourceManager.h"

#include <Utiles/ToolBox.h>


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
    return resources.size();
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
