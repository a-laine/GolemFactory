#pragma once

#include "TerrainVirtualTexture.h"

#include <Utiles/WorkerThread.h>
#include <Physics/Shapes/AxisAlignedBox.h>

class Terrain;
class Entity;
class Mesh;

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
		std::vector<vec4ui> generateDetails(float density, vec2f probability, vec2f scaleRange, const std::vector<int>& allowedMaterials, const uint64_t* dataPtr = nullptr);
		Entity* addDetailsInstance(const std::string& meshName, float density, const std::vector<int>& allowedMaterials, vec2f probability, vec2f scaleRange, float worldNormalWeight, float modelOffset);
		Entity* addDetailsInstance(Mesh* mesh, int identifier, float worldNormalWeight, float modelOffset);
		void setLod(int lod);
		void loadInstanceData();
		void unloadInstanceData();
		int getLod();
		std::string getNameFromIndex() const;
		vec4f getCenter() const;
		const uint64_t* getRawData() const;
		const TerrainVirtualTexture::TextureTile& getTileData(int lod) const;
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
		static float g_snowLevel;
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

		struct Details
		{
			std::string m_name;
			int m_lod;
			std::vector<Entity*> m_detailEntities;
		};
		std::vector<Details> m_details;
		//

		struct MapData
		{
			vec4f normalTerrain;
			vec4f normalWater;
			float heightTerrain;
			float heightWater;
			bool holeTerrain;
			bool holeWater;
			uint8_t material;

			uint64_t pack();
			void unpack(uint64_t data);
			uint64_t octahedralPack(vec4f n, int bits);
			vec4f octahedralUnpack(uint64_t n, int bits);
		};
		struct DetailInstanceDataHeader
		{
			int m_arrayCount;
			std::vector<int> m_arrayIdentifier;
			std::vector<int> m_arraySizes;
		};
		DetailInstanceDataHeader m_instanceHeader;
		std::vector<std::vector<vec4ui>> m_instanceDatas;
};