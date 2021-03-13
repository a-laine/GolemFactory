#include "Font.h"
#include "Loader/FontLoader.h"

#include <Utiles/Parser/Reader.h>
#include <Utiles/Assert.hpp>

//  Static attributes
char const * const Font::directory = "Font/";
char const * const Font::extension = ".font";
std::string Font::defaultName;
//

//  Default
Font::Font(const std::string& fontName) : ResourceVirtual(fontName), texture(0), begin(0), end(0), defaultChar(0) {}
Font::~Font()
{
    if(glIsTexture(texture)) glDeleteTextures(1,&texture);
}
//

//  Public functions
void Font::initialize(uint8_t* image, const glm::vec2& imageSize, unsigned short beginC, unsigned short endC, unsigned short defaultC, const std::vector<Patch>& table)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    if(!initOpenGL(image, (int) imageSize.x, (int) imageSize.y))
    {
        if(logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading font : " << name << " : fail create OPENGL texture" << std::endl;
        state = INVALID;
        return;
    }

    size = imageSize;
    begin = beginC;
    end = endC;
    defaultChar = defaultC;
    charTable = table;
    state = VALID;
}

void Font::initialize(uint8_t* image, const glm::vec2& imageSize, unsigned short beginC, unsigned short endC, unsigned short defaultC, std::vector<Patch>&& table)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    if(!initOpenGL(image, (int) imageSize.x, (int) imageSize.y))
    {
        if(logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading font : " << name << " : fail create OPENGL texture" << std::endl;
        state = INVALID;
        return;
    }

    size = imageSize;
    begin = beginC;
    end = endC;
    defaultChar = defaultC;
    charTable = std::move(table);
    state = VALID;
}

Font::Patch Font::getPatch(char c) const
{
	if (!glIsTexture(texture)) return Patch();
	else if (c > end || c < begin) return charTable[defaultChar - begin];
	else return charTable[c - begin];
}
//

//  Set/get functions
char Font::getDefaultChar() const { return (char)defaultChar; }
char Font::getBeginChar() const { return (char)begin; }
char Font::getEndChar() const { return (char)end; }
int Font::getArraySize() const { return (int)charTable.size(); }

std::string Font::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
std::string Font::getIdentifier() const
{
    return getIdentifier(name);
}

std::string Font::getLoaderId(const std::string& resourceName) const
{
    return extension;
}

const std::string& Font::getDefaultName() { return defaultName; }
void Font::setDefaultName(const std::string& name) { defaultName = name; }
//

//  Private functions
bool Font::initOpenGL(uint8_t* image, int sizeX, int sizeY)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return glIsTexture(texture) != 0;
}

void Font::clear()
{
	if (glIsTexture(texture))
		glDeleteTextures(1, &texture);
    texture = 0;
    charTable.clear();
}
//

//	Internal classes
Font::Patch::Patch() : corner1(0.f), corner2(0.f) {}
//
