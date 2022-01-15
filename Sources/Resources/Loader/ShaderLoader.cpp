#include "ShaderLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Utiles/Parser/Writer.h>


ShaderLoader::ShaderLoader() : vertexShader(0), fragmentShader(0), geometricShader(0), tessControlShader(0), tessEvalShader(0), program(0)
{
    codeBlockKeys.push_back("vertex");
    codeBlockKeys.push_back("fragment");
    codeBlockKeys.push_back("geometry");
    codeBlockKeys.push_back("evaluation");
    codeBlockKeys.push_back("control");
}

bool ShaderLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
	//  Initialization
    clear();
    std::string fullFileName = getFileName(resourceDirectory, fileName);

    Variant v; Variant* tmp = nullptr;
    try
    {
        std::ifstream strm(fullFileName.c_str());
        if (!strm.is_open())
            throw std::invalid_argument("Reader::parseFile : Cannot opening file");

        Reader reader(&strm);
        reader.codeBlocksKeys = &codeBlockKeys;
        reader.parse(v);

        tmp = &(v.getMap().begin()->second);
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading shader : " << fileName << " : fail to open or parse file" << std::endl;
        return false;
    }
    Variant& shaderMap = *tmp;
    std::string path = resourceDirectory + Shader::directory;

    if(shaderMap.getType() != Variant::MAP)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : loading shader : " << fileName << " : wrong file formating" << std::endl;
        return false;
    }

    // main shaders
    bool vertSuccess = tryCompile(shaderMap, Shader::ShaderType::VERTEX_SH, "vertex", vertexShader, resourceDirectory, fileName);
    bool fragSuccess = tryCompile(shaderMap, Shader::ShaderType::FRAGMENT_SH, "fragment", fragmentShader, resourceDirectory, fileName);
    if (!fragSuccess || !vertSuccess)
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    //  Optional staging
    tryAttach(shaderMap, Shader::ShaderType::GEOMETRIC_SH, "geometry", geometricShader, program, resourceDirectory, fullFileName);
    tryAttach(shaderMap, Shader::ShaderType::TESS_EVAL_SH, "evaluation", tessEvalShader, program, resourceDirectory, fullFileName);
    tryAttach(shaderMap, Shader::ShaderType::TESS_CONT_SH, "control", tessControlShader, program, resourceDirectory, fullFileName);

    //  Linking program
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if(glIsShader(geometricShader))   glDeleteShader(geometricShader);
    if(glIsShader(tessControlShader)) glDeleteShader(tessControlShader);
    if(glIsShader(tessEvalShader))    glDeleteShader(tessEvalShader);

    GLint compile_status = GL_TRUE;
    glGetShaderiv(program, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        GLint logsize;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logsize);
        char *log = new char[logsize];
        glGetShaderInfoLog(program, logsize, &logsize, log);
        std::cout << log << std::endl;
        delete[] log;
    }

    //	get default matrix attribute location
    if(shaderMap["uniform"].getType() == Variant::MAP)
    {
        for(auto it = shaderMap["uniform"].getMap().begin(); it != shaderMap["uniform"].getMap().end(); it++)
        {
            try
            {
                attributesType[it->first] = it->second.toString();
            }
            catch(std::exception&)
            {
                if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
                    std::cerr << "WARNING : loading shader : " << fileName << " : fail to load uniform : " << it->first << std::endl;
            }
        }
    }

    try
    {
        size_t textureCount = shaderMap["textures"].size();
        textures.resize(textureCount);
        for(int i = 0; i < textureCount; i++)
        {
            textures[i] = shaderMap["textures"][i].toString();
        }
    }
    catch(std::exception&) { textures.clear(); }

    return true;
}

void ShaderLoader::initialize(ResourceVirtual* resource)
{
    Shader* shader = static_cast<Shader*>(resource);
    shader->initialize(vertexShader, fragmentShader, geometricShader, tessControlShader, tessEvalShader, program, attributesType, textures);
}

void ShaderLoader::getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList)
{}

std::string ShaderLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName)
{
    std::string str = resourceDirectory;
    str += Shader::directory;
    str += fileName;
    str += Shader::extension;
    return str;
}



void ShaderLoader::clear()
{
    vertexShader = 0;
    fragmentShader = 0;
    geometricShader = 0;
    tessControlShader = 0;
    tessEvalShader = 0;
    program = 0;

    attributesType.clear();
    textures.clear();
}

bool ShaderLoader::tryCompile(Variant& shaderMap, Shader::ShaderType shaderType, const std::string& key, GLuint& shader, const std::string& resourceDirectory, const std::string& filename)
{
    try
    {
        if (shaderMap[key].getType() == Variant::VariantType::STRING)
        {
            std::string file = resourceDirectory + Shader::directory + shaderMap[key].toString();
            if (!loadShader(shaderType, file, shader))
                return false;
        }
        else
        {
            std::string source = shaderMap[key].toString();
            if (!source.empty()) source[0] = ' ';
            std::string header = "ERROR : loading shader : " + filename + " : in resource ";
            if (!loadSource(shaderType, source, shader, header, ResourceVirtual::VerboseLevel::ALL))
                return false;
        }
    }
    catch (std::exception&)
    {
        if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
            std::cerr << "WARNING : loading shader : " << filename << " : fail to parse vertex shader name or source, try loading default.vs instead" << std::endl;
        std::string file = resourceDirectory + Shader::directory + "default.vs";
        if (!loadShader(shaderType, file, shader))
            return false;
    }
    return true;
}

void ShaderLoader::tryAttach(Variant& shaderMap, Shader::ShaderType shaderType, const std::string& key, GLuint& shader, GLuint& program, const std::string& resourceDirectory, const std::string& filename)
{
    try
    {
        if (shaderMap[key].getType() == Variant::VariantType::STRING)
        {
            std::string value = resourceDirectory + Shader::directory + shaderMap[key].toString();
            if (loadShader(Shader::ShaderType::GEOMETRIC_SH, value, shader))
                glAttachShader(program, shader);
        }
        else
        {
            std::string source = shaderMap[key].toString();
            if (!source.empty()) source[0] = ' ';
            std::string header = "ERROR : loading shader : " + filename + " : in resource ";
            if (!loadSource(Shader::ShaderType::GEOMETRIC_SH, source, shader, header, ResourceVirtual::VerboseLevel::WARNINGS))
                return;
            glAttachShader(program, shader);

        }
    }
    catch (std::exception&) {}
}

bool ShaderLoader::loadShader(Shader::ShaderType shaderType, std::string fileName, GLuint& shader)
{
    std::string source;
    std::ifstream file(fileName);

    ResourceVirtual::VerboseLevel errorLevel;
    std::string gravityIssue;
    if(shaderType == Shader::ShaderType::VERTEX_SH || shaderType == Shader::ShaderType::FRAGMENT_SH)
    {
        gravityIssue = "ERROR";
        errorLevel = ResourceVirtual::VerboseLevel::ERRORS;
    }
    else
    {
        gravityIssue = "WARRNING";
        errorLevel = ResourceVirtual::VerboseLevel::WARNINGS;
    }

    if(!file.good())
    {
        if(ResourceVirtual::logVerboseLevel >= errorLevel)
            std::cerr << gravityIssue << " : loading shader : " << fileName << " : in resource " << Shader::toString(shaderType) << " : fail to open file" << std::endl;
        return false;
    }
    else
    {
        source.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        file.close();
        if(source.empty())
        {
            if(ResourceVirtual::logVerboseLevel >= errorLevel)
                std::cerr << gravityIssue << " : loading shader : " << fileName << " : in resource " << Shader::toString(shaderType) << " : empty file" << std::endl;
            return false;
        }
    }

    return loadSource(shaderType, source, shader, gravityIssue + " : loading shader : " + fileName + " : in resource ", errorLevel);
}

bool ShaderLoader::loadSource(Shader::ShaderType shaderType, const std::string& source, GLuint& shader, const std::string& errorHeader, ResourceVirtual::VerboseLevel errorLevel)
{
    // Generate ID from OpenGL
    switch (shaderType)
    {
        case Shader::ShaderType::VERTEX_SH:     shader = glCreateShader(GL_VERTEX_SHADER);          break;
        case Shader::ShaderType::GEOMETRIC_SH:  shader = glCreateShader(GL_GEOMETRY_SHADER);        break;
        case Shader::ShaderType::FRAGMENT_SH:   shader = glCreateShader(GL_FRAGMENT_SHADER);        break;
        case Shader::ShaderType::TESS_EVAL_SH:  shader = glCreateShader(GL_TESS_EVALUATION_SHADER); break;
        case Shader::ShaderType::TESS_CONT_SH:  shader = glCreateShader(GL_TESS_CONTROL_SHADER);    break;
        default:
            if (ResourceVirtual::logVerboseLevel >= errorLevel)
                std::cerr << errorHeader << Shader::toString(shaderType) << std::endl;
            return false;
    }
    if (!shader)
    {
        if (ResourceVirtual::logVerboseLevel >= errorLevel)
            std::cerr << errorHeader << Shader::toString(shaderType) << " : fail to create OPENGL shader" << std::endl;
        return false;
    }

    // Compile shader source
    const char* sourceData = source.data();
    glShaderSource(shader, 1, (const GLchar**)(&sourceData), NULL);
    glCompileShader(shader);

    GLint compile_status = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE)
    {
        GLint logsize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
        char* log = new char[logsize];
        glGetShaderInfoLog(shader, logsize, &logsize, log);

        if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
        {
            std::cerr << errorHeader << std::endl;
            std::cerr << "FATAL ERROR : " << Shader::toString(shaderType) << " : fail to compile" << std::endl << std::endl;
            std::cerr << log << std::endl << std::endl;
        }
        delete[] log;
        return false;
    }
    return true;
}