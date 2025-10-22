#include "WidgetVirtual.h"
#include "WidgetGLDebugger.h"

#include <Physics/Collision.h>

//  Default
WidgetVirtual::WidgetVirtual(const WidgetType& t, const uint8_t& config, const std::string& shaderName) : type(t), configuration(config)
{
	sizes[State::DEFAULT] = vec2f::one;
	sizes[State::HOVER] = vec2f::one;
	sizes[State::ACTIVE] = vec2f::one;
	sizes[State::CURRENT] = vec2f::one;

	positions[State::DEFAULT] = vec4f::zero;
	positions[State::HOVER] = vec4f::zero;
	positions[State::ACTIVE] = vec4f::zero;
	positions[State::CURRENT] = vec4f::zero;

	colors[State::DEFAULT] = vec4f::one;
	colors[State::HOVER] = vec4f::one;
	colors[State::ACTIVE] = vec4f::one;
	colors[State::CURRENT] = vec4f::one;

	shader = ResourceManager::getInstance()->getResource<Shader>(shaderName);
	texture = nullptr;
}
WidgetVirtual::~WidgetVirtual()
{
	//	free all batch
	const std::string errorHeader = "destroy batch ";
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		batchList[i].vertices.clear();
		batchList[i].textures.clear();
		batchList[i].faces.clear();

		glDeleteBuffers(1, &batchList[i].verticesBuffer);	CheckGLError(errorHeader + std::to_string(i), "glDeleteBuffers(vertex)");
		glDeleteBuffers(1, &batchList[i].texturesBuffer);	CheckGLError(errorHeader + std::to_string(i), "glDeleteBuffers(textures)");
		glDeleteBuffers(1, &batchList[i].facesBuffer);		CheckGLError(errorHeader + std::to_string(i), "glDeleteBuffers(faces)");
		glDeleteVertexArrays(1, &batchList[i].vao);			CheckGLError(errorHeader + std::to_string(i), "glDeleteVertexArrays(vao)");

		batchList[i].verticesBuffer = 0;
		batchList[i].texturesBuffer = 0;
		batchList[i].facesBuffer = 0;
		batchList[i].vao = 0;
	}

	//	free shared resources
	ResourceManager::getInstance()->release(shader);
	ResourceManager::getInstance()->release(texture);
}
//


//	Public functions
void WidgetVirtual::draw(Shader* s, uint8_t& stencilMask, const mat4f& model)
{
	//	texture related stuff
	if (texture) glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) 
	{
		glUniform1i(loc, (texture ? 1 : 0));	CheckGLError("useTexture uniform", "glUniform1i()");
	}

	//	draw all batches
	State state = (State)(configuration & (uint8_t)State::STATE_MASK);
	loc = s->getUniformLocation("color");
	if (loc >= 0) 
	{
		glUniform4fv(loc, 1, &colors[state].x); CheckGLError("color uniform", "glUniform4fv()");
	}
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glBindVertexArray(batchList[i].vao);	CheckGLError("widgetDraw", "glBindVertexArray()");
		glDrawElements(GL_TRIANGLES, (int)batchList[i].faces.size(), GL_UNSIGNED_SHORT, NULL); CheckGLError("widgetDraw", "glDrawElements");
	}
}
void WidgetVirtual::update(const float& elapseTime)
{
	State s = (State)(configuration & (uint8_t)State::STATE_MASK);
	colors[State::CURRENT] = colors[s];
	positions[State::CURRENT] = positions[s];
	sizes[State::CURRENT] = sizes[s];
}
bool WidgetVirtual::intersect(const mat4f& base, const vec4f& ray)
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		for (unsigned int j = 0; j < batchList[i].faces.size(); j +=3 )
		{
			//	compute triangles vertices in eyes space
			vec4f p1 = base * batchList[i].vertices[batchList[i].faces[j]];
			vec4f p2 = base * batchList[i].vertices[batchList[i].faces[j + 1]];
			vec4f p3 = base * batchList[i].vertices[batchList[i].faces[j + 2]];

			if (Collision::collide_SegmentvsTriangle(vec4f::zero, 10.f * ray, p1, p2, p3))
				return true;
		}
	}
	return false;
}
bool WidgetVirtual::mouseEvent(const mat4f& base, const vec4f& ray, const float& parentscale, const bool& clicked) { return false; }


void WidgetVirtual::setBoolean(const bool& b) {}
void WidgetVirtual::setString(const std::string& s) {}
std::string WidgetVirtual::getString() const { return ""; }
void WidgetVirtual::append(const std::string& s) {}
bool WidgetVirtual::getBoolean() const { return false; }
//


//  Set/get functions
void WidgetVirtual::setState(State state)
{
	configuration &= ~(uint8_t)State::STATE_MASK;
	configuration |= (uint8_t)state % (uint8_t)State::CURRENT;
}
void WidgetVirtual::setSize(const vec2f& s, const State& state)
{
	if (state == State::ALL)
	{
		sizes[State::DEFAULT] = s;
		sizes[State::HOVER] = s;
		sizes[State::ACTIVE] = s;
		sizes[State::CURRENT] = s;
	}
	else sizes[state] = s;
	if(configuration & (uint8_t)OrphanFlags::RESPONSIVE) configuration |= (uint8_t)OrphanFlags::NEED_UPDATE;
}
void WidgetVirtual::setPosition(const vec4f& p, const State& state)
{
	if (state == State::ALL)
	{
		positions[State::DEFAULT] = p;
		positions[State::HOVER] = p;
		positions[State::ACTIVE] = p;
		positions[State::CURRENT] = p;
	}
	else positions[state] = p;
}
void WidgetVirtual::setColor(const vec4f& c, const State& state)
{
	if (state == State::ALL)
	{
		colors[State::DEFAULT] = c;
		colors[State::HOVER] = c;
		colors[State::ACTIVE] = c;
		colors[State::CURRENT] = c;
	}
	else colors[state] = c;
}
void WidgetVirtual::setVisibility(const bool& visible)
{
	if (visible) configuration |= (uint8_t)OrphanFlags::VISIBLE;
	else configuration &= ~(uint8_t)OrphanFlags::VISIBLE;
}
void WidgetVirtual::setResponsive(const bool& responsive)
{
	if (responsive) configuration |= (uint8_t)OrphanFlags::RESPONSIVE;
	else configuration &= ~(uint8_t)OrphanFlags::RESPONSIVE;
}
void WidgetVirtual::setTexture(const std::string& textureName)
{
	ResourceManager::getInstance()->release(texture);
	if (!textureName.empty()) texture = ResourceManager::getInstance()->getResource<Texture>(textureName);
	else texture = nullptr;
}
void WidgetVirtual::setShader(const std::string& shaderName)
{
	ResourceManager::getInstance()->release(shader);
	shader = ResourceManager::getInstance()->getResource<Shader>(shaderName);
}
void WidgetVirtual::setConfiguration(const uint8_t& config)
{
	configuration = config;
}


WidgetVirtual::WidgetType WidgetVirtual::getType() const { return type; }
WidgetVirtual::State WidgetVirtual::getState() const { return (State)(configuration & (uint8_t)State::STATE_MASK); }
vec2f WidgetVirtual::getSize(State state)
{
	if (state == State::CURRENT) return sizes[(State)(configuration & (uint8_t)State::STATE_MASK)];
	else return sizes[(State)((int)state % (uint8_t)State::CURRENT)];
}
vec4f WidgetVirtual::getPosition(State state)
{
	if (state == State::CURRENT) return positions[(State)(configuration & (uint8_t)State::STATE_MASK)];
	else return positions[(State)((int)state % (uint8_t)State::CURRENT)];
}
vec4f WidgetVirtual::getColor(const unsigned int& index, State state)
{
	if (state == State::CURRENT) return colors[(State)(configuration & (uint8_t)State::STATE_MASK)];
	else return colors[(State)((int)state % (uint8_t)State::CURRENT)];
}
bool WidgetVirtual::isVisible() const { return (configuration & (uint8_t)OrphanFlags::VISIBLE)!=0; }
bool WidgetVirtual::isResponsive() const { return (configuration & (uint8_t)OrphanFlags::RESPONSIVE) != 0; }
Shader* WidgetVirtual::getShader() const { return shader; }
Texture* WidgetVirtual::getTexture() const { return texture; }
unsigned int WidgetVirtual::getNumberFaces() const
{
	unsigned int result = 0;
	for (unsigned int i = 0; i < batchList.size(); i++)
		result += (unsigned int)batchList[i].faces.size();
	return result;
}
std::vector<WidgetVirtual*>& WidgetVirtual::getChildrenList() { return children; }
//

//	Hierarchy modifiers
void WidgetVirtual::addChild(WidgetVirtual* w) { children.push_back(w); }
bool WidgetVirtual::removeChild(WidgetVirtual* w)
{
	std::vector<WidgetVirtual*>::iterator it = std::find(children.begin(), children.end(), w);
	if (it != children.end())
	{
		children.erase(it);
		return true;
	}
	else return false;
}
//

//	Protected functions
void WidgetVirtual::drawClippingShape(const unsigned int& batchIndex, const bool& enableClipping, Shader* s, uint8_t& stencilMask)
{
	const std::string errorHeader = "scisor";
	glDisable(GL_DEPTH_TEST); CheckGLError(errorHeader, "glDisable(GL_DEPTH_TEST)");
	if (enableClipping)
	{
		stencilMask++;
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); CheckGLError(errorHeader, "glStencilOp(GL_KEEP, GL_KEEP, GL_INCR)");
	}
	else
	{
		stencilMask--;
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR); CheckGLError(errorHeader, "glStencilOp(GL_KEEP, GL_KEEP, GL_DECR)");
	}
	glStencilFunc(GL_ALWAYS, stencilMask, 0xFF);

	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) 
	{
		glUniform1i(loc, 0);					CheckGLError(errorHeader, "glUniform1i(useTexture)");
	}
	loc = s->getUniformLocation("color");
	if (loc >= 0) 
	{
		glUniform4fv(loc, 1, &vec4f::zero[0]);	CheckGLError(errorHeader, "glUniform4fv(color)");
	}

	glBindVertexArray(batchList[batchIndex].vao);														CheckGLError(errorHeader, "glUniform4fv(color)");
	glDrawElements(GL_TRIANGLES, (int)batchList[batchIndex].faces.size(), GL_UNSIGNED_SHORT, NULL);		CheckGLError(errorHeader, "glDrawElements");

	glStencilFunc(GL_EQUAL, stencilMask, 0xFF);															CheckGLError(errorHeader, "glStencilFunc(GL_EQUAL, stencilMask, 0xFF)");
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);																CheckGLError(errorHeader, "glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP)");
	glEnable(GL_DEPTH_TEST);																			CheckGLError(errorHeader, "glEnable(GL_DEPTH_TEST)");
}
void WidgetVirtual::initializeVBO(const unsigned int& batchIndex, int VBOtype)
{
	const std::string errorHeader = "vbo init";
	uint32_t bytesize = batchList[batchIndex].vertices.size() * sizeof(vec4f);
	glGenBuffers(1, &batchList[batchIndex].verticesBuffer);												CheckGLError(errorHeader, "glGenBuffers(vertices)");
	glBindBuffer(GL_ARRAY_BUFFER, batchList[batchIndex].verticesBuffer);								CheckGLError(errorHeader, "glBindBuffer(vertices)");
	glBufferData(GL_ARRAY_BUFFER, bytesize, batchList[batchIndex].vertices.data(), VBOtype);			CheckGLError(errorHeader, "glBufferData(vertices)");

	bytesize = batchList[batchIndex].textures.size() * sizeof(vec2f);
	glGenBuffers(1, &batchList[batchIndex].texturesBuffer);												CheckGLError(errorHeader, "glGenBuffers(textures)");
	glBindBuffer(GL_ARRAY_BUFFER, batchList[batchIndex].texturesBuffer);								CheckGLError(errorHeader, "glBindBuffer(textures)");
	glBufferData(GL_ARRAY_BUFFER, bytesize, batchList[batchIndex].textures.data(), VBOtype);			CheckGLError(errorHeader, "glBufferData(textures)");

	bytesize = batchList[batchIndex].faces.size() * sizeof(unsigned short);
	glGenBuffers(1, &batchList[batchIndex].facesBuffer);												CheckGLError(errorHeader, "glGenBuffers(faces)");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[batchIndex].facesBuffer);							CheckGLError(errorHeader, "glBindBuffer(faces)");
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytesize, batchList[batchIndex].faces.data(), VBOtype);		CheckGLError(errorHeader, "glBufferData(faces)");
}
void WidgetVirtual::initializeVAOs()
{
	const std::string errorHeader = "vao init";
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glGenVertexArrays(1, &batchList[i].vao);							CheckGLError(errorHeader, "glGenVertexArrays(vao)");
		glBindVertexArray(batchList[i].vao);								CheckGLError(errorHeader, "glBindVertexArray(vao)");

		glEnableVertexAttribArray(0);										CheckGLError(errorHeader, "glEnableVertexAttribArray(0)");
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);			CheckGLError(errorHeader, "glBindBuffer(vertices)");
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);			CheckGLError(errorHeader, "glVertexAttribPointer(0)");

		glEnableVertexAttribArray(1);										CheckGLError(errorHeader, "glEnableVertexAttribArray(1)");
		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);			CheckGLError(errorHeader, "glBindBuffer(textures)");
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);			CheckGLError(errorHeader, "glVertexAttribPointer(1)");

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);	CheckGLError(errorHeader, "glBindBuffer(faces)");
		glBindVertexArray(0);												CheckGLError(errorHeader, "glBindVertexArray(0)");
	}
}
void WidgetVirtual::indentLine(std::ostream& out, const int& i) const
{
	for (int j = 0; j < i; j++)
		out << '\t';
}
//
