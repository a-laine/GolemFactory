#include "ShaderLoader.h"

#include <iostream>

#include <Utiles/Parser/Reader.h>
#include <Utiles/Parser/Writer.h>
#include <Resources/ResourceManager.h>
#include <Utiles/ConsoleColor.h>



ShaderLoader::ShaderLoader()
{
    codeBlockKeys.push_back("includes");
    codeBlockKeys.push_back("vertex");
    codeBlockKeys.push_back("fragment");
    codeBlockKeys.push_back("geometry");
    codeBlockKeys.push_back("evaluation");
    codeBlockKeys.push_back("control");
    codeBlockKeys.push_back("compute");
    clear();
}

void ShaderLoader::PrintError(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : ShaderLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}

void ShaderLoader::PrintWarning(const char* filename, const char* msg)
{
    if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : ShaderLoader : " << filename << " : " << msg << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }
}

bool ShaderLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
	//  Initialization
    const uint16_t transparentBit = 1 << 15;
    const uint16_t faceCullingBit = 1 << 14;

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
        PrintError(fullFileName.c_str(), "fail to open or parse file");
        return false;
    }
    Variant& shaderMap = *tmp;
    std::string path = resourceDirectory + Shader::directory;

    if (shaderMap.getType() != Variant::MAP)
    {
        PrintError(fileName.c_str(), "wrong file formating");
        return false;
    }

    // see if it's compute shader
    auto it = shaderMap.getMap().find("computeShader");
    if (it != shaderMap.getMap().end())
    {
        bool b = false;
        if (it->second.getType() == Variant::BOOL)
            b = it->second.toBool();
        else
        {
            PrintWarning(fileName.c_str(), "computeShader attribute need to be a boolean");

            switch (it->second.getType())
            {
            case Variant::INT:      b = it->second.toInt() == 0;        break;
            case Variant::FLOAT:    b = it->second.toFloat() == 0.f;    break;
            case Variant::DOUBLE:   b = it->second.toDouble() == 0.0;   break;
            default: break;
            }
        }

        isComputeShader = b;
    }

    // load includes bloc
    it = shaderMap.getMap().find("includes");
    if (it != shaderMap.getMap().end() && it->second.getType() == Variant::CODEBLOCK)
    {
        includes = shaderMap["includes"].toString();
        if (!includes.empty())
            includes[0] = ' ';
    }

    // renderQueue
    renderQueue = 1000;
    it = shaderMap.getMap().find("renderQueue");
    if (it != shaderMap.getMap().end() && it->second.getType() == Variant::INT)
    {
        int value = shaderMap["renderQueue"].toInt();
        if (value > 4095)
        {
            PrintWarning(fileName.c_str(), "renderQueue attribute need to be an integer between 0 and 4095, value has been clamped");
            renderQueue = 4095;
        }
        else if (value < 0)
        {
            PrintWarning(fileName.c_str(), "renderQueue attribute need to be an integer between 0 and 4095, value has been clamped");
            renderQueue = 0;
        }
        else
        {
            renderQueue = value;
        }
    }

    // transparent
    it = shaderMap.getMap().find("transparent");
    if (it != shaderMap.getMap().end())
    {
        bool b = false;
        if (it->second.getType() == Variant::BOOL)
            b = it->second.toBool();
        else
        {
            PrintWarning(fileName.c_str(), "transparent attribute need to be a boolean");

            switch (it->second.getType())
            {
                case Variant::INT:      b = it->second.toInt() == 0;        break;
                case Variant::FLOAT:    b = it->second.toFloat() == 0.f;    break;
                case Variant::DOUBLE:   b = it->second.toDouble() == 0.0;   break;
                default: break;
            }
        }

        if (b)
            renderQueue |= transparentBit;
    }
    
    // faceCulling
    it = shaderMap.getMap().find("faceCulling");
    if (it != shaderMap.getMap().end())
    {
        bool b = false;
        if (it->second.getType() == Variant::BOOL)
            b = it->second.toBool();
        else
        {
            PrintWarning(fileName.c_str(), "faceCulling attribute need to be a boolean");

            switch (it->second.getType())
            {
                case Variant::INT:      b = it->second.toInt() == 0;        break;
                case Variant::FLOAT:    b = it->second.toFloat() == 0.f;    break;
                case Variant::DOUBLE:   b = it->second.toDouble() == 0.0;   break;
                default: break;
            }
        }

        if (b)
            renderQueue |= faceCullingBit;
    }
    else renderQueue |= faceCullingBit;

    // load sources
    loadSourceCode(shaderMap, "vertex", vertexSourceCode, fileName, resourceDirectory);
    loadSourceCode(shaderMap, "fragment", fragmentSourceCode, fileName, resourceDirectory);
    loadSourceCode(shaderMap, "geometry", geometrySourceCode, fileName, resourceDirectory);
    loadSourceCode(shaderMap, "evaluation", evaluationSourceCode, fileName, resourceDirectory);
    loadSourceCode(shaderMap, "control", controlSourceCode, fileName, resourceDirectory);
    loadSourceCode(shaderMap, "compute", computeSourceCode, fileName, resourceDirectory);

    // it's a compute shader, do special loading and return
    if (isComputeShader)
    {
        if (!vertexSourceCode.empty())
            PrintWarning(fileName.c_str(), "computeShader attribute was set, vertex codeblock will be ignored");
        if (!fragmentSourceCode.empty())
            PrintWarning(fileName.c_str(), "computeShader attribute was set, fragment codeblock will be ignored");
        if (!geometrySourceCode.empty())
            PrintWarning(fileName.c_str(), "computeShader attribute was set, geometry codeblock will be ignored");
        if (!evaluationSourceCode.empty())
            PrintWarning(fileName.c_str(), "computeShader attribute was set, evaluation codeblock will be ignored");
        if (!controlSourceCode.empty())
            PrintWarning(fileName.c_str(), "computeShader attribute was set, control codeblock will be ignored");

        std::vector<std::string> dummyDefine;
        std::string errorHeader = "ERROR : loading shader : " + fileName;
        if (!compileSource(Shader::ShaderType::COMPUTE_SH, computeSourceCode, dummyDefine, compute, errorHeader))
        {
            glDeleteShader(compute);
            return false;
        }

        computeProgram = glCreateProgram();
        glAttachShader(computeProgram, compute);
        glLinkProgram(computeProgram);
        glDeleteShader(compute);

        GLint compile_status = GL_TRUE;
        glGetShaderiv(computeProgram, GL_COMPILE_STATUS, &compile_status);
        if (compile_status != GL_TRUE)
        {
            GLint logsize;
            glGetShaderiv(computeProgram, GL_INFO_LOG_LENGTH, &logsize);
            char* log = new char[logsize];
            glGetShaderInfoLog(computeProgram, logsize, &logsize, log);
            std::cout << log << std::endl;
            delete[] log;
        }
    }
    else
    {
        if (!computeSourceCode.empty())
            PrintWarning(fileName.c_str(), "computeShader attribute was not set, compute codeblock will be ignored");

        if (vertexSourceCode.empty() || fragmentSourceCode.empty())
            return false;

        vertexPragmas = extractPragmas(vertexSourceCode);
        fragmentPragmas = extractPragmas(fragmentSourceCode);
        geometryPragmas = extractPragmas(geometrySourceCode);
        evaluationPragmas = extractPragmas(evaluationSourceCode);
        controlPragmas = extractPragmas(controlSourceCode);
        std::vector<ShaderLoader::InternalVariantDefine> variantDefines = createVariantDefines();

        for (int i = 0; i < variantDefines.size(); i++)
        {
            // compile vertex & fragment
            GLuint tmpVertexId, tmpFragmentId;
            std::string errorHeader = "ERROR : loading shader : " + fileName + variantDefines[i].allDefines;
            bool vertexOk = compileSource(Shader::ShaderType::VERTEX_SH, vertexSourceCode, variantDefines[i].defines, tmpVertexId, errorHeader);
            bool fragmentOk = compileSource(Shader::ShaderType::FRAGMENT_SH, fragmentSourceCode, variantDefines[i].defines, tmpFragmentId, errorHeader);
            if (!vertexOk || !fragmentOk)
            {
                glDeleteShader(tmpVertexId);
                glDeleteShader(tmpFragmentId);
                continue;
            }

            // create a new variant
            shaderVariants.emplace_back();
            ShaderStruct& shader = shaderVariants.back();
            shader.variantCode = variantDefines[i].shaderCode;
            shader.program = glCreateProgram();
            shader.vertexShader = tmpVertexId;
            shader.fragmentShader = tmpFragmentId;
            shader.geometricShader = 0;
            shader.tessEvalShader = 0;
            shader.tessControlShader = 0;
            shader.allDefines = variantDefines[i].allDefines;
            glAttachShader(shader.program, shader.vertexShader);
            glAttachShader(shader.program, shader.fragmentShader);

            // attach optional stages
            if (!geometrySourceCode.empty() && shouldAttachStage(geometryPragmas, variantDefines[i].defines))
            {
                if (compileSource(Shader::ShaderType::GEOMETRIC_SH, geometrySourceCode, variantDefines[i].defines, shader.geometricShader, errorHeader))
                    glAttachShader(shader.program, shader.geometricShader);
            }
            if (!evaluationSourceCode.empty() && shouldAttachStage(evaluationPragmas, variantDefines[i].defines))
            {
                if (compileSource(Shader::ShaderType::TESS_EVAL_SH, evaluationSourceCode, variantDefines[i].defines, shader.tessEvalShader, errorHeader))
                    glAttachShader(shader.program, shader.tessEvalShader);
            }
            if (!controlSourceCode.empty() && shouldAttachStage(controlPragmas, variantDefines[i].defines))
            {
                if (compileSource(Shader::ShaderType::TESS_CONT_SH, controlSourceCode, variantDefines[i].defines, shader.tessControlShader, errorHeader))
                    glAttachShader(shader.program, shader.tessControlShader);
            }

            //  Linking program
            glLinkProgram(shader.program);
            glDeleteShader(shader.vertexShader);
            glDeleteShader(shader.fragmentShader);
            if (glIsShader(shader.geometricShader))   glDeleteShader(shader.geometricShader);
            if (glIsShader(shader.tessControlShader)) glDeleteShader(shader.tessControlShader);
            if (glIsShader(shader.tessEvalShader))    glDeleteShader(shader.tessEvalShader);

            GLint compile_status = GL_TRUE;
            glGetShaderiv(shader.program, GL_COMPILE_STATUS, &compile_status);
            if (compile_status != GL_TRUE)
            {
                GLint logsize;
                glGetShaderiv(shader.program, GL_INFO_LOG_LENGTH, &logsize);
                char* log = new char[logsize];
                glGetShaderInfoLog(shader.program, logsize, &logsize, log);
                std::cout << log << std::endl;
                delete[] log;
            }
        }
    }

    //	get attribute locations
    it = shaderMap.getMap().find("uniform");
    if (it != shaderMap.getMap().end() && it->second.getType() == Variant::MAP)
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

    auto textureVariant = shaderMap.getMap().find("textures");
    if (textureVariant != shaderMap.getMap().end())
    {
        if (textureVariant->second.getType() == Variant::ARRAY || textureVariant->second.getArray().size() <= 0)
        {
            auto& textureArray = textureVariant->second.getArray();
            bool hasError = false;
            for (auto it2 = textureArray.begin(); it2 != textureArray.end(); it2++)
            {
                if (it2->getType() == Variant::MAP)
                {
                    auto nameVariant = it2->getMap().find("name");
                    auto resourceVariant = it2->getMap().find("resource");
                    if (nameVariant != it2->getMap().end() && resourceVariant != it2->getMap().end() &&
                        nameVariant->second.getType() == Variant::STRING && resourceVariant->second.getType() == Variant::STRING)
                    {
                        std::string texFileName = resourceVariant->second.toString();
                        textures.push_back({ nameVariant->second.toString(), texFileName });
                    }
                    else hasError = true;
                }
            }

            if (hasError)
                PrintError(fileName.c_str(), "Texture array contain invalid object field");
        }
        else PrintError(fileName.c_str(), "Texture field is not a valid array (or is empty)");
    }
    
    if (isComputeShader)
        return glIsProgram(computeProgram);
    else
        return shaderVariants.size() > 0;
}

void ShaderLoader::initialize(ResourceVirtual* resource)
{
    Shader* baseShader = static_cast<Shader*>(resource);
    if (isComputeShader)
    {
        baseShader->initialize(compute, computeProgram, attributesType, textures);

        //for (int i = 0; i < textureResources.size(); i++)
        //    baseShader->pushTexture(textureResources[i]);
    }
    else
    {
        ShaderStruct& base = shaderVariants.front();
        baseShader->initialize(base.vertexShader, base.fragmentShader, base.geometricShader, base.tessControlShader, base.tessEvalShader, base.program,
            attributesType, textures, renderQueue);

        //for (int i = 0; i < textureResources.size(); i++)
        //    baseShader->pushTexture(textureResources[i]);

        for (int i = 1; i < shaderVariants.size(); i++)
        {
            ShaderStruct& variant = shaderVariants[i];
            std::string variantName = baseShader->name;
            if (!variant.allDefines.empty())
                variantName += '+' + variant.allDefines;

            Shader* variantShader = new Shader(variantName);
            variantShader->initialize(variant.vertexShader, variant.fragmentShader, variant.geometricShader, variant.tessControlShader, variant.tessEvalShader, variant.program,
                attributesType, textures, renderQueue);
            //for (int i = 0; i < textureResources.size(); i++)
            //    variantShader->pushTexture(textureResources[i]);

            baseShader->addVariant(variant.variantCode, variantShader);
        }
    }
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
    renderQueue = 1000;
    compute = 0;
    computeProgram = 0;
    isComputeShader = false;
    attributesType.clear();
    textures.clear();
    textureResources.clear();
    includes.clear();
    shaderVariants.clear();
    vertexPragmas.clear();
    fragmentPragmas.clear();
    geometryPragmas.clear();
    evaluationPragmas.clear();
    controlPragmas.clear();

    vertexSourceCode.clear();
    fragmentSourceCode.clear();
    geometrySourceCode.clear();
    evaluationSourceCode.clear();
    controlSourceCode.clear();
    computeSourceCode.clear();
}

bool ShaderLoader::compileSource(Shader::ShaderType shaderType, std::string source, std::vector<std::string> defines, GLuint& shader, const std::string& errorHeader)
{
    // Generate ID from OpenGL
    switch (shaderType)
    {
        case Shader::ShaderType::VERTEX_SH:     shader = glCreateShader(GL_VERTEX_SHADER);          break;
        case Shader::ShaderType::GEOMETRIC_SH:  shader = glCreateShader(GL_GEOMETRY_SHADER);        break;
        case Shader::ShaderType::FRAGMENT_SH:   shader = glCreateShader(GL_FRAGMENT_SHADER);        break;
        case Shader::ShaderType::TESS_EVAL_SH:  shader = glCreateShader(GL_TESS_EVALUATION_SHADER); break;
        case Shader::ShaderType::TESS_CONT_SH:  shader = glCreateShader(GL_TESS_CONTROL_SHADER);    break;
        case Shader::ShaderType::COMPUTE_SH:    shader = glCreateShader(GL_COMPUTE_SHADER);         break;
        default:
            std::cerr << errorHeader << Shader::toString(shaderType) << std::endl;
            return false;
    }
    if (!shader)
    {
        std::cerr << errorHeader << Shader::toString(shaderType) << " : fail to create OPENGL shader" << std::endl;
        return false;
    }

    // inject variant define
    std::string alldefine = "";
    for (int i = 0; i < defines.size(); i++)
        alldefine += defines[i];
    source = includes + alldefine + source;

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
            std::cerr << sourceData << std::endl << std::endl;
        }
        delete[] log;
        return false;
    }
    return true;
}


std::vector<std::string> ShaderLoader::extractPragmas(std::string& source)
{
    std::string pragmaToken = "#pragma";
    std::vector<std::string> result;

    int index = 0;
    while ((index = source.find(pragmaToken, index)) != std::string::npos)
    {
        int start = index + pragmaToken.size() + 1;
        int index2 = start;
        while (source[index2] != '\n')
            index2++;

        result.push_back(source.substr(start, index2 - start));
        source.erase(index, index2 - index);
    }
    return result;
}
bool ShaderLoader::loadSourceCode(Variant& shaderMap, const std::string& key, std::string& destination, const std::string& filename, const std::string& resourceDirectory)
{
    // source string
    auto it = shaderMap.getMap().find(key);
    if (it != shaderMap.getMap().end())
    {
        destination = it->second.toString();
        if (!destination.empty())
            destination[0] = ' ';
    }
    else if ((key == "vertex" || key == "fragment") && !isComputeShader)
    {
        std::string msg = "no " + key + " codeblock found";
        PrintError(filename.c_str(), msg.c_str());
        return false;
    }
    else if (key == "compute" && isComputeShader)
    {
        std::string msg = "no " + key + " codeblock found";
        PrintError(filename.c_str(), msg.c_str());
        return false;
    }

    // search for includes
    std::string includeToken = "#include";
    std::vector<std::string> result;

    int index = 0;
    while ((index = destination.find(includeToken, index)) != std::string::npos)
    {
        int start = index + includeToken.size() + 1;
        int index2 = start;
        while (destination[index2] != '\n')
            index2++;

        std::string includeName = destination.substr(start + 1, index2 - start - 2);
        destination.erase(index, index2 - index);

        std::string includeFullPath = resourceDirectory + Shader::directory + includeName;
        std::string file = getFile(includeFullPath);
        destination.insert(index, file);
        index += file.size();
        //result.push_back(destination.substr(start, index2 - start));
    }

    return true;
}
std::string ShaderLoader::getFile(std::string& filename)
{
    std::ifstream f(filename);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}
bool ShaderLoader::shouldAttachStage(std::vector<std::string>& stagePragmas, std::vector<std::string>& defines)
{
    if (stagePragmas.empty())
        return true;

    for (std::string& s1 : defines)
        for (std::string& s2 : stagePragmas)
        {
            if (s1.find(s2) != std::string::npos)
            {
                return true;
            }
        }
    return false;
}

std::vector<ShaderLoader::InternalVariantDefine> ShaderLoader::createVariantDefines()
{
    bool instancing = false;
    bool shadow = false;
    bool wired = false;

    const auto testPragma = [&](std::vector<std::string>& list)
    {
        for (int i = 0; i < list.size(); i++)
        {
            instancing |= (list[i] == "INSTANCING");
            shadow |= (list[i] == "SHADOW_PASS");
            wired |= (list[i] == "WIRED_MODE");
        }
    }; 
    const auto testSource = [&](std::string& source)
    {
        if (source.empty())
            return;
        instancing |= source.find("INSTANCING") != std::string::npos;
        shadow |= source.find("SHADOW_PASS") != std::string::npos;
        wired |= source.find("WIRED_MODE") != std::string::npos;
    };

    testPragma(vertexPragmas);
    testPragma(fragmentPragmas);
    testPragma(geometryPragmas);
    testPragma(evaluationPragmas);
    testPragma(controlPragmas);
    testSource(vertexSourceCode);
    testSource(fragmentSourceCode);
    testSource(geometrySourceCode);
    testSource(evaluationSourceCode);
    testSource(controlSourceCode);

    std::vector<ShaderLoader::InternalVariantDefine> result;
    result.emplace_back();
    result.back().shaderCode = Shader::computeVariantCode(false, 0, false);

    if (instancing)
    {
        result.emplace_back();
        result.back().shaderCode = Shader::computeVariantCode(true, 0, false);
        result.back().defines.push_back("#define INSTANCING\n");
        result.back().allDefines = "INSTANCING";
    }

    if (shadow)
    {
        result.emplace_back();
        result.back().shaderCode = Shader::computeVariantCode(false, 1, false);
        result.back().defines.push_back("#define SHADOW_PASS\n");
        result.back().allDefines = "SHADOW_PASS";

        result.emplace_back();
        result.back().shaderCode = Shader::computeVariantCode(false, 2, false);
        result.back().defines.push_back("#define SHADOW_PASS\n");
        result.back().defines.push_back("#define GEOMETRY_INVOCATION 6\n");
        result.back().allDefines = "SHADOW_OMNI_PASS";

        if (instancing)
        {
            result.emplace_back();
            result.back().shaderCode = Shader::computeVariantCode(true, 1, false);
            result.back().defines.push_back("#define INSTANCING\n");
            result.back().defines.push_back("#define SHADOW_PASS\n");
            result.back().allDefines = "INSTANCING+SHADOW_PASS";

            result.emplace_back();
            result.back().shaderCode = Shader::computeVariantCode(true, 2, false);
            result.back().defines.push_back("#define INSTANCING\n");
            result.back().defines.push_back("#define SHADOW_PASS\n");
            result.back().defines.push_back("#define GEOMETRY_INVOCATION 6\n");
            result.back().allDefines = "SHADOW_OMNI_PASS";
        }
    }

    if (wired)
    {
        result.emplace_back();
        result.back().shaderCode = Shader::computeVariantCode(false, 0, true);
        result.back().defines.push_back("#define WIRED_MODE\n");
        result.back().allDefines = "WIRED_MODE";

        if (instancing)
        {
            result.emplace_back();
            result.back().shaderCode = Shader::computeVariantCode(true, 0, true);
            result.back().defines.push_back("#define INSTANCING\n");
            result.back().defines.push_back("#define WIRED_MODE\n");
            result.back().allDefines = "INSTANCING+WIRED_MODE";
        }
    }
    return result;
}
