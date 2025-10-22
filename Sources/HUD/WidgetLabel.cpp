#include "WidgetLabel.h"
#include "WidgetGLDebugger.h"

#include <Physics/Collision.h>

//	string define
#define TEXT_MAX_CHAR			200

//	drawing define
#define SIDE_LINE_MARGIN		0.5f
#define TEX_OFFSET				0.00586f
#define ITALIC_RATIO			0.5f

//	batch index define
#define BATCH_INDEX_TEXT		0
#define BATCH_INDEX_CLIPPING	1


//  Default
WidgetLabel::WidgetLabel(const uint8_t& config, const std::string& shaderName) : 
	WidgetVirtual(WidgetVirtual::WidgetType::LABEL, config | (uint8_t)OrphanFlags::NEED_UPDATE, shaderName), 
	textConfiguration(0x00), sizeChar(0.1f), updateCooldown(0.f)
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
	State s = (State)(configuration & (uint8_t)State::STATE_MASK);
	colors[State::CURRENT] = colors[s];
	positions[State::CURRENT] = positions[s];
	sizes[State::CURRENT] = sizes[s];
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
	State s = (State)(configuration & (uint8_t)State::STATE_MASK);
	colors[State::CURRENT] = colors[s];
	positions[State::CURRENT] = positions[s];
	sizes[State::CURRENT] = sizes[s];

	//	update buffers if needed
	updateCooldown += elapseTime;
	if (configuration & (uint8_t)OrphanFlags::NEED_UPDATE && updateCooldown > 100.f)
	{
		configuration &= ~(uint8_t)OrphanFlags::NEED_UPDATE;
		updateCooldown = 0.f;
		if (!font) return;

		parseText();
		updateBuffers();
		updateVBOs();
	}
}
void WidgetLabel::draw(Shader* s, uint8_t& stencilMask, const mat4f& model)
{
	//	clipping zone (batch 1)
	if(textConfiguration & CLIPPING)
		drawClippingShape(BATCH_INDEX_CLIPPING, true, s, stencilMask);

	//	texture related stuff
	if (font) 
	{
		glBindTexture(GL_TEXTURE_2D, font->texture);	CheckGLError("draw", "glBindTexture(texid)");
	}
	else 
	{
		glBindTexture(GL_TEXTURE_2D, 0);				CheckGLError("draw", "glBindTexture(0)");
	}
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) 
	{
		glUniform1i(loc, (font ? 1 : 0));				CheckGLError("draw", "glUniform1i(useTexture)");
	}

	//	draw batch 0 (text)
	loc = s->getUniformLocation("color");
	if (loc >= 0) 
	{
		glUniform4fv(loc, 1, &colors[State::CURRENT].x);CheckGLError("draw", "glUniform4fv(color)");
	}

	glBindVertexArray(batchList[BATCH_INDEX_TEXT].vao);		CheckGLError("draw", "glBindVertexArray(vao)");
	glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_TEXT].faces.size(), GL_UNSIGNED_SHORT, NULL);
	CheckGLError("draw", "glDrawElements(BATCH_INDEX_TEXT)");

	//	unclip zone (batch 1)
	if (textConfiguration & CLIPPING)
		drawClippingShape(BATCH_INDEX_CLIPPING, false, s, stencilMask);
}
bool WidgetLabel::intersect(const mat4f& base, const vec4f& ray)
{
	if (textConfiguration & CLIPPING)
	{
		for (unsigned int j = 0; j < batchList[BATCH_INDEX_CLIPPING].faces.size(); j += 3)
		{
			//	compute triangles vertices in eyes space
			vec4f p1 = base * batchList[BATCH_INDEX_CLIPPING].vertices[batchList[BATCH_INDEX_CLIPPING].faces[j]];
			vec4f p2 = base * batchList[BATCH_INDEX_CLIPPING].vertices[batchList[BATCH_INDEX_CLIPPING].faces[j + 1]];
			vec4f p3 = base * batchList[BATCH_INDEX_CLIPPING].vertices[batchList[BATCH_INDEX_CLIPPING].faces[j + 2]];

			if (Collision::collide_SegmentvsTriangle(vec4f::zero, 10.f * ray, p1, p2, p3))
				return true;
		}
		return false;
	}
	else
	{
		for (unsigned int j = 0; j < batchList[BATCH_INDEX_TEXT].faces.size(); j += 3)
		{
			//	compute triangles vertices in eyes space
			vec4f p1 = base * batchList[BATCH_INDEX_TEXT].vertices[batchList[BATCH_INDEX_TEXT].faces[j]];
			vec4f p2 = base * batchList[BATCH_INDEX_TEXT].vertices[batchList[BATCH_INDEX_TEXT].faces[j + 1]];
			vec4f p3 = base * batchList[BATCH_INDEX_TEXT].vertices[batchList[BATCH_INDEX_TEXT].faces[j + 2]];

			if (Collision::collide_SegmentvsTriangle(vec4f::zero, 10.f * ray, p1, p2, p3))
				return true;
		}
		return false;
	}

}


void WidgetLabel::setString(const std::string& newText)
{
	if (newText != text)
	{
		text = newText;
		configuration |= (uint8_t)OrphanFlags::NEED_UPDATE;
	}
}
std::string WidgetLabel::getString() const { return text; }
void WidgetLabel::append(const std::string& s)
{
	if (!s.empty())
	{
		text += s;
		configuration |= (uint8_t)OrphanFlags::NEED_UPDATE;
	}
}
//


//	Set / get functions
void WidgetLabel::setFont(const std::string& fontName)
{
	ResourceManager::getInstance()->release(font);
	font = ResourceManager::getInstance()->getResource<Font>(fontName);
	configuration |= (uint8_t)OrphanFlags::NEED_UPDATE;
}
void WidgetLabel::setSizeChar(const float& f)
{
	sizeChar = f;
	configuration |= (uint8_t)OrphanFlags::NEED_UPDATE;
}


Font* WidgetLabel::getFont() const { return font; }
float WidgetLabel::getSizeChar() const { return sizeChar; }
//


//	Protected functions
void WidgetLabel::initVBOtext()
{
	uint32_t maxbytesize = TEXT_MAX_CHAR * 4 * sizeof(vec4f);
	uint32_t bytesize = batchList[BATCH_INDEX_TEXT].vertices.size() * sizeof(vec4f);
	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].verticesBuffer);										CheckGLError("vbo vertex", "glGenBuffers()");
	glBindBuffer(GL_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].verticesBuffer);							CheckGLError("vbo vertex", "glBindBuffer()");
	glBufferData(GL_ARRAY_BUFFER, maxbytesize, nullptr, GL_DYNAMIC_DRAW);								CheckGLError("vbo vertex", "glBufferData()");
	glBufferSubData(GL_ARRAY_BUFFER, 0, bytesize, batchList[BATCH_INDEX_TEXT].vertices.data());			CheckGLError("vbo vertex", "glBufferSubData()");

	maxbytesize = TEXT_MAX_CHAR * 4 * sizeof(vec2f);
	bytesize = batchList[BATCH_INDEX_TEXT].textures.size() * sizeof(vec2f);
	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].texturesBuffer);										CheckGLError("vbo texture", "glGenBuffers()");
	glBindBuffer(GL_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].texturesBuffer);							CheckGLError("vbo texture", "glBindBuffer()");
	glBufferData(GL_ARRAY_BUFFER, maxbytesize, nullptr, GL_DYNAMIC_DRAW);								CheckGLError("vbo texture", "glBufferData()");
	glBufferSubData(GL_ARRAY_BUFFER, 0, bytesize, batchList[BATCH_INDEX_TEXT].textures.data());			CheckGLError("vbo texture", "glBufferSubData()");

	maxbytesize = TEXT_MAX_CHAR * 6 * sizeof(unsigned short);
	bytesize = batchList[BATCH_INDEX_TEXT].faces.size() * sizeof(unsigned short);
	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].facesBuffer);											CheckGLError("vbo indices", "glGenBuffers()");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].facesBuffer);						CheckGLError("vbo indices", "glBindBuffer()");
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxbytesize, nullptr, GL_DYNAMIC_DRAW);						CheckGLError("vbo indices", "glBufferData()");
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bytesize, batchList[BATCH_INDEX_TEXT].faces.data());	CheckGLError("vbo indices", "glBufferSubData()");
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
		vec2f o = getLineOrigin(line, textConfiguration);
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
				batch.vertices.push_back(vec4f(o.x + x, 0.f, o.y, 1.f));
				batch.vertices.push_back(vec4f(o.x + x + italic*sizeChar, 0.f, o.y + sizeChar, 1.f));
				batch.vertices.push_back(vec4f(o.x + x + (charLength + italic)*sizeChar, 0.f, o.y + sizeChar, 1.f));
				batch.vertices.push_back(vec4f(o.x + x + charLength*sizeChar, 0.f, o.y, 1.f));

				batch.textures.push_back(vec2f(patch.corner1.x + TEX_OFFSET, patch.corner2.y - TEX_OFFSET));
				batch.textures.push_back(vec2f(patch.corner1.x + TEX_OFFSET, patch.corner1.y + TEX_OFFSET));
				batch.textures.push_back(vec2f(patch.corner2.x - TEX_OFFSET, patch.corner1.y + TEX_OFFSET));
				batch.textures.push_back(vec2f(patch.corner2.x - TEX_OFFSET, patch.corner2.y - TEX_OFFSET));


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
		quad.vertices.push_back(vec4f(-0.5f * sizes[State::CURRENT].x, 0.f, -0.5f * sizes[State::CURRENT].y, 1.f));
		quad.vertices.push_back(vec4f(-0.5f * sizes[State::CURRENT].x, 0.f,  0.5f * sizes[State::CURRENT].y, 1.f));
		quad.vertices.push_back(vec4f( 0.5f * sizes[State::CURRENT].x, 0.f,  0.5f * sizes[State::CURRENT].y, 1.f));
		quad.vertices.push_back(vec4f( 0.5f * sizes[State::CURRENT].x, 0.f, -0.5f * sizes[State::CURRENT].y, 1.f));

		quad.textures.push_back(vec2f(0.f, 1.f));
		quad.textures.push_back(vec2f(0.f, 0.f));
		quad.textures.push_back(vec2f(1.f, 0.f));
		quad.textures.push_back(vec2f(1.f, 1.f));

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
		glBindVertexArray(0);																										CheckGLError("vbo update", "glBindVertexArray(0)");

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);																	CheckGLError("vbo update", "glBindBuffer(0)");
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].vertices.size() * sizeof(vec4f), batchList[i].vertices.data());			CheckGLError("vbo update", "glBufferSubData(0)");

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);																	CheckGLError("vbo update", "glBindBuffer(0)");
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].textures.size() * sizeof(vec2f), batchList[i].textures.data());			CheckGLError("vbo update", "glBufferSubData(0)");

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);															CheckGLError("vbo update", "glBindBuffer(0)");
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[i].faces.size() * sizeof(unsigned short), batchList[i].faces.data());	CheckGLError("vbo update", "glBufferSubData(0)");
	}
}
void WidgetLabel::parseText()
{
	if (!font)
		return;

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
	if (!text.empty())
		linesLength.push_back(length);
}
vec2f WidgetLabel::getLineOrigin(const unsigned int& lineIndex, const uint8_t& textConfiguration)
{
	vec2f origin = vec2f::zero;
	vec2f size = sizes[State::CURRENT];
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

	vec2f margin;
	switch (textConfiguration & HORIZONTAL_MASK)
	{
		case LEFT:
			margin = vec2f(sizeChar * SIDE_LINE_MARGIN, 0.f);
			break;
		case RIGHT:
			margin = -vec2f(sizeChar * SIDE_LINE_MARGIN, 0.f);;
			break;
		default:
			margin = vec2f(0.f);
			break;
	}

	return origin + margin;
}
//
