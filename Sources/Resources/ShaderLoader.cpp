#include "ShaderLoader.h"

#include <iostream>

#include "Utiles/Parser/Reader.h"



bool ShaderLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    //  Initialization
    vertexShader = 0;       fragmentShader = 0;
    geometricShader = 0;    tessControlShader = 0;      tessEvalShader = 0;
    program = 0;

    Variant v; Variant* tmp = nullptr;
    try
    {
        Reader::parseFile(v, getFileName(resourceDirectory, fileName));
        tmp = &(v.getMap().begin()->second);
    }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading shader : " << fileName << " : fail to open or parse file" << std::endl;
        return false;
    }
    Variant& shaderMap = *tmp;
    std::string path = resourceDirectory + Shader::directory;

    if(shaderMap.getType() != Variant::MAP)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
            std::cerr << "ERROR : loading shader : " << fileName << " : wrong file formating" << std::endl;
        return false;
    }


    //  Vertex shader
    std::string tmpName;
    try { tmpName = path + shaderMap["vertex"].toString(); }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::WARNINGS)
            std::cerr << "WARNING : loading shader : " << fileName << " : fail to parse vertex shader name, try loading default.vs instead" << std::endl;
        tmpName = path + "default.vs";
    }
    if(!loadShader(Shader::VERTEX_SH, tmpName, vertexShader))
    {
        return false;
    }

    //  Fragment shader
    try { tmpName = path + shaderMap["fragment"].toString(); }
    catch(std::exception&)
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::WARNINGS)
            std::cerr << "WARNING : loading shader : " << fileName << " : fail to parse fragment shader name, try loading default.fs instead" << std::endl;
        tmpName = path + "default.fs";
    }
    if(!loadShader(Shader::FRAGMENT_SH, tmpName, fragmentShader))
    {
        glDeleteShader(vertexShader);
        return false;
    }

    //  Program
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    //  Geometry shader
    try
    {
        tmpName = path + shaderMap["geometry"].toString();
        if(loadShader(Shader::GEOMETRIC_SH, tmpName, geometricShader))
            glAttachShader(program, geometricShader);
    }
    catch(std::exception&) {}

    //  Tessellation evaluation shader
    try
    {
        tmpName = path + shaderMap["evaluation"].toString();
        if(loadShader(Shader::TESS_EVAL_SH, tmpName, tessEvalShader))
            glAttachShader(program, tessEvalShader);
    }
    catch(std::exception&) {}

    //  Tessellation control shader
    try
    {
        tmpName = path + shaderMap["control"].toString();
        if(loadShader(Shader::TESS_CONT_SH, tmpName, tessControlShader))
            glAttachShader(program, tessControlShader);
    }
    catch(std::exception&) {}

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
    GLint uniformLocation;
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
                if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::WARNINGS)
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

std::string ShaderLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Shader::directory;
    str += fileName;
    str += Shader::extension;
    return str;
}

bool ShaderLoader::loadShader(Shader::ShaderType shaderType, std::string fileName, GLuint& shader)
{
    std::string source;
    std::ifstream file(fileName);

    ResourceVirtual::VerboseLevel errorLevel;
    const char* gravityIssue;
    if(shaderType == Shader::VERTEX_SH || shaderType == Shader::FRAGMENT_SH)
    {
        gravityIssue = "ERROR";
        errorLevel = ResourceVirtual::ERRORS;
    }
    else
    {
        gravityIssue = "WARRNING";
        errorLevel = ResourceVirtual::WARNINGS;
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

    // Generate ID from OpenGL
    switch(shaderType)
    {
        case Shader::VERTEX_SH:     shader = glCreateShader(GL_VERTEX_SHADER);          break;
        case Shader::GEOMETRIC_SH:  shader = glCreateShader(GL_GEOMETRY_SHADER);        break;
        case Shader::FRAGMENT_SH:   shader = glCreateShader(GL_FRAGMENT_SHADER);        break;
        case Shader::TESS_EVAL_SH:  shader = glCreateShader(GL_TESS_EVALUATION_SHADER); break;
        case Shader::TESS_CONT_SH:  shader = glCreateShader(GL_TESS_CONTROL_SHADER);    break;
        default:
            if(ResourceVirtual::logVerboseLevel >= errorLevel)
                std::cerr << gravityIssue << " : loading shader : " << fileName << " : in resource " << Shader::toString(shaderType) << std::endl;
            return false;
    }
    if(!shader)
    {
        if(ResourceVirtual::logVerboseLevel >= errorLevel)
            std::cerr << gravityIssue << " : loading shader : " << fileName << " : in resource " << Shader::toString(shaderType) << " : fail to create OPENGL shader" << std::endl;
        return false;
    }

    // Compile shader source
    const char* sourceData = source.data();
    glShaderSource(shader, 1, (const GLchar**) (&sourceData), NULL);
    glCompileShader(shader);

    GLint compile_status = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        GLint logsize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
        char *log = new char[logsize];
        glGetShaderInfoLog(shader, logsize, &logsize, log);

        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::ERRORS)
        {
            std::cerr << "ERROR : loading shader : " << fileName << " : in resource " << Shader::toString(shaderType) << " : fail to compile" << std::endl << std::endl;
            std::cerr << log << std::endl << std::endl;
        }
        delete[] log;
        return false;
    }

    // End
    return true;
}

