#pragma once

#include <vector>

#include <Math/TMath.h>
#include <EntityComponent\Component.hpp>
#include "Renderer.h"


class Shader;
class Mesh;
class Skeleton;
class Batch;

class DrawableComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(DrawableComponent, Component)
	public:
		explicit DrawableComponent();
		explicit DrawableComponent(const std::string& meshName, const std::string& shaderName);
		virtual ~DrawableComponent() override;
		DrawableComponent(const DrawableComponent* other);

		bool load(Variant& jsonObject, const std::string& objectName) override;
		bool load(Variant& jsonObject, const std::string& objectName, const Skeleton* skeleton = nullptr);
		void save(Variant& jsonObject) override;

		void setShader(const std::string& shaderName);
		void setShader(Shader* shader);
		void setMesh(const std::string& meshName);
		void setMesh(Mesh* mesh);
		void setCastShadow(bool enabled);

		virtual void pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass);

		Shader* getShader() const;
		Mesh* getMesh() const;
		virtual bool hasCustomDraw() const;
		virtual void customDraw(Renderer* _renderer, unsigned int& _instanceDrawnCounter, unsigned int& _drawCallsCounter, unsigned int& _trianglesDrawnCounter) const;

        bool isValid() const;
		bool castShadow() const;

		virtual unsigned short getInstanceDataSize() const;
		virtual void pushInstanceData(Shader* _shader) const;
		virtual void writeInstanceData(vec4f* _destination) const;
		virtual bool hasConstantData() const;
		virtual void pushConstantData(Shader* _shader) const;

        virtual bool hasSkeleton() const;
		vec4f getMeshBBMax() const;
		vec4f getMeshBBMin() const;

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;

#ifdef USE_IMGUI
		bool visible() const { return m_visible; };
#endif

	protected:
		Mesh* m_mesh;
		Shader* m_shader;

		bool m_castShadow;

#ifdef USE_IMGUI
		bool m_drawMeshBoundingBox = false;
		bool m_visible = true;
#endif

};

