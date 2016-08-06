#pragma once

#include <fstream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"

class Texture : public ResourceVirtual
{
    public:
        //  Miscellaneous
        enum TextureConfiguration
        {
            TEXTURE_1D = 1,         //!< 1 dimension texture
            TEXTURE_2D = 2,         //!< 2 dimensions texture
            TEXTURE_3D = 3,         //!< 3 dimensions texture
            TYPE_MASK = 0x03,       //!< the type mask byte

            USE_MIPMAP = 1<<2,      //!< the mip-map flag
            MIN_NEAREST = 1<<3,     //!< if not the GL_LINEAR criterion is used for minify
            MAG_NEAREST = 1<<4,     //!< if not the GL_LINEAR criterion is used for magify

            WRAP_CLAMP = 0<<5,      //!< clamp coordinates to the texture limit
            WRAP_REPEAT = 1<<5,     //!< repeat if coordinates greater than 1
            WRAP_MIRROR = 2<<5,     //!< same as repeat but mirrored the repeated texture
            WRAP_MASK = 0x03<<5,    //!< wrap mask byte
        };
        //

        //  Default
        Texture(std::string path,std::string fontName,uint8_t conf = 0x0);
        ~Texture();

        bool isValid() const;
        //

        //  Set/get functions
        int getType();
        GLenum getGLenumType();
        GLuint getTextureId() const;
        GLuint* getTextureIdPt();
        //

        //  Attributes
        glm::vec3 size;                //!< Texture size
        static std::string extension;   //!< Default extension used for infos files
        //

    private:
        //  Attributes
        GLuint texture;                 //!< Texture Id
        uint8_t configuration;          //!< Texture configuration byte
        //
};
