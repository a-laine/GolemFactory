#pragma once

#include <Resources/IResourceLoader.h>
#include <Resources/Shader.h>
#include <Utiles/Parser/Variant.h>

class MaterialLoader : public IResourceLoader
{
    public:
        // Default
        MaterialLoader();

        bool load(const std::string& directory, const std::string& filename) override;
        void initialize(ResourceVirtual* resource) override;

        void PrintError(const char* filename, const char* msg) override;
        void PrintWarning(const char* filename, const char* msg) override;

        static std::string getFileName(const std::string& resourceDirectory, const std::string& fileName);
        //


    protected:
        void clear();


        std::string m_shaderName;
        int m_maxShadowCascade;
        std::vector<std::pair<std::string, std::string>> m_textureOverride;   //ex : {"albedo", "PolygonDungeon/Grass_01.png"}
        std::vector<Shader::Property> m_properties;
};
