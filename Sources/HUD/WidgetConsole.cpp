#include "WidgetConsole.h"
//#include <Physics/SpecificCollision/CollisionUtils.h>
#include <Physics/Collision.h>


//	string define
#define MAX_LINE				40			//	max nb of line in text string
#define TEXT_MAX_CHAR			1000		//	max char in string to reserve vbo buffer size

//	drawing define
#define ELEVATOR_MARGIN_FACTOR	0.5f		//	factor relative to border length (%)
#define TEXTURE_OFFSET			0.00586f	//	offset for texture overlap
#define TEXT_DEPTH_OFFSET		-0.01f		//	offset position for texture batch and elevator
#define ELEVATOR_LENGTH_FACTOR  3.f
#define TAB_SIZE				9.f

//	batch index define
#define BATCH_INDEX_BORDER				0
#define BATCH_INDEX_CENTER				1
#define BATCH_INDEX_CLIPPING			2
#define BATCH_INDEX_TEXT				3
#define BATCH_INDEX_CLIPPING_ELEVATOR	4
#define BATCH_INDEX_ELEVATOR			5


//  Default
WidgetConsole::WidgetConsole(const uint8_t& config, const std::string& shaderName) : 
	WidgetBoard(config | (uint8_t)OrphanFlags::NEED_UPDATE | (uint8_t)ConsoleFlags::NEED_TEXT_UPDATE, shaderName),
	sizeChar(0.1f), margin(0.07f), firstCursory(0.f), elevator(0.f), elevatorLength(0.f), elevatorRange(0.f)
{
	type = WidgetVirtual::WidgetType::CONSOLE;
	font = nullptr;
}
WidgetConsole::~WidgetConsole()
{
	ResourceManager::getInstance()->release(font);
}
//


//	Public functions
void WidgetConsole::update(const float& elapseTime)
{
	State s = (State)(configuration & (uint8_t)State::STATE_MASK);
	colors[State::CURRENT] = 0.9f * colors[State::CURRENT] + 0.1f * colors[s];
	positions[State::CURRENT] = positions[s];
	sizes[State::CURRENT] = sizes[s];

	//	update buffers if needed
	updateCooldown += elapseTime;
	if (updateCooldown > 500.f)
	{
		updateCooldown = 0.f;
		if (configuration & (uint8_t)OrphanFlags::NEED_UPDATE)
		{
			configuration &= ~(uint8_t)OrphanFlags::NEED_UPDATE;
			updateBuffers();
		}
		if (configuration & (uint8_t)ConsoleFlags::NEED_TEXT_UPDATE)
		{
			configuration &= ~(uint8_t)ConsoleFlags::NEED_TEXT_UPDATE;
			parseText();
			updateTextBuffer();
		}
		updateVBOs();
	}
}
void WidgetConsole::initialize(const float& bThickness, const float& bWidth, const uint8_t& corner)
{
	colors[State::CURRENT] = colors[(State)(configuration & (uint8_t)State::STATE_MASK)];
	positions[State::CURRENT] = positions[(State)(configuration & (uint8_t)State::STATE_MASK)];
	sizes[State::CURRENT] = sizes[(State)(configuration & (uint8_t)State::STATE_MASK)];

	borderThickness = bThickness;
	borderWidth = bWidth;
	cornerConfiguration = corner;

	batchList.push_back(DrawBatch());	//	BATCH_INDEX_BORDER
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CENTER
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CLIPPING
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_TEXT
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CLIPPING_ELEVATOR
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_TEXT_ELEVATOR
	updateBuffers(true);

	initializeVBO(BATCH_INDEX_BORDER, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_CENTER, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_CLIPPING, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_CLIPPING_ELEVATOR, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_ELEVATOR, GL_DYNAMIC_DRAW);
	initVBOtext();
	initializeVAOs();
}
void WidgetConsole::draw(Shader* s, uint8_t& stencilMask, const mat4f& model)
{
	//	draw board part
	if (texture) glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (texture ? 1 : 0));

		//	draw border
		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &colors[State::CURRENT].x);
		glBindVertexArray(batchList[BATCH_INDEX_BORDER].vao);
		glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_BORDER].faces.size(), GL_UNSIGNED_SHORT, NULL);

		//	draw center at different alpha
		vec4f color = colors[State::CURRENT]; color.w *= 0.5f;

		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &color.x);
		glBindVertexArray(batchList[BATCH_INDEX_CENTER].vao);
		glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_CENTER].faces.size(), GL_UNSIGNED_SHORT, NULL);

	//	elevator
	float selection = sizeChar * linesLength.size() - (sizes[State::CURRENT].y - 2.f * borderThickness);
	if (false || selection > 0.f)
	{
		drawClippingShape(BATCH_INDEX_CLIPPING_ELEVATOR, true, s, stencilMask);

			//	draw elevator
			vec4f color = 0.7f * colors[State::CURRENT]; 
			color.w = colors[State::CURRENT].w;

			loc = s->getUniformLocation("color");
			if (loc >= 0) glUniform4fv(loc, 1, &color.x);
			float selectionPart = (elevator - 0.5f) * elevatorRange;
			loc = s->getUniformLocation("model");
			if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &mat4f::translate(model, positions[State::CURRENT] + vec4f(0.f, 0.f, selectionPart, 0.f))[0][0]);

			glBindVertexArray(batchList[BATCH_INDEX_ELEVATOR].vao);
			glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_ELEVATOR].faces.size(), GL_UNSIGNED_SHORT, NULL);

			//	reset model matrix on shader
			loc = s->getUniformLocation("model");
			if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &mat4f::translate(model, positions[State::CURRENT])[0][0]);

		//	unclip
		drawClippingShape(BATCH_INDEX_CLIPPING_ELEVATOR, false, s, stencilMask);
	}



	//	draw text
	drawClippingShape(BATCH_INDEX_CLIPPING, true, s, stencilMask);

		//	texture related stuff
		if (font) glBindTexture(GL_TEXTURE_2D, font->texture);
		else glBindTexture(GL_TEXTURE_2D, 0);
		loc = s->getUniformLocation("useTexture");
		if (loc >= 0) glUniform1i(loc, (font ? 1 : 0));

		//	go to elevator selection part
		loc = s->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &mat4f::translate(model, positions[State::CURRENT] + vec4f(0.f, 0.f, -elevator * selection, 0.f))[0][0]);

		//	draw text
		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &vec4f::one[0]);

		glBindVertexArray(batchList[BATCH_INDEX_TEXT].vao);
		glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_TEXT].faces.size(), GL_UNSIGNED_SHORT, NULL);

		//	reset model matrix on shader
		loc = s->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, &mat4f::translate(model, positions[State::CURRENT])[0][0]);

	//	unclip zone
	drawClippingShape(BATCH_INDEX_CLIPPING, false, s, stencilMask);
}
bool WidgetConsole::intersect(const mat4f& base, const vec4f& ray)
{
	//	compute intersect just with border and center
	for (unsigned int i = 0; i < BATCH_INDEX_CLIPPING; i++)
	{
		for (unsigned int j = 0; j < batchList[i].faces.size(); j += 3)
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
bool WidgetConsole::mouseEvent(const mat4f& base, const vec4f& ray, const float& parentscale, const bool& clicked)
{
	if (clicked)
	{
		//	compute plane attribute
		vec4f p1 = base * batchList[BATCH_INDEX_CENTER].vertices[batchList[BATCH_INDEX_CENTER].faces[0]];	 //	p1 is actualy the origin of board Shape (see center construction, vertex 0)
		vec4f p2 = base * batchList[BATCH_INDEX_CENTER].vertices[batchList[BATCH_INDEX_CENTER].faces[0 + 1]];
		vec4f p3 = base * batchList[BATCH_INDEX_CENTER].vertices[batchList[BATCH_INDEX_CENTER].faces[0 + 2]];
		vec4f normal = vec4f::cross(p2 - p1, p3 - p1);
		if (normal != vec4f::zero)
			normal.normalize();

		//	compute relative cursor y (0 = at bottom, 1 = at top)
		float depth = vec4f::dot(normal, p1) / vec4f::dot(normal, ray);
		vec4f intersection = depth * ray - p1;
		float cursory = (intersection.y / parentscale + sizes[State::CURRENT].y/2 - elevatorLength) / elevatorRange;

		if ((configuration & (uint8_t)State::STATE_MASK) != (uint8_t)State::ACTIVE)
			firstCursory = cursory - elevator;

		elevator = clamp(cursory - firstCursory, 0.f, 1.f);
		setState(State::ACTIVE);
		return true;
	}
	else return false;
}
//


//	Set / get functions
void WidgetConsole::setFont(const std::string& fontName)
{
	ResourceManager::getInstance()->release(font);
	font = ResourceManager::getInstance()->getResource<Font>(fontName);
	configuration |= (uint8_t)ConsoleFlags::NEED_TEXT_UPDATE;
}
void WidgetConsole::setSizeChar(const float& f)
{
	sizeChar = f;
	configuration |= (uint8_t)ConsoleFlags::NEED_TEXT_UPDATE;
}
void WidgetConsole::setMargin(const float& f)
{
	margin = f;
	configuration |= (uint8_t)ConsoleFlags::NEED_TEXT_UPDATE;
}
void WidgetConsole::append(const std::string& s)
{
	if (!s.empty())
	{
		text += s + '\n';
		configuration |= (uint8_t)ConsoleFlags::NEED_TEXT_UPDATE;
		elevator = 0.f;
	}
}


Font* WidgetConsole::getFont() const { return font; }
float WidgetConsole::getSizeChar() const { return sizeChar; }
float WidgetConsole::getMargin() const { return margin; }
//


//	Protected functions
void WidgetConsole::initVBOtext()
{
	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, TEXT_MAX_CHAR * 4 * sizeof(vec4f), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[BATCH_INDEX_TEXT].vertices.size() * sizeof(vec4f), batchList[BATCH_INDEX_TEXT].vertices.data());

	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].texturesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].texturesBuffer);
	glBufferData(GL_ARRAY_BUFFER, TEXT_MAX_CHAR * 4 * sizeof(vec2f), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[BATCH_INDEX_TEXT].textures.size() * sizeof(vec2f), batchList[BATCH_INDEX_TEXT].textures.data());

	glGenBuffers(1, &batchList[BATCH_INDEX_TEXT].facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[BATCH_INDEX_TEXT].facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TEXT_MAX_CHAR * 6 * sizeof(unsigned short), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[BATCH_INDEX_TEXT].faces.size() * sizeof(unsigned short), batchList[BATCH_INDEX_TEXT].faces.data());
}
void WidgetConsole::updateBuffers(const bool& firstInit)
{
	//	background like standard board
	WidgetBoard::updateBuffers(firstInit);

	//	text clipping Shape (like center but a little smaller)
	DrawBatch clippingShape;
	vec4f dimension = vec4f(0.5f * sizes[State::CURRENT].x - borderThickness, 0.f, 0.5f * sizes[State::CURRENT].y - borderThickness, 1.f);
	vec4f elevatorMax, elevatorMin;

	clippingShape.vertices.push_back(vec4f::zero);
	clippingShape.faces.push_back(0);
	if (cornerConfiguration & TOP_LEFT)
	{
		clippingShape.vertices.push_back(vec4f(-dimension.x + borderWidth, TEXT_DEPTH_OFFSET, dimension.z, 1.f));
		clippingShape.faces.push_back(1);
	}
	else
	{
		clippingShape.vertices.push_back(vec4f(-dimension.x, TEXT_DEPTH_OFFSET, dimension.z, 1.f));
		clippingShape.faces.push_back(1);
	}
	if (cornerConfiguration & TOP_RIGHT)
	{
		clippingShape.vertices.push_back(vec4f(dimension.x - borderWidth, TEXT_DEPTH_OFFSET, dimension.z, 1.f));
		clippingShape.faces.push_back(2);

		clippingShape.vertices.push_back(vec4f(dimension.x - ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, dimension.z - borderWidth + ELEVATOR_MARGIN_FACTOR * borderWidth, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(2); clippingShape.faces.push_back(3);

		elevatorMax = vec4f(dimension.x - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, dimension.z - borderWidth + 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, 1.f);
	}
	else
	{
		clippingShape.vertices.push_back(vec4f(dimension.x - ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, dimension.z, 1.f));
		clippingShape.faces.push_back(2);

		elevatorMax = vec4f(dimension.x - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, dimension.z, 1.f);
	}
	if (cornerConfiguration & BOTTOM_RIGHT)
	{
		unsigned int index = (int)clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(vec4f(dimension.x - ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, -dimension.z + ELEVATOR_MARGIN_FACTOR * borderWidth, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		clippingShape.vertices.push_back(vec4f(dimension.x - borderWidth, TEXT_DEPTH_OFFSET, -dimension.z, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index + 1); clippingShape.faces.push_back(index + 2);

		elevatorMin = vec4f(dimension.x - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, -dimension.z + borderWidth - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, 1.f);
	}
	else
	{
		unsigned int index = (int)clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(vec4f(dimension.x - ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, -dimension.z, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		elevatorMin = vec4f(dimension.x - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, -dimension.z, 1.f);
	}
	if (cornerConfiguration & BOTTOM_LEFT)
	{
		unsigned int index = (int)clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(vec4f(-dimension.x + borderWidth, TEXT_DEPTH_OFFSET, -dimension.z, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		clippingShape.vertices.push_back(vec4f(-dimension.x, TEXT_DEPTH_OFFSET, -dimension.z + borderWidth, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index + 1); clippingShape.faces.push_back(index + 2);
	}
	else
	{
		unsigned int index = (int)clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(vec4f(-dimension.x, TEXT_DEPTH_OFFSET, -dimension.z, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);
	}
	if (cornerConfiguration & TOP_LEFT)
	{
		unsigned int index = (int)clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(vec4f(-dimension.x, TEXT_DEPTH_OFFSET, dimension.z - borderWidth, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		clippingShape.vertices.push_back(vec4f(-dimension.x + borderWidth, TEXT_DEPTH_OFFSET, dimension.z, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index + 1); clippingShape.faces.push_back(index + 2);
	}
	else
	{
		unsigned int index = (int)clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(vec4f(-dimension.x, TEXT_DEPTH_OFFSET, dimension.z, 1.f));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);
	}

	//	end clipping Shape
	if (firstInit)
	{
		for (unsigned int i = 0; i < clippingShape.vertices.size(); i++)
			clippingShape.textures.push_back(vec2f(0.f, 0.f));
		batchList[BATCH_INDEX_CLIPPING].textures.swap(clippingShape.textures);
	}
	batchList[BATCH_INDEX_CLIPPING].vertices.swap(clippingShape.vertices);
	batchList[BATCH_INDEX_CLIPPING].faces.swap(clippingShape.faces);

	//	compute elevator attributes
	elevatorLength = ELEVATOR_LENGTH_FACTOR * borderThickness;
	elevatorRange = elevatorMax.z - elevatorMin.z - 2.f * elevatorLength;
	float s = elevatorMax.z - elevatorMin.z - elevatorLength;
	vec2f p(0.5f * (elevatorMax + elevatorMin).x, 0.5f * (elevatorMax + elevatorMin).z);

	//	elevator clipping quad init
	DrawBatch elevatorClipping;
		elevatorClipping.vertices.push_back(vec4f(p.x - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, p.y - 0.5f * s, 1.f));
		elevatorClipping.vertices.push_back(vec4f(p.x - 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, p.y + 0.5f * s, 1.f));
		elevatorClipping.vertices.push_back(vec4f(p.x + 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, p.y + 0.5f * s, 1.f));
		elevatorClipping.vertices.push_back(vec4f(p.x + 0.5f * ELEVATOR_MARGIN_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, p.y - 0.5f * s, 1.f));

		elevatorClipping.textures.push_back(vec2f::zero);
		elevatorClipping.textures.push_back(vec2f::zero);
		elevatorClipping.textures.push_back(vec2f::zero);
		elevatorClipping.textures.push_back(vec2f::zero);

		elevatorClipping.faces.push_back(0); elevatorClipping.faces.push_back(1); elevatorClipping.faces.push_back(2);
		elevatorClipping.faces.push_back(0); elevatorClipping.faces.push_back(2); elevatorClipping.faces.push_back(3);

	batchList[BATCH_INDEX_CLIPPING_ELEVATOR].vertices.swap(elevatorClipping.vertices);
	batchList[BATCH_INDEX_CLIPPING_ELEVATOR].textures.swap(elevatorClipping.textures);
	batchList[BATCH_INDEX_CLIPPING_ELEVATOR].faces.swap(elevatorClipping.faces);

	//	elevator
	const float pi = (float)PI;
	DrawBatch elevator;
		elevator.vertices.push_back(vec4f(p.x - 0.3f * borderThickness, TEXT_DEPTH_OFFSET, p.y - (elevatorMax.z - elevatorMin.z), 1.f));
		elevator.vertices.push_back(vec4f(p.x - 0.3f * borderThickness, TEXT_DEPTH_OFFSET, p.y + (elevatorMax.z - elevatorMin.z), 1.f));
		elevator.vertices.push_back(vec4f(p.x + 0.3f * borderThickness, TEXT_DEPTH_OFFSET, p.y + (elevatorMax.z - elevatorMin.z), 1.f));
		elevator.vertices.push_back(vec4f(p.x + 0.3f * borderThickness, TEXT_DEPTH_OFFSET, p.y - (elevatorMax.z - elevatorMin.z), 1.f));

		elevator.textures.push_back(vec2f::zero);
		elevator.textures.push_back(vec2f::zero);
		elevator.textures.push_back(vec2f::zero);
		elevator.textures.push_back(vec2f::zero);

		elevator.faces.push_back(0); elevator.faces.push_back(1); elevator.faces.push_back(2);
		elevator.faces.push_back(0); elevator.faces.push_back(2); elevator.faces.push_back(3);

		//	circle
		elevator.vertices.push_back(vec4f(p.x, TEXT_DEPTH_OFFSET, p.y, 1.f));
		elevator.textures.push_back(vec2f::zero);
		for (int i = 0; i < 16; i++)
		{
			elevator.vertices.push_back(vec4f(p.x + 0.4f * ELEVATOR_MARGIN_FACTOR * borderWidth * cos(i * pi / 8.f),
												  TEXT_DEPTH_OFFSET,
												  p.y + 0.4f * ELEVATOR_MARGIN_FACTOR * borderWidth * sin(i * pi / 8.f), 1.f));
			elevator.textures.push_back(vec2f::zero);

			if (i != 0)
			{
				elevator.faces.push_back(4);
				elevator.faces.push_back((unsigned short)elevator.vertices.size() - 2);
				elevator.faces.push_back((unsigned short)elevator.vertices.size() - 1);
			}
		}
		elevator.faces.push_back(4);
		elevator.faces.push_back(5);
		elevator.faces.push_back((unsigned short)elevator.vertices.size() - 1);

	batchList[BATCH_INDEX_ELEVATOR].vertices.swap(elevator.vertices);
	batchList[BATCH_INDEX_ELEVATOR].textures.swap(elevator.textures);
	batchList[BATCH_INDEX_ELEVATOR].faces.swap(elevator.faces);
}
void WidgetConsole::updateTextBuffer()
{
	if (!font) return;

	DrawBatch textBatch;
	float x = 0.f;
	unsigned int line = 0;

	for (unsigned int i = 0; i < text.size(); i++)
	{
		//	init parameters
		vec2f o(positions[State::CURRENT].x - 0.5f * sizes[State::CURRENT].x - borderThickness + margin,
			positions[State::CURRENT].z + borderThickness - 0.5f * sizes[State::CURRENT].y + sizeChar * (linesLength.size() - 1 - line));
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
				x = TAB_SIZE * sizeChar * ((int)(x / (TAB_SIZE * sizeChar)) + 1);
				break;

			default:
				textBatch.vertices.push_back(vec4f(o.x + x, TEXT_DEPTH_OFFSET, o.y, 1.f));
				textBatch.vertices.push_back(vec4f(o.x + x, TEXT_DEPTH_OFFSET, o.y + sizeChar, 1.f));
				textBatch.vertices.push_back(vec4f(o.x + x + charLength*sizeChar, TEXT_DEPTH_OFFSET, o.y + sizeChar, 1.f));
				textBatch.vertices.push_back(vec4f(o.x + x + charLength*sizeChar, TEXT_DEPTH_OFFSET, o.y, 1.f));

				textBatch.textures.push_back(vec2f(patch.corner1.x + TEXTURE_OFFSET, patch.corner2.y - TEXTURE_OFFSET));
				textBatch.textures.push_back(vec2f(patch.corner1.x + TEXTURE_OFFSET, patch.corner1.y + TEXTURE_OFFSET));
				textBatch.textures.push_back(vec2f(patch.corner2.x - TEXTURE_OFFSET, patch.corner1.y + TEXTURE_OFFSET));
				textBatch.textures.push_back(vec2f(patch.corner2.x - TEXTURE_OFFSET, patch.corner2.y - TEXTURE_OFFSET));

				textBatch.faces.push_back((unsigned short)(textBatch.vertices.size() - 4));
				textBatch.faces.push_back((unsigned short)(textBatch.vertices.size() - 3));
				textBatch.faces.push_back((unsigned short)(textBatch.vertices.size() - 2));

				textBatch.faces.push_back((unsigned short)(textBatch.vertices.size() - 4));
				textBatch.faces.push_back((unsigned short)(textBatch.vertices.size() - 2));
				textBatch.faces.push_back((unsigned short)(textBatch.vertices.size() - 1));

				x += sizeChar*charLength;
				break;
		}
	}
	batchList[BATCH_INDEX_TEXT].vertices.swap(textBatch.vertices);
	batchList[BATCH_INDEX_TEXT].textures.swap(textBatch.textures);
	batchList[BATCH_INDEX_TEXT].faces.swap(textBatch.faces);
}
void WidgetConsole::parseText()
{
	if (!font) return;
	
	//	crop text if needed
	if(text.size() > TEXT_MAX_CHAR)
		text.erase(0, text.size() - TEXT_MAX_CHAR);

	//	compute length of each line
	std::vector<std::pair<unsigned int, unsigned int> > lineIndexes;
	lineIndexes.push_back(std::pair<unsigned int, unsigned int>(0, 0));
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
				lineIndexes.back().second = i;
				lineIndexes.push_back(std::pair<unsigned int, unsigned int>(i + 1, 0));
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

	//	remove extra line
	if (linesLength.size() > MAX_LINE)
	{
		int eraseLineCount = (int)linesLength.size() - MAX_LINE;
		text.erase(0, lineIndexes[eraseLineCount - 1].second + 1);
		linesLength.erase(linesLength.begin(), linesLength.begin() + eraseLineCount);
	}
}
//
