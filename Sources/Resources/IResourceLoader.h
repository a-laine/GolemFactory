#pragma once

#include <string>
#include <vector>


class ResourceVirtual;

class IResourceLoader
{
    public:
        virtual ~IResourceLoader() = default;

        virtual bool load(const std::string& directory, const std::string& filename) = 0;
        virtual void initialize(ResourceVirtual* resource) = 0;
        virtual void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) = 0;
};

