#include "Shader.h"
#include "Loader/ShaderLoader.h"

#include <Utiles/Parser/Reader.h>
#include <Utiles/Assert.hpp>
#include <Resources/ResourceManager.h>

//  Static attributes
char const * const Shader::directory = "Shaders/";
char const * const Shader::extension = ".shader";
std::string Shader::defaultName;
//

//  Default
Shader::Shader(const std::string& shaderName)
    : ResourceVirtual(shaderName, ResourceVirtual::ResourceType::SHADER)
    , vertexShader(0), fragmentShader(0), geometricShader(0)
    , tessControlShader(0), tessEvalShader(0), program(0)
    , textureCount(0), renderQueue(1000), m_isComputeShader(false)
{
#ifdef USE_IMGUI
    dynamicQueue = 1000;
#endif
}
Shader::~Shader()
{
    glDeleteProgram(program);

    for (int i = 0; i < textures.size(); i++)
        ResourceManager::getInstance()->release(textures[i]);

    for (auto it = variants.begin(); it != variants.end(); it++)
        delete it->second;
}
//


//  Public functions
void Shader::initialize(GLuint  vertexSh, GLuint fragSh, GLuint geomShr, GLuint tessControlSh, GLuint tessEvalSh, GLuint prog,
    const std::map<std::string, std::string>& attType, const std::vector<std::string>& textures, uint16_t queue)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;
    m_isComputeShader = false;

    // initialize attributes
    vertexShader = vertexSh;
    fragmentShader = fragSh;
    geometricShader = geomShr;
    tessControlShader = tessControlSh;
    tessEvalShader = tessEvalSh;
    program = prog;
    textureCount = 0;
    attributesType = attType;
    renderQueue = queue;

#ifdef USE_IMGUI
    dynamicQueue = (renderQueue & ~(0x03 << 14));
#endif

    //	get attributes location
    for(auto it = attType.begin(); it != attType.end(); it++)
    {
        const std::string& uniformName = it->first;
        const std::string& type = it->first;

        {
            GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
            attributesLocation[uniformName] = uniformLocation;
            if(uniformLocation < 0)
            {
                if(uniformName.size() >= 3 && uniformName[0] == 'g' && uniformName[1] == 'l' && uniformName[2] == '_' && 
                    ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                    std::cerr << "ERROR : loading shader : " << name << " : error in loading '" << uniformName << "' : name format not allowed, remove the 'gl_' prefix." << std::endl;
                else if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                    std::cerr << "ERROR : loading shader : " << name << " : ERROR in loading '" << uniformName << "' variable location : " << uniformLocation << "; maybe the variable name does not correspond to an active uniform variable" << std::endl;
            }
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

        textureIdentifiers = textures;
    }

    if(glIsProgram(program))
        state = VALID;
    else
        state = INVALID;
}

void Shader::initialize(GLuint computeSh, GLuint prog, const std::map<std::string, std::string>& attType, const std::vector<std::string>& textures)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;
    m_isComputeShader = true;

    vertexShader = computeSh;
    program = prog;

    //	get attributes location
    for (auto it = attType.begin(); it != attType.end(); it++)
    {
        const std::string& uniformName = it->first;
        const std::string& type = it->first;

        {
            GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
            attributesLocation[uniformName] = uniformLocation;
            if (uniformLocation < 0)
            {
                if (uniformName.size() >= 3 && uniformName[0] == 'g' && uniformName[1] == 'l' && uniformName[2] == '_' &&
                    ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                    std::cerr << "ERROR : loading shader : " << name << " : error in loading '" << uniformName << "' : name format not allowed, remove the 'gl_' prefix." << std::endl;
                else if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                    std::cerr << "ERROR : loading shader : " << name << " : ERROR in loading '" << uniformName << "' variable location : " << uniformLocation << "; maybe the variable name does not correspond to an active uniform variable" << std::endl;
            }
        }
    }

    //  Binding texture to sample
    if (!textures.empty())
    {
        glUseProgram(program);
        try
        {
            textureCount = (uint8_t)textures.size();
            GLuint location;
            for (int i = 0; i < textureCount; i++)
            {
                location = glGetUniformLocation(program, textures[i].c_str());
                attributesLocation[textures[i]] = location;
                glUniform1i(location, i);
            }
        }
        catch (std::exception&) { textureCount = 0; }
        glUseProgram(0);

        textureIdentifiers = textures;
    }

    if (glIsProgram(program))
        state = VALID;
    else
        state = INVALID;
}

void Shader::enable()
{
    glUseProgram(program);

    for (int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->getTextureId());
    }
}
//void Shader::setInstanciable(Shader* instaciedVersion) { instanciable = instaciedVersion; }
void Shader::addVariant(int variantCode, Shader* variantShader)
{
    variants[variantCode] = variantShader;
}
GLuint Shader::getProgram() const { return program; }
int Shader::getTextureCount() const { return textureCount; }
GLuint Shader::getShaderID(ShaderType shaderType) const
{
    switch(shaderType)
    {
        case ShaderType::VERTEX_SH:     return vertexShader;
        case ShaderType::GEOMETRIC_SH:  return geometricShader;
        case ShaderType::FRAGMENT_SH:   return fragmentShader;
        case ShaderType::PROGRAM_SH:    return program;
        case ShaderType::TESS_EVAL_SH:  return tessEvalShader;
        case ShaderType::TESS_CONT_SH:  return tessControlShader;
        default:            return 0;
    }
}
bool Shader::useShaderType(ShaderType shaderType) const
{
    switch(shaderType)
    {
        case ShaderType::VERTEX_SH:     return glIsShader(vertexShader) != 0;
        case ShaderType::GEOMETRIC_SH:  return glIsShader(geometricShader) != 0;
        case ShaderType::FRAGMENT_SH:   return glIsShader(fragmentShader) != 0;
        case ShaderType::PROGRAM_SH:    return glIsShader(program) != 0;
        case ShaderType::TESS_EVAL_SH:  return glIsShader(tessEvalShader) != 0;
        case ShaderType::TESS_CONT_SH:  return glIsShader(tessControlShader) != 0;
        default:            return false;
    }
}
uint16_t Shader::getRenderQueue() const { return renderQueue; }

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

int Shader::computeVariantCode(bool instanced, bool shadow, bool wireframe)
{
    int code = instanced ? (1 << 0) : 0;
    if (shadow)
        code |= 1 << 1;
    else if (wireframe)
        code |= 2 << 1;
    return code;
}
Shader* Shader::getVariant(int variantCode)
{
    auto it = variants.find(variantCode);
    if (it != variants.end() && it->second)
        return it->second;
    return this;
}
bool Shader::supportInstancing() const
{
    auto it = variants.find(1 << 0);
    return it != variants.end() && it->second;
}
bool Shader::isComputeShader() const
{
    return m_isComputeShader;
}

const std::string& Shader::getDefaultName() { return defaultName; }
void Shader::setDefaultName(const std::string& name) { defaultName = name; }

void Shader::pushTexture(Texture* texture)
{
    textures.push_back(texture);
}

void Shader::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Shader infos");

    const uint16_t transparentBit = 1 << 15;
    const uint16_t faceCullingBit = 1 << 14;
    uint16_t queueMask = ~(transparentBit | faceCullingBit);
    bool isTransparent = renderQueue & transparentBit;
    bool isUsingBackFaceCulling = renderQueue & faceCullingBit;

    if (ImGui::SliderInt("Render queue", &dynamicQueue, 0, (int)queueMask))
    {
        dynamicQueue = clamp(dynamicQueue, 0, (int)queueMask);
        renderQueue = (renderQueue & (transparentBit | faceCullingBit)) | dynamicQueue;
    }
    if (ImGui::Checkbox("Transparent", &isTransparent))
    {
        if (isTransparent)
            renderQueue |= transparentBit;
        else
            renderQueue &= ~transparentBit;
    }
    if (ImGui::Checkbox("Use backface culling", &isUsingBackFaceCulling))
    {
        if (isUsingBackFaceCulling)
            renderQueue |= faceCullingBit;
        else
            renderQueue &= ~faceCullingBit;
    }

    if (!attributesType.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Uniforms");
        for (auto it = attributesType.begin(); it != attributesType.end(); it++)
        {
            int location = attributesLocation[it->first];
            ImGui::Text("%s %s : location = %d", it->second.c_str(), it->first.c_str(), location);
        }
    }

    if (!textures.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Samplers");
        for (int i = 0; i < textures.size(); i++)
        {
            if (ImGui::TreeNode(textureIdentifiers[i].c_str()))
            {
                ImGui::Text("Location : %d", i);
                //ImGui::Text("File : %s", textures[i]->name.c_str());

                // file & file selection
                std::vector<std::string> textureList = ResourceManager::getInstance()->getAllResourceName(ResourceVirtual::ResourceType::TEXTURE);
                if (ImGui::BeginCombo("File : ", textures[i]->name.c_str(), 0))
                {
                    int currentId = 0;
                    for (int n = 0; n < textureList.size(); n++)
                    {
                        const bool is_selected = textureList[n] == textures[i]->name;
                        if (ImGui::Selectable(textureList[n].c_str(), is_selected))
                        {
                            Texture* previous = textures[i];
                            textures[i] = ResourceManager::getInstance()->getResource<Texture>(textureList[n]);
                            ResourceManager::getInstance()->release(previous);

                            for (auto it = variants.begin(); it != variants.end(); it++)
                                it->second->textures[i] = textures[i];
                        }

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                // overview
                float ratio = (ImGui::GetContentRegionAvail().x - 5) / textures[i]->size.x;
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Overview");
                ImGui::Image((void*)textures[i]->getTextureId(), ImVec2(textures[i]->size.x * ratio, textures[i]->size.y * ratio), 
                    ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));

                ImGui::TreePop();
            }
        }
    }

    if (!variants.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0.5, 1), "Variations");

        int shadowCode = computeVariantCode(false, true, false);
        int wiredCode = computeVariantCode(false, false, true);
        int instancedCode = computeVariantCode(true, false, false);
        int instancedShadowCode = computeVariantCode(true, true, false);
        int instancedWiredCode = computeVariantCode(true, false, true);

        auto shadow = variants.find(shadowCode);
        auto wired = variants.find(wiredCode);
        auto instanced = variants.find(instancedCode);
        auto instancedShadow = variants.find(instancedShadowCode);
        auto instancedWired = variants.find(instancedWiredCode);

        if (shadow != variants.end() && shadow->second)
            ImGui::Text("Shadow pass");
        if (wired != variants.end() && wired->second)
            ImGui::Text("Wireframe mode");
        if (instanced != variants.end() && instanced->second)
            ImGui::Text("Instancing draw");
        if (instancedShadow != variants.end() && instancedShadow->second)
            ImGui::Text("Instanced shadow pass");
        if (instancedWired != variants.end() && instancedWired->second)
            ImGui::Text("Instanced wireframe mode");
    }

#endif
}
//


//  Private functions
std::string Shader::toString(ShaderType shaderType)
{
	switch (shaderType)
	{
		case ShaderType::VERTEX_SH:     return "vertex shader";
		case ShaderType::GEOMETRIC_SH:  return "geometry shader";
		case ShaderType::FRAGMENT_SH:   return "fragment shader";
		case ShaderType::TESS_EVAL_SH:  return "tesselation evaluation shader";
		case ShaderType::TESS_CONT_SH:  return "tesselation control shader";
		case ShaderType::COMPUTE_SH:    return "compute shader";
		default: return "unknown shader";
	}
}
//
