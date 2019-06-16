#pragma once

#include "Resources/KeyMapping.h"
#include "Resources/IResourceLoader.h"


class KeyMappingLoader : public IResourceLoader
{
    public:
        KeyMappingLoader();

        bool load(const std::string& resourceDirectory, const std::string& fileName) override { return true; }
        void initialize(ResourceVirtual* resource) override {}
        /*
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;*/
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        //std::multimap<std::string, Event*> eventMap;
};
