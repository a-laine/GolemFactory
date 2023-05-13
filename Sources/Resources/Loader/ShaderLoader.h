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
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

        static std::string getFileName(const std::string& resourceDirectory, const std::string& fileName);
        //

        // Loaded attributes
        GLuint  vertexShader,
                fragmentShader,
                geometricShader,
                tessControlShader,
                tessEvalShader,
                program;
        std::map<std::string, std::string> attributesType;
        std::vector<std::string> textureNames;
        std::vector<Texture*> textureResources;
        //

    private:
        // Helpers
        void clear();
        bool tryCompile(Variant& shaderMap, Shader::ShaderType shaderType, const std::string& key, GLuint& shader, const std::string& resourceDirectory, const std::string& filename);
        void tryAttach(Variant& shaderMap, Shader::ShaderType shaderType, const std::string& key, GLuint& shader, GLuint& program, const std::string& resourceDirectory, const std::string& filename);

        bool loadShader(Shader::ShaderType shaderType, std::string filename, GLuint& shader);
        bool loadSource(Shader::ShaderType shaderType, const std::string& source, GLuint& shader, const std::string& errorHeader, ResourceVirtual::VerboseLevel errorLevel);
        //

        // Private attributes
        std::vector<std::string> codeBlockKeys;
        //
};

