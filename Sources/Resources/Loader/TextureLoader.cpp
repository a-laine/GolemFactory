#include "TextureLoader.h"
#include "ImageLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Resources/Texture.h>



TextureLoader::TextureLoader() : size(0) , textureData(nullptr) , configuration(0) , isImage(false)
{}

bool TextureLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    //  Initialization
    Variant v; Variant* tmp = nullptr;
    try
    {
        Reader::parseFile(v, getFileName(resourceDirectory, fileName));
        tmp = &(v.getMap().begin()->second);
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading texture : " << fileName << " : fail to open or parse file" << std::endl;
        return false;
    }
    Variant& textureInfo = *tmp;

    //  Configuration
    try
    {
        if(textureInfo["type"].toString() == "D1")      
            configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_1D;
        else if(textureInfo["type"].toString() == "D2") 
            configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_2D;
        else if(textureInfo["type"].toString() == "D3") 
            configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_3D;
        else 
            throw std::exception();
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading texture : " << fileName << " : invalid or inexistant texture type" << std::endl;
        return false;
    }

    try 
    { 
        if(textureInfo["mipmap"].toBool()) 
            configuration |= (uint8_t)Texture::TextureConfiguration::USE_MIPMAP; 
    }
    catch(std::exception) {}

    try 
    { 
        if(textureInfo["minnearest"].toBool()) 
            configuration |= (uint8_t)Texture::TextureConfiguration::MIN_NEAREST; 
    }
    catch(std::exception) {}

    try 
    { 
        if(textureInfo["magnearest"].toBool()) 
            configuration |= (uint8_t)Texture::TextureConfiguration::MAG_NEAREST; 
    }
    catch(std::exception) {}

    try
    {
        if(textureInfo["wrap"].toString() == "repeat")      
            configuration |= (uint8_t)Texture::TextureConfiguration::WRAP_REPEAT;
        else if(textureInfo["wrap"].toString() == "mirror") 
            configuration |= (uint8_t)Texture::TextureConfiguration::WRAP_MIRROR;
    }
    catch(std::exception) {}

    //  Get layers & generate texture data
    uint8_t* textureData = nullptr;
    try
    {
        if ((configuration & (uint8_t)Texture::TextureConfiguration::TYPE_MASK) == (uint8_t)Texture::TextureConfiguration::TEXTURE_2D &&
            textureInfo["texture"].getType() == Variant::STRING) //texture="a.png";
        {
            int x, y, n;
            textureData = ImageLoader::loadFromFile(resourceDirectory + Texture::directory + textureInfo["texture"].toString(), x, y, n, ImageLoader::RGB_ALPHA);
            if (!textureData) throw std::runtime_error("fail loading 2D texture");
            isImage = true;

            size = glm::vec3((float)x, (float)y, 0.f);
        }
        else if (textureInfo["texture"].getType() == Variant::ARRAY && textureInfo["texture"][0].getType() == Variant::STRING)//texture=["a.png","b.png"];
        {
            configuration &= ~(uint8_t)Texture::TextureConfiguration::TYPE_MASK;
            configuration |= (uint8_t)Texture::TextureConfiguration::TEXTURE_3D;
            size.x = (float)textureInfo["width"].toInt();
            size.y = (float)textureInfo["height"].toInt();
            size.z = (float)textureInfo["texture"].size();

            textureData = new uint8_t[4 * (int)(size.x * size.y * size.z)];
            if (!textureData) throw std::runtime_error("fail init texture ptr");
            isImage = false;
            uint8_t* textureEnd = textureData;

            int x, y, n;
            uint8_t* image = nullptr;

            for (unsigned int i = 0; i < textureInfo["texture"].size(); i++)
            {
                image = ImageLoader::loadFromFile(resourceDirectory + Texture::directory + textureInfo["texture"][i].toString(), x, y, n, ImageLoader::RGB_ALPHA);
                if (!image)
                    throw std::runtime_error("fail loading 3D texture");
                else if (x != (int)size.x || y != (int)size.y)
                    throw std::runtime_error("wrong 2D texture size in 3D texture layer description");

                memcpy(textureEnd, image, (size_t)4 * x * y);
                textureEnd += (size_t)4 * x * y;
                ImageLoader::freeImage(image);
            }
        }
        else if (textureInfo["texture"].getType() == Variant::ARRAY && textureInfo["texture"][0].getType() == Variant::INT)//texture=[0,0,0,255 , 255,0,0,255];
        {
            unsigned int n = textureInfo["width"].toInt();
            size.x = (float)n;
            if ((configuration & (uint8_t)Texture::TextureConfiguration::TYPE_MASK) == (uint8_t)Texture::TextureConfiguration::TEXTURE_2D) 
            { 
                size.y = (float)textureInfo["height"].toInt(); 
                n *= textureInfo["height"].toInt(); 
            }
            if ((configuration & (uint8_t)Texture::TextureConfiguration::TYPE_MASK) == (uint8_t)Texture::TextureConfiguration::TEXTURE_3D) 
            { 
                size.z = (float)textureInfo["depth"].toInt();  
                n *= textureInfo["depth"].toInt(); 
            }

            textureData = new uint8_t[(size_t)4 * n];
            if (!textureData) 
                throw std::runtime_error("error allocation array");
            isImage = false;

            for (unsigned int i = 0; i < textureInfo["texture"].size(); i++)
            {
                if (i >= 4 * n) 
                    throw std::runtime_error("invalid width, height, depth");
                textureData[i] = (uint8_t)(textureInfo["texture"][0].toInt());
            }
        }
        else 
            throw std::runtime_error("fail to parse file description");
    }
    catch(const std::exception& e)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading texture : " << fileName << " : "<< e.what() << std::endl;
        if(textureData)
        {
            if(isImage)
                ImageLoader::freeImage(textureData);
            else
                delete[] textureData;
        }
        return false;
    }

    return true;
}

void TextureLoader::initialize(ResourceVirtual* resource)
{
    Texture* texture = static_cast<Texture*>(resource);
    texture->initialize(size, textureData, configuration);

    if(isImage)
        ImageLoader::freeImage(textureData);
    else
        delete[] textureData;
}

void TextureLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string TextureLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Texture::directory;
    str += fileName;
    str += Texture::extension;
    return str;
}

