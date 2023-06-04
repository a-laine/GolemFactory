#include "OccluderComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Utiles/Debug.h>
#include <Utiles/Parser/Variant.h>
#include <Utiles/ConsoleColor.h>


OccluderComponent::OccluderComponent(const std::string& meshName)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
	m_backfaceCulling = false;
}
OccluderComponent::OccluderComponent(const OccluderComponent* other)
{
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(other->m_mesh->name);
	m_backfaceCulling = other->m_backfaceCulling;
}
OccluderComponent::~OccluderComponent()
{
	ResourceManager::getInstance()->release(m_mesh);
}

bool OccluderComponent::load(Variant& jsonObject, const std::string& objectName)
{
	if (jsonObject.getType() == Variant::MAP)
	{
		std::string meshName;
		auto it1 = jsonObject.getMap().find("meshName");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
		{
			meshName = it1->second.toString();
			if (meshName.find('.') == std::string::npos)
				meshName += ".fbx";
		}
		
		it1 = jsonObject.getMap().find("backfaceCulling");
		if (it1 != jsonObject.getMap().end())
		{
			if (it1->second.getType() == Variant::BOOL)
				m_backfaceCulling = it1->second.toBool();
			else if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
			{
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "WARNING : " << objectName << 
					" : OccluderComponent loading : backfaceCulling attribute need to be a boolean" << std::flush;
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
			}
		}

		if (!meshName.empty())
		{
			m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
			return true;
		}
		else
		{
			if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			{
				if (meshName.empty())
				{
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : " << objectName << " : OccluderComponent loading : no mesh name" << std::flush;
					std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
				}
			}
		}
	}
	return false;
}
void OccluderComponent::save(Variant& jsonObject)
{

}

void OccluderComponent::setMesh(const std::string& meshName)
{
	ResourceManager::getInstance()->release(m_mesh);
	m_mesh = ResourceManager::getInstance()->getResource<Mesh>(meshName);
}
void OccluderComponent::setMesh(Mesh* mesh)
{
	ResourceManager::getInstance()->release(m_mesh);
	if (mesh) m_mesh = ResourceManager::getInstance()->getResource<Mesh>(mesh);
	else m_mesh = nullptr;
}

Mesh* OccluderComponent::getMesh() const
{
	return m_mesh;
}

bool OccluderComponent::isValid() const
{
	return m_mesh && m_mesh->isValid();
}

bool OccluderComponent::backFaceCulling() const { return m_backfaceCulling; }

void OccluderComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Occluder);
}
void OccluderComponent::onDrawImGui()
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
		ImGui::Unindent();



		ImGui::TreePop();
	}
#endif // USE_IMGUI
}