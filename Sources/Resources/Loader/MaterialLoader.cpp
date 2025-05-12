#include "MaterialLoader.h"

#include <Utiles/ConsoleColor.h>
#include <Utiles/Parser/Reader.h>
#include <Resources/Material.h>


// Default
MaterialLoader::MaterialLoader()
{
    clear();
}

std::string MaterialLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName)
{
    std::string str = resourceDirectory;
    str += Material::directory;
    str += fileName;

    if (fileName.find('.') == std::string::npos)
        str += Material::extension;
    return str;
}

bool MaterialLoader::load(const std::string& directory, const std::string& filename)
{
    clear();
    std::string fullFileName = getFileName(directory, filename);

    Variant v; Variant* tmp = nullptr;
    try
    {
        Reader::parseFile(v, fullFileName);
        tmp = &(v.getMap().begin()->second);
    }
    catch (std::exception&)
    {
        PrintError(fullFileName.c_str(), "fail to open or parse file");
        return false;
    }
    Variant& shaderMap = *tmp;
    std::string path = directory + Shader::directory;


    if (shaderMap.getType() != Variant::MAP)
    {
        PrintError(filename.c_str(), "wrong file formating");
        return false;
    }

    // see if it's compute shader
    auto it = shaderMap.getMap().find("shaderName");
    if (it != shaderMap.getMap().end() && it->second.getType() == Variant::STRING)
    {
        m_shaderName = it->second.toString();
    }
    else
    {
        PrintError(filename.c_str(), "no shaderName property");
        return false;
    }

    it = shaderMap.getMap().find("textureOverride");
    if (it != shaderMap.getMap().end())
    {
        if (it->second.getType() == Variant::ARRAY && it->second.getArray().size() > 0)
        {
            bool hasError = false;
            auto& overrideArray = it->second.getArray();
            for (auto it2 = overrideArray.begin(); it2 != overrideArray.end(); it2++)
            {
                if (it2->getType() == Variant::MAP)
                {
                    auto identifierVariant = it2->getMap().find("identifier");
                    auto textureVariant = it2->getMap().find("texture");
                    if (identifierVariant != it2->getMap().end() && textureVariant != it2->getMap().end() &&
                        identifierVariant->second.getType() == Variant::STRING && textureVariant->second.getType() == Variant::STRING)
                    {
                        m_textureOverride.push_back({ identifierVariant->second.toString(), textureVariant->second.toString() });
                    }
                    else hasError = true;
                }
            }

            if (hasError)
                PrintError(filename.c_str(), "Override array contain invalid object field (error fields ignored)");
        }
        else
            PrintWarning(filename.c_str(), "textureOverride property is not an array, or is empty");
    }

    auto propertiesVariant = shaderMap.getMap().find("properties");
    if (propertiesVariant != shaderMap.getMap().end())
    {
        if (propertiesVariant->second.getType() == Variant::ARRAY && propertiesVariant->second.getArray().size() > 0)
        {
            using ptype = Shader::Property::PropertyType;
            auto& propertiesArray = propertiesVariant->second.getArray();
            bool hasError = false;
            for (auto it2 = propertiesArray.begin(); it2 != propertiesArray.end(); it2++)
            {
                if (it2->getType() == Variant::MAP)
                {
                    auto nameVariant = it2->getMap().find("name");
                    auto typeVariant = it2->getMap().find("type");
                    auto defaultVariant = it2->getMap().find("default");

                    if (nameVariant != it2->getMap().end() && typeVariant != it2->getMap().end() && defaultVariant != it2->getMap().end() &&
                        nameVariant->second.getType() == Variant::STRING && typeVariant->second.getType() == Variant::STRING)
                    {
                        Shader::Property property;
                        property.m_name = nameVariant->second.toString();
                        std::string typestr = typeVariant->second.toString();
                        bool properror = false;

                        if (typestr == "int")
                        {
                            property.m_type = ptype::eInteger;
                            property.m_floatValues = vec4f::zero;
                            if (defaultVariant->second.getType() == Variant::INT)
                                property.m_integerValues = vec4i(defaultVariant->second.toInt(), 0, 0, 0);
                            else properror = true;
                        }
                        else if (typestr == "ivec4")
                        {
                            property.m_type = ptype::eIntegerVector;
                            property.m_floatValues = vec4f::zero;
                            property.m_integerValues = vec4i::zero;
                            if (defaultVariant->second.getType() == Variant::ARRAY)
                            {
                                auto& varray = defaultVariant->second.getArray();
                                for (int i = 0; i < 4 && i < varray.size(); i++)
                                {
                                    auto& element = varray[i];
                                    if (element.getType() == Variant::INT)
                                        property.m_floatValues[i] = (float)element.toInt();
                                    else properror = true;
                                }
                            }
                            else properror = true;
                        }
                        else if (typestr == "float")
                        {
                            property.m_type = ptype::eFloat;
                            property.m_integerValues = vec4i::zero;
                            if (defaultVariant->second.getType() == Variant::INT)
                                property.m_floatValues = vec4f(defaultVariant->second.toInt(), 0, 0, 0);
                            else if (defaultVariant->second.getType() == Variant::FLOAT)
                                property.m_floatValues = vec4f(defaultVariant->second.toFloat(), 0, 0, 0);
                            else if (defaultVariant->second.getType() == Variant::DOUBLE)
                                property.m_floatValues = vec4f(defaultVariant->second.toDouble(), 0, 0, 0);
                            else properror = true;
                        }
                        else if (typestr == "vec4" || typestr == "color")
                        {
                            property.m_type = typestr == "vec4" ? ptype::eFloatVector : ptype::eColor;
                            property.m_integerValues = vec4i::zero;
                            property.m_floatValues = vec4f::zero;
                            if (defaultVariant->second.getType() == Variant::ARRAY)
                            {
                                auto& varray = defaultVariant->second.getArray();
                                for (int i = 0; i < 4 && i < varray.size(); i++)
                                {
                                    auto& element = varray[i];
                                    if (element.getType() == Variant::FLOAT)
                                        property.m_floatValues[i] = element.toFloat();
                                    else if (element.getType() == Variant::DOUBLE)
                                        property.m_floatValues[i] = (float)element.toDouble();
                                    else if (element.getType() == Variant::INT)
                                        property.m_floatValues[i] = (float)element.toInt();
                                    else properror = true;
                                }
                            }
                            else properror = true;
                        }
                        else properror = true;

                        if (!properror)
                        {
                            m_properties.push_back(property);
                        }
                        else
                        {
                            std::string msg = "Property (" + property.m_name + ") parsing error";
                            PrintWarning(filename.c_str(), msg.c_str());
                            hasError = true;
                        }
                    }
                    else hasError = true;
                }
            }
        }
    }
    return true;
}

void MaterialLoader::initialize(ResourceVirtual* resource)
{
    Material* material = static_cast<Material*>(resource);
    material->initialize(m_shaderName, m_maxShadowCascade);
    if (!m_textureOverride.empty())
        material->setTextureOverride(m_textureOverride);
    material->copyPropertiesFromShader();

    for (Shader::Property& property : material->m_properties)
    {
        for (Shader::Property& overrided : m_properties)
        {
            if (property.m_name == overrided.m_name && property.m_type == overrided.m_type)
            {
                property.m_floatValues = overrided.m_floatValues;
                property.m_integerValues = overrided.m_integerValues;
            }
            else
            {
                PrintWarning(resource->name.c_str(), "property missmatch");
            }
        }

    }
}


void MaterialLoader::PrintError(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : MaterialLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}

void MaterialLoader::PrintWarning(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : MaterialLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}
//


void MaterialLoader::clear()
{
    m_maxShadowCascade = 4;
    m_properties.clear();
    m_textureOverride.clear();
}

