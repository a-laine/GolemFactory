#pragma once

#include <glm/glm.hpp>

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

        glm::vec3 size;
        uint8_t* textureData;
        uint8_t configuration;
        bool isImage;
};

