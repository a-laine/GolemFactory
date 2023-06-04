#pragma once

#include <Math/TMath.h>
#include <EntityComponent\Component.hpp>


class Shader;
class Mesh;


class DrawableComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(DrawableComponent, Component)
	public:
		DrawableComponent(const std::string& meshName = "default", const std::string& shaderName = "default");
		virtual ~DrawableComponent() override;
		DrawableComponent(const DrawableComponent* other);

		bool load(Variant& jsonObject, const std::string& objectName) override;
		void save(Variant& jsonObject) override;

		void setShader(const std::string& shaderName);
		void setShader(Shader* shader);
		void setMesh(const std::string& meshName);
		void setMesh(Mesh* mesh);

		Shader* getShader() const;
		Mesh* getMesh() const;

        bool isValid() const;

        bool hasSkeleton() const;
		vec4f getMeshBBMax() const;
		vec4f getMeshBBMin() const;

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;

#ifdef USE_IMGUI
		bool visible() const { return m_visible; };
#endif

	private:
		Mesh* m_mesh;
		Shader* m_shader;

#ifdef USE_IMGUI
		bool m_drawMeshBoundingBox = false;
		bool m_visible = true;
#endif

};

