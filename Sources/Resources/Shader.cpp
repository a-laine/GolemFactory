#include "Shader.h"

#include <Resources/Loader/ShaderLoader.h>
#include <Utiles/Parser/Reader.h>
#include <Utiles/Assert.hpp>

//  Static attributes
char const * const Shader::directory = "Shaders/";
char const * const Shader::extension = ".shader";
std::string Shader::defaultName;
//

//  Default
Shader::Shader(const std::string& shaderName)
    : ResourceVirtual(shaderName)
    , vertexShader(0), fragmentShader(0), geometricShader(0)
    , tessControlShader(0), tessEvalShader(0), program(0)
    , textureCount(0), instanciable(nullptr)
{}
Shader::~Shader()
{
    glDeleteProgram(program);
}
//


//  Public functions
void Shader::initialize(GLuint  vertexSh, GLuint fragSh, GLuint geomShr, GLuint tessControlSh, GLuint tessEvalSh, GLuint prog,
    const std::map<std::string, std::string>& attType, const std::vector<std::string>& textures)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    // initialize attributes
    vertexShader = vertexSh;
    fragmentShader = fragSh;
    geometricShader = geomShr;
    tessControlShader = tessControlSh;
    tessEvalShader = tessEvalSh;
    program = prog;
    textureCount = 0;
    attributesType = attType;

    //	get default matrix attribute location
    for(auto& it = attType.begin(); it != attType.end(); it++)
    {
        const std::string& uniformName = it->first;
        GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
        attributesLocation[uniformName] = uniformLocation;
        if(uniformLocation < 0)
        {
            if(uniformName.size() >= 3 && uniformName[0] == 'g' && uniformName[1] == 'l' && uniformName[2] == '_' && ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                std::cerr << "ERROR : loading shader : " << name << " : error in loading '" << uniformName << "' : name format not allowed, remove the 'gl_' prefix." << std::endl;
            else if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
                std::cerr << "ERROR : loading shader : " << name << " : ERROR in loading '" << uniformName << "' variable location : " << uniformLocation << "; maybe the variable name does not correspond to an active uniform variable" << std::endl;
        }
    }

    //  Binding texture to sample
    if(!textures.empty())
    {
        glUseProgram(program);
        try
        {
            textureCount = (uint8_t) textures.size();
            GLuint location;
            for(int i = 0; i < textureCount; i++)
            {
                location = glGetUniformLocation(program, textures[i].c_str());
                attributesLocation[textures[i]] = location;
                glUniform1i(location, i);
            }
        }
        catch(std::exception&) { textureCount = 0; }
        glUseProgram(0);
    }

    if(glIsProgram(program))
        state = VALID;
    else
        state = INVALID;
}

void Shader::enable() { glUseProgram(program); }
void Shader::setInstanciable(Shader* instaciedVersion) { instanciable = instaciedVersion; }
GLuint Shader::getProgram() const { return program; }
int Shader::getTextureCount() const { return textureCount; }
GLuint Shader::getShaderID(ShaderType shaderType) const
{
    switch(shaderType)
    {
        case VERTEX_SH:     return vertexShader;
        case GEOMETRIC_SH:  return geometricShader;
        case FRAGMENT_SH:   return fragmentShader;
        case PROGRAM_SH:    return program;
        case TESS_EVAL_SH:  return tessEvalShader;
        case TESS_CONT_SH:  return tessControlShader;
        default:            return 0;
    }
}
bool Shader::useShaderType(ShaderType shaderType) const
{
    switch(shaderType)
    {
        case VERTEX_SH:     return glIsShader(vertexShader) != 0;
        case GEOMETRIC_SH:  return glIsShader(geometricShader) != 0;
        case FRAGMENT_SH:   return glIsShader(fragmentShader) != 0;
        case PROGRAM_SH:    return glIsShader(program) != 0;
        case TESS_EVAL_SH:  return glIsShader(tessEvalShader) != 0;
        case TESS_CONT_SH:  return glIsShader(tessControlShader) != 0;
        default:            return false;
    }
}

int Shader::getUniformLocation(const std::string& uniform)
{
	auto it = attributesLocation.find(uniform);
	if (it != attributesLocation.end())
		return (int) it->second;
	else return -1;
}
std::string Shader::getUniformType(const std::string& uniform)
{
	auto it = attributesType.find(uniform);
	if (it != attributesType.end())
		return it->second;
	else return "";
}
std::string Shader::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
std::string Shader::getIdentifier() const
{
    return getIdentifier(name);
}
std::string Shader::getLoaderId(const std::string& resourceName) const
{
    return extension;
}
Shader* Shader::getInstanciable() const { return instanciable; }

const std::string& Shader::getDefaultName() { return defaultName; }
void Shader::setDefaultName(const std::string& name) { defaultName = name; }
//


//  Private functions
std::string Shader::toString(ShaderType shaderType)
{
	switch (shaderType)
	{
		case VERTEX_SH:     return "vertex shader";
		case GEOMETRIC_SH:  return "geometry shader";
		case FRAGMENT_SH:   return "fragment shader";
		case TESS_EVAL_SH:  return "tesselation evaluation shader";
		case TESS_CONT_SH:  return "tesselation control shader";
		default: return "unknown shader";
	}
}
//
