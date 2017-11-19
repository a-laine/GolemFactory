#include "WidgetImage.h"

//  Default
WidgetImage::WidgetImage(const std::string& textureName, const uint8_t& config, const std::string& shaderName) : WidgetVirtual(WidgetVirtual::IMAGE, config, shaderName)
{
	texture = ResourceManager::getInstance()->getTexture2D(textureName);
}
WidgetImage::~WidgetImage() {}
//

//	Public functions
void WidgetImage::initialize()
{
	drawBatch quad;
	quad.color = glm::vec4(1.f, 1.f, 1.f, 1.f);

	quad.vertices.push_back(glm::vec3(-0.5f * size.x, 0.f, -0.5f * size.y));
	quad.vertices.push_back(glm::vec3(-0.5f * size.x, 0.f,  0.5f * size.y));
	quad.vertices.push_back(glm::vec3( 0.5f * size.x, 0.f,  0.5f * size.y));
	quad.vertices.push_back(glm::vec3( 0.5f * size.x, 0.f, -0.5f * size.y));

	quad.textures.push_back(glm::vec2(0.f, 1.f));
	quad.textures.push_back(glm::vec2(0.f, 0.f));
	quad.textures.push_back(glm::vec2(1.f, 0.f));
	quad.textures.push_back(glm::vec2(1.f, 1.f));

	quad.faces.push_back(0); quad.faces.push_back(1); quad.faces.push_back(2);
	quad.faces.push_back(0); quad.faces.push_back(2); quad.faces.push_back(3);

	batchList.push_back(quad);
	initializeVBOs();
	initializeVAOs();
}
//

