#include "WidgetConsole.h"

//	same as widget label
#define SCROLL_SIZE_FACTOR	0.5f
#define MAX_LINE			10
#define LINE_OFFSET			0.7f
#define TEX_OFFSET			0.00586f
#define TEXT_DEPTH_OFFSET	-0.01f
#define TEXT_MAX_CHAR		1000


#define BATCH_INDEX_BORDER	 0
#define BATCH_INDEX_CENTER	 1
#define BATCH_INDEX_CLIPPING 2
#define BATCH_INDEX_TEXT	 3


//  Default
WidgetConsole::WidgetConsole(const uint8_t& config, const std::string& shaderName) : 
	WidgetBoard(config | NEED_UPDATE | SPECIAL, shaderName), sizeChar(0.1f)
{
	type = CONSOLE;
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
	State s = (State)(configuration & STATE_MASK);
	colors[CURRENT] = 0.9f * colors[CURRENT] + 0.1f * colors[s];
	positions[CURRENT] = positions[s];
	sizes[CURRENT] = sizes[s];
	lastConfiguration = configuration;

	//	update buffers if needed
	updateCooldown += elapseTime;
	if (updateCooldown > 500.f)
	{
		updateCooldown = 0.f;
		if (configuration & NEED_UPDATE)
		{
			configuration &= ~NEED_UPDATE;
			updateBuffers();
		}
		if (configuration & SPECIAL)
		{
			configuration &= ~SPECIAL;
			parseText();
			updateTextBuffer();
		}
		updateVBOs();
	}
}
void WidgetConsole::initialize(const float& bThickness, const float& bWidth, const uint8_t& corner)
{
	colors[CURRENT] = colors[(State)(configuration & STATE_MASK)];
	positions[CURRENT] = positions[(State)(configuration & STATE_MASK)];
	sizes[CURRENT] = sizes[(State)(configuration & STATE_MASK)];
	lastConfiguration = configuration;

	borderThickness = bThickness;
	borderWidth = bWidth;
	cornerConfiguration = corner;

	batchList.push_back(DrawBatch());	//	BATCH_INDEX_BORDER
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CENTER
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CLIPPING
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_TEXT
	updateBuffers(true);

	initializeVBO(BATCH_INDEX_BORDER, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_CENTER, GL_DYNAMIC_DRAW);
	initializeVBO(BATCH_INDEX_CLIPPING, GL_DYNAMIC_DRAW);
	initializeVAOs();
}
void WidgetConsole::draw(Shader* s, uint8_t& stencilMask)
{
	//	draw board part
	if (texture) glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
	else glBindTexture(GL_TEXTURE_2D, 0);
	int loc = s->getUniformLocation("useTexture");
	if (loc >= 0) glUniform1i(loc, (texture ? 1 : 0));

		//	draw border
		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &colors[CURRENT].x);

		glBindVertexArray(batchList[BATCH_INDEX_BORDER].vao);
		glDrawElements(GL_TRIANGLES, batchList[BATCH_INDEX_BORDER].faces.size(), GL_UNSIGNED_SHORT, NULL);

		//	draw center at different alpha
		glm::vec4 color(colors[CURRENT].x, colors[CURRENT].y, colors[CURRENT].z, 0.5f * colors[CURRENT].w);
		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &color.x);

		glBindVertexArray(batchList[BATCH_INDEX_CENTER].vao);
		glDrawElements(GL_TRIANGLES, batchList[BATCH_INDEX_CENTER].faces.size(), GL_UNSIGNED_SHORT, NULL);

	//	draw text
	drawClippingShape(BATCH_INDEX_CLIPPING, true, s, stencilMask);

		//	texture related stuff
		if (font) glBindTexture(GL_TEXTURE_2D, font->texture);
		else glBindTexture(GL_TEXTURE_2D, 0);
		loc = s->getUniformLocation("useTexture");
		if (loc >= 0) glUniform1i(loc, (font ? 1 : 0));

		//	draw text
		loc = s->getUniformLocation("color");
		if (loc >= 0) glUniform4fv(loc, 1, &glm::vec4(1.f)[0]);

		glBindVertexArray(batchList[BATCH_INDEX_TEXT].vao);
		glDrawElements(GL_TRIANGLES, batchList[BATCH_INDEX_TEXT].faces.size(), GL_UNSIGNED_SHORT, NULL);

	//	unclip zone
	drawClippingShape(BATCH_INDEX_CLIPPING, false, s, stencilMask);
}
bool WidgetConsole::intersect(const glm::mat4& base, const glm::vec3& ray, const glm::vec3 origin, glm::vec3& result)
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		if (i == BATCH_INDEX_CLIPPING || i == BATCH_INDEX_TEXT) continue;
		for (unsigned int j = 0; j < batchList[i].faces.size(); j += 3)
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

			//	ray actually intersect this triangle
			result = depth * ray;
			return true;
		}
	}
	return false;
}
bool WidgetConsole::mouseEvent(const glm::vec3& eventLocation, const bool& clicked)
{
	if (clicked)
	{
		std::cout << (eventLocation.z + sizes[CURRENT].y/2) / sizes[CURRENT].y << std::endl;
		setState(ACTIVE);
		return true;
	}
	else return false;
}
//



//	Set / get functions
void WidgetConsole::setFont(const std::string& fontName)
{
	ResourceManager::getInstance()->release(font);
	font = ResourceManager::getInstance()->getFont(fontName);
	configuration |= SPECIAL;
}
void WidgetConsole::setSizeChar(const float& f)
{
	sizeChar = f;
	configuration |= SPECIAL;
}
void WidgetConsole::append(const std::string& s)
{
	text += s + '\n';
	configuration |= SPECIAL;
}


Font* WidgetConsole::getFont() const { return font; }
float WidgetConsole::getSizeChar() const { return sizeChar; }
//

//	Protected functions
void WidgetConsole::initVBOtext()
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
void WidgetConsole::updateBuffers(const bool& firstInit)
{
	//	background like standard board
	WidgetBoard::updateBuffers(firstInit);

	//	text clipping shape (like center but a little smaller)
	DrawBatch clippingShape;
	glm::vec3 dimension = glm::vec3(0.5f * sizes[CURRENT].x - borderThickness, 0.f, 0.5f * sizes[CURRENT].y - borderThickness);

	clippingShape.vertices.push_back(glm::vec3(0.f, 0.f, 0.f));
	clippingShape.faces.push_back(0);
	if (cornerConfiguration & TOP_LEFT)
	{
		clippingShape.vertices.push_back(glm::vec3(-dimension.x + borderWidth, TEXT_DEPTH_OFFSET, dimension.z));
		clippingShape.faces.push_back(1);
	}
	else
	{
		clippingShape.vertices.push_back(glm::vec3(-dimension.x, TEXT_DEPTH_OFFSET, dimension.z));
		clippingShape.faces.push_back(1);
	}
	if (cornerConfiguration & TOP_RIGHT)
	{
		clippingShape.vertices.push_back(glm::vec3(dimension.x - borderWidth, TEXT_DEPTH_OFFSET, dimension.z));
		clippingShape.faces.push_back(2);

		clippingShape.vertices.push_back(glm::vec3(dimension.x - SCROLL_SIZE_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, dimension.z - borderWidth + SCROLL_SIZE_FACTOR * borderWidth));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(2); clippingShape.faces.push_back(3);
	}
	else
	{
		clippingShape.vertices.push_back(glm::vec3(dimension.x - SCROLL_SIZE_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, dimension.z));
		clippingShape.faces.push_back(2);
	}
	if (cornerConfiguration & BOTTOM_RIGHT)
	{
		unsigned int index = clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(glm::vec3(dimension.x - SCROLL_SIZE_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, -dimension.z + borderWidth));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		clippingShape.vertices.push_back(glm::vec3(dimension.x - borderWidth, TEXT_DEPTH_OFFSET, -dimension.z));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index + 1); clippingShape.faces.push_back(index + 2);
	}
	else
	{
		unsigned int index = clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(glm::vec3(dimension.x - SCROLL_SIZE_FACTOR * borderWidth, TEXT_DEPTH_OFFSET, -dimension.z));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);
	}
	if (cornerConfiguration & BOTTOM_LEFT)
	{
		unsigned int index = clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(glm::vec3(-dimension.x + borderWidth, TEXT_DEPTH_OFFSET, -dimension.z));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		clippingShape.vertices.push_back(glm::vec3(-dimension.x, TEXT_DEPTH_OFFSET, -dimension.z + borderWidth));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index + 1); clippingShape.faces.push_back(index + 2);
	}
	else
	{
		unsigned int index = clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(glm::vec3(-dimension.x, TEXT_DEPTH_OFFSET, -dimension.z));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);
	}
	if (cornerConfiguration & TOP_LEFT)
	{
		unsigned int index = clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(glm::vec3(-dimension.x, TEXT_DEPTH_OFFSET, dimension.z - borderWidth));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);

		clippingShape.vertices.push_back(glm::vec3(-dimension.x + borderWidth, TEXT_DEPTH_OFFSET, dimension.z));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index + 1); clippingShape.faces.push_back(index + 2);
	}
	else
	{
		unsigned int index = clippingShape.vertices.size() - 1;
		clippingShape.vertices.push_back(glm::vec3(-dimension.x, TEXT_DEPTH_OFFSET, dimension.z));
		clippingShape.faces.push_back(0); clippingShape.faces.push_back(index); clippingShape.faces.push_back(index + 1);
	}

	//	end clipping shape
	if (firstInit)
	{
		for (unsigned int i = 0; i < clippingShape.vertices.size(); i++)
			clippingShape.textures.push_back(glm::vec2(0.f, 0.f));
		batchList[BATCH_INDEX_CLIPPING].textures.swap(clippingShape.textures);
	}
	batchList[BATCH_INDEX_CLIPPING].vertices.swap(clippingShape.vertices);
	batchList[BATCH_INDEX_CLIPPING].faces.swap(clippingShape.faces);
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
		glm::vec2 o(positions[CURRENT].x - 0.5f * sizes[CURRENT].x - borderThickness + sizeChar * LINE_OFFSET,
			positions[CURRENT].z + borderThickness - 0.5f * sizes[CURRENT].y + sizeChar * (linesLength.size() - 1 - line));
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
				textBatch.vertices.push_back(glm::vec3(o.x + x, TEXT_DEPTH_OFFSET, o.y));
				textBatch.vertices.push_back(glm::vec3(o.x + x, TEXT_DEPTH_OFFSET, o.y + sizeChar));
				textBatch.vertices.push_back(glm::vec3(o.x + x + (charLength)*sizeChar, TEXT_DEPTH_OFFSET, o.y + sizeChar));
				textBatch.vertices.push_back(glm::vec3(o.x + x + charLength*sizeChar, TEXT_DEPTH_OFFSET, o.y));

				textBatch.textures.push_back(glm::vec2(patch.corner1.x + TEX_OFFSET, patch.corner2.y - TEX_OFFSET));
				textBatch.textures.push_back(glm::vec2(patch.corner1.x + TEX_OFFSET, patch.corner1.y + TEX_OFFSET));
				textBatch.textures.push_back(glm::vec2(patch.corner2.x - TEX_OFFSET, patch.corner1.y + TEX_OFFSET));
				textBatch.textures.push_back(glm::vec2(patch.corner2.x - TEX_OFFSET, patch.corner2.y - TEX_OFFSET));

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
		int eraseLineCount = linesLength.size() - MAX_LINE;
		text.erase(0, lineIndexes[eraseLineCount - 1].second + 1);
		linesLength.erase(linesLength.begin(), linesLength.begin() + eraseLineCount);
	}
}
//