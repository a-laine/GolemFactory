#pragma once

#include <GL/glew.h>
//#include <glm/glm.hpp>

#include "Math/TMath.h"
#include "ResourceVirtual.h"

class Shader;
class Texture : public ResourceVirtual
{
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
            TEXTURE_ARRAY = 4,
            CUBEMAP_ARRAY = 5,  
            TYPE_MASK = 0x07,       //!< the type mask byte

            MIN_NEAREST = 1 << 3,     //!< if not the GL_LINEAR criterion is used for minify
            MAG_NEAREST = 1 << 4,     //!< if not the GL_LINEAR criterion is used for magify
            USE_MIPMAP = 1 << 5,      //!< the mip-map flag

            WRAP_CLAMP = 0 << 6,      //!< clamp coordinates to the texture limit
            WRAP_REPEAT = 1 << 6,     //!< repeat if coordinates greater than 1
            WRAP_MIRROR = 2 << 6,     //!< same as repeat but mirrored the repeated texture
            WRAP_MASK = 0x03 << 6     //!< wrap mask byte
        };
        //

        //  Default
        explicit Texture();
        explicit Texture(const std::string& textureName, TextureConfiguration conf);
		explicit Texture(const std::string& textureName, uint8_t conf = 0x0);
        ~Texture();

        void initialize(const vec3i& imageSize, const uint8_t* data, uint8_t config = 0);
        void initialize(const std::string& textureName, const vec3i& imageSize, const void* data, uint8_t config, unsigned int internalFormat, unsigned int pixelFormat, unsigned int colorFormat, bool immutable = false);

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
        //

        //  Attributes
        vec3i size;                     //!< Texture size
        unsigned int m_internalFormat;
        unsigned int m_type;

        #ifdef USE_IMGUI
            bool isEnginePrivate = false;
        #endif

    private:
        static std::string defaultName;

        //void initOpenGL(const uint8_t* textureData, const std::string& textureName);


        //  Attributes
        GLuint texture;                 //!< Texture Id
        uint8_t configuration;          //!< Texture configuration byte

#ifdef USE_IMGUI
        int layerOverview;
        GLuint* textureLayers;
        static Texture* sharedTextureOverview;
#endif
        //
};
