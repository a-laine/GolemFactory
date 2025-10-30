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
			vec4f padding;
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
			uint32_t debug;
		};

		TerrainAreaDrawableComponent(TerrainArea* _area); 
		~TerrainAreaDrawableComponent() override;

		unsigned short getInstanceDataSize() const override;
		void pushInstanceData(Shader* _shader) const override;
		void writeInstanceData(vec4f* _destination) const override;
		bool hasConstantData() const override;
		void pushConstantData(Shader* _shader) const override;
		void onAddToEntity(Entity* entity) override;
		void pushDraw(std::vector<Renderer::DrawElement>& drawQueue, uint32_t distance, bool isShadowPass) override;
		void updateData(TerrainVirtualTexture::TextureTile& tile);

		bool hasWater() const;
		AxisAlignedBox getBoundingBox() const;
		const TerrainArea* getArea();

	protected:
		TerrainAreaData m_data;
		TerrainArea* m_area;
};

