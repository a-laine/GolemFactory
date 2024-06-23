#pragma once

#include <fstream>

#include <Resources/Texture.h>
#include <Utiles/Parser/Variant.h>

class TextureSaver
{
	public:
		//	Public functions
		static void save(Texture* texture, const std::string& resourcesPath, bool withMetadata, std::string fileName = "");
		//
};
