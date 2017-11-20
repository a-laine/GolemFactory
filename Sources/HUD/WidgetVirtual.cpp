#include "WidgetVirtual.h"

//  Default
WidgetVirtual::WidgetVirtual(const WidgetType& t, const uint8_t& config, const std::string& shaderName) : type(t), configuration(config), position(0.f, 0.f, 0.f), size(1.f,1.f)
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
	ResourceManager::getInstance()->release(texture);
}
//


//	Public functions
void WidgetVirtual::initialize(int VBOtype)
{
	initializeVBOs(VBOtype);
	initializeVAOs();
}
void WidgetVirtual::draw(Shader* s, uint8_t& stencilMask)
{
	//	texture related stuff
	if (texture) glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (texture ? 1 : 0));

	//	draw all batches
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		int loc2 = s->getUniformLocation("color");
		if (loc2 >= 0) glUniform4fv(loc2, 1, &batchList[i].color.x);

		glBindVertexArray(batchList[i].vao);
		glDrawElements(GL_TRIANGLES, batchList[i].faces.size(), GL_UNSIGNED_SHORT, NULL);
	}
}
void WidgetVirtual::update(const float& elapseTime) {}

void WidgetVirtual::setString(const std::string& s) {}
std::string WidgetVirtual::getString() { return std::string(); }
std::stringstream* WidgetVirtual::getStream() { return nullptr; }
//


//  Set/get functions
void WidgetVirtual::setSize(const glm::vec2& s) { size = s; }
void WidgetVirtual::setPosition(const glm::vec3& p) { position = p; }
void WidgetVirtual::setVisibility(const bool& visible)
{
	if (visible) configuration |= VISIBLE;
	else configuration &= ~VISIBLE;
}
void WidgetVirtual::setTexture(const std::string& textureName)
{
	ResourceManager::getInstance()->release(texture);
	if (!textureName.empty()) texture = ResourceManager::getInstance()->getTexture(textureName);
	else texture = nullptr;
}
void WidgetVirtual::setShader(const std::string& shaderName)
{
	ResourceManager::getInstance()->release(shader);
	shader = ResourceManager::getInstance()->getShader(shaderName);
}


WidgetVirtual::WidgetType WidgetVirtual::getType() const { return type; }
glm::vec3 WidgetVirtual::getPosition() const { return position; }
glm::vec4* WidgetVirtual::getColor(const unsigned int& index) { return &(batchList[index].color); }
bool WidgetVirtual::isVisible() const { return (configuration & VISIBLE)!=0; }
Shader* WidgetVirtual::getShader() const { return shader; }
Texture* WidgetVirtual::getTexture() const { return texture; }
unsigned int WidgetVirtual::getNumberFaces() const
{
	unsigned int result = 0;
	for (unsigned int i = 0; i < batchList.size(); i++)
		result += batchList[i].faces.size();
	return result;
}
//

//	Protected functions
void WidgetVirtual::drawClippingShape(const unsigned int& batchIndex, const bool& enableClipping, Shader* s, uint8_t& stencilMask)
{
	glDisable(GL_DEPTH_TEST);
	if (enableClipping)
	{
		stencilMask++;
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	}
	else
	{
		stencilMask--;
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	}
	glStencilFunc(GL_ALWAYS, stencilMask, 0xFF);

	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, 0);
	loc = s->getUniformLocation("color");
	if (loc >= 0) glUniform4fv(loc, 1, (const float*)&glm::vec4(0.f, 0.f, 0.f, 0.f));

	glBindVertexArray(batchList[1].vao);
	glDrawElements(GL_TRIANGLES, batchList[1].faces.size(), GL_UNSIGNED_SHORT, NULL);

	glStencilFunc(GL_EQUAL, stencilMask, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_DEPTH_TEST);
}
void WidgetVirtual::initializeVBOs(int VBOtype)
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glGenBuffers(1, &batchList[i].verticesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);
		glBufferData(GL_ARRAY_BUFFER, batchList[i].vertices.size() * sizeof(glm::vec3), batchList[i].vertices.data(), VBOtype);

		glGenBuffers(1, &batchList[i].texturesBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);
		glBufferData(GL_ARRAY_BUFFER, batchList[i].textures.size() * sizeof(glm::vec2), batchList[i].textures.data(), VBOtype);

		glGenBuffers(1, &batchList[i].facesBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, batchList[i].faces.size() * sizeof(unsigned short), batchList[i].faces.data(), VBOtype);
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
//
