#pragma once

#include <string>
#include <cstdint>
//#include <glm/glm.hpp>

#include "Math/TMath.h"
#include "Resources/IResourceLoader.h"


class ImageLoader : public IResourceLoader
{
    public:
        enum Mode {
            DEFAULT = 0, // only used for req_comp

            GREY       = 1,
            GREY_ALPHA = 2,
            RGB        = 3,
            RGB_ALPHA  = 4
        };
        static uint8_t* loadFromFile(std::string file,int& width,int& heigth, int& channel, Mode forceChannel);
        static void freeImage(uint8_t* data);


        ImageLoader();

        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;


    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        vec3f size;
        uint8_t* textureData;
        uint8_t configuration;
};
