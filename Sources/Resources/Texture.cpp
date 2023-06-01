#include "Texture.h"
#include "Loader/TextureLoader.h"

#include <Utiles/Assert.hpp>

//  Static attributes
char const * const Texture::directory = "Textures/";
char const * const Texture::extension = ".texture";
std::string Texture::defaultName;
//

//  Default
Texture::Texture(const std::string& textureName, TextureConfiguration conf)
    : ResourceVirtual(textureName, ResourceVirtual::ResourceType::TEXTURE)
    , texture(0), configuration((uint8_t)conf) 
{
#ifdef USE_IMGUI
    layerOverview = 0;
    textureLayers = nullptr;
#endif
}
Texture::Texture(const std::string& textureName, uint8_t conf)
    : ResourceVirtual(textureName, ResourceVirtual::ResourceType::TEXTURE)
    , texture(0), configuration(conf) 
{
#ifdef USE_IMGUI
    layerOverview = 0;
    textureLayers = nullptr;
#endif
}
Texture::~Texture()
{
	glDeleteTextures(1, &texture);

#ifdef USE_IMGUI
    if (textureLayers)
    {
        glDeleteTextures(size.z, textureLayers);
        delete[] textureLayers;
    }
#endif
}

void Texture::initialize(const vec3i& imageSize, const uint8_t* data, uint8_t config)
{
    /*GF_ASSERT(state == INVALID);
    state = LOADING;

    //configuration = config;
    size = imageSize;

    if(glIsTexture(texture))
        state = VALID;
    else
        state = INVALID;*/

    initialize(name, imageSize, data, config, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
}
//

//  Set/get functions
int Texture::getType() { return (configuration & (uint8_t)TextureConfiguration::TYPE_MASK); }
GLenum Texture::getGLenumType()
{
    switch((TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::TYPE_MASK))
    {
        case TextureConfiguration::TEXTURE_1D: return GL_TEXTURE_1D;
        case TextureConfiguration::TEXTURE_3D: return GL_TEXTURE_3D;
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
    if((configuration & (uint8_t)TextureConfiguration::TYPE_MASK) == (uint8_t)TextureConfiguration::TEXTURE_2D)
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



/*void Texture::initOpenGL(const uint8_t* textureData, const std::string& textureName)
{
    const auto CheckError = [textureName](const char* label)
    {
        GLenum error = glGetError();
        if (!label)
            return;
        switch (error)
        {
            case GL_INVALID_ENUM: std::cout << textureName << " : " << label << " : GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE: std::cout << textureName << " : " << label << " : GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cout << textureName << " : " << label << " : GL_INVALID_OPERATION" << std::endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << label << " : GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
            case GL_OUT_OF_MEMORY: std::cout << textureName << " : " << label << " : GL_OUT_OF_MEMORY" << std::endl; break;
            case GL_STACK_UNDERFLOW: std::cout << textureName << " : " << label << " : GL_STACK_UNDERFLOW" << std::endl; break;
            case GL_STACK_OVERFLOW: std::cout << textureName << " : " << label << " : GL_STACK_OVERFLOW" << std::endl; break;
            default: break;
        }
    };

    CheckError(nullptr);
    glGenTextures(1, &texture); CheckError("glGenTextures");
    unsigned int type = GL_TEXTURE_2D;
    switch ((TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::TYPE_MASK))
    {
        case TextureConfiguration::TEXTURE_1D:
            glBindTexture(GL_TEXTURE_1D, texture); CheckError("glBindTexture");
            glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, (int) size.x,
                0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            CheckError("glTexImage1D");
            type = GL_TEXTURE_1D;
            break;

        case TextureConfiguration::TEXTURE_2D:
            glBindTexture(GL_TEXTURE_2D, texture); CheckError("glBindTexture");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int) size.x, (int) size.y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            CheckError("glTexImage2D");
            type = GL_TEXTURE_2D;
            break;

        case TextureConfiguration::TEXTURE_3D:
            glBindTexture(GL_TEXTURE_3D, texture); CheckError("glBindTexture");
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, (int) size.z, (int) size.x, (int) size.y,
                0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
            CheckError("glTexImage3D");
            type = GL_TEXTURE_3D;
            break;

        default:
            if(logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading texture : " << textureName << " : unknown type" << std::endl;
            glDeleteTextures(1, &texture);
            texture = 0;
            return;
    }

    if (configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
    {
        glGenerateMipmap(type);
        CheckError("glGenerateMipmap");
    }

    //  MAG & MIN filter parameter
    if(configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
    {
        if(configuration & (uint8_t)TextureConfiguration::MAG_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        else
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        CheckError("glTexParameteri MAG_FILTER & MIPMAP");

        if(configuration & (uint8_t)TextureConfiguration::MIN_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        else
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        CheckError("glTexParameteri MIN_FILTER & MIPMAP");
    }
    else
    {
        if(configuration & (uint8_t)TextureConfiguration::MAG_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        else
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CheckError("glTexParameteri MIN_FILTER");

        if(configuration & (uint8_t)TextureConfiguration::MIN_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CheckError("glTexParameteri MIN_FILTER");
    }

    //  WRAP parameter
    unsigned int wrapMode = 0;
    switch((TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::WRAP_MASK))
    {
        case TextureConfiguration::WRAP_CLAMP:
            wrapMode = GL_CLAMP_TO_EDGE;
            break;

        case TextureConfiguration::WRAP_REPEAT:
            wrapMode = GL_REPEAT;
            break;

        case TextureConfiguration::WRAP_MIRROR:
            wrapMode = GL_MIRRORED_REPEAT;
            break;
    }

    if (wrapMode)
    {
        glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapMode);
        CheckError("glTexParameteri WRAP");
        if ((configuration & (uint8_t)TextureConfiguration::TYPE_MASK) >= 2)
        {
            glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapMode);
            CheckError("glTexParameteri WRAP");
        }
        if ((configuration & (uint8_t)TextureConfiguration::TYPE_MASK) >= 3)
        {
            glTexParameteri(type, GL_TEXTURE_WRAP_R, wrapMode);
            CheckError("glTexParameteri WRAP");
        }
    }

    //  End
    glBindTexture(type, 0);
}*/

void Texture::initialize(const std::string& textureName, const vec3i& imageSize, const void* data, uint8_t config, unsigned int internalFormat, unsigned int pixelFormat, unsigned int colorFormat)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;
    size = imageSize;
    configuration = config;

    const auto CheckError = [textureName](const char* label)
    {
        GLenum error = glGetError();
        if (!label)
            return;
        switch (error)
        {
            case GL_INVALID_ENUM: std::cout << textureName << " : " << label << " : GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE: std::cout << textureName << " : " << label << " : GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cout << textureName << " : " << label << " : GL_INVALID_OPERATION" << std::endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << label << " : GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
            case GL_OUT_OF_MEMORY: std::cout << textureName << " : " << label << " : GL_OUT_OF_MEMORY" << std::endl; break;
            case GL_STACK_UNDERFLOW: std::cout << textureName << " : " << label << " : GL_STACK_UNDERFLOW" << std::endl; break;
            case GL_STACK_OVERFLOW: std::cout << textureName << " : " << label << " : GL_STACK_OVERFLOW" << std::endl; break;
            default: break;
        }
    };

    CheckError(nullptr);
    glGenTextures(1, &texture); CheckError("glGenTextures");
    unsigned int type = GL_TEXTURE_2D;
    switch ((TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::TYPE_MASK))
    {
        case TextureConfiguration::TEXTURE_1D:
            glBindTexture(GL_TEXTURE_1D, texture); CheckError("glBindTexture");
            glTexImage1D(GL_TEXTURE_1D, 0, internalFormat, (int)size.x,
                0, pixelFormat, colorFormat, data);
            CheckError("glTexImage1D");
            type = GL_TEXTURE_1D;
            break;

        case TextureConfiguration::TEXTURE_2D:
            glBindTexture(GL_TEXTURE_2D, texture); CheckError("glBindTexture");
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, (int)size.x, (int)size.y,
                0, pixelFormat, colorFormat, data);
            CheckError("glTexImage2D");
            type = GL_TEXTURE_2D;
            break;

        case TextureConfiguration::TEXTURE_3D:
            glBindTexture(GL_TEXTURE_3D, texture); CheckError("glBindTexture");
            glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, (int)size.x, (int)size.y, (int)size.z, 
                0, pixelFormat, colorFormat, data);
            CheckError("glTexImage3D");
            type = GL_TEXTURE_3D;
            break;

        default:
            if (logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading texture : " << textureName << " : unknown type" << std::endl;
            glDeleteTextures(1, &texture);
            texture = 0;
            state = INVALID;
            return;
    }

    if (configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
    {
        glGenerateMipmap(type);
        CheckError("glGenerateMipmap");
    }

    //  MAG & MIN filter parameter
    if (configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
    {
        if (configuration & (uint8_t)TextureConfiguration::MAG_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        else
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        CheckError("glTexParameteri MAG_FILTER & MIPMAP");

        if (configuration & (uint8_t)TextureConfiguration::MIN_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        else
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        CheckError("glTexParameteri MIN_FILTER & MIPMAP");
    }
    else
    {
        if (configuration & (uint8_t)TextureConfiguration::MAG_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        else
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CheckError("glTexParameteri MIN_FILTER");

        if (configuration & (uint8_t)TextureConfiguration::MIN_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        else
            glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        CheckError("glTexParameteri MIN_FILTER");
    }

    //  WRAP parameter
    unsigned int wrapMode = 0;
    switch ((TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::WRAP_MASK))
    {
        case TextureConfiguration::WRAP_CLAMP:
            wrapMode = GL_CLAMP_TO_EDGE;
            break;

        case TextureConfiguration::WRAP_REPEAT:
            wrapMode = GL_REPEAT;
            break;

        case TextureConfiguration::WRAP_MIRROR:
            wrapMode = GL_MIRRORED_REPEAT;
            break;
    }

    if (wrapMode)
    {
        glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapMode);
        CheckError("glTexParameteri WRAP");
        if ((configuration & (uint8_t)TextureConfiguration::TYPE_MASK) >= 2)
        {
            glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapMode);
            CheckError("glTexParameteri WRAP");
        }
        if ((configuration & (uint8_t)TextureConfiguration::TYPE_MASK) >= 3)
        {
            glTexParameteri(type, GL_TEXTURE_WRAP_R, wrapMode);
            CheckError("glTexParameteri WRAP");
        }
    }

#ifdef USE_IMGUI
    if (type == GL_TEXTURE_3D)
    {
        textureLayers = new GLuint[size.z];
        glGenTextures(size.z, textureLayers);
        for (int i = 0; i < size.z; ++i)
            glTextureView(textureLayers[i], GL_TEXTURE_2D, texture, GL_RGBA, 0, 1, i, 1);
    }
#endif

    //  End
    glBindTexture(type, 0);
    if (glIsTexture(texture))
        state = VALID;
    else state = INVALID;
}

void Texture::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Texture infos");

    // type
    TextureConfiguration type = (TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::TYPE_MASK);
    switch (type)
    {
        case Texture::TextureConfiguration::TEXTURE_1D:
            ImGui::Text("Type : 1D");
            ImGui::Text("Width : %d", (int)size.x);
            break;
        case Texture::TextureConfiguration::TEXTURE_2D:
            ImGui::Text("Type : 2D");
            ImGui::Text("Width : %d", (int)size.x);
            ImGui::Text("Height : %d", (int)size.y);
            break;
        case Texture::TextureConfiguration::TEXTURE_3D:
            ImGui::Text("Type : 3D");
            ImGui::Text("Width : %d", (int)size.x);
            ImGui::Text("Height : %d", (int)size.y);
            ImGui::Text("Depth : %d", (int)size.z);
            break;
        default:
            break;
    }

    // mipmaps
    if (configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
        ImGui::Text("Mipmap : True");
    else ImGui::Text("Mipmap : False");

    // Filtering (point, linear, ...)
    if (configuration & (uint8_t)TextureConfiguration::MAG_NEAREST)
        ImGui::Text("Magify filter : Nearest");
    else ImGui::Text("Magify filter : Linear");
    if (configuration & (uint8_t)TextureConfiguration::MIN_NEAREST)
        ImGui::Text("Minify filter : Nearest");
    else ImGui::Text("Minify filter : Linear");

    // wrap mode
    TextureConfiguration wrap = (TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::WRAP_MASK);
    switch (type)
    {
        case Texture::TextureConfiguration::WRAP_CLAMP:
            ImGui::Text("Wrap mode : Clamp");
            break;
        case Texture::TextureConfiguration::WRAP_REPEAT:
            ImGui::Text("Wrap mode : Repeat");
            break;
        case Texture::TextureConfiguration::WRAP_MIRROR:
            ImGui::Text("Wrap mode : Miror");
            break;
        default:
            break;
    }

    // overview
    float ratio = (ImGui::GetContentRegionAvail().x - 5) / size.x;
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Overview");

    if (textureLayers)
    {
        ImGui::SliderInt("Layer", &layerOverview, 0, size.z - 1);
        ImGui::Image((void*)textureLayers[layerOverview], ImVec2(size.x * ratio, size.y * ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    }
    else
    {
        ImGui::Image((void*)texture, ImVec2(size.x * ratio, size.y * ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    }

#endif
}