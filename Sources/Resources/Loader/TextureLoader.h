#pragma once

//#include <glm/glm.hpp>

#include "Math/TMath.h"
#include "Resources/IResourceLoader.h"



class TextureLoader : public IResourceLoader
{
    public:
        TextureLoader();

        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        vec3f m_size;
        uint8_t* m_textureData;
        uint16_t m_configuration;

        unsigned int m_internalFormat;

        bool m_isImage;
};

