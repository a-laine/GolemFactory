#include "Material.h"


#include <Utiles/Parser/Reader.h>
#include <Utiles/Assert.hpp>
#include <Resources/ResourceManager.h>
#include <Utiles/ConsoleColor.h>

//  Static attributes
char const* const Material::directory = "Materials/";
char const* const Material::extension = ".material";
std::string Material::defaultName;
//


//  Default
Material::Material(const std::string& materialName) :
    ResourceVirtual(materialName, ResourceVirtual::ResourceType::MATERIAL),
    m_shader(nullptr), m_shadowMaxCascade(4)
{

}
Material::~Material()
{

}
//

std::string Material::getIdentifier(const std::string& resourceName) { return std::string(directory) + resourceName; }
std::string Material::getIdentifier() const { return getIdentifier(name); }
std::string Material::getLoaderId(const std::string& resourceName) const { return extension; }
const std::string& Material::getDefaultName() { return defaultName; }
void Material::setDefaultName(const std::string& name) { defaultName = name; }
Shader* Material::getShader() { return m_shader; }
int Material::getMaxShadowCascade() const { return m_shadowMaxCascade; }
const std::vector<Texture*>& Material::getTextures() const { return m_textures; }
const std::vector<Shader::Property>& Material::getProperties() const { return m_properties; }
bool Material::isValid() const { return m_shader && m_shader->isValid(); }

void Material::initialize(const std::string& _shaderName, int _maxShadowCascade)
{
    m_shadowMaxCascade = _maxShadowCascade;
    m_shader = ResourceManager::getInstance()->getResource<Shader>(_shaderName);

	auto& textures = m_shader->getTextures();
	m_textures.clear();
	m_textures.reserve(textures.size());
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures[i].texture)
			m_textures.push_back(ResourceManager::getInstance()->getResource(textures[i].texture));
		else
			m_textures.push_back(nullptr);
	}
}

void Material::setTextureOverride(const std::vector<std::pair<std::string, std::string>>& _overrides)
{
    if (!m_shader)
        return;

	auto& textures = m_shader->getTextures();
    for (int i = 0; i < _overrides.size(); i++)
    {
        const std::string& identifier = _overrides[i].first;
        const std::string& textureName = _overrides[i].second;

		for (int j = 0; j < textures.size(); j++)
		{
			if (identifier == textures[j].identifier)
			{
				ResourceManager::getInstance()->release(m_textures[j]);
				m_textures[j] = ResourceManager::getInstance()->getResource<Texture>(textureName);
				break;
			}
		}
    }
}

void Material::copyPropertiesFromShader()
{
	m_properties.clear();
	if (!m_shader)
		return;

	const std::vector<Shader::Property>& original = m_shader->getProperties();
	m_properties = original;
}

void Material::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Material infos");
    ImGui::Text("Shader name : %s", m_shader->name);

    /*const uint16_t transparentBit = 1 << 15;
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
    }*/

    if (!m_properties.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Properties");
        using ptype = Shader::Property::PropertyType;

        for (Shader::Property& property : m_properties)
        {
            int location = m_shader->getUniformLocation(property.m_name);
            std::string typestr;
            switch (property.m_type)
            {
                case ptype::eFloat: typestr = "float";  break;
                case ptype::eInteger: typestr = "int";  break;
                case ptype::eFloatVector: typestr = "vec4";  break;
                case ptype::eIntegerVector: typestr = "ivec4";  break;
                case ptype::eColor: typestr = "color";  break;
                default: typestr = "??";  break;
            }

            ImGui::Text("%s %s : location = %d", typestr.c_str(), property.m_name.c_str(), location);

            switch (property.m_type)
            {
                case ptype::eFloat:
                    ImGui::DragFloat(property.m_name.c_str(), &property.m_floatValues.x, 0.001f);
                    break;
                case ptype::eInteger:
                    ImGui::DragInt(property.m_name.c_str(), &property.m_integerValues.x, 1);
                    break;
                case ptype::eFloatVector:
                    ImGui::DragFloat4(property.m_name.c_str(), &property.m_floatValues.x, 0.001f);
                    break;
                case ptype::eIntegerVector:
                    ImGui::DragInt4(property.m_name.c_str(), &property.m_integerValues.x, 1);
                    break;
                case ptype::eColor:
                    ImGui::ColorEdit4(property.m_name.c_str(), &property.m_floatValues.x);
                    break;
                default:
                    ImGui::Text("error : %s", property.m_name.c_str());
                    break;
            }
        }
    }

    if (!m_textures.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Textures");
        const std::vector<Shader::TextureInfos>& shaderTexInfos = m_shader->getTextures();
        for (int i = 0; i < m_textures.size(); i++)
        {
            if (ImGui::TreeNode(shaderTexInfos[i].identifier.c_str()))
            {
                ImGui::Text("Texture Unit : %d", shaderTexInfos[i].unit);
                ImGui::Text("Default resource : %s", shaderTexInfos[i].defaultResource.c_str());
                if (!shaderTexInfos[i].isGlobalAttribute)
                    ImGui::Text("Material resource : %s", m_textures[i]->name.c_str());
                else
                    ImGui::Text("(global texture)");
                ImGui::Text("Uniform location : %d", shaderTexInfos[i].location);

                if (!shaderTexInfos[i].isGlobalAttribute)
                {
                    std::vector<std::string> textureList = ResourceManager::getInstance()->getAllResourceName(ResourceVirtual::ResourceType::TEXTURE);
                    if (ImGui::BeginCombo("File : ", m_textures[i]->name.c_str(), 0))
                    {
                        int currentId = 0;
                        for (int n = 0; n < textureList.size(); n++)
                        {
                            const bool is_selected = textureList[n] == m_textures[i]->name;
                            if (ImGui::Selectable(textureList[n].c_str(), is_selected))
                            {
                                Texture* previous = m_textures[i];
                                m_textures[i] = ResourceManager::getInstance()->getResource<Texture>(textureList[n]);
                                ResourceManager::getInstance()->release(previous);
                            }

                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    // overview
                    float ratio = (ImGui::GetContentRegionAvail().x - 5) / m_textures[i]->size.x;
                    ImGui::Spacing();
                    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Overview");
                    ImGui::Image((void*)m_textures[i]->getTextureId(), ImVec2(m_textures[i]->size.x * ratio, m_textures[i]->size.y * ratio),
                        ImVec2(0.f, 0.f), ImVec2(1.f, 1.f), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
                }

                ImGui::TreePop();
            }
        }
    }
#endif
}