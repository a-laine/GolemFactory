#pragma once

#include <EntityComponent\Component.hpp>


class Mesh;

class OccluderComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(OccluderComponent, Component)

	public:
		OccluderComponent(const std::string& meshName = "default");
		virtual ~OccluderComponent() override;
		OccluderComponent(const OccluderComponent* other);

		bool load(Variant& jsonObject, const std::string& objectName) override;
		void save(Variant& jsonObject) override;

		void setMesh(const std::string& meshName);
		void setMesh(Mesh* mesh);

		Mesh* getMesh() const;

		bool isValid() const;
		bool backFaceCulling() const;

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;

	private:
		Mesh* m_mesh;
		bool m_backfaceCulling;


#ifdef USE_IMGUI
		bool m_drawMesh = false;
#endif
};