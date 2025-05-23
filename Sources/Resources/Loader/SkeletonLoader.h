#pragma once

//#include <glm/glm.hpp>

#include <Resources/IResourceLoader.h>
#include <Resources/Joint.h>
#include <Resources/Skeleton.h>



class SkeletonLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        std::vector<unsigned int> roots;
        std::vector<Skeleton::Bone> bones;
};

