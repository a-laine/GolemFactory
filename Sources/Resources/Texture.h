#pragma once

#include <vector>
#include <GL/glew.h>
//#include <glm/glm.hpp>

#include "Math/TMath.h"
#include "ResourceVirtual.h"

class Shader;
class TextureSaver;
class Texture : public ResourceVirtual
{
    friend class TextureSaver;

    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        //  Miscellaneous
        enum class TextureConfiguration
        {
            TEXTURE_1D = 1,         //!< 1 dimension texture
            TEXTURE_2D = 2,         //!< 2 dimensions texture
            TEXTURE_3D = 3,         //!< 3 dimensions texture
            CUBEMAP = 4,
            TEXTURE_ARRAY = 5,
            CUBEMAP_ARRAY = 6,  
            TYPE_MASK = 0x0F,       //!< the type mask byte

            MIN_NEAREST = 1 << 4,     //!< if not the GL_LINEAR criterion is used for minify
            MAG_NEAREST = 1 << 5,     //!< if not the GL_LINEAR criterion is used for magify
            USE_MIPMAP = 1 << 6,      //!< the mip-map flag

            WRAP_CLAMP = 0 << 7,      //!< clamp coordinates to the texture limit
            WRAP_REPEAT = 1 << 7,     //!< repeat if coordinates greater than 1
            WRAP_MIRROR = 2 << 7,     //!< same as repeat but mirrored the repeated texture
            WRAP_MASK = 0x03 << 7     //!< wrap mask byte
        };
        //

        //  Default
        explicit Texture();
        explicit Texture(const std::string& textureName, TextureConfiguration conf);
		explicit Texture(const std::string& textureName, uint16_t conf = 0x0);
        ~Texture();

        void initialize(const vec3i& imageSize, const uint8_t* data, uint16_t config = 0);
        void initialize(const std::string& textureName, const vec3i& imageSize, const void* data, uint16_t config, unsigned int internalFormat,
            unsigned int pixelFormat, unsigned int colorFormat, bool immutable = false);

        void update(const void* data, unsigned int pixelFormat, unsigned int colorFormat, 
            vec3i offset = vec3i(0), vec3i subSize = vec3i(std::numeric_limits<uint16_t>::max()));

        void onDrawImGui() override;
        //

        //  Set/get functions
        int getType();
        GLenum getGLenumType();
        GLuint getTextureId() const;
        GLuint* getTextureIdPointer();

        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        std::vector<std::string>& getLayerDescriptor();
        //

        //  Attributes
        vec3i size;                     //!< Texture size
        unsigned int m_internalFormat;
        unsigned int m_type;

#ifdef USE_IMGUI
        bool isEnginePrivate = false;
        bool enableExport = false;
#endif

    private:
        static std::string defaultName;

        //  Attributes
        uint16_t configuration;         //!< Texture configuration byte
        std::vector<std::string> m_layerDescriptor;
        GLuint texture;                 //!< Texture Id

#ifdef USE_IMGUI
        int layerOverview;
        GLuint* textureLayers;
        static Texture* sharedTextureOverview;
#endif
        //
};
