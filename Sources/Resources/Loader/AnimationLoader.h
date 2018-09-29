#pragma once

#include <map>

#include "Resources/IResourceLoader.h"
#include "Resources/Joint.h"


class AnimationLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        std::vector<KeyFrame> timeLine;
        std::map<std::string, KeyLabel> labels;
};

