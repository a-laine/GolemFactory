#include "DrawableComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Resources/Material.h>
#include <Utiles/Debug.h>
#include <Utiles/Parser/Variant.h>
#include <Utiles/ConsoleColor.h>


DrawableComponent::DrawableComponent() : m_mesh(nullptr), m_material(nullptr)
{

}
DrawableComponent::DrawableComponent(const std::string& meshName, const std::string& materialName)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
	m_material = ResourceManager::getInstance()->getResource<Material>(materialName);
}

DrawableComponent::DrawableComponent(const DrawableComponent* other)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(other->m_mesh->name);
	m_material = ResourceManager::getInstance()->getResource<Material>(other->m_material->name);
}

DrawableComponent::~DrawableComponent()
{
	ResourceManager::getInstance()->release(m_mesh);
	ResourceManager::getInstance()->release(m_material);
}

bool DrawableComponent::load(Variant& jsonObject, const std::string& objectName)
{
	return load(jsonObject, objectName, nullptr);
}
bool DrawableComponent::load(Variant& jsonObject, const std::string& objectName, const Skeleton* skeleton)
{
	if (jsonObject.getType() == Variant::MAP)
	{
		std::string meshName, materialName;
		auto it1 = jsonObject.getMap().find("meshName");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
		{
			meshName = it1->second.toString();
			if (meshName.find('.') == std::string::npos)
				meshName += ".fbx";
		}

		it1 = jsonObject.getMap().find("materialName");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
			materialName = it1->second.toString();

		if (!meshName.empty() && !materialName.empty())
		{
			m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
			if (skeleton) m_mesh->retargetSkin(skeleton);
			m_material = ResourceManager::getInstance()->getResource<Material>(materialName);
			return true;
		}
		else
		{
			if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			{
				if (meshName.empty())
				{
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : DrawableComponent load : " << objectName << " : DrawableComponent loading : no mesh name" << std::flush;
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
				}
				if (materialName.empty())
				{
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : DrawableComponent load : " << objectName << " : DrawableComponent loading : no material name" << std::flush;
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
				}
			}
		}
	}
	return false;
}
void DrawableComponent::save(Variant& jsonObject)
{

}

void DrawableComponent::setMaterial(const std::string& materialName)
{
	ResourceManager::getInstance()->release(m_material);
	m_material = ResourceManager::getInstance()->getResource<Material>(materialName);
}

void DrawableComponent::setMaterial(Material* material)
{
	ResourceManager::getInstance()->release(m_material);
	if(material) m_material = ResourceManager::getInstance()->getResource<Material>(material);
	else m_material = nullptr;
}

void DrawableComponent::setMesh(const std::string& meshName)
{
	ResourceManager::getInstance()->release(m_mesh);
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
}

void DrawableComponent::setMesh(Mesh* mesh)
{
	ResourceManager::getInstance()->release(m_mesh);
	if(mesh) m_mesh = ResourceManager::getInstance()->getResource<Mesh>(mesh);
	else m_mesh = nullptr;
}

Material* DrawableComponent::getMaterial() const
{
	return m_material;
}

Mesh* DrawableComponent::getMesh() const
{
	return m_mesh;
}

bool DrawableComponent::hasCustomDraw() const
{
	return false;
}

void DrawableComponent::customDraw(Renderer* _renderer, unsigned int& _instanceDrawnCounter, unsigned int& _drawCallsCounter, unsigned int& _trianglesDrawnCounter) const
{

}

bool DrawableComponent::isValid() const
{
    return m_mesh && m_mesh->isValid() && m_material && m_material->isValid();
}

bool DrawableComponent::castShadow() const
{
	return m_material->getMaxShadowCascade() >= 0;
}

bool DrawableComponent::hasSkeleton() const
{
    GF_ASSERT(isValid());
    return m_mesh->hasSkeleton();
}

vec4f DrawableComponent::getMeshBBMax() const
{
    GF_ASSERT(isValid());
	return m_mesh->getBoundingBox().max;
}

vec4f DrawableComponent::getMeshBBMin() const
{
    GF_ASSERT(isValid());
	return m_mesh->getBoundingBox().min;
}

void DrawableComponent::pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass)
{
	Renderer::DrawElement element;
	element.material = m_material;
	element.mesh = m_mesh;
	element.batch = nullptr;
	element.entity = getParentEntity();



	uint64_t queue = m_material->getShader()->getRenderQueue();
	queue = queue << 48;
	if (!isShadowPass && (queue & TransparentMask))
	{
		//compute 2's complement of d
		distance = ~distance;
		distance++;
	}
	if (isShadowPass)
		element.hash = distance;
	else
		element.hash = queue | distance;

	if (m_ClockWise)
		element.hash |= CullingModeMask;

	drawQueue.push_back(element);
}


unsigned short DrawableComponent::getInstanceDataSize() const { return 0; }
bool DrawableComponent::hasConstantData() const { return false; }
void DrawableComponent::pushConstantData(Shader* _shader) const {}
void DrawableComponent::pushInstanceData(Shader* _shader) const {}
void DrawableComponent::writeInstanceData(vec4f* _destination) const {}

void DrawableComponent::setClockWise(bool ccwEnable)
{
	m_ClockWise = ccwEnable;
}
bool DrawableComponent::isClockWise() const
{
	return m_ClockWise;
}

void DrawableComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Drawable);
}

void DrawableComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(1, 0.5, 0, 1);
	std::ostringstream unicName;
	unicName << "Drawable component##" << (uintptr_t)this;

	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Mesh");
		ImGui::Indent();
		ImGui::Text("name : %s", m_mesh->name.c_str());
		ImGui::Text("vertices count : %d", m_mesh->getNumberVertices());
		ImGui::Text("faces count : %d", m_mesh->getNumberFaces());
		vec4f center = 0.5f * (m_mesh->getBoundingBox().max + m_mesh->getBoundingBox().min);
		vec4f size = 0.5f * (m_mesh->getBoundingBox().max - m_mesh->getBoundingBox().min);
		ImGui::Text("skinned : %s", m_mesh->hasSkeleton() ? "true" : "false");
		ImGui::Text("local aabb center : %.2f, %.2f, %.2f", center.x, center.y, center.z);
		ImGui::Text("local aabb size : %.2f, %.2f, %.2f", size.x, size.y, size.z);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Material");
		ImGui::Indent();
		ImGui::Text("name : %s", m_material->name.c_str());
		
		auto& textures = m_material->getTextures();
		const auto& shaderTextures = m_material->getShader()->getTextures();
		if (!textures.empty())
		{
			ImGui::TextColored(ImVec4(0.7f, 1.f, 0.7f, 1.f), "Binded textures :");
			ImGui::Indent();
			for (int i = 0; i < shaderTextures.size(); i++)
			{
				if (shaderTextures[i].isGlobalAttribute)
					ImGui::Text("Global : %s", shaderTextures[i].defaultResource.c_str());
				else if (textures[i])
					ImGui::Text("Location %d : %s", i, textures[i]->name.c_str());
			}
			ImGui::Unindent();
		}
		const auto& uniforms = m_material->getShader()->getUniforms();
		if (!uniforms.empty())
		{
			ImGui::TextColored(ImVec4(0.7f, 1.f, 0.7f, 1.f), "Parameters :");
			ImGui::Indent();
			for (auto it = uniforms.begin(); it != uniforms.end(); it++)
			{
				ImGui::Text("%s : %s", it->first.c_str(), it->second.c_str());
			}
			ImGui::Unindent();
		}
		ImGui::Unindent();

		/*ImGui::Spacing();
		ImGui::TextColored(componentColor, "Shader");
		ImGui::Indent();
		ImGui::SliderInt("Max shadow cascade", &m_material->getMaxShadowCascade(), -1, 4, "%d", ImGuiSliderFlags_AlwaysClamp);
		ImGui::Unindent();*/

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw mesh bounding box", &m_drawMeshBoundingBox);
		ImGui::Checkbox("Visible", &m_visible);
		ImGui::Unindent();

		ImGui::TreePop();
	}


	if (m_drawMeshBoundingBox)
	{
		Debug::color = vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w);
		const auto& aabb = m_mesh->getBoundingBox();
		Debug::drawLineCube(getParentEntity()->getWorldTransformMatrix(), aabb.min, aabb.max);
	}
#endif // USE_IMGUI
}