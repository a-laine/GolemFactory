#include "TextureSaver.h"

#include <Utiles/Parser/Writer.h>
#include <Utiles/ToolBox.h>
#include <Utiles/ConsoleColor.h>

#pragma warning(push, 0)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma warning(pop)


//	Public functions
void TextureSaver::save(Texture* texture, const std::string& resourcesPath, bool withMetadata, std::string fileName)
{
    using config = Texture::TextureConfiguration;
	if (!texture) return;

	const std::string& baseName = texture->name;
	auto PrintError = [&baseName](const std::string& msg)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
		{
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR : exporting texture : " << baseName << " : " << msg << std::flush;
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	};
	auto PrintWarning = [&baseName](const std::string& msg)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
		{
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "WARNING : exporting texture : " << baseName << " : " << msg << std::flush;
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	};

	//	initialize fileName
	if (fileName.size() == 0)
		fileName = texture->name;

	//	clear buffers
	Variant rootVariant;
	rootVariant.createMap();

	// type
	uint16_t configuration = texture->configuration;
    config type = (config)(configuration & (uint16_t)config::TYPE_MASK);
    switch (type)
    {
        case Texture::TextureConfiguration::TEXTURE_1D:		rootVariant.insert("type", "1D"); break;
        case Texture::TextureConfiguration::TEXTURE_2D:		rootVariant.insert("type", "2D"); break;
        case Texture::TextureConfiguration::TEXTURE_3D:		rootVariant.insert("type", "3D"); break;
        case Texture::TextureConfiguration::CUBEMAP:		rootVariant.insert("type", "CUBEMAP"); break;
        case Texture::TextureConfiguration::TEXTURE_ARRAY:	rootVariant.insert("type", "ARRAY"); break;
        default: rootVariant.insert("type", "unknown"); break;
    }

	// size
	int width = (int)texture->size.x;
	int height = (int)texture->size.y;
	rootVariant.insert("width", width);
	rootVariant.insert("height", height);
	rootVariant.insert("mipmap", (configuration & (uint16_t)config::USE_MIPMAP) ? true : false);

	if (configuration & (uint16_t)config::MAG_NEAREST)
		rootVariant.insert("minnearest", true);
	if (configuration & (uint16_t)config::MAG_NEAREST)
		rootVariant.insert("magnearest", true);

	config wrap = (config)(configuration & (uint16_t)config::WRAP_MASK);
	switch (wrap)
	{
		case Texture::TextureConfiguration::WRAP_REPEAT: rootVariant.insert("wrap", "repeat"); break;
		case Texture::TextureConfiguration::WRAP_MIRROR: rootVariant.insert("wrap", "mirror");  break;
		default: break;
	}

	// textures
	rootVariant.insert("texture", Variant::ArrayType());
	{
		Variant textureVariant;
		auto& textureArray = textureVariant.createArray();

		if (type == Texture::TextureConfiguration::TEXTURE_2D)
		{
			uint8_t* buffer = new uint8_t[4 * width * height];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

			std::string imageName = fileName;
			if (imageName.find('.') == std::string::npos)
				imageName += ".png";
			textureArray.push_back(imageName);
			std::string imagePath = resourcesPath + Texture::directory + imageName;
			stbi_write_png(imagePath.c_str(), width, height, 4, buffer, width * 4);

			delete[] buffer;
		}
		else if (type == Texture::TextureConfiguration::CUBEMAP)
		{
			uint8_t* buffer = new uint8_t[4 * width * height];

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, texture->getTextureId());
			for (int i = 0; i < 6; i++)
			{
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

				std::string imageName = fileName + "_" + std::to_string(i) + ".png";
				textureArray.push_back(imageName);
				std::string imagePath = resourcesPath + Texture::directory + imageName;
				stbi_write_png(imagePath.c_str(), width, height, 4, buffer, width * 4);
			}
			delete[] buffer;
		}
		else
		{
			PrintError("no texture image to export");
		}
		rootVariant.insert("texture", textureVariant);
	}

	//	save into file
	if (withMetadata)
	{
		std::ofstream file(resourcesPath + Texture::directory + fileName + Texture::extension, std::ofstream::out | std::ofstream::trunc);
		Writer writer(&file);
		file << std::fixed;
		file.precision(5);
		writer.setInlineArray(true);
		file << "{" << std::endl;
		file << writer.writeInString(rootVariant);
		file << "}" << std::endl;
		file.close();
	}
}
//