#pragma once

#include <map>
#include <GL/glew.h>

#include "Resources/IResourceLoader.h"
#include "Resources/Shader.h"



class ShaderLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

    //private:
        static std::string getFileName(const std::string& resourceDirectory, const std::string& fileName);
        bool loadShader(Shader::ShaderType shaderType, std::string filename, GLuint& shader);


        GLuint  vertexShader,
                fragmentShader,
                geometricShader,
                tessControlShader,
                tessEvalShader,
                program;
        std::map<std::string, std::string> attributesType;
        std::vector<std::string> textures;
};

