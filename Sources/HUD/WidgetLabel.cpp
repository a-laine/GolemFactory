#include "WidgetLabel.h"


#define TEX_OFFSET		0.00586f
#define ITALIC_RATIO	0.5f

//  Default
WidgetLabel::WidgetLabel(const std::string& fontName, const uint8_t& config, const std::string& shaderName) : 
	WidgetVirtual(WidgetVirtual::LABEL, config, shaderName), textConfiguration(0x00), sizeChar(0.1f), needUpdate(true), updateCooldown(0.f)
{
	font = nullptr;
}
WidgetLabel::~WidgetLabel()
{
	ResourceManager::getInstance()->release(font);
}
//

//	Public functions
void WidgetLabel::update(const float& elapseTime)
{
	//	update buffers if needed
	updateCooldown += elapseTime;
	if (needUpdate && updateCooldown > 100.f)
	{
		parseText();
		updateBuffers();
		updateVBOs();
		needUpdate = false;
		updateCooldown = 0.f;
	}
}
void WidgetLabel::initialize(const std::string& t, uint8_t textConfig)
{
	if (!font) return;

	//	init
	text = t;
	textConfiguration = textConfig;
	parseText();

	//	prepare batch (with text)
	DrawBatch batch;
	batch.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	batchList.push_back(batch);
	updateBuffers();

	//	clipping rectangle
	DrawBatch quad;
	quad.color = glm::vec4(0.f, 0.f, 0.f, 0.f);

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

	//	end
	initializeVBOs(GL_DYNAMIC_DRAW);
	initializeVAOs();
	needUpdate = false;
}
void WidgetLabel::draw(Shader* s, uint8_t& stencilMask)
{
	//	clipping zone (batch 1)
	if(textConfiguration & CLIPPING)
		drawClippingShape(1, true, s, stencilMask);

	//	texture related stuff
	if (font) glBindTexture(GL_TEXTURE_2D, font->texture);
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (font ? 1 : 0));

	//	draw batch 0 (text)
		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &batchList[0].color.x);

		glBindVertexArray(batchList[0].vao);
		glDrawElements(GL_TRIANGLES, batchList[0].faces.size(), GL_UNSIGNED_SHORT, NULL);

	//	unclip zone (batch 1)
	if (textConfiguration & CLIPPING)
		drawClippingShape(1, false, s, stencilMask);
}
bool WidgetLabel::intersect(const glm::mat4& base, const glm::vec3& ray, const glm::vec3 origin, glm::vec3& result)
{
	for (unsigned int j = 0; j < batchList[0].faces.size(); j += 3)
	{
		//	compute triangles vertices in eyes space
		glm::vec3 p1 = glm::vec3(base * glm::vec4(batchList[0].vertices[batchList[0].faces[j]], 1.f));
		glm::vec3 p2 = glm::vec3(base * glm::vec4(batchList[0].vertices[batchList[0].faces[j + 1]], 1.f));
		glm::vec3 p3 = glm::vec3(base * glm::vec4(batchList[0].vertices[batchList[0].faces[j + 2]], 1.f));

		//	compute local base (triangle edge), and triangle normal
		glm::vec3 v1 = p2 - p1;
		glm::vec3 v2 = p3 - p1;
		glm::vec3 normal = glm::cross(v1, v2);
		if (normal == glm::vec3(0.f)) continue;
		glm::normalize(normal);

		//	compute intersection point
		if (glm::dot(normal, ray) == 0.f) return false;
		float depth = glm::dot(normal, p1) / glm::dot(normal, ray);
		glm::vec3 intersection = depth * ray - p1;

		//	check if point is inside triangle (checking barycentric coordinates)
		float magnitute = glm::dot(v2, v2)*glm::dot(v1, v1) - glm::dot(v1, v2)*glm::dot(v1, v2);
		glm::vec2 barry;
		barry.x = (glm::dot(v2, v2) * glm::dot(intersection, v1) - glm::dot(v2, v1) * glm::dot(intersection, v2)) / magnitute;
		barry.y = (glm::dot(v1, v1) * glm::dot(intersection, v2) - glm::dot(v2, v1) * glm::dot(intersection, v1)) / magnitute;
		if (barry.x < -PICKING_MARGIN || barry.y < -PICKING_MARGIN || barry.x + barry.y > 1.f + PICKING_MARGIN) continue;

		//	ray actually intersect this triangle
		result = intersection;
		return true;
	}
	return false;
}
//

//	Set / get functions
void WidgetLabel::setString(const std::string& newText)
{
	text = newText;
	needUpdate = true;
}
void WidgetLabel::setFont(const std::string& fontName)
{
	ResourceManager::getInstance()->release(font);
	font = ResourceManager::getInstance()->getFont(fontName);
	needUpdate = true;
}
void WidgetLabel::setSizeChar(const float& f)
{
	sizeChar = f;
	needUpdate = true;
}

std::string WidgetLabel::getString() const { return text; }
Font* WidgetLabel::getFont() const { return font; }
float WidgetLabel::getSizeChar() const { return sizeChar; }
//

//	Protected functions
void WidgetLabel::updateBuffers()
{
	DrawBatch batch;
	parseText();
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
				batch.vertices.push_back(glm::vec3(o.x + x, 0.f, o.y + 0.f));
				batch.vertices.push_back(glm::vec3(o.x + x + italic*sizeChar, 0.f, o.y + sizeChar));
				batch.vertices.push_back(glm::vec3(o.x + x + (charLength + italic)*sizeChar, 0.f, o.y + sizeChar));
				batch.vertices.push_back(glm::vec3(o.x + x + charLength*sizeChar, 0.f, o.y + 0.f));

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
	batchList[0].vertices = batch.vertices;
	batchList[0].textures = batch.textures;
	batchList[0].faces = batch.faces;
}
void WidgetLabel::updateVBOs()
{
	if (batchList.empty()) return;
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, batchList[0].verticesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[0].vertices.size() * sizeof(glm::vec3), batchList[0].vertices.data());

	glBindBuffer(GL_ARRAY_BUFFER, batchList[0].texturesBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[0].textures.size() * sizeof(glm::vec2), batchList[0].textures.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[0].facesBuffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[0].faces.size() * sizeof(unsigned short), batchList[0].faces.data());
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
glm::vec2 WidgetLabel::getLineOrigin(const unsigned int& lineIndex, const uint8_t& textConfiguration) const
{
	glm::vec2 origin(position);
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
	return origin;
}
//