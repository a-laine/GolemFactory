#pragma once

#include <map>
#include <GL/glew.h>

#include <Resources/IResourceLoader.h>
#include <Resources/Shader.h>
#include <Utiles/Parser/Variant.h>



class ShaderLoader : public IResourceLoader
{
    public:
        // Default
        ShaderLoader();
        //

        // Interface
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;

        void PrintError(const char* filename, const char* msg) override;
        void PrintWarning(const char* filename, const char* msg) override;

        static std::string getFileName(const std::string& resourceDirectory, const std::string& fileName);
        //

        // Loaded attributes
        struct ShaderStruct
        {
            unsigned int  vertexShader,
                fragmentShader,
                geometricShader,
                tessControlShader,
                tessEvalShader,
                program;
            int variantCode;
            std::string allDefines;
        };
        std::vector<ShaderStruct> shaderVariants;
        uint16_t renderQueue;

        std::map<std::string, std::string> attributesType;
        std::vector<std::pair<std::string, std::string>> textures;
        std::vector<Shader::Property> m_properties;

        bool isComputeShader;
        bool usePeterPanning;
        unsigned int compute, computeProgram;
        //

    private:        
        struct InternalVariantDefine
        {
            std::vector<std::string> defines;
            std::string allDefines;
            int shaderCode;
        };

        // Helpers
        void clear();

        std::vector<std::string> extractPragmas(std::string& source);
        bool loadSourceCode(Variant& shaderMap, const std::string& key, std::string& destination, const std::string& filename, const std::string& resourceDirectory);
        bool compileSource(Shader::ShaderType shaderType, std::string source, std::vector<std::string> defines, GLuint& shader, const std::string& errorHeader);
        bool shouldAttachStage(std::vector<std::string>& stagePragmas, std::vector<std::string>& defines);
        //void getVariants(bool& instance, bool& shadow, bool& wired);
        std::vector<InternalVariantDefine> createVariantDefines();

        std::string getFile(std::string& filename);
        //

        // Private attributes
        std::string includes;
        std::vector<std::string> codeBlockKeys;
        std::vector<std::string> vertexPragmas;
        std::vector<std::string> fragmentPragmas;
        std::vector<std::string> geometryPragmas;
        std::vector<std::string> evaluationPragmas;
        std::vector<std::string> controlPragmas;

        std::string vertexSourceCode, fragmentSourceCode, geometrySourceCode, evaluationSourceCode, controlSourceCode, computeSourceCode;
        //
};

