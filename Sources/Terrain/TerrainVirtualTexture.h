#pragma once

#include "Math/TMath.h"

#include <atomic>
#include <vector>
#include <Utiles/Mutex.h>
#include <Resources/Texture.h>

class TerrainArea;
class TerrainVirtualTexture
{ 
	public:
		// Structures
		struct TextureTile
		{
			int m_lod;
			TerrainArea* m_owner;
			vec2i m_min, m_max, m_size;
		};
		// 
		
		// Default
		TerrainVirtualTexture();
		~TerrainVirtualTexture();
		//

		// Public methods
		void initialize(int _physicalTextureSize);
		void updateCPUTexture(TextureTile* _destination, uint64_t* _sourceHeightMap);
		void updateGPUTexture();
		void syncroGPUTexture();

		TextureTile getFreeTextureTile(int _lod);
		GLuint getTextureId() const;

		void releaseTextureTile(TextureTile _textureTile);
		//


	protected:
		// Protected fnctions

		//

		// Members
		Mutex m_mutexPool;
		std::vector<std::vector<TextureTile>> m_freeTilesPools;

		Mutex m_mutexPhysicalTexture;
		std::vector<TextureTile*> m_pendingTiles;
		Texture m_GPUTexture;

		vec3i m_physicalTextureSize;
		uint16_t* m_CPUTexture;
		//
};