#include "WidgetVirtual.h"


//  Default
WidgetVirtual::WidgetVirtual(const WidgetType& t, const uint8_t& config, const std::string& shaderName) : type(t), configuration(config)
{
	sizes[DEFAULT] = glm::vec2(1.f);
	sizes[HOVER] = glm::vec2(1.f);
	sizes[ACTIVE] = glm::vec2(1.f);
	sizes[CURRENT] = glm::vec2(1.f);

	positions[DEFAULT] = glm::vec3(0.f);
	positions[HOVER] = glm::vec3(0.f);
	positions[ACTIVE] = glm::vec3(0.f);
	positions[CURRENT] = glm::vec3(0.f);

	colors[DEFAULT] = glm::vec4(1.f);
	colors[HOVER] = glm::vec4(1.f);
	colors[ACTIVE] = glm::vec4(1.f);
	colors[CURRENT] = glm::vec4(1.f);

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
void WidgetVirtual::draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model)
{
	//	texture related stuff
	if (texture) glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (texture ? 1 : 0));

	//	draw all batches
	State state = (State)(configuration & STATE_MASK);
	loc = s->getUniformLocation("color");
	if (loc >= 0) glUniform4fv(loc, 1, &colors[state].x);
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glBindVertexArray(batchList[i].vao);
		glDrawElements(GL_TRIANGLES, (int)batchList[i].faces.size(), GL_UNSIGNED_SHORT, NULL);
	}
}
void WidgetVirtual::update(const float& elapseTime)
{
	State s = (State)(configuration & STATE_MASK);
	colors[CURRENT] = colors[s];
	positions[CURRENT] = positions[s];
	sizes[CURRENT] = sizes[s];
}
bool WidgetVirtual::intersect(const glm::mat4& base, const glm::vec3& ray)
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		for (unsigned int j = 0; j < batchList[i].faces.size(); j +=3 )
		{
			//	compute triangles vertices in eyes space
			glm::vec3 p1 = glm::vec3(base * glm::vec4(batchList[i].vertices[batchList[i].faces[j]], 1.f));
			glm::vec3 p2 = glm::vec3(base * glm::vec4(batchList[i].vertices[batchList[i].faces[j + 1]], 1.f));
			glm::vec3 p3 = glm::vec3(base * glm::vec4(batchList[i].vertices[batchList[i].faces[j + 2]], 1.f));

			//	compute local base (triangle edge), and triangle normal
			glm::vec3 v1 = p2 - p1;
			glm::vec3 v2 = p3 - p1;
			glm::vec3 normal = glm::cross(v1, v2);
			if (normal == glm::vec3(0.f)) continue;
			glm::normalize(normal);

			//	compute intersection point
			if (glm::dot(normal, ray) == 0.f) continue;
			float depth = glm::dot(normal, p1) / glm::dot(normal, ray);
			glm::vec3 intersection = depth * ray - p1;

			//	check if point is inside triangle (checking barycentric coordinates)
			float magnitute = glm::dot(v2, v2)*glm::dot(v1, v1) - glm::dot(v1, v2)*glm::dot(v1, v2);
			glm::vec2 barry;
				barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - glm::dot(v2, v1) * glm::dot(intersection, v2)) / magnitute;
				barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - glm::dot(v2, v1) * glm::dot(intersection, v1)) / magnitute;
			if (barry.x < 0.f || barry.y < 0.f || barry.x + barry.y > 1.f) continue;
			else return true;
		}
	}
	return false;
}
bool WidgetVirtual::mouseEvent(const glm::mat4& base, const glm::vec3& ray, const float& parentscale, const bool& clicked) { return false; }


void WidgetVirtual::setBoolean(const bool& b) {}
void WidgetVirtual::setString(const std::string& s) {}
std::string WidgetVirtual::getString() const { return ""; }
void WidgetVirtual::append(const std::string& s) {}
bool WidgetVirtual::getBoolean() const { return false; }
//


//  Set/get functions
void WidgetVirtual::setState(State state)
{
	configuration &= ~STATE_MASK;
	configuration |= state % CURRENT;
}
void WidgetVirtual::setSize(const glm::vec2& s, const State& state)
{
	if (state == ALL)
	{
		sizes[DEFAULT] = s;
		sizes[HOVER] = s;
		sizes[ACTIVE] = s;
		sizes[CURRENT] = s;
	}
	else sizes[state] = s;
	if(configuration & RESPONSIVE) configuration |= NEED_UPDATE;
}
void WidgetVirtual::setPosition(const glm::vec3& p, const State& state)
{
	if (state == ALL)
	{
		positions[DEFAULT] = p;
		positions[HOVER] = p;
		positions[ACTIVE] = p;
		positions[CURRENT] = p;
	}
	else positions[state] = p;
}
void WidgetVirtual::setColor(const glm::vec4& c, const State& state) 
{
	if (state == ALL)
	{
		colors[DEFAULT] = c;
		colors[HOVER] = c;
		colors[ACTIVE] = c;
		colors[CURRENT] = c;
	}
	else colors[state] = c;
}
void WidgetVirtual::setVisibility(const bool& visible)
{
	if (visible) configuration |= VISIBLE;
	else configuration &= ~VISIBLE;
}
void WidgetVirtual::setResponsive(const bool& responsive)
{
	if (responsive) configuration |= RESPONSIVE;
	else configuration &= ~RESPONSIVE;
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
WidgetVirtual::State WidgetVirtual::getState() const { return (State)(configuration & STATE_MASK); }
glm::vec2 WidgetVirtual::getSize(State state)
{
	if (state == CURRENT) return sizes[(State)(configuration & STATE_MASK)];
	else return sizes[(State)((int)state % CURRENT)];
}
glm::vec3 WidgetVirtual::getPosition(State state)
{
	if (state == CURRENT) return positions[(State)(configuration & STATE_MASK)];
	else return positions[(State)((int)state % CURRENT)];
}
glm::vec4 WidgetVirtual::getColor(const unsigned int& index, State state)
{
	if (state == CURRENT) return colors[(State)(configuration & STATE_MASK)];
	else return colors[(State)((int)state % CURRENT)];
}
bool WidgetVirtual::isVisible() const { return (configuration & VISIBLE)!=0; }
bool WidgetVirtual::isResponsive() const { return (configuration & RESPONSIVE) != 0; }
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
	if (loc >= 0) glUniform4fv(loc, 1, &glm::vec4(0.f)[0]);

	glBindVertexArray(batchList[batchIndex].vao);
	glDrawElements(GL_TRIANGLES, (int)batchList[batchIndex].faces.size(), GL_UNSIGNED_SHORT, NULL);

	glStencilFunc(GL_EQUAL, stencilMask, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_DEPTH_TEST);
}
void WidgetVirtual::initializeVBO(const unsigned int& batchIndex, int VBOtype)
{
	glGenBuffers(1, &batchList[batchIndex].verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[batchIndex].verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, batchList[batchIndex].vertices.size() * sizeof(glm::vec3), batchList[batchIndex].vertices.data(), VBOtype);

	glGenBuffers(1, &batchList[batchIndex].texturesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[batchIndex].texturesBuffer);
	glBufferData(GL_ARRAY_BUFFER, batchList[batchIndex].textures.size() * sizeof(glm::vec2), batchList[batchIndex].textures.data(), VBOtype);

	glGenBuffers(1, &batchList[batchIndex].facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[batchIndex].facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, batchList[batchIndex].faces.size() * sizeof(unsigned short), batchList[batchIndex].faces.data(), VBOtype);
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
void WidgetVirtual::indentLine(std::ostream& out, const int& i) const
{
	for (int j = 0; j < i; j++)
		out << '\t';
}
//
