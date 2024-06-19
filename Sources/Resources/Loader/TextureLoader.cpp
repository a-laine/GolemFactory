#include "TextureLoader.h"
#include "ImageLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Resources/Texture.h>
#include <Utiles/ConsoleColor.h>



TextureLoader::TextureLoader() : m_size(0) , m_textureData(nullptr) , m_configuration(0) , m_isImage(false)
{}

bool TextureLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    auto PrintError = [&fileName](const std::string& msg)
    {
        if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
        {
            std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR : loading texture : " << fileName << " : " << msg << std::flush;
            std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
        }
    };
    auto PrintWarning = [&fileName](const std::string& msg)
    {
        if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
        {
            std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : loading texture : " << fileName << " : " << msg << std::flush;
            std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
        }
    };

    //  Initialization
    Variant v; Variant* tmp = nullptr;
    try
    {
        Reader::parseFile(v, getFileName(resourceDirectory, fileName));
        tmp = &(v.getMap().begin()->second);
        if (tmp->getType() != Variant::VariantType::MAP)
        {
            PrintError("wrong format");
            return false;
        }
    }
    catch(std::exception&)
    {
        PrintError("fail to open or parse file");
        return false;
    }
    Variant::MapType& textureInfo = tmp->getMap();

    //  Configuration
    Variant::MapType::iterator it = textureInfo.find("type");
    if (it != textureInfo.end())
    {
        if (it->second.getType() == Variant::VariantType::INT)
        {
            switch (it->second.toInt())
            {
                case 1:
                    m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_1D;
                    break;
                case 2:
                    m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_2D;
                    break;
                case 3:
                    m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_3D;
                    break;
                default:
                    PrintError("unsuported type");
                    return false;
            }
        }
        else if (it->second.getType() == Variant::VariantType::STRING)
        {
            std::string typestr = it->second.toString();
            if (typestr == "1D")
                m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_1D;
            else if (typestr == "2D")
                m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_2D;
            else if (typestr == "3D")
                m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_3D;
            else if (typestr == "ARRAY")
                m_configuration = (uint8_t)Texture::TextureConfiguration::TEXTURE_ARRAY;
            else
            {
                PrintError("unsuported type");
                return false;
            }
        }
        else
        {
            PrintError("unsuported type");
            return false;
        }
    }
    else
    {
        PrintError("invalid or inexistant texture type field");
        return false;
    }

    it = textureInfo.find("mipmap");
    if (it != textureInfo.end())
    {
        if (it->second.toBool())
            m_configuration |= (uint8_t)Texture::TextureConfiguration::USE_MIPMAP;
    }
    it = textureInfo.find("minnearest");
    if (it != textureInfo.end())
    {
        if (it->second.toBool())
            m_configuration |= (uint8_t)Texture::TextureConfiguration::MIN_NEAREST;
    }
    it = textureInfo.find("magnearest");
    if (it != textureInfo.end())
    {
        if (it->second.toBool())
            m_configuration |= (uint8_t)Texture::TextureConfiguration::MAG_NEAREST;
    }
    it = textureInfo.find("wrap");
    if (it != textureInfo.end())
    {
        std::string typestr = it->second.toString();
        if (typestr == "repeat")
            m_configuration |= (uint8_t)Texture::TextureConfiguration::WRAP_REPEAT;
        else if (typestr == "mirror")
            m_configuration |= (uint8_t)Texture::TextureConfiguration::WRAP_MIRROR;
        else
            PrintWarning("unknown \"wrap\" param value");
    }

    //  Get layers & generate texture data
    //uint8_t* textureData = nullptr;
    it = textureInfo.find("texture");
    if (it == textureInfo.end())
    {
        PrintError("no\"texture\" field to parse");
        return false;
    }

    uint8_t type = m_configuration & (uint8_t)Texture::TextureConfiguration::TYPE_MASK;
    bool isReadableArray = it->second.getType() == Variant::ARRAY && it->second.getArray().size() > 0;

    if (type == (uint8_t)Texture::TextureConfiguration::TEXTURE_2D && it->second.getType() == Variant::STRING)
    {
        int x, y, n;
        m_textureData = ImageLoader::loadFromFile(resourceDirectory + Texture::directory + textureInfo["texture"].toString(), x, y, n, ImageLoader::RGB_ALPHA);
        if (!m_textureData)
        {
            PrintError("fail loading 2D texture");
            return false;
        }

        m_isImage = true;
        m_size = vec3f((float)x, (float)y, 0.f);
    }
    else if (isReadableArray && it->second[0].getType() == Variant::STRING)
    {
        if (type != (uint8_t)Texture::TextureConfiguration::TEXTURE_3D && type != (uint8_t)Texture::TextureConfiguration::TEXTURE_ARRAY)
        {
            PrintError("texture data is array, but not texture type");
            return false;
        }

        // read size
        m_size = vec3f(-1, -1, it->second.getArray().size());
        Variant::MapType::iterator it2 = textureInfo.find("width");
        if (it2 != textureInfo.end() && it2->second.getType() == Variant::VariantType::INT)
            m_size.x = it2->second.toInt();
        it2 = textureInfo.find("height");
        if (it2 != textureInfo.end() && it2->second.getType() == Variant::VariantType::INT)
            m_size.y = it2->second.toInt();

        if (m_size.x <= 0.f || m_size.y <= 0.f)
        {
            PrintError("texture has invalid size field values (width, height)");
            return false;
        }

        // load layers
        m_textureData = new uint8_t[4 * (int)(m_size.x * m_size.y * m_size.z)];
        m_isImage = false;
        uint8_t* copyEnd = m_textureData;
        uint8_t* image = nullptr;
        int x, y, n;
        for (unsigned int i = 0; i < it->second.getArray().size(); i++)
        {
            const std::string& layerName = it->second[i].toString();
            image = ImageLoader::loadFromFile(resourceDirectory + Texture::directory + layerName, x, y, n, ImageLoader::RGB_ALPHA);
            if (!image)
            {
                PrintWarning("fail loading 3D texture layer (" + layerName + ")[index " + std::to_string(i) + "]");
            }
            else if (x != (int)m_size.x || y != (int)m_size.y)
            {
                PrintWarning("invalid texture layer size (" + layerName + ")[index " + std::to_string(i) + "]");
                ImageLoader::freeImage(image);
            }
            else
            {
                memcpy(copyEnd, image, (size_t)4 * x * y);
                ImageLoader::freeImage(image);
            }
            copyEnd += (size_t)(4 * m_size.x * m_size.y);
        }
    }
    else if (isReadableArray)
    {
        if (type != (uint8_t)Texture::TextureConfiguration::TEXTURE_3D && type != (uint8_t)Texture::TextureConfiguration::TEXTURE_ARRAY)
        {
            PrintError("texture data is array, but not texture type");
            return false;
        }

    }

    /*try
    {
        if ((configuration & (uint8_t)Texture::TextureConfiguration::TYPE_MASK) == (uint8_t)Texture::TextureConfiguration::TEXTURE_2D &&
            textureInfo["texture"].getType() == Variant::STRING) //texture="a.png";
        {
            int x, y, n;
            textureData = ImageLoader::loadFromFile(resourceDirectory + Texture::directory + textureInfo["texture"].toString(), x, y, n, ImageLoader::RGB_ALPHA);
            if (!textureData) throw std::runtime_error("fail loading 2D texture");
            isImage = true;

            size = vec3f((float)x, (float)y, 0.f);
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
            if(m_isImage)
                ImageLoader::freeImage(textureData);
            else
                delete[] textureData;
        }
        return false;
    }*/

    return true;
}

void TextureLoader::initialize(ResourceVirtual* resource)
{
    Texture* texture = static_cast<Texture*>(resource);
    texture->initialize(m_size, m_textureData, m_configuration);

    if(m_isImage)
        ImageLoader::freeImage(m_textureData);
    else
        delete[] m_textureData;
}

void TextureLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string TextureLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Texture::directory;
    str += fileName;
    if (fileName.find('.') == std::string::npos)
        str += Texture::extension;
    return str;
}

