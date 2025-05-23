#include "WidgetRadioButton.h"
//#include <Physics/SpecificCollision/CollisionUtils.h>
#include <Physics/Collision.h>

//	string define
#define TEXT_MAX_CHAR			40

//	drawing define
#define LINE_MARGIN_LEFT		0.5f
#define TEX_OFFSET				0.00586f
#define ITALIC_RATIO			0.5f

//	batch index define
#define BATCH_INDEX_TEXT		0
#define BATCH_INDEX_CLIPPING	1
#define BATCH_INDEX_CHECKBOX	2


//  Default
WidgetRadioButton::WidgetRadioButton(const uint8_t& config, const std::string& shaderName) : 
	WidgetLabel(config, shaderName), checked(false), onTexture(nullptr), offTexture(nullptr), lastEventState(false)
{
	type = WidgetVirtual::WidgetType::RADIO_BUTTON;
}
WidgetRadioButton::~WidgetRadioButton()
{
	ResourceManager::getInstance()->release(onTexture);
	ResourceManager::getInstance()->release(offTexture);
}
//

//	Public functions
void WidgetRadioButton::update(const float& elapseTime)
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
void WidgetRadioButton::initialize(const std::string& txt, uint8_t textConfig)
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
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CHECKBOX
	updateBuffers();

	//	end
	initializeVBO(BATCH_INDEX_CLIPPING, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_CHECKBOX, GL_DYNAMIC_DRAW);
	initVBOtext();
	initializeVAOs();
}
void WidgetRadioButton::draw(Shader* s, uint8_t& stencilMask, const mat4f& model)
{
	//	clipping zone (batch 1)
	if (textConfiguration & CLIPPING)
		drawClippingShape(BATCH_INDEX_CLIPPING, true, s, stencilMask);

	//	texture related stuff
	if (font) glBindTexture(GL_TEXTURE_2D, font->texture);
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (font ? 1 : 0));

	//	draw batch 0 (text)
	loc = s->getUniformLocation("color");
	if (loc >= 0) glUniform4fv(loc, 1, &colors[State::CURRENT].x);

	glBindVertexArray(batchList[BATCH_INDEX_TEXT].vao);
	glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_TEXT].faces.size(), GL_UNSIGNED_SHORT, NULL);//BATCH_INDEX_TEXT

	//	unclip zone (batch 1)
	if (textConfiguration & CLIPPING)
		drawClippingShape(BATCH_INDEX_CLIPPING, false, s, stencilMask);

	//	texture related stuff
	if (checked && onTexture)
	{
		glBindTexture(GL_TEXTURE_2D, onTexture->getTextureId());
		loc = s->getUniformLocation("useTexture");
		if (loc >= 0) glUniform1i(loc, 1);
	}
	else if (!checked && offTexture)
	{
		glBindTexture(GL_TEXTURE_2D, offTexture->getTextureId());
		loc = s->getUniformLocation("useTexture");
		if (loc >= 0) glUniform1i(loc, 1);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		loc = s->getUniformLocation("useTexture");
		if (loc >= 0) glUniform1i(loc, 0);
	}

	//	draw checked or not sprite
	vec4f white = vec4f::one;
	loc = s->getUniformLocation("color");
	if (loc >= 0) glUniform4fv(loc, 1, &white.x);
	glBindVertexArray(batchList[BATCH_INDEX_CHECKBOX].vao);
	glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_CHECKBOX].faces.size(), GL_UNSIGNED_SHORT, NULL);
}
bool WidgetRadioButton::intersect(const mat4f& base, const vec4f& ray)
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
	for (unsigned int j = 0; j < batchList[BATCH_INDEX_CHECKBOX].faces.size(); j += 3)
	{
		//	compute triangles vertices in eyes space
		vec4f p1 = base * batchList[BATCH_INDEX_CHECKBOX].vertices[batchList[BATCH_INDEX_CHECKBOX].faces[j]];
		vec4f p2 = base * batchList[BATCH_INDEX_CHECKBOX].vertices[batchList[BATCH_INDEX_CHECKBOX].faces[j + 1]];
		vec4f p3 = base * batchList[BATCH_INDEX_CHECKBOX].vertices[batchList[BATCH_INDEX_CHECKBOX].faces[j + 2]];

		if (Collision::collide_SegmentvsTriangle(vec4f::zero, 10.f * ray, p1, p2, p3))
			return true;
	}
	return false;
}
bool WidgetRadioButton::mouseEvent(const mat4f& base, const vec4f& ray, const float& parentscale, const bool& clicked)
{
	if (clicked && !lastEventState)
	{
		checked = !checked;
		lastEventState = clicked;
		return true;
	}
	lastEventState = clicked;
	return intersect(base, ray);
}
//


//	Set / get functions
void WidgetRadioButton::setTextureOn(const std::string& name)
{
	ResourceManager::getInstance()->release(onTexture);
	onTexture = ResourceManager::getInstance()->getResource<Texture>(name, (uint16_t)Texture::TextureConfiguration::TEXTURE_2D | (uint16_t)Texture::TextureConfiguration::USE_MIPMAP);
}
void WidgetRadioButton::setTextureOff(const std::string& name)
{
	ResourceManager::getInstance()->release(offTexture);
	offTexture = ResourceManager::getInstance()->getResource<Texture>(name, (uint16_t)Texture::TextureConfiguration::TEXTURE_2D | (uint16_t)Texture::TextureConfiguration::USE_MIPMAP);
}

void WidgetRadioButton::setBoolean(const bool& b) { checked = b; }
bool WidgetRadioButton::getBoolean() const { return checked; }
//

//	Protected functions
void WidgetRadioButton::updateBuffers()
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
		quad.vertices.push_back(vec4f( 0.5f * sizes[State::CURRENT].x - sizes[State::CURRENT].y, 0.f,  0.5f * sizes[State::CURRENT].y, 1.f));
		quad.vertices.push_back(vec4f( 0.5f * sizes[State::CURRENT].x - sizes[State::CURRENT].y, 0.f, -0.5f * sizes[State::CURRENT].y, 1.f));

		quad.textures.push_back(vec2f(0.f, 1.f));
		quad.textures.push_back(vec2f(0.f, 0.f));
		quad.textures.push_back(vec2f(1.f, 0.f));
		quad.textures.push_back(vec2f(1.f, 1.f));

		quad.faces.push_back(0); quad.faces.push_back(1); quad.faces.push_back(2);
		quad.faces.push_back(0); quad.faces.push_back(2); quad.faces.push_back(3);

	batchList[BATCH_INDEX_CLIPPING].vertices.swap(quad.vertices);
	batchList[BATCH_INDEX_CLIPPING].textures.swap(quad.textures);
	batchList[BATCH_INDEX_CLIPPING].faces.swap(quad.faces);

	//	clipping rectangle
	DrawBatch checkbox;
		checkbox.vertices.push_back(vec4f(0.5f * sizes[State::CURRENT].x - sizes[State::CURRENT].y, 0.f, -0.5f * sizes[State::CURRENT].y, 1.f));
		checkbox.vertices.push_back(vec4f(0.5f * sizes[State::CURRENT].x - sizes[State::CURRENT].y, 0.f,  0.5f * sizes[State::CURRENT].y, 1.f));
		checkbox.vertices.push_back(vec4f(0.5f * sizes[State::CURRENT].x, 0.f,  0.5f * sizes[State::CURRENT].y, 1.f));
		checkbox.vertices.push_back(vec4f(0.5f * sizes[State::CURRENT].x, 0.f, -0.5f * sizes[State::CURRENT].y, 1.f));

		checkbox.textures.push_back(vec2f(0.f, 1.f));
		checkbox.textures.push_back(vec2f(0.f, 0.f));
		checkbox.textures.push_back(vec2f(1.f, 0.f));
		checkbox.textures.push_back(vec2f(1.f, 1.f));

		checkbox.faces.push_back(0); checkbox.faces.push_back(1); checkbox.faces.push_back(2);
		checkbox.faces.push_back(0); checkbox.faces.push_back(2); checkbox.faces.push_back(3);

	batchList[BATCH_INDEX_CHECKBOX].vertices.swap(checkbox.vertices);
	batchList[BATCH_INDEX_CHECKBOX].textures.swap(checkbox.textures);
	batchList[BATCH_INDEX_CHECKBOX].faces.swap(checkbox.faces);
}
//