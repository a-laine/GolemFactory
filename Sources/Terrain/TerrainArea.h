#pragma once

#include "TerrainVirtualTexture.h"

#include <Utiles/WorkerThread.h>
#include <Physics/Shapes/AxisAlignedBox.h>

class Terrain;
class Entity;

class TerrainArea
{
	friend class Terrain;

	public:
		// Default
		TerrainArea(vec2i index, Terrain* terrain);
		~TerrainArea();
		//

		// Public methods
		void generate(const std::string& directory);
		void setLod(int lod);
		int getLod();
		std::string getNameFromIndex() const;
		vec4f getCenter() const;
		const uint64_t* getRawData() const;
		const TerrainVirtualTexture::TextureTile& getTileData() const;
		Terrain* getTerrain() const;
		bool hasWater() const;
		AxisAlignedBox getBoundingBox() const;
		//
		
		// Members
		static int g_lodPixelCount[];
		static int g_lodSpacing[];
		static float g_noiseCurve[];
		static float g_heightAmplitude;
		static float g_seeLevel;
		static float g_erosion;
		static int g_seed;

	protected:
		// Members
		Terrain* m_terrain;
		uint64_t* m_data;
		vec2i m_areaIndex;
		vec2i m_gridIndex;
		vec4f m_center;
		int m_lod;
		bool m_hasWater;
		AxisAlignedBox m_boundingBox;

		std::vector<TerrainVirtualTexture::TextureTile> m_tiles;

		Entity* m_entity;
		//

		struct MapData
		{
			float height;
			float water;
			vec4f normal;
			uint8_t material;
			uint8_t hole;

			uint64_t pack();
			void unpack(uint64_t data);
		};
};