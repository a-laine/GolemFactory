#include "Texture.h"
#include "Loader/TextureLoader.h"

#include <Utiles/Assert.hpp>
#include <Utiles/Debug.h>

//  Static attributes
char const * const Texture::directory = "Textures/";
char const * const Texture::extension = ".texture";
std::string Texture::defaultName;

#ifdef USE_IMGUI
Texture* Texture::sharedTextureOverview = nullptr;
#endif
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
Texture::Texture()
    : ResourceVirtual("unknown", ResourceVirtual::ResourceType::TEXTURE)
    , texture(0), configuration(0)
{
    state = State::INVALID;

#ifdef USE_IMGUI
    layerOverview = 0;
    textureLayers = nullptr;
#endif
}
Texture::~Texture()
{
    if (glIsTexture(texture))
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
    initialize(name, imageSize, data, config, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
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
    size_t ext = resourceName.find_last_of('.');
    if (ext != std::string::npos && resourceName.substr(ext) != extension)
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


void Texture::initialize(const std::string& textureName, const vec3i& imageSize, const void* data, uint8_t config, unsigned int internalFormat, 
    unsigned int pixelFormat, unsigned int colorFormat, bool immutable)
{
    GF_ASSERT(state == INVALID);
    name = textureName;
    state = LOADING;
    size = imageSize;
    configuration = config;


    const auto CheckError = [textureName](const char* label)
    {
        GLenum error = glGetError();
        if (!label)
            return false;
        switch (error)
        {
            case GL_INVALID_ENUM: std::cout << textureName << " : " << label << " : GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE: std::cout << textureName << " : " << label << " : GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cout << textureName << " : " << label << " : GL_INVALID_OPERATION" << std::endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << textureName << " : " << label << " : GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
            case GL_OUT_OF_MEMORY: std::cout << textureName << " : " << label << " : GL_OUT_OF_MEMORY" << std::endl; break;
            case GL_STACK_UNDERFLOW: std::cout << textureName << " : " << label << " : GL_STACK_UNDERFLOW" << std::endl; break;
            case GL_STACK_OVERFLOW: std::cout << textureName << " : " << label << " : GL_STACK_OVERFLOW" << std::endl; break;
            default: break;
        }
        return error != GL_NO_ERROR;
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

        case TextureConfiguration::TEXTURE_ARRAY:
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture); CheckError("glBindTexture");
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, (int)size.x, (int)size.y, (int)size.z,
                0, pixelFormat, colorFormat, data);
            type = GL_TEXTURE_2D_ARRAY;
            break;

        case TextureConfiguration::CUBEMAP_ARRAY:
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture); CheckError("glBindTexture");
            glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, internalFormat, (int)size.x, (int)size.y, 6 * (int)size.z,
                0, pixelFormat, colorFormat, data);
            type = GL_TEXTURE_CUBE_MAP_ARRAY;
            break;

        default:
            if (logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading texture : " << textureName << " : unknown type" << std::endl;
            glDeleteTextures(1, &texture);
            texture = 0;
            state = INVALID;
            return;
    }
    m_type = type;

    if (configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
    {
        glGenerateMipmap(type);
        CheckError("glGenerateMipmap");
    }

    //  MAG & MIN filter parameter
    if (configuration & (uint8_t)TextureConfiguration::USE_MIPMAP)
    {
        if (configuration & (uint8_t)TextureConfiguration::MAG_NEAREST)
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        else
            glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
        CheckError("glTexParameteri WRAP S");
        if (type != GL_TEXTURE_1D)
        {
            glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapMode);
            CheckError("glTexParameteri WRAP T");
        }
        if (type == GL_TEXTURE_3D)
        {
            glTexParameteri(type, GL_TEXTURE_WRAP_R, wrapMode);
            CheckError("glTexParameteri WRAP R");
        }
    }

#ifdef USE_IMGUI
    m_internalFormat = internalFormat;

    GLint status;
    glGetTexParameteriv(type, GL_TEXTURE_IMMUTABLE_FORMAT, &status);
    if (type == GL_TEXTURE_2D_ARRAY && status)
    {
        bool printedError = false;
        textureLayers = new GLuint[size.z];
        glGenTextures(size.z, textureLayers);
        for (int i = 0; i < size.z; ++i)
        {
            glTextureView(textureLayers[i], GL_TEXTURE_2D, texture, GL_RGBA, 0, 1, i, 1);
            if (!printedError)
                CheckError("glTextureView");
        }
        CheckError(nullptr);
    }
#endif

    //  End
    glBindTexture(type, 0);
    if (glIsTexture(texture))
        state = VALID;
    else state = INVALID;
}

void Texture::update(const void* data, unsigned int pixelFormat, unsigned int colorFormat, vec3i offset, vec3i subSize)
{
    auto& n = name;
    const auto CheckError = [n](const char* label)
    {
        GLenum error = glGetError();
        if (!label)
            return false;
        switch (error)
        {
            case GL_INVALID_ENUM: std::cout << n << " : " << label << " : GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE: std::cout << n << " : " << label << " : GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cout << n << " : " << label << " : GL_INVALID_OPERATION (or called out of render thread)" << std::endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: std::cout << n << label << " : GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl; break;
            case GL_OUT_OF_MEMORY: std::cout << n << " : " << label << " : GL_OUT_OF_MEMORY" << std::endl; break;
            case GL_STACK_UNDERFLOW: std::cout << n << " : " << label << " : GL_STACK_UNDERFLOW" << std::endl; break;
            case GL_STACK_OVERFLOW: std::cout << n << " : " << label << " : GL_STACK_OVERFLOW" << std::endl; break;
            default: break;
        }
        return error != GL_NO_ERROR;
    };

    vec3i updateSize = vec3i::min(size, subSize);
    switch ((TextureConfiguration)(configuration & (uint8_t)TextureConfiguration::TYPE_MASK))
    {
        case TextureConfiguration::TEXTURE_1D:
            glBindTexture(GL_TEXTURE_1D, texture);
            glTexSubImage1D(GL_TEXTURE_1D, 0, offset.x, updateSize.x, pixelFormat, colorFormat, data);
            CheckError("glTexSubImage1D");
            break;

        case TextureConfiguration::TEXTURE_2D:
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y, updateSize.x, updateSize.y, pixelFormat, colorFormat, data);
            CheckError("glTexSubImage2D");
            break;

        case TextureConfiguration::TEXTURE_3D:
            glBindTexture(GL_TEXTURE_3D, texture);
            glTexSubImage3D(GL_TEXTURE_3D, 0, offset.x, offset.y, offset.z, updateSize.x, updateSize.y, updateSize.z, pixelFormat, colorFormat, data);
            CheckError("glTexSubImage3D");
            break;

        case TextureConfiguration::TEXTURE_ARRAY:
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, offset.x, offset.y, offset.z, updateSize.x, updateSize.y, updateSize.z, pixelFormat, colorFormat, data);
            CheckError("glTexSubImage3D");
            break;

        default:
            return;
    }
    glBindTexture(m_type, 0);
}


#ifdef USE_IMGUI
const char* GetInternalFormatString(unsigned int format)
{
    switch (format)
    {
        case GL_RGBA: return "GL_RGBA";
        case GL_R8: return "GL_R8";
        case GL_R8_SNORM: return "GL_R8_SNORM";
        case GL_R16: return "GL_R16";
        case GL_R16_SNORM: return "GL_R16_SNORM";
        case GL_RG8: return "GL_RG8";
        case GL_RG8_SNORM: return "GL_RG8_SNORM";
        case GL_RG16: return "GL_RG16";
        case GL_RG16_SNORM: return "GL_RG16_SNORM";
        case GL_R3_G3_B2: return "GL_R3_G3_B2";
        case GL_RGB4: return "GL_RGB4";
        case GL_RGB5: return "GL_RGB5";
        case GL_RGB8: return "GL_RGB8";
        case GL_RGB8_SNORM: return "GL_RGB8_SNORM";
        case GL_RGB10: return "GL_RGB10";
        case GL_RGB12: return "GL_RGB12";
        case GL_RGB16_SNORM: return "GL_RGB16_SNORM";
        case GL_RGBA2: return "GL_RGBA2";
        case GL_RGBA4: return "GL_RGBA4";
        case GL_RGB5_A1: return "GL_RGB5_A1";
        case GL_RGBA8: return "GL_RGBA8";
        case GL_RGBA8_SNORM: return "GL_RGBA8_SNORM";
        case GL_RGB10_A2: return "GL_RGB10_A2";
        case GL_RGB10_A2UI: return "GL_RGB10_A2UI";
        case GL_RGBA12: return "GL_RGBA12";
        case GL_RGBA16: return "GL_RGBA16";
        case GL_SRGB8: return "GL_SRGB8";
        case GL_SRGB8_ALPHA8: return "GL_SRGB8_ALPHA8";
        case GL_R16F: return "GL_R16F";
        case GL_RG16F: return "GL_RG16F";
        case GL_RGB16F: return "GL_RGB16F";
        case GL_RGBA16F: return "GL_RGBA16F";
        case GL_R32F: return "GL_R32F";
        case GL_RG32F: return "GL_RG32F";
        case GL_RGB32F: return "GL_RGB32F";
        case GL_RGBA32F: return "GL_RGBA32F";
        case GL_R11F_G11F_B10F: return "GL_R11F_G11F_B10F";
        case GL_RGB9_E5: return "GL_RGB9_E5";
        case GL_R8I: return "GL_R8I";
        case GL_R8UI: return "GL_R8UI";
        case GL_R16I: return "GL_R16I";
        case GL_R16UI: return "GL_R16UI";
        case GL_R32I: return "GL_R32I";
        case GL_R32UI: return "GL_R32UI";
        case GL_RG8I: return "GL_RG8I";
        case GL_RG8UI: return "GL_RG8UI";
        case GL_RG16I: return "GL_RG16I";
        case GL_RG16UI: return "GL_RG16UI";
        case GL_RG32I: return "GL_RG32I";
        case GL_RG32UI: return "GL_RG32UI";
        case GL_RGB8I: return "GL_RGB8I";
        case GL_RGB8UI: return "GL_RGB8UI";
        case GL_RGB16I: return "GL_RGB16I";
        case GL_RGB16UI: return "GL_RGB16UI";
        case GL_RGB32I: return "GL_RGB32I";
        case GL_RGB32UI: return "GL_RGB32UI";
        case GL_RGBA8UI: return "GL_RGBA8UI";
        case GL_RGBA8I: return "GL_RGBA8I";
        case GL_RGBA16I: return "GL_RGBA16I";
        case GL_RGBA16UI: return "GL_RGBA16UI";
        case GL_RGBA32I: return "GL_RGBA32I";
        case GL_RGBA32UI: return "GL_RGBA32UI";
        case GL_DEPTH_COMPONENT32F: return "GL_DEPTH_COMPONENT32F";
        case GL_DEPTH_COMPONENT24: return "GL_DEPTH_COMPONENT24";
        case GL_DEPTH_COMPONENT16: return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH32F_STENCIL8: return "GL_DEPTH32F_STENCIL8";
        case GL_DEPTH24_STENCIL8: return "GL_DEPTH24_STENCIL8";
        case GL_STENCIL_INDEX8: return "GL_STENCIL_INDEX8";
        default: return "Unknown";
    }
}
bool IsInternalFormatOverviewable(unsigned int format)
{
    switch (format)
    {
        case GL_RGBA:
        case GL_RGBA8:
            return true;
        default: return false;
    }
}

#endif

void Texture::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Texture infos");

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
        case Texture::TextureConfiguration::TEXTURE_ARRAY:
            ImGui::Text("Type : 2D Array");
            ImGui::Text("Width : %d", (int)size.x);
            ImGui::Text("Height : %d", (int)size.y);
            ImGui::Text("Layers : %d", (int)size.z);
            break;
        default:
            break;
    }
    ImGui::Text("Internal format : %s", GetInternalFormatString(m_internalFormat));

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
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Overview");

    if (type == Texture::TextureConfiguration::TEXTURE_2D && IsInternalFormatOverviewable(m_internalFormat))
    {
        Debug::setBlending(false);
        ImGui::Image((void*)texture, ImVec2(size.x * ratio, size.y * ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    }
    else if (textureLayers)
    {
        ImGui::SliderInt("Layer", &layerOverview, 0, size.z - 1);
        ImGui::Image((void*)textureLayers[layerOverview], ImVec2(size.x * ratio, size.y * ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    }
    else
    {
        if (!Texture::sharedTextureOverview)
        {
            Texture::sharedTextureOverview = new Texture("sharedTextureOverview", Texture::TextureConfiguration::TEXTURE_2D);
            Texture::sharedTextureOverview->initialize(vec3i(size.x, size.y, 0), nullptr, (uint8_t)Texture::TextureConfiguration::TEXTURE_2D);
            ResourceManager::getInstance()->addResource(Texture::sharedTextureOverview);
            Texture::sharedTextureOverview->isEnginePrivate = true;
        }

        float layer = 0;
        if (size.z > 0)
        {
            int layerCount = size.z;
            ImGui::SliderInt("Layer", &layerOverview, 0, layerCount - 1);
            layer = (float)layerOverview;
        }
        else if (m_internalFormat == GL_RGBA16UI && m_type == GL_TEXTURE_2D && isEnginePrivate) // terrain virtual texture
        {
            // 5 "layer" : height, water, normal, material, hole
            ImGui::SliderInt("Layer", &layerOverview, 0, 4);
            layer = (float)layerOverview;
        }

        Debug::reinterpreteTexture(this, sharedTextureOverview, layer);
        ImGui::Image((void*)sharedTextureOverview->getTextureId(), ImVec2(size.x * ratio, size.y * ratio), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    }

#endif
}