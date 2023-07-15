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

        void loadStateMachine(const std::string& fileName, Variant& variant, std::vector<AG::StateMachine>& targetArray, std::vector<std::string>& _subGrapTarget);
        void loadStateMachineData(const std::string& fileName, Variant& variant, std::vector<AG::StateMachine::StateMachineData>& targetData);
        void loadBlendTree(const std::string& fileName, Variant& variant, std::vector<AG::BlendTree>& targetArray, std::vector<std::string>& _subGrapTarget);
        void loadBlendTreeData(const std::string& fileName, Variant& variant, std::vector<AG::BlendTree::BlendTreeData>& targetData);

        std::map<std::string, int> paramIds;

        std::vector<AnimationGraph::Layer> m_layers;
        std::map<std::string, std::vector<AnimationGraph::LayerData>> m_variants;
        std::vector<AnimationParameter> m_parameters;
};