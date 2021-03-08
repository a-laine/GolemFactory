#include "Texture.h"
#include "Loader/TextureLoader.h"

#include <Utiles/Assert.hpp>

//  Static attributes
char const * const Texture::directory = "Textures/";
char const * const Texture::extension = ".texture";
std::string Texture::defaultName;
//

//  Default
Texture::Texture(const std::string& textureName, uint8_t conf) : ResourceVirtual(textureName), texture(0), configuration(conf) {}
Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}

void Texture::initialize(const glm::vec3& imageSize, const uint8_t* data, uint8_t config)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    //configuration = config;
    size = imageSize;
    initOpenGL(data, name);

    if(glIsTexture(texture))
        state = VALID;
    else
        state = INVALID;
}
//

//  Set/get functions
int Texture::getType() { return (configuration&TYPE_MASK); }
GLenum Texture::getGLenumType()
{
    switch(configuration&TYPE_MASK)
    {
        case TEXTURE_1D: return GL_TEXTURE_1D;
        case TEXTURE_3D: return GL_TEXTURE_3D;
        default: return GL_TEXTURE_2D;
    }
}
std::string Texture::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
std::string Texture::getIdentifier() const
{
    return getIdentifier(name);
}
std::string Texture::getLoaderId(const std::string& resourceName) const
{
    if((configuration & Texture::TYPE_MASK) == Texture::TEXTURE_2D)
        return "image";
    else
        return extension;
}

const std::string& Texture::getDefaultName() { return defaultName; }
void Texture::setDefaultName(const std::string& name) { defaultName = name; }
//

//  Public functions
GLuint Texture::getTextureId() const { return texture; }
GLuint* Texture::getTextureIdPointer() { return &texture; }
//



void Texture::initOpenGL(const uint8_t* textureData, const std::string& textureName)
{
    glGenTextures(1, &texture);
    switch(configuration&TYPE_MASK)
    {
        case TEXTURE_1D:
            glBindTexture(GL_TEXTURE_1D, texture);
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, (int) size.x,
                0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            if(configuration&USE_MIPMAP) glGenerateMipmap(GL_TEXTURE_1D);
            break;
        case TEXTURE_2D:
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) size.x, (int) size.y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            if(configuration&USE_MIPMAP) glGenerateMipmap(GL_TEXTURE_2D);
            break;
        case TEXTURE_3D:
            glBindTexture(GL_TEXTURE_3D, texture);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, (int) size.z, (int) size.x, (int) size.y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            if(configuration&USE_MIPMAP) glGenerateMipmap(GL_TEXTURE_3D);
            break;
        default:
            if(logVerboseLevel >= ResourceVirtual::ERRORS)
                std::cerr << "ERROR : loading texture : " << textureName << " : unknown type" << std::endl;
            glDeleteTextures(1, &texture);
            texture = 0;
            return;
    }

    //  MAG & MIN filter parameter
    if(configuration&USE_MIPMAP)
    {
        if(configuration&MAG_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        if(configuration&MIN_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        if(configuration&MAG_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if(configuration&MIN_NEAREST) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    //  WRAP parameter
    switch(configuration&WRAP_MASK)
    {
        case WRAP_CLAMP:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            if((configuration&TYPE_MASK)>=2) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            if((configuration&TYPE_MASK)>=3) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            break;
        case WRAP_REPEAT:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            if((configuration&TYPE_MASK)>=2) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if((configuration&TYPE_MASK)>=3) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
            break;
        case WRAP_MIRROR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            if((configuration&TYPE_MASK)>=2) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            if((configuration&TYPE_MASK)>=3) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
            break;
    }

    //  End
    glBindTexture(GL_TEXTURE_2D, 0);
}

