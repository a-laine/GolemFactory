#pragma once

#include <fstream>
#include <map>
#include <GL/glew.h>

#include "ResourceVirtual.h"

class Shader : public ResourceVirtual
{
    public:
        //  Miscellaneous
        enum ShaderType
        {
            VERTEX_SH = 1,      //!< vertex shader
            FRAGMENT_SH = 2,    //!< fragment shader

            GEOMETRIC_SH = 3,   //!< geometry shader
            TESS_EVAL_SH = 4,   //!< tessellation evaluation shader
            TESS_CONT_SH = 5,   //!< tessellation control shader

            PROGRAM_SH = 6      //!< shader program (vertex + fragment + ...)
        };
        //

        //  Default
        Shader(std::string path,std::string shaderName);
        ~Shader();

        bool isValid() const;
        //

        //  Public functions
        void enable();

        GLuint getProgram() const;
        int getTextureCount() const;
        GLuint getShaderID(ShaderType shaderType) const;
		bool useShaderType(ShaderType shaderType) const;

		int getUniformLocation(std::string uniform);
        //

        //  Attributes
        static std::string extension;   //!< Default extension used for infos files
        //

    private:
        //  Private functions
        bool loadShader(ShaderType shaderType, std::string filename,GLuint& shader); //!< Load a shader in a specific place, with a specific source code defined in filename
        //

        //  Attributes
        GLuint  vertexShader,                           //!< Vertex shader opengl id
                fragmentShader,                         //!< Fragment shader opengl id
                geometricShader,                        //!< Geometry shader opengl id
                tessControlShader,                      //!< Tesselation control shader opengl id
                tessEvalShader,                         //!< Tesselation evaluation shader opengl id
                program;                                //!< Program opengl id
        uint8_t textureCount;                           //!< The number of texture use by the program
        std::map<std::string,GLuint> attributesLocation;//!< The shader attribute map with their opengl location
        //
};
