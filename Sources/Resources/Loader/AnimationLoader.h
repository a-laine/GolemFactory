#pragma once

#include <map>

#include <Resources/IResourceLoader.h>
#include <Resources/AnimationClip.h>


class AnimationLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

        void PrintError(const char* filename, const char* msg) override;
        void PrintWarning(const char* filename, const char* msg) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        float m_duration;
        std::vector<AnimationClip::BoneCurves> m_curves;
};

