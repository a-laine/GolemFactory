#pragma once

#include <fstream>
#include <map>
#include <vector>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Texture.h"


class Shader : public ResourceVirtual
{
    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        static int computeVariantCode(bool instanced, int shadow, bool wireframe);

        //  Miscellaneous
        enum class ShaderType
        {
            VERTEX_SH = 1,      //!< vertex shader
            FRAGMENT_SH = 2,    //!< fragment shader

            GEOMETRIC_SH = 3,   //!< geometry shader
            TESS_EVAL_SH = 4,   //!< tessellation evaluation shader
            TESS_CONT_SH = 5,   //!< tessellation control shader

            PROGRAM_SH = 6,     //!< shader program (vertex + fragment + ...)

            COMPUTE_SH = 7      //!< compute shader
        };
        static std::string toString(ShaderType shaderType);

        struct TextureInfos
        {
            std::string identifier;
            std::string defaultResource;
            uint8_t unit;
            GLuint location;
            bool isGlobalAttribute;
            Texture* texture;
        };
        //

        //  Default
		explicit Shader(const std::string& shaderName);
        ~Shader();
        //

        //  Public functions
        void initialize(GLuint vertexSh, GLuint fragSh, GLuint geomShr, GLuint tessControlSh, GLuint tessEvalSh, GLuint prog,
            const std::map<std::string, std::string>& attType, const std::vector<std::pair<std::string, std::string>>& textures, uint16_t queue);
        void initialize(GLuint computeSh, GLuint prog, const std::map<std::string, std::string>& attType, const std::vector<std::pair<std::string, std::string>>& textures);
        void enable();

		//void setInstanciable(Shader* instaciedVersion);
        void addVariant(int variantCode, Shader* variantShader);
		
		GLuint getProgram() const;
        int getTextureCount() const;
        GLuint getShaderID(ShaderType shaderType) const;
		bool useShaderType(ShaderType shaderType) const;
        uint16_t getRenderQueue() const;
        uint8_t getGlobalTextureUnit(std::string _name) const;

		int getUniformLocation(const std::string& uniform);
		std::string getUniformType(const std::string& uniform);
        bool supportInstancing() const;
        bool isComputeShader() const;
        Shader* getVariant(int variantCode);

        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        //void pushTexture(Texture* texture);

        const std::vector<TextureInfos>& getTextures() const { return m_textures; }
        const std::map<std::string, std::string>& getUniforms() const {return attributesType; }

        void onDrawImGui() override;
        //

    private:
        void loadGlobalTexture(const std::string& type, const std::string& identifier, GLuint location, uint8_t unit);


        static std::string defaultName;

        //  Attributes
        GLuint  vertexShader,								//!< Vertex shader opengl id
                fragmentShader,								//!< Fragment shader opengl id
                geometricShader,							//!< Geometry shader opengl id
                tessControlShader,							//!< Tesselation control shader opengl id
                tessEvalShader,								//!< Tesselation evaluation shader opengl id
                program;									//!< Program opengl id
        //uint8_t textureCount;								//!< The number of texture use by the program
        uint16_t renderQueue;
        bool m_isComputeShader;
        std::map<std::string, GLint> attributesLocation;	//!< The shader attribute map with their opengl location
		std::map<std::string, std::string> attributesType;
        //std::vector<std::string> textureIdentifiers;
        //std::vector<Texture*> textures;
        std::vector<TextureInfos> m_textures;
        std::map<int, Shader*> variants;

#ifdef USE_IMGUI
        int dynamicQueue;
#endif
};
