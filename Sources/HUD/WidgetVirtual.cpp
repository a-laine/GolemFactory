#include "WidgetVirtual.h"

//  Default
WidgetVirtual::WidgetVirtual(const uint8_t& config, const std::string& shaderName) : configuration(config), position(0, 0), size(1.f,1.f)
{
	shader = ResourceManager::getInstance()->getShader(shaderName);

	///
	drawBatch b;
		b.color = glm::vec4(1.f, 1.f, 1.f, 0.2f);

		b.vertices.push_back(glm::vec3(-0.5f*size.x, 0.f, -0.5f*size.y));
		b.vertices.push_back(glm::vec3(-0.5f*size.x, 0.f,  0.5f*size.y));
		b.vertices.push_back(glm::vec3( 0.f*size.x,  0.f,  0.5f*size.y));
		b.vertices.push_back(glm::vec3( 0.f*size.x,  0.f, -0.5f*size.y));

		b.textures.push_back(glm::vec2(0.f, 0.f));
		b.textures.push_back(glm::vec2(0.f, 0.f));
		b.textures.push_back(glm::vec2(0.f, 0.f));
		b.textures.push_back(glm::vec2(0.f, 0.f));

		b.faces.push_back(0); b.faces.push_back(1); b.faces.push_back(2);
		b.faces.push_back(0); b.faces.push_back(2); b.faces.push_back(3);
		batchList.push_back(b);

	initializeVBO(0);
	initializeVAO(0);
	
		b.color = glm::vec4(1.f, 0.f, 1.f, 1.f);

		b.vertices.clear();
		b.vertices.push_back(glm::vec3(0.f*size.x,  0.f, -0.5f*size.y));
		b.vertices.push_back(glm::vec3(0.f*size.x,  0.f, 0.5f*size.y));
		b.vertices.push_back(glm::vec3(0.5f*size.x, 0.f, 0.5f*size.y));
		b.vertices.push_back(glm::vec3(0.5f*size.x, 0.f, -0.5f*size.y));

		batchList.push_back(b);

	initializeVBO(1);
	initializeVAO(1);
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
void WidgetVirtual::initializeVBO(const unsigned int& index)
{
	glGenBuffers(1, &batchList[index].verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[index].verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, batchList[index].vertices.size() * sizeof(glm::vec3), batchList[index].vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &batchList[index].texturesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[index].texturesBuffer);
	glBufferData(GL_ARRAY_BUFFER, batchList[index].textures.size() * sizeof(glm::vec2), batchList[index].textures.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &batchList[index].facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[index].facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, batchList[index].faces.size() * sizeof(unsigned int), batchList[index].faces.data(), GL_STATIC_DRAW);
}
void WidgetVirtual::initializeVAO(const unsigned int& index)
{
	glGenVertexArrays(1, &batchList[index].vao);
	glBindVertexArray(batchList[index].vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[index].verticesBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[index].texturesBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[index].facesBuffer);
	glBindVertexArray(0);
}

void WidgetVirtual::draw(const unsigned int& index)
{
	glBindVertexArray(batchList[index].vao);
	glDrawElements(GL_TRIANGLES, batchList[index].faces.size(), GL_UNSIGNED_INT, NULL);
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
glm::vec4* WidgetVirtual::getColor(const unsigned int& index) { return &(batchList[index].color); }
uint8_t WidgetVirtual::getOriginPosition() const { return configuration & (HORIZONTAL_MASK | VERTICAL_MASK); }
bool WidgetVirtual::isVisible() const { return (configuration & VISIBLE)!=0; }
bool WidgetVirtual::isActive() const { return (configuration & ACTIVE) != 0; }
uint8_t WidgetVirtual::getState() const { return configuration & STATE_MASK; }
unsigned int WidgetVirtual::getBatchListSize() const { return batchList.size(); }
Shader* WidgetVirtual::getShader() const { return shader; }
//
