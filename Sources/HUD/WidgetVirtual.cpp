#include "WidgetVirtual.h"

//  Default
WidgetVirtual::WidgetVirtual(const uint8_t& config, const std::string& shaderName) : configuration(config), position(0, 0), size(1.f,1.f)//size(0.07f, 0.05f)
{
	shader = ResourceManager::getInstance()->getShader(shaderName);

	vertices.push_back(glm::vec3(-0.5f*size.x, 0.f, -0.5f*size.y));
	vertices.push_back(glm::vec3(-0.5f*size.x, 0.f,  0.5f*size.y));
	vertices.push_back(glm::vec3( 0.5f*size.x, 0.f,  0.5f*size.y));
	vertices.push_back(glm::vec3( 0.5f*size.x, 0.f, -0.5f*size.y));

	textures.push_back(glm::vec2(0.f, 0.f));
	textures.push_back(glm::vec2(0.f, 0.f));
	textures.push_back(glm::vec2(0.f, 0.f));
	textures.push_back(glm::vec2(0.f, 0.f));

	faces.push_back(0); faces.push_back(1); faces.push_back(2);
	faces.push_back(0); faces.push_back(2); faces.push_back(3);

	initializeVBO();
	initializeVAO();
}
WidgetVirtual::~WidgetVirtual()
{
	vertices.clear();
	textures.clear();
	faces.clear();

	glDeleteBuffers(1, &verticesBuffer);
	glDeleteBuffers(1, &texturesBuffer);
	glDeleteBuffers(1, &facesBuffer);
	glDeleteVertexArrays(1, &vao);

	ResourceManager::getInstance()->release(shader);
}
//


//	Public functions
void WidgetVirtual::initializeVBO()
{
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &texturesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, texturesBuffer);
	glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(glm::vec2), textures.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned int), faces.data(), GL_STATIC_DRAW);
}
void WidgetVirtual::initializeVAO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, texturesBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);
}

void WidgetVirtual::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, NULL);
}
void WidgetVirtual::update(const float& elapseTime) {}
//


//  Set/get functions
void WidgetVirtual::setSize(const glm::vec2& s) { size = s; }
void WidgetVirtual::setPosition(const glm::vec2& p) { position = p; }
void WidgetVirtual::setOrigin(const uint8_t& origin)
{
	configuration &= ~(HORIZONTAL_MASK | VERTICAL_MASK);
	configuration |= origin & (HORIZONTAL_MASK | VERTICAL_MASK);
}
void WidgetVirtual::setVisibility(const bool& visible)
{
	if (visible) configuration |= VISIBLE;
	else configuration &= ~VISIBLE;
}
void WidgetVirtual::setActive(const bool& active)
{
	if (active) configuration |= ACTIVE;
	else configuration &= ~ACTIVE;
}


glm::vec2 WidgetVirtual::getSize() const { return size; }
glm::vec2 WidgetVirtual::getPosition() const { return position; }
uint8_t WidgetVirtual::getOriginPosition() const { return configuration & (HORIZONTAL_MASK | VERTICAL_MASK); }
bool WidgetVirtual::isVisible() const { return (configuration & VISIBLE)!=0; }
bool WidgetVirtual::isActive() const { return (configuration & ACTIVE) != 0; }
uint8_t WidgetVirtual::getState() const { return configuration & STATE_MASK; }
glm::vec2 WidgetVirtual::getWidgetCenter() const
{
	glm::vec2 center = position;
	switch (configuration&(HORIZONTAL_MASK | VERTICAL_MASK))
	{
	case CENTER:   break;
	case (MIDDLE_H | TOP):          center.y -= size.y / 2;   break;
	case (MIDDLE_H | BOTTOM):       center.y += size.y / 2;   break;

	case (LEFT | MIDDLE_V):         center.x += size.x / 2;   break;
	case (LEFT | TOP):              center.x += size.x / 2;   center.y -= size.y / 2;   break;
	case (LEFT | BOTTOM):           center.x += size.x / 2;   center.y += size.y / 2;   break;

	case (RIGHT | MIDDLE_V):		center.x -= size.x / 2;   break;
	case (RIGHT | TOP):             center.x -= size.x / 2;   center.y -= size.y / 2;   break;
	case (RIGHT | BOTTOM):          center.x -= size.x / 2;   center.y += size.y / 2;   break;

	default: break;
	}
	return center;
}

Shader* WidgetVirtual::getShader() const { return shader; }
//
