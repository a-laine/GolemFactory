#include "FontLoader.h"

#include <iostream>
#include <sstream>

#include <Utiles/Parser/Reader.h>
#include <Utiles/ToolBox.h>
#include <Resources/Loader/ImageLoader.h>



FontLoader::FontLoader()
    : size(0)
    , image(nullptr)
    , begin(0)
    , end(0)
    , defaultChar(0)
{}

bool FontLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    //  Initialization
    Variant v;
    Variant* tmp = NULL;

    //	Extract root variant from file
    try
    {
        Reader::parseFile(v, getFileName(resourceDirectory, fileName));
        tmp = &(v.getMap().begin()->second);
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading font : " << fileName << " : fail to open or parse file" << std::endl;
        return false;
    }
    Variant& fontInfo = *tmp;

    //	Extract texture name
    std::string textureFile;
    try
    {
        textureFile = resourceDirectory + Font::directory + fontInfo["texture"].toString();
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading font : " << fileName << " : texture attribute field not found" << std::endl;
        return false;
    }

    //  Loading the image
    int x, y, n;
    image = ImageLoader::loadFromFile(textureFile, x, y, n, ImageLoader::RGB_ALPHA);
    if(!image)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading font : " << fileName << " : fail loading texture image" << std::endl;
        return false;
    }

    //  Parse patches infos
    size.x = (float) x; size.y = (float) y;

    try { begin = fontInfo["begin"].toInt(); }
    catch(std::exception&) { begin = 0; }

    try { end = fontInfo["end"].toInt(); }
    catch(std::exception&) { end = 512; }

    try { defaultChar = fontInfo["default"].toInt(); }
    catch(std::exception&) { defaultChar = begin; }
    if(defaultChar > end) defaultChar = begin;
    charTable.assign(end - begin + 1, Font::Patch());

    //  Extract and prepare file array for parsing
    std::string arrayFile;
    try
    {
        arrayFile = ToolBox::openAndCleanCStyleFile(resourceDirectory + Font::directory + fontInfo["font"].toString());
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading font : " << fileName << " : fail open char array" << std::endl;
        charTable.clear();
        return false;
    }
    if(arrayFile.empty())
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::WARNINGS)
            std::cerr << "WARNING : loading font : " << fileName << " : char array file empty" << std::endl;
        begin = 0;
        end = 0;
        defaultChar = 0;
    }



    //  Load coordinate array
    std::string line;
    glm::u16vec2 c1, c2;
    bool errorOccured = false;
    unsigned int index = 0;
    unsigned int lineCount = 0;
    std::stringstream arrayFileStream(arrayFile);
    while(!arrayFileStream.eof())
    {
        //	get next line
        std::getline(arrayFileStream, line);
        lineCount++;

        //	remove unrevelant char and replace coma by space
        for(auto it = line.begin(); it != line.end();)
        {
            if(*it == ' ' || *it == '=' || *it == '}')
                it = line.erase(it);
            else if(*it == ',' || *it == '{')
            {
                *it = ' ';
                it++;
            }
            else it++;
        }
        if(line.empty()) continue;

        //	parsing
        std::stringstream convertor(line);
        convertor >> index >> c1.x >> c1.y >> c2.x >> c2.y;

        if(!convertor.fail())
        {
            charTable[index - begin].corner1.x = c1.x / size.x;
            charTable[index - begin].corner1.y = c1.y / size.y;
            charTable[index - begin].corner2.x = c2.x / size.x;
            charTable[index - begin].corner2.y = c2.y / size.y;
        }
        else ResourceVirtual::printErrorLog(fileName, lineCount, errorOccured);
    }
    charTable.shrink_to_fit();

    return true;
}

void FontLoader::initialize(ResourceVirtual* resource)
{
    Font* font = static_cast<Font*>(resource);
    font->initialize(image, size, begin, end, defaultChar, std::move(charTable));
    ImageLoader::freeImage(image);
}

void FontLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string FontLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Font::directory;
    str += fileName;
    str += Font::extension;
    return str;
}

