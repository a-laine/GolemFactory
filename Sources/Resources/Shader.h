#pragma once

#include <fstream>
#include <map>
#include <vector>
#include <GL/glew.h>

#include "ResourceVirtual.h"

class Shader : public ResourceVirtual
{
    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

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

        static std::string toString(ShaderType shaderType);
        //

        //  Default
        Shader(const std::string& shaderName);
        ~Shader();
        //

        //  Public functions
        void initialize(GLuint  vertexSh, GLuint fragSh, GLuint geomShr, GLuint tessControlSh, GLuint tessEvalSh, GLuint prog,
            const std::map<std::string, std::string>& attType, const std::vector<std::string>& textures);
        void enable();

		void setInstanciable(Shader* instaciedVersion);
		
		GLuint getProgram() const;
        int getTextureCount() const;
        GLuint getShaderID(ShaderType shaderType) const;
		bool useShaderType(ShaderType shaderType) const;

		int getUniformLocation(const std::string& uniform);
		std::string getUniformType(const std::string& uniform);
		Shader* getInstanciable() const;

        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        //

    private:
        static std::string defaultName;

        //  Attributes
        GLuint  vertexShader,								//!< Vertex shader opengl id
                fragmentShader,								//!< Fragment shader opengl id
                geometricShader,							//!< Geometry shader opengl id
                tessControlShader,							//!< Tesselation control shader opengl id
                tessEvalShader,								//!< Tesselation evaluation shader opengl id
                program;									//!< Program opengl id
        uint8_t textureCount;								//!< The number of texture use by the program
        std::map<std::string,GLint> attributesLocation;	//!< The shader attribute map with their opengl location
		std::map<std::string, std::string> attributesType;
		Shader* instanciable;
        //
};
