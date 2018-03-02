#include "Shader.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Shader::extension = ".shader";
//

//  Default
Shader::Shader(const std::string& path, const std::string& shaderName) : ResourceVirtual(shaderName, ResourceVirtual::SHADER)
{
    //  Initialization
    vertexShader = 0;       fragmentShader = 0;
    geometricShader = 0;    tessControlShader = 0;      tessEvalShader = 0;
    program = 0;
    textureCount = 0;

    Variant v; Variant* tmp = nullptr;
    try
	{
		Reader::parseFile(v, path + shaderName + extension);
		tmp = &(v.getMap().begin()->second);
	}
    catch(std::exception&)
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading shader : " << shaderName << " : fail to open or parse file" << std::endl;
		return;
	}
    Variant& shaderMap = *tmp;

    //  Vertex shader
	std::string tmpName;
    try { tmpName = path + shaderMap["vertex"].toString(); }
    catch(std::exception&)
	{
		if (logVerboseLevel >= ResourceVirtual::WARNINGS)
			std::cerr << "WARNING : loading shader : " << shaderName << " : fail to parse vertex shader name, try loading default.vs instead" << std::endl;
		tmpName = path + "default.vs";
	}
	if (!loadShader(VERTEX_SH, tmpName, vertexShader)) return;

    //  Fragment shader
    try { tmpName = path + shaderMap["fragment"].toString(); }
	catch (std::exception&)
	{
		if (logVerboseLevel >= ResourceVirtual::WARNINGS)
			std::cerr << "WARNING : loading shader : " << shaderName << " : fail to parse fragment shader name, try loading default.fs instead" << std::endl;
		tmpName = path + "default.fs";
	}
	if (!loadShader(FRAGMENT_SH, tmpName, fragmentShader)) { glDeleteShader(vertexShader); return; }

    //  Program
    program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

    //  Geometry shader
    try
	{
		tmpName = path + shaderMap["geometry"].toString();
		if(loadShader(GEOMETRIC_SH,tmpName,geometricShader))
			glAttachShader(program,geometricShader); 
	}
    catch(std::exception&){}

    //  Tessellation evaluation shader
    try
	{
        tmpName = path + shaderMap["evaluation"].toString();
        if(loadShader(TESS_EVAL_SH,tmpName,tessEvalShader))
            glAttachShader(program,tessEvalShader);
	}
    catch(std::exception&){}

    //  Tessellation control shader
    try
	{
        tmpName = path + shaderMap["control"].toString();
        if(loadShader(TESS_CONT_SH,tmpName,tessControlShader))
            glAttachShader(program,tessControlShader);
	}
    catch(std::exception&){}

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
	try
	{
		std::string uniformName;
		std::string uniformType;
		GLint uniformLocation;

		if (glIsProgram(program))
		{
			for (auto it = shaderMap["uniform"].getMap().begin(); it != shaderMap["uniform"].getMap().end(); it++)
			{
				try
				{
					uniformName = it->first;
					uniformType = it->second.toString();
					uniformLocation = glGetUniformLocation(program, uniformName.c_str());
					attributesLocation[uniformName] = uniformLocation;
					attributesType[uniformName] = uniformType;
					if (uniformLocation < 0)
					{
						if (uniformName.size() >= 3 && uniformName[0] == 'g' && uniformName[0] == 'l' && uniformName[0] == '_' && logVerboseLevel >= ResourceVirtual::ERRORS)
								std::cerr << "ERROR : loading shader : " << shaderName << " : error in loading '" << uniformName << "' : name format not allowed remove the 'gl_' prefix." << std::endl;
						else if (logVerboseLevel >= ResourceVirtual::ERRORS)
							std::cerr << "ERROR : loading shader : " << shaderName << " : ERROR in loading '" << uniformName << "' variable location : " << uniformLocation << "; maybe the variable name does not correspond to an active uniform variable" << std::endl;
					}
				}
				catch(std::exception&)
				{
					if (logVerboseLevel >= ResourceVirtual::WARNINGS)
						std::cerr << "WARNING : loading shader : " << shaderName << " : fail to load uniform : " << it->first << std::endl;
				}
			}
		}
	}
	catch (std::exception&) {}

    //  Binding texture to sample
    glUseProgram(program);
    try
    {
		textureCount = (uint8_t)shaderMap["textures"].size();
        GLuint location;
		for (int i = 0; i < textureCount; i++)
        {
			location = glGetUniformLocation(program, shaderMap["textures"][i].toString().c_str());
			attributesLocation[shaderMap["textures"][i].toString()] = location;
			glUniform1i(location, i);
        }
    }
	catch (std::exception&) { textureCount = 0; }
    glUseProgram(0);
}
Shader::~Shader()
{
    glDeleteProgram(program);
}

bool Shader::isValid() const { return glIsProgram(program) != 0; }
//


//  Public functions
void Shader::enable() { glUseProgram(program); }
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
//


//  Private functions
bool Shader::loadShader(ShaderType shaderType, std::string fileName, GLuint& shader)
{
	std::string source;
	std::ifstream file(fileName);

	ResourceVirtual::VerboseLevel errorLevel = ResourceVirtual::WARNINGS;
	std::string gravityIssue = "WARRNING";
	if (shaderType == VERTEX_SH || shaderType == FRAGMENT_SH)
	{
		gravityIssue = "ERROR";
		errorLevel = ResourceVirtual::ERRORS;
	}

	if (!file.good())
	{
		if (logVerboseLevel >= errorLevel)
			std::cerr << gravityIssue << " : loading shader : " << name << " : in resource " << toString(shaderType) << " : fail to open file" << std::endl;
		return false;
	}
	else
	{
		source.assign((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
		file.close();
		if (source.empty())
		{
			if (logVerboseLevel >= errorLevel)
				std::cerr << gravityIssue << " : loading shader : " << name << " : in resource " << toString(shaderType) << " : empty file" << std::endl;
			return false;
		}
	}

    // Generate ID from OpenGL
    switch(shaderType)
    {
        case VERTEX_SH:     shader = glCreateShader(GL_VERTEX_SHADER);          break;
        case GEOMETRIC_SH:  shader = glCreateShader(GL_GEOMETRY_SHADER);        break;
        case FRAGMENT_SH:   shader = glCreateShader(GL_FRAGMENT_SHADER);        break;
        case TESS_EVAL_SH:  shader = glCreateShader(GL_TESS_EVALUATION_SHADER); break;
        case TESS_CONT_SH:  shader = glCreateShader(GL_TESS_CONTROL_SHADER);    break;
        default:
			if (logVerboseLevel >= errorLevel)
				std::cerr << gravityIssue << " : loading shader : " << name << " : in resource " << toString(shaderType) << std::endl;
            return false;
    }
    if(!shader)
    {
		if (logVerboseLevel >= errorLevel)
			std::cerr << gravityIssue << " : loading shader : " << name << " : in resource " << toString(shaderType) << " : fail to create OPENGL shader" << std::endl;
        return false;
    }

    // Compile shader source
	const char* sourceData = source.data();
    glShaderSource(shader,1,(const GLchar**)(&sourceData),NULL);
    glCompileShader(shader);

    GLint compile_status = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        GLint logsize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
        char *log = new char[logsize];
        glGetShaderInfoLog(shader, logsize, &logsize, log);

		if (logVerboseLevel >= ResourceVirtual::ERRORS)
		{
			std::cerr << "ERROR : loading shader : " << name << " : in resource " << toString(shaderType) << " : fail to compile" << std::endl << std::endl;
			std::cerr << log << std::endl << std::endl;
		}
        delete[] log;
        return false;
    }

    // End
    return true;
}
std::string Shader::toString(const ShaderType& shaderType) const
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
