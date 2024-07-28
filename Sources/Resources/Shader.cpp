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
    , renderQueue(1000), m_isComputeShader(false), m_usePeterPanning(true)
{
#ifdef USE_IMGUI
    dynamicQueue = 1000;
#endif
}
Shader::~Shader()
{
    glDeleteProgram(program);

    for (int i = 0; i < m_textures.size(); i++)
        ResourceManager::getInstance()->release(m_textures[i].texture);

    for (auto it = variants.begin(); it != variants.end(); it++)
        delete it->second;
}
//


//  Public functions
void Shader::initialize(GLuint  vertexSh, GLuint fragSh, GLuint geomShr, GLuint tessControlSh, GLuint tessEvalSh, GLuint prog,
    const std::map<std::string, std::string>& attType, const std::vector<std::pair<std::string, std::string>>& textures, uint16_t queue)
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
    attributesType = attType;
    renderQueue = queue;

#ifdef USE_IMGUI
    dynamicQueue = (renderQueue & ~(0x03 << 14));
#endif

    //  Binding texture to sample
    glUseProgram(program);
    if(!textures.empty())
    {
        using TC = Texture::TextureConfiguration;
        uint8_t std2Dconfig = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::USE_MIPMAP | (uint8_t)TC::WRAP_REPEAT;


        for (int i = 0; i < textures.size(); i++)
        {
            TextureInfos tex;
            tex.identifier = textures[i].first;
            tex.defaultResource = textures[i].second;
            tex.location = glGetUniformLocation(program, tex.identifier.c_str());
            tex.texture = ResourceManager::getInstance()->getResource<Texture>(tex.defaultResource, std2Dconfig);
            tex.unit = (uint8_t)m_textures.size();
            tex.isGlobalAttribute = false;
            glUniform1i(tex.location, tex.unit);

            m_textures.push_back(tex);
        }
    }

    //	get attributes location
    for(auto it = attType.begin(); it != attType.end(); it++)
    {
        const std::string& uniformName = it->first;
        const std::string& type = it->second;


        GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
        attributesLocation[uniformName] = uniformLocation;
        if (uniformLocation < 0 && false)
        {
            if (uniformName.size() >= 3 && uniformName[0] == 'g' && uniformName[1] == 'l' && uniformName[2] == '_' &&
                ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading shader : " << name << " : error in loading '" << uniformName << "' : name format not allowed, remove the 'gl_' prefix." << std::endl;
            else if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading shader : " << name << " : ERROR in loading '" << uniformName << "' variable location : " << uniformLocation << "; maybe the variable name does not correspond to an active uniform variable" << std::endl;

            continue;
        }

        if (type == "_globalLightClusters")
            loadGlobalTexture(type, uniformName, uniformLocation, 0); // force image unit 0
        else if (type == "_terrainVirtualTexture")
            loadGlobalTexture(type, uniformName, uniformLocation, 1); // force image unit 1
        else if (type == "_globalShadowCascades")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
        else if (type == "_globalTerrainMaterialCollection")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
        else if (type == "_globalOmniShadow")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
        else if (type == "_globalSkybox")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
    }

    glUseProgram(0);
    if(glIsProgram(program))
        state = VALID;
    else
        state = INVALID;
}

void Shader::initialize(GLuint computeSh, GLuint prog, const std::map<std::string, std::string>& attType, const std::vector<std::pair<std::string, std::string>>& textures)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;
    m_isComputeShader = true;

    vertexShader = computeSh;
    program = prog;

    //  Binding texture to sample
    glUseProgram(program);
    if (!textures.empty())
    {
        using TC = Texture::TextureConfiguration;
        uint8_t std2Dconfig = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::USE_MIPMAP | (uint8_t)TC::WRAP_REPEAT;


        for (int i = 0; i < textures.size(); i++)
        {
            TextureInfos tex;
            tex.identifier = textures[i].first;
            tex.defaultResource = textures[i].second;
            tex.location = glGetUniformLocation(program, tex.identifier.c_str());
            tex.texture = ResourceManager::getInstance()->getResource<Texture>(tex.defaultResource, std2Dconfig);
            tex.unit = (uint8_t)m_textures.size();
            tex.isGlobalAttribute = false;
            glUniform1i(tex.location, tex.unit);

            m_textures.push_back(tex);
        }
    }

    //	get attributes location
    for (auto it = attType.begin(); it != attType.end(); it++)
    {
        const std::string& uniformName = it->first;
        const std::string& type = it->second;

        GLint uniformLocation = glGetUniformLocation(program, uniformName.c_str());
        attributesLocation[uniformName] = uniformLocation;
        if (uniformLocation < 0 && false)
        {
            if (uniformName.size() >= 3 && uniformName[0] == 'g' && uniformName[1] == 'l' && uniformName[2] == '_' &&
                ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading shader : " << name << " : error in loading '" << uniformName << "' : name format not allowed, remove the 'gl_' prefix." << std::endl;
            else if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : loading shader : " << name << " : ERROR in loading '" << uniformName << "' variable location : " << uniformLocation << "; maybe the variable name does not correspond to an active uniform variable" << std::endl;
        }

        if (type == "_globalLightClusters")
            loadGlobalTexture(type, uniformName, uniformLocation, 0);
        else if (type == "_terrainVirtualTexture")
            loadGlobalTexture(type, uniformName, uniformLocation, 1);
        else if (type == "_globalShadowCascades")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
        else if (type == "_globalTerrainMaterialCollection")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
        else if (type == "_globalOmniShadow")
            loadGlobalTexture(type, uniformName, uniformLocation, (uint8_t)m_textures.size());
    }

    glUseProgram(0);
    if (glIsProgram(program))
        state = VALID;
    else
        state = INVALID;
}
void Shader::loadGlobalTexture(const std::string& type, const std::string& identifier, GLuint location, uint8_t unit)
{
    attributesLocation[type] = location;

    TextureInfos tex;
    tex.identifier = identifier;
    tex.defaultResource = type;
    tex.location = location;
    tex.texture = nullptr;
    tex.unit = unit;
    tex.isGlobalAttribute = true;

    glUniform1i(tex.location, tex.unit);

    m_textures.push_back(tex);
}

void Shader::enable()
{
    glUseProgram(program);

    for (int i = 0; i < m_textures.size(); i++)
    {
        if (!m_textures[i].isGlobalAttribute)
        {
            glActiveTexture(GL_TEXTURE0 + m_textures[i].unit);
            glBindTexture(GL_TEXTURE_2D, m_textures[i].texture->getTextureId());
        }
    }
}

void Shader::addVariant(int variantCode, Shader* variantShader)
{
    variants[variantCode] = variantShader;
}
GLuint Shader::getProgram() const { return program; }
int Shader::getTextureCount() const { return (int)m_textures.size(); }
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
uint8_t Shader::getGlobalTextureUnit(std::string _name) const
{
    for (const auto& tex : m_textures)
    {
        if (_name == tex.defaultResource)
            return tex.unit;
    }
    return -1;
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

int Shader::computeVariantCode(bool instanced, int shadow, bool wireframe)
{
    int code = instanced ? (1 << 0) : 0;
    if (shadow)
        code |= 1 << 1;
    else if (wireframe)
        code |= 2 << 1;
    if (shadow > 1)
        code |= 1 << 3;

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
bool Shader::usePeterPanning() const
{
    return m_usePeterPanning;
}

const std::string& Shader::getDefaultName() { return defaultName; }
void Shader::setDefaultName(const std::string& name) { defaultName = name; }

void Shader::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Shader infos");

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
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Uniforms");
        for (auto it = attributesType.begin(); it != attributesType.end(); it++)
        {
            int location = attributesLocation[it->first];
            ImGui::Text("%s %s : location = %d", it->second.c_str(), it->first.c_str(), location);
        }
    }

    if (!m_textures.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Samplers");
        for (int i = 0; i < m_textures.size(); i++)
        {
            if (ImGui::TreeNode(m_textures[i].identifier.c_str()))
            {
                ImGui::Text("Texture Unit : %d", m_textures[i].unit);
                ImGui::Text("Default resource : %s", m_textures[i].defaultResource.c_str());
                ImGui::Text("Uniform location : %d", m_textures[i].location);

                // file & file selection
                if (!m_textures[i].isGlobalAttribute)
                {
                    std::vector<std::string> textureList = ResourceManager::getInstance()->getAllResourceName(ResourceVirtual::ResourceType::TEXTURE);
                    if (ImGui::BeginCombo("File : ", m_textures[i].texture->name.c_str(), 0))
                    {
                        int currentId = 0;
                        for (int n = 0; n < textureList.size(); n++)
                        {
                            const bool is_selected = textureList[n] == m_textures[i].texture->name;
                            if (ImGui::Selectable(textureList[n].c_str(), is_selected))
                            {
                                Texture* previous = m_textures[i].texture;
                                m_textures[i].texture = ResourceManager::getInstance()->getResource<Texture>(textureList[n]);
                                ResourceManager::getInstance()->release(previous);

                                for (auto it = variants.begin(); it != variants.end(); it++)
                                    it->second->m_textures[i].texture = m_textures[i].texture;
                            }

                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    // overview
                    float ratio = (ImGui::GetContentRegionAvail().x - 5) / m_textures[i].texture->size.x;
                    ImGui::Spacing();
                    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Overview");
                    ImGui::Image((void*)m_textures[i].texture->getTextureId(), ImVec2(m_textures[i].texture->size.x * ratio, m_textures[i].texture->size.y * ratio),
                        ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
                }

                ImGui::TreePop();
            }
        }
    }

    if (!variants.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Variations");

        int shadowCode = computeVariantCode(false, 1, false);
        int wiredCode = computeVariantCode(false, 0, true);
        int instancedCode = computeVariantCode(true, 0, false);
        int instancedShadowCode = computeVariantCode(true, 1, false);
        int instancedWiredCode = computeVariantCode(true, 0, true);

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
