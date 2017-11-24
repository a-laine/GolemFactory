#include "WidgetImage.h"

#define BATCH_INDEX_QUAD 0

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
	//	init
	colors[CURRENT] = colors[(State)(configuration & STATE_MASK)];
	positions[CURRENT] = positions[(State)(configuration & STATE_MASK)];
	sizes[CURRENT] = sizes[(State)(configuration & STATE_MASK)];

	//	fill batch
	DrawBatch quad;
		quad.vertices.push_back(glm::vec3(-0.5f * sizes[CURRENT].x, 0.f, -0.5f * sizes[CURRENT].y));
		quad.vertices.push_back(glm::vec3(-0.5f * sizes[CURRENT].x, 0.f,  0.5f * sizes[CURRENT].y));
		quad.vertices.push_back(glm::vec3( 0.5f * sizes[CURRENT].x, 0.f,  0.5f * sizes[CURRENT].y));
		quad.vertices.push_back(glm::vec3( 0.5f * sizes[CURRENT].x, 0.f, -0.5f * sizes[CURRENT].y));

		quad.textures.push_back(glm::vec2(0.f, 1.f));
		quad.textures.push_back(glm::vec2(0.f, 0.f));
		quad.textures.push_back(glm::vec2(1.f, 0.f));
		quad.textures.push_back(glm::vec2(1.f, 1.f));

		quad.faces.push_back(0); quad.faces.push_back(1); quad.faces.push_back(2);
		quad.faces.push_back(0); quad.faces.push_back(2); quad.faces.push_back(3);

	//	end
	batchList.push_back(quad);
	initializeVBO(BATCH_INDEX_QUAD, GL_STATIC_DRAW);
	initializeVAOs();
}
//

