#pragma once

#include <Utiles/Singleton.h>
#include <Utiles/Mutex.h>
#include <Utiles/Assert.hpp>
#include <Utiles/ToolBox.h>
#include "ResourceVirtual.h"
#include "IResourceLoader.h"

#include <vector>
#include <map>
#include <string>




class World;

class ResourceManager : public Singleton<ResourceManager>
{
    friend class Singleton<ResourceManager>;

    public:
        //  Public functions
		void release(ResourceVirtual* resource);
        void clearGarbage();

        template<typename T, typename... Args>
        T* getResource(const std::string& name, Args&&... args);
		template<typename T>
		T* findResource(const std::string& name);
		template<typename T>
		bool loadableResource(const std::string& name);
        template<typename T>
        T* getResource(T* resource);

        void addResource(ResourceVirtual* resource);
        template<typename T, typename... Args>
        void addDefaultResource(const std::string& name, Args&&... args);

        void addNewResourceLoader(const std::string& id, IResourceLoader* loader);
        //

        //  Set/get functions
		void setRepository(const std::string& path);
		std::string getRepository() const;

        unsigned int getNumberOfRessources() const;
        //

        //
        void drawImGui(World& world);
        std::vector<std::string> getAllResourceName(ResourceVirtual::ResourceType type);
        //

    private:
        //  Default
        ResourceManager(const std::string& path = "Resources/");	//!< Default constructor.
        ~ResourceManager();											//!< Default destructor.
        //

		//  Private functions
        void loadResource_internal(ResourceVirtual* resource, const std::string& fileName, const std::string& loaderId);
        bool addResource_internal(ResourceVirtual* resource);
        ResourceVirtual* findResource_internal(const std::string& identifier);
        IResourceLoader* findLoader_internal(const std::string& loaderId);
		//

        //  Attributes
        Mutex mutexGarbage;								//!< A mutex to prevent garbage collision.
        std::vector<ResourceVirtual*> garbage;			//!< The list of resources to delete.

        Mutex mutexList;								//!< A mutex to prevent lists collisions.
        std::map<std::string, ResourceVirtual*> resources;

        std::string repository;							//!< The repository path.
        std::map<std::string, IResourceLoader*> loaders;
        //


#ifdef USE_IMGUI
        std::map<ResourceVirtual::ResourceType, ResourceVirtual*> selectedResources;
        ImGuiTextFilter m_nameFilter;
#endif // USE_IMGUI
};



template<typename T, typename... Args>
T* ResourceManager::getResource(const std::string& name, Args&&... args)
{
    const std::string& realName = (name == "default") ? T::getDefaultName() : name;
    T* resource = static_cast<T*>(findResource_internal(T::getIdentifier(realName)));

    if(!resource)
    {
        resource = new T(realName, std::forward<Args>(args) ...);
        bool inserted = addResource_internal(resource);
        GF_ASSERT(!inserted, "Resource with same name doesn't correspond");

        std::string loaderId = resource->getLoaderId(realName);
		loadResource_internal(resource, realName, loaderId);
		if(!resource->isValid())
		{
            loaderId = resource->getLoaderId(T::getDefaultName());
			loadResource_internal(resource, T::getDefaultName(), loaderId);
		}
    }

    resource->count++;
    return resource;
}

template<typename T>
T* ResourceManager::findResource(const std::string& name)
{
	const std::string& realName = (name == "default") ? T::getDefaultName() : name;
	T* resource = static_cast<T*>(findResource_internal(T::getIdentifier(realName)));

	if (!resource)
		return nullptr;
	else
	{
		resource->count++;
		return resource;
	}
}

template<typename T>
bool ResourceManager::loadableResource(const std::string& name)
{
	const std::string& realName = (name == "default") ? T::getDefaultName() : name;
	return ToolBox::isPathExist(repository + T::getIdentifier(realName));
}

template<typename T>
T* ResourceManager::getResource(T* resource)
{
    GF_ASSERT(resource);
    bool inserted = addResource_internal(resource);
    GF_ASSERT(!inserted, "Resource with same name doesn't correspond or 'emplace' error");
    resource->count++;
    return resource;
}

template<typename T, typename... Args>
void ResourceManager::addDefaultResource(const std::string& name, Args&&... args)
{
    T* resource = new T(name, std::forward<Args>(args) ...);
    bool inserted = addResource_internal(resource);
    GF_ASSERT(!inserted, "Resource with same name doesn't correspond");
    resource->count++;

    std::string loaderId = resource->getLoaderId(name);
    loadResource_internal(resource, name, loaderId);
}

