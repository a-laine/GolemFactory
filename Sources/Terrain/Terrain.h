#pragma once

#include "TerrainArea.h"

#include <map>
#include <set>
#include <vector>
#include <Resources/Mesh.h>
#include <Resources/Shader.h>

class World;
class Terrain
{
	public:
		// Default
		Terrain();
		~Terrain();
		//

		// public functions
		void initializeClipmaps();

		void addArea(TerrainArea& area, bool recomputeIfNeeded = true);
		void removeArea(vec2i index);

		void recomputeGrid();
		void clear();
		void generate(const std::string& directory);
		void generate(const std::string& directory, vec2i tileIndex);
		void load(const std::string& directory);

		void addLodRadius(float _radiusIncrement);
		void update(vec4f _cameraPosition);
		//

		//
		void setWorld(World* _world);
		void setVirtualTexture(TerrainVirtualTexture* virtualTexture);
		TerrainVirtualTexture* getVirtualTexture();
		std::string getDirectory() const;
		const TerrainArea* getArea(vec2i index) const;
		const std::vector<float>& getRadius() const;

		void drawImGui(World& world);
		//

		//static float g_heightAmplitude;
		static float g_morphingRange;

	protected:
		// structs
		struct vec2i_ordered
		{
			vec2i_ordered(int x, int y) : v(x, y) {};
			vec2i_ordered(const vec2f& u) : v(u) {};

			vec2i v;
			bool operator<(const vec2i_ordered& o) const
			{
				if (v.x != o.v.x)
					return v.x < o.v.x;
				else return v.y < o.v.y;
			}
		};
		struct Clipmap
		{
			GLuint m_VAO, m_vertexbuffer, m_arraybuffer;
			std::vector<vec4f> m_vertices;
			std::vector<uint16_t> m_indices;
		};
		//
		
		// Members
		World* m_world;
		Entity* m_areaContainer;
		TerrainVirtualTexture* m_virtualTexture;
		std::string m_directory;
		std::map<vec2i_ordered, TerrainArea> m_areas;
		vec2i m_gridSize, m_gridMinIndex;
		vec2i m_currentStreamingMin, m_currentStreamingMax;
		vec2f m_currentPlayerPosInTile;
		TerrainArea*** m_grid;
		std::set<TerrainArea*> m_previousAreas;
		std::vector<float> m_lodRadius;
		Job updateJob;
		std::vector<Mesh*> m_clipmapMeshes;
		Shader* m_terrainShader;
		Shader* m_waterShader;
		//
};