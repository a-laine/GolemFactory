#pragma once

#include <map>
//#include <glm/glm.hpp>
#include "Math/TMath.h"

#include <Resources/IResourceLoader.h>
#include <Resources/Font.h>


class FontLoader : public IResourceLoader
{
    public:
        FontLoader();
        
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;

        vec2f size;
        uint8_t* image;
        unsigned short int begin, end, defaultChar;
        std::vector<Font::Patch> charTable;
};

