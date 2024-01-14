#pragma once

#include "Math/TMath.h"

#include <atomic>
#include <vector>
#include <Utiles/Mutex.h>
#include <Resources/Texture.h>

class TerrainVirtualTexture
{
	public:
		// Structures
		struct TextureTile
		{
			int m_lod;
			std::atomic_bool* m_updateHook;
			vec2i m_min, m_max, m_size;
		};
		// 
		
		// Default
		TerrainVirtualTexture();
		~TerrainVirtualTexture();
		//

		// Public methods
		void initialize(int _physicalTextureSize);
		void updateCPUTexture(const TextureTile& _destination, uint16_t* _source);
		void updateGPUTexture();

		TextureTile getFreeTextureTile(int _lod);

		void releaseTextureTile(TextureTile& _textureTile);
		//


	protected:
		// Protected fnctions

		//

		// Members
		Mutex m_mutexPool;
		std::vector<std::vector<TextureTile>> m_freeTilesPools;

		Mutex m_mutexPhysicalTexture;
		std::vector<TextureTile*> m_pendingTiles;
		vec2i m_pendingUpdateMin, m_pendingUpdateMax;
		Texture m_GPUTexture;

		vec3i m_physicalTextureSize;
		uint16_t* m_CPUTexture;
		//
};