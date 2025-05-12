#pragma once

#include <vector>

#include <Math/TMath.h>
#include <EntityComponent\Component.hpp>
#include "Renderer.h"


class Material;
class Mesh;
class Skeleton;

class DrawableComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(DrawableComponent, Component)
	public:
		explicit DrawableComponent();
		explicit DrawableComponent(const std::string& meshName, const std::string& materialName);
		virtual ~DrawableComponent() override;
		DrawableComponent(const DrawableComponent* other);

		bool load(Variant& jsonObject, const std::string& objectName) override;
		bool load(Variant& jsonObject, const std::string& objectName, const Skeleton* skeleton = nullptr);
		void save(Variant& jsonObject) override;

		void setMaterial(const std::string& materialName);
		void setMaterial(Material* material);
		void setMesh(const std::string& meshName);
		void setMesh(Mesh* mesh);

		virtual void pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass);

		Material* getMaterial() const;
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
		//const std::vector<Texture*>* getTextureOverride() const;
		//bool setTextureOverride(const std::string& texIdentifier, const std::string& texOverride);
		void setClockWise(bool ccwEnable);

        virtual bool hasSkeleton() const;
		vec4f getMeshBBMax() const;
		vec4f getMeshBBMin() const;
		bool isClockWise() const;

		void onDrawImGui() override;
		void onAddToEntity(Entity* entity) override;

#ifdef USE_IMGUI
		bool visible() const { return m_visible; };
#endif

	protected:
		Mesh* m_mesh;
		Material* m_material;
		//std::vector<Texture*> m_textureOverride;

		//int m_shadowMaxCascade;
		bool m_ClockWise;

#ifdef USE_IMGUI
		bool m_drawMeshBoundingBox = false;
		bool m_visible = true;
#endif

};

