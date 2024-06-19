#pragma once

#include <Renderer/DrawableComponent.h>
#include <Terrain/TerrainArea.h>

class TerrainAreaDrawableComponent : public DrawableComponent
{
	GF_DECLARE_COMPONENT_CLASS(TerrainAreaDrawableComponent, DrawableComponent)
	public:
		struct alignas(16) TerrainAreaData
		{
			vec2f position;
			vec2f textureOffset;
			float morphingCase;
			vec3f padding;
		};
		struct alignas(16) TerrainConstantData
		{
			float lod;
			float textureSize;
			float heightAmplitude;
			float seeLevel;
			float morphRadius[8];
			float morphDistance;
			float animatedTime;
		};

		TerrainAreaDrawableComponent(TerrainArea* _area);

		unsigned short getInstanceDataSize() const override;
		void pushInstanceData(Shader* _shader) const override;
		void writeInstanceData(vec4f* _destination) const override;
		bool hasConstantData() const override;
		void pushConstantData(Shader* _shader) const override;
		void onAddToEntity(Entity* entity) override;
		void pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass) override;
		void updateData(TerrainVirtualTexture::TextureTile& tile);

		Shader* getWaterShader() const;
		void setWaterShader(Shader* _shader);
		bool hasWater() const;
		AxisAlignedBox getBoundingBox() const;

	protected:
		TerrainAreaData m_data;
		TerrainArea* m_area;
		Shader* m_waterShader;
};

