#pragma once

#include <Resources/IResourceLoader.h>
#include <Resources/AnimationGraph.h>


class AnimationGraphLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

        void PrintError(const char* filename, const char* msg) override;
        void PrintWarning(const char* filename, const char* msg) override;


    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        unsigned int m_entryState;
        std::vector<AnimationGraphState> m_states;
        std::vector<AnimationGraphTransition> m_transitions;

        std::map<std::string, AnimationGraphData> m_dataVariant;
        std::vector<AnimationGraphParameter> m_parameters;
};