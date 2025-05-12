#pragma once

#include "ResourceVirtual.h"
#include "Shader.h"
#include "Texture.h"

class Material : public ResourceVirtual
{
    friend class MaterialLoader;
    public:
        static char const* const directory;
        static char const* const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        //  Default
        Material(const std::string& meshName = "unknown");
        virtual ~Material();

        void initialize(const std::string& _shaderName, int _maxShadowCascade);
        void setTextureOverride(const std::vector<std::pair<std::string, std::string>>& _overrides);
        void copyPropertiesFromShader();

        std::string getIdentifier() const;
        std::string getLoaderId(const std::string& resourceName) const;
        Shader* getShader();
        int getMaxShadowCascade() const;
        const std::vector<Texture*>& getTextures() const;
        const std::vector<Shader::Property>& getProperties() const;
        bool isValid() const override;

        void onDrawImGui() override;
        //


    private:
        static std::string defaultName;

        Shader* m_shader;
        std::vector<Texture*> m_textures;
        int m_shadowMaxCascade;
        std::vector<Shader::Property> m_properties;
};