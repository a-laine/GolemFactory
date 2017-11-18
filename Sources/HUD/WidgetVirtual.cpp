#include "WidgetVirtual.h"

//  Default
WidgetVirtual::WidgetVirtual(const uint8_t& config, const std::string& shaderName) : configuration(config), position(0.f, 0.f, 0.f), size(1.f,1.f)
{
	shader = ResourceManager::getInstance()->getShader(shaderName);
	texture = nullptr;
}
WidgetVirtual::~WidgetVirtual()
{
	//	free all batch
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		batchList[i].vertices.clear();
		batchList[i].textures.clear();
		batchList[i].faces.clear();

		glDeleteBuffers(1, &batchList[i].verticesBuffer);
		glDeleteBuffers(1, &batchList[i].texturesBuffer);
		glDeleteBuffers(1, &batchList[i].facesBuffer);
		glDeleteVertexArrays(1, &batchList[i].vao);
	}

	//	free shared resources
	ResourceManager::getInstance()->release(shader);
}
//


//	Public functions
void WidgetVirtual::initializeVBOs()
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glGenBuffers(1, &batchList[i].verticesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);
		glBufferData(GL_ARRAY_BUFFER, batchList[i].vertices.size() * sizeof(glm::vec3), batchList[i].vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &batchList[i].texturesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);
		glBufferData(GL_ARRAY_BUFFER, batchList[i].textures.size() * sizeof(glm::vec2), batchList[i].textures.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &batchList[i].facesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, batchList[i].faces.size() * sizeof(unsigned short), batchList[i].faces.data(), GL_STATIC_DRAW);
	}
}
void WidgetVirtual::initializeVAOs()
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glGenVertexArrays(1, &batchList[i].vao);
		glBindVertexArray(batchList[i].vao);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);
		glBindVertexArray(0);
	}
}

void WidgetVirtual::draw(Shader* s)
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		int loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &batchList[i].color.x);

		glBindVertexArray(batchList[i].vao);
		glDrawElements(GL_TRIANGLES, batchList[i].faces.size(), GL_UNSIGNED_SHORT, NULL);
	}
}
void WidgetVirtual::update(const float& elapseTime) {}
//


//  Set/get functions
void WidgetVirtual::setSize(const glm::vec2& s) { size = s; }
void WidgetVirtual::setPosition(const glm::vec3& p) { position = p; }
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
void WidgetVirtual::setTexture(const std::string& shaderName)
{
	ResourceManager::getInstance()->release(texture);
	if (!shaderName.empty()) texture = ResourceManager::getInstance()->getTexture(shaderName);
	else texture = nullptr;
}

glm::vec3 WidgetVirtual::getPosition() const { return position; }
glm::vec4* WidgetVirtual::getColor(const unsigned int& index) { return &(batchList[index].color); }
uint8_t WidgetVirtual::getOriginPosition() const { return configuration & (HORIZONTAL_MASK | VERTICAL_MASK); }
bool WidgetVirtual::isVisible() const { return (configuration & VISIBLE)!=0; }
bool WidgetVirtual::isActive() const { return (configuration & ACTIVE) != 0; }
uint8_t WidgetVirtual::getState() const { return configuration & STATE_MASK; }
Shader* WidgetVirtual::getShader() const { return shader; }
Texture* WidgetVirtual::getTexture() const { return texture; }
//
