#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION

#pragma warning(push, 0)
#include "stb_image.h"
#pragma warning(pop)

#include <Utiles/Parser/Reader.h>
#include <Resources/Texture.h>


uint8_t* ImageLoader::loadFromFile(std::string file,int& width,int& heigth, int& channel, Mode forceChannel)
{
    return stbi_load(file.c_str(),&width,&heigth,&channel,forceChannel);
}

void ImageLoader::freeImage(uint8_t* data)
{
    stbi_image_free(data);
}


ImageLoader::ImageLoader()
    : size(0)
    , textureData(nullptr)
    , configuration(0)
{}

bool ImageLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    int x, y, n;
    textureData = ImageLoader::loadFromFile(getFileName(resourceDirectory, fileName), x, y, n, ImageLoader::RGB_ALPHA);
    if(!textureData)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading texture : " << fileName << " : fail to open file";
        return false;
    }

    size = glm::vec3((float) x, (float) y, 0.f);
	return true;
}

void ImageLoader::initialize(ResourceVirtual* resource)
{
    Texture* texture = static_cast<Texture*>(resource);
    texture->initialize(size, textureData, configuration);
    ImageLoader::freeImage(textureData);
}

void ImageLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{
}

std::string ImageLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Texture::directory;
    str += fileName;
    return str;
}

