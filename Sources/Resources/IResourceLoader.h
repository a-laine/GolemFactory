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

        virtual void PrintError(const char* filename, const char* msg) {};
        virtual void PrintWarning(const char* filename, const char* msg) {};
};

