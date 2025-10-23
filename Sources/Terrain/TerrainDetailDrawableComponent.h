#pragma once

#include <Renderer/DrawableComponent.h>
#include <Terrain/TerrainArea.h>

class TerrainDetailDrawableComponent : public DrawableComponent
{
	GF_DECLARE_COMPONENT_CLASS(TerrainDetailDrawableComponent, DrawableComponent)
	public:
		struct alignas(16) TerrainDetailConstantData
		{
			float lod, textureSize, heightAmplitude, seeLevel;		// 0
			float morphRadius[8];									// 1, 2
			float morphDistance, animatedTime;						// 3
			vec2f position;											// 3
			vec2f textureOffset;									// 4
			float normalWorldWeight, fullModelScale;				// 4
			float modelOffset;										// 5
			vec3f color0; 										    // 5
			float alphaThs;										    // 6
			vec3f color1;										    // 6
		};

		TerrainDetailDrawableComponent(TerrainArea* _area);
		~TerrainDetailDrawableComponent() override;

		bool hasConstantData() const override;
		void pushConstantData(Shader* _shader) const override;
		void onAddToEntity(Entity* entity) override;
		void pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass) override;
		bool hasCustomDraw() const override;
		void customDraw(Renderer* _renderer, unsigned int& _instanceDrawnCounter, unsigned int& _drawCallsCounter, unsigned int& _trianglesDrawnCounter) const override;

		AxisAlignedBox getBoundingBox() const;
		void setInstanceData(std::vector<vec4ui>& data);
		void initializeVBO();
		void initializeVAO();

		void setFullModelScale(float scale);
		float getFullModelScale() const;
		void setWorldNormalWeight(float weight);
		void setModelOffset(float offset);
		void setDoubleSidedFaces(bool enable);
		void setColorTintGradient(vec3f c0, vec3f c1);
		void setAlphaClipThs(float ths);
		void setDetailIndex(uint16_t index);

		void onDrawImGui() override;

	protected:
		TerrainArea* m_area;
		AxisAlignedBox m_boundingBox;
		std::vector<vec4ui> m_instancesDatas;
		GLuint  m_vao, m_instancesBuffer;
		float m_normalWorldWeight;
		float m_fullModelScale;
		float m_modelOffset;
		vec3f m_color0, m_color1;
		float m_alphaThs;
		uint16_t m_detailIndex;
		bool m_forceDoublesided;

};
