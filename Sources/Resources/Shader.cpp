#include "Shader.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Shader::extension = ".shader";
//

//  Default
Shader::Shader(std::string path,std::string shaderName) : ResourceVirtual(path,shaderName,ResourceVirtual::SHADER)
{
    //  Initialization
    vertexShader = 0;       fragmentShader = 0;
    geometricShader = 0;    tessControlShader = 0;      tessEvalShader = 0;
    program = 0;
    textureCount = 0;
    Variant v; Variant* tmp = NULL;
    std::string tmpName;

    try { Reader::parseFile(v, path + shaderName + extension);
          tmp = &(v.getMap().begin()->second);                       }
    catch(std::exception& e) { return; }
    Variant& shaderMap = *tmp;

    //  Vertex shader
    try { tmpName = path + shaderMap["vertex"].toString(); }
    catch(std::exception& e) { std::cerr<<"fail to parse vertex shader name"<<std::endl; tmpName = path + "default.vs"; }
    if(!loadShader(VERTEX_SH,loadSource(tmpName),vertexShader)) return;

    //  Fragment shader
    try { tmpName = path + shaderMap["fragment"].toString(); }
    catch(std::exception& e) { std::cerr<<"fail to parse fragment shader name"<<std::endl; tmpName = path + "default.fs"; }
    if(!loadShader(FRAGMENT_SH,loadSource(tmpName),fragmentShader)) { glDeleteShader(vertexShader); return; }

    //  Program
    program = glCreateProgram();
    glAttachShader(program,vertexShader);
    glAttachShader(program,fragmentShader);

    //  Geometry shader
    try {
        tmpName = path + shaderMap["geometry"].toString();
         if(loadShader(GEOMETRIC_SH,loadSource(tmpName),geometricShader))
            glAttachShader(program,geometricShader);                         }
    catch(std::exception& e){}

    //  Tessellation evaluation shader
    try {
        tmpName = path + shaderMap["evaluation"].toString();
        if(loadShader(TESS_EVAL_SH,loadSource(tmpName),tessEvalShader))
            glAttachShader(program,tessEvalShader);                         }
    catch(std::exception& e){}

    //  Tessellation control shader
    try {
        tmpName = path + shaderMap["control"].toString();
        if(loadShader(TESS_CONT_SH,loadSource(tmpName),tessControlShader))
            glAttachShader(program,tessControlShader);                         }
    catch(std::exception& e){}

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
        std::cout<<log<<std::endl;
        delete[] log;
    }

    //  Binding texture to sample
    glUseProgram(program);
    try
    {
        textureCount = shaderMap["textures"].size();
        GLuint location;
        for(int i=0;i<textureCount;i++)
        {
            location = glGetUniformLocation(program,shaderMap["textures"][i].toString().c_str());
            attributesLocation[shaderMap["textures"][i].toString()] = location;
            glUniform1i(location,i);
        }
    }
    catch(std::exception& e) { textureCount = 0; }
    glUseProgram(0);
}
Shader::~Shader()
{
    glDeleteProgram(program);
}

bool Shader::isValid() const { return glIsProgram(program); }
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
        case VERTEX_SH:     return glIsShader(vertexShader);
        case GEOMETRIC_SH:  return glIsShader(geometricShader);
        case FRAGMENT_SH:   return glIsShader(fragmentShader);
        case PROGRAM_SH:    return glIsShader(program);
        case TESS_EVAL_SH:  return glIsShader(tessEvalShader);
        case TESS_CONT_SH:  return glIsShader(tessControlShader);
        default:            return false;
    }
}
//


//  Private functions
bool Shader::loadShader(ShaderType shaderType,char* source,GLuint& shader)
{
    if(!source) std::cerr<<"no source"<<std::endl;
    if(!source) return false;

    // Generate ID from OpenGL
    switch(shaderType)
    {
        case VERTEX_SH:     shader = glCreateShader(GL_VERTEX_SHADER);          break;
        case GEOMETRIC_SH:  shader = glCreateShader(GL_GEOMETRY_SHADER);        break;
        case FRAGMENT_SH:   shader = glCreateShader(GL_FRAGMENT_SHADER);        break;
        case TESS_EVAL_SH:  shader = glCreateShader(GL_TESS_EVALUATION_SHADER); break;
        case TESS_CONT_SH:  shader = glCreateShader(GL_TESS_CONTROL_SHADER);    break;
        default:
            shader = 0;
            std::cerr<<"invalid shader type"<<std::endl;
            return false;
    }
    if(!shader)
    {
        std::cout<<"Fail to create shader of type("<<(int)shaderType<<')'<<std::endl;
        delete[] source;
        return false;
    }

    // Compile shader source
    glShaderSource(shader,1,(const GLchar**)(&source),NULL);
    glCompileShader(shader);

    GLint compile_status = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        GLint logsize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
        char *log = new char[logsize];
        glGetShaderInfoLog(shader, logsize, &logsize, log);

		std::cerr << "\n\nERROR : when compiling ";
		switch (shaderType)
		{
			case VERTEX_SH:     std::cerr << "vertex";					break;
			case GEOMETRIC_SH:  std::cerr << "geometry";				break;
			case FRAGMENT_SH:   std::cerr << "fragment";				break;
			case TESS_EVAL_SH:  std::cerr << "tesselation evaluation";	break;
			case TESS_CONT_SH:  std::cerr << "tesselation control";		break;
			default: break;
		}
		std::cout << "shader\n\n" << log << std::endl << std::endl;

        delete[] log;
        delete[] source;
        return false;
    }

    // End
    delete[] source;
    return true;
}
char* Shader::loadSource(std::string file)
{
    // Open file
    FILE *fi = fopen(file.c_str(), "r");
    if(!fi)
    {
        std::cout<<"Impossible to open:\n"<<file<<std::endl;
        return NULL;
    }
    fseek(fi, 0, SEEK_END);
    long size = ftell(fi);
    rewind(fi);

    // Import file in char*
    char *source = new char[size+1];
    if(!source)
    {
        std::cout<<"Memory allocation error"<<std::endl;
        fclose(fi);
        return NULL;
    }
    size = fread(source,sizeof(char),size,fi);
    source[size] = '\0';
    fclose(fi);
    return source;
}
//
