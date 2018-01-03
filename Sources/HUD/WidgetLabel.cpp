#include "WidgetLabel.h"


//	string define
#define TEXT_MAX_CHAR			200

//	drawing define
#define LINE_MARGIN_LEFT		0.5f
#define TEX_OFFSET				0.00586f
#define ITALIC_RATIO			0.5f

//	batch index define
#define BATCH_INDEX_TEXT		0
#define BATCH_INDEX_CLIPPING	1


//  Default
WidgetLabel::WidgetLabel(const uint8_t& config, const std::string& shaderName) : 
	WidgetVirtual(WidgetVirtual::LABEL, config | NEED_UPDATE, shaderName), textConfiguration(0x00), sizeChar(0.1f), updateCooldown(0.f)
{
	font = nullptr;
}
WidgetLabel::~WidgetLabel()
{
	ResourceManager::getInstance()->release(font);
}
//


//  Public functions
void WidgetLabel::initialize(const std::string& txt, uint8_t textConfig)
{
	//	init
	State s = (State)(configuration & STATE_MASK);
	colors[CURRENT] = colors[s];
	positions[CURRENT] = positions[s];
	sizes[CURRENT] = sizes[s];
	if (!font) return;
		
	text = txt;
	textConfiguration = textConfig;
	parseText();

	//	prepare batch (with text)
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_TEXT
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CLIPPING
	updateBuffers();

	//	end
	initializeVBO(BATCH_INDEX_CLIPPING, GL_DYNAMIC_DRAW);
	initVBOtext();
	initializeVAOs();
}
void WidgetLabel::update(const float& elapseTime)
{
	State s = (State)(configuration & STATE_MASK);
	colors[CURRENT] = colors[s];
	positions[CURRENT] = positions[s];
	sizes[CURRENT] = sizes[s];

	//	update buffers if needed
	updateCooldown += elapseTime;
	if (configuration & NEED_UPDATE && updateCooldown > 100.f)
	{
		configuration &= ~NEED_UPDATE;
		updateCooldown = 0.f;
		if (!font) return;

		parseText();
		updateBuffers();
		updateVBOs();
	}
}
void WidgetLabel::draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model)
{
	//	clipping zone (batch 1)
	if(textConfiguration & CLIPPING)
		drawClippingShape(BATCH_INDEX_CLIPPING, true, s, stencilMask);

	//	texture related stuff
	if (font) glBindTexture(GL_TEXTURE_2D, font->texture);
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (font ? 1 : 0));

	//	draw batch 0 (text)
	loc = s->getUniformLocation("color");
	if (loc >= 0) glUniform4fv(loc, 1, &colors[CURRENT].x);

	glBindVertexArray(batchList[BATCH_INDEX_TEXT].vao);
	glDrawElements(GL_TRIANGLES, batchList[BATCH_INDEX_TEXT].faces.size(), GL_UNSIGNED_SHORT, NULL);

	//	unclip zone (batch 1)
	if (textConfiguration & CLIPPING)
		drawClippingShape(BATCH_INDEX_CLIPPING, false, s, stencilMask);
}
bool WidgetLabel::intersect(const glm::mat4& base, const glm::vec3& ray)
{
	for (unsigned int j = 0; j < batchList[BATCH_INDEX_TEXT].faces.size(); j += 3)
	{
		//	compute triangles vertices in eyes space
		glm::vec3 p1 = glm::vec3(base * glm::vec4(batchList[BATCH_INDEX_TEXT].vertices[batchList[BATCH_INDEX_TEXT].faces[j]], 1.f));
		glm::vec3 p2 = glm::vec3(base * glm::vec4(batchList[BATCH_INDEX_TEXT].vertices[batchList[BATCH_INDEX_TEXT].faces[j + 1]], 1.f));
		glm::vec3 p3 = glm::vec3(base * glm::vec4(batchList[BATCH_INDEX_TEXT].vertices[batchList[BATCH_INDEX_TEXT].faces[j + 2]], 1.f));

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
	return false;
}


void WidgetLabel::setString(const std::string& newText)
{
	if (newText != text)
	{
		text = newText;
		configuration |= NEED_UPDATE;
	}
}
std::string WidgetLabel::getString() const { return text; }
void WidgetLabel::append(const std::string& s)
{
	if (!s.empty())
	{
		text += s;
		configuration |= NEED_UPDATE;
	}
}
//


//	Set / get functions
void WidgetLabel::setFont(const std::string& fontName)
{
	ResourceManager::getInstance()->release(font);
	font = ResourceManager::getInstance()->getFont(fontName);
	configuration |= NEED_UPDATE;
}
void WidgetLabel::setSizeChar(const float& f)
{
	sizeChar = f;
	configuration |= NEED_UPDATE;
}


Font* WidgetLabel::getFont() const { return font; }
float WidgetLabel::getSizeChar() const { return sizeChar; }
//


//	Protected functions
void WidgetLabel::initVBOtext()
{
	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, TEXT_MAX_CHAR * 4 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[BATCH_INDEX_TEXT].vertices.size() * sizeof(glm::vec3), batchList[BATCH_INDEX_TEXT].vertices.data());

	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].texturesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].texturesBuffer);
	glBufferData(GL_ARRAY_BUFFER, TEXT_MAX_CHAR * 4 * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[BATCH_INDEX_TEXT].textures.size() * sizeof(glm::vec2), batchList[BATCH_INDEX_TEXT].textures.data());

	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TEXT_MAX_CHAR * 6 * sizeof(unsigned short), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[BATCH_INDEX_TEXT].faces.size() * sizeof(unsigned short), batchList[BATCH_INDEX_TEXT].faces.data());
}
void WidgetLabel::updateBuffers()
{
	DrawBatch batch;
	float x = 0.f;
	unsigned int line = 0;
	float italic = ((textConfiguration & ITALIC) ? ITALIC_RATIO : 0.f);

	for (unsigned int i = 0; i < text.size(); i++)
	{
		//	init parameters
		glm::vec2 o = getLineOrigin(line, textConfiguration);
		Font::Patch patch = font->getPatch(text[i]);
		float charLength = std::abs((patch.corner2.x - patch.corner1.x) / (patch.corner2.y - patch.corner1.y));

		//	push patch into batch
		switch (text[i])
		{
			case '\n':
				line++;
				x = 0.f;
				break;

			case '\t':
				x = sizeChar * ((int)(x / sizeChar) + 1);
				x += sizeChar*charLength;
				break;

			default:
				batch.vertices.push_back(glm::vec3(o.x + x, 0.f, o.y));
				batch.vertices.push_back(glm::vec3(o.x + x + italic*sizeChar, 0.f, o.y + sizeChar));
				batch.vertices.push_back(glm::vec3(o.x + x + (charLength + italic)*sizeChar, 0.f, o.y + sizeChar));
				batch.vertices.push_back(glm::vec3(o.x + x + charLength*sizeChar, 0.f, o.y));

				batch.textures.push_back(glm::vec2(patch.corner1.x + TEX_OFFSET, patch.corner2.y - TEX_OFFSET));
				batch.textures.push_back(glm::vec2(patch.corner1.x + TEX_OFFSET, patch.corner1.y + TEX_OFFSET));
				batch.textures.push_back(glm::vec2(patch.corner2.x - TEX_OFFSET, patch.corner1.y + TEX_OFFSET));
				batch.textures.push_back(glm::vec2(patch.corner2.x - TEX_OFFSET, patch.corner2.y - TEX_OFFSET));


				batch.faces.push_back((unsigned short)(batch.vertices.size() - 4));
				batch.faces.push_back((unsigned short)(batch.vertices.size() - 3));
				batch.faces.push_back((unsigned short)(batch.vertices.size() - 2));

				batch.faces.push_back((unsigned short)(batch.vertices.size() - 4));
				batch.faces.push_back((unsigned short)(batch.vertices.size() - 2));
				batch.faces.push_back((unsigned short)(batch.vertices.size() - 1));

				x += sizeChar*charLength;
				break;
		}
	}
	batchList[BATCH_INDEX_TEXT].vertices.swap(batch.vertices);
	batchList[BATCH_INDEX_TEXT].textures.swap(batch.textures);
	batchList[BATCH_INDEX_TEXT].faces.swap(batch.faces);

	//	clipping rectangle
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

	batchList[BATCH_INDEX_CLIPPING].vertices.swap(quad.vertices);
	batchList[BATCH_INDEX_CLIPPING].textures.swap(quad.textures);
	batchList[BATCH_INDEX_CLIPPING].faces.swap(quad.faces);
}
void WidgetLabel::updateVBOs()
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].vertices.size() * sizeof(glm::vec3), batchList[i].vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].textures.size() * sizeof(glm::vec2), batchList[i].textures.data());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[i].faces.size() * sizeof(unsigned short), batchList[i].faces.data());
	}
}
void WidgetLabel::parseText()
{
	if (!font) return;

	linesLength.clear();
	float length = 0.f;
	for (unsigned int i = 0; i < text.size(); i++)
	{
		Font::Patch patch = font->getPatch(text[i]);
		float charLength = std::abs((patch.corner2.x - patch.corner1.x) / (patch.corner2.y - patch.corner1.y));

		switch (text[i])
		{
			case '\n':
				linesLength.push_back(length);
				length = 0.f;
				break;
			case '\t':
				length = sizeChar*((int)(length / sizeChar) + 1);
				break;
			default:
				length += sizeChar*charLength;
				break;
		}
	}
	if (!text.empty()) linesLength.push_back(length);
}
glm::vec2 WidgetLabel::getLineOrigin(const unsigned int& lineIndex, const uint8_t& textConfiguration)
{
	glm::vec2 origin(0.f, 0.f);
	glm::vec2 size(sizes[CURRENT]);
	switch (textConfiguration & (HORIZONTAL_MASK | VERTICAL_MASK))
	{
		case CENTER:
			origin.x -= 0.5f * linesLength[lineIndex];
			origin.y += sizeChar * (0.5f * linesLength.size() - lineIndex - 1);
			break;
		case (MIDDLE_H | TOP):
			origin.x -= 0.5f * linesLength[lineIndex];
			origin.y += 0.5f * size.y - sizeChar * (lineIndex + 1);
			break;
		case (MIDDLE_H | BOTTOM):
			origin.x -= 0.5f * linesLength[lineIndex];
			origin.y -= 0.5f * size.y - sizeChar * (linesLength.size() - 1 - lineIndex);
			break;

		case (LEFT | MIDDLE_V):
			origin.x -= 0.5f * size.x;
			origin.y += sizeChar* ( 0.5f * linesLength.size() - lineIndex - 1);
			break;
		case (LEFT | TOP):
			origin.x -= 0.5f * size.x;
			origin.y += 0.5f * size.y - sizeChar * (lineIndex + 1);
			break;
		case (LEFT | BOTTOM):
			origin.x -= 0.5f * size.x;
			origin.y -= 0.5f * size.y - sizeChar * (linesLength.size() - 1 - lineIndex);
			break;

		case (RIGHT | MIDDLE_V):
			origin.x += 0.5f * size.x - linesLength[lineIndex];
			origin.y += sizeChar * (0.5f * linesLength.size() - lineIndex - 1);
			break;
		case (RIGHT | TOP):
			origin.x += 0.5f * size.x - linesLength[lineIndex];
			origin.y += 0.5f * size.y - sizeChar * (lineIndex + 1);
			break;
		case (RIGHT | BOTTOM):
			origin.x += 0.5f * size.x - linesLength[lineIndex];
			origin.y -= 0.5f * size.y - sizeChar * (linesLength.size() - 1 - lineIndex);
			break;
		default: break;
	}
	return origin + glm::vec2(sizeChar * LINE_MARGIN_LEFT, 0.f);
}
//
