#pragma once

#include <GL/glew.h>
#include <fstream>
#include <map>

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
		void loadUniformMatrix(float* model, float* view, float* projection);
		void loadUniformMatrix(char matrix,float* matrixPointer);

        GLuint getProgram() const;
        int getTextureCount() const;
        GLuint getShaderID(ShaderType shaderType) const;
        bool useShaderType(ShaderType shaderType) const;
        //

        //  Attributes
        static std::string extension;   //!< Default extension used for infos files
        //

    private:
        //  Private functions
        bool loadShader(ShaderType shaderType,char* source,GLuint& shader); //!< Load a shader in a specific place, with a specific source code
        char* loadSource(std::string file);             //!< Simple load source in string function
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

		GLuint projectionLoc, viewLoc, modelLoc;
        //
};
