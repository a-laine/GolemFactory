#pragma once

#include <glm/glm.hpp>

#include "Resources/IResourceLoader.h"
#include "Resources/Joint.h"



class SkeletonLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        std::vector<unsigned int> roots;
        std::vector<Joint> joints;
};

