#include "WidgetBoard.h"

#define BATCH_INDEX_BORDER 0
#define BATCH_INDEX_CENTER 1

//  Default
WidgetBoard::WidgetBoard(const uint8_t& config, const std::string& shaderName) : 
	WidgetVirtual(WidgetVirtual::WidgetType::BOARD, config | (uint8_t)WidgetVirtual::OrphanFlags::NEED_UPDATE, shaderName), 
	cornerConfiguration(0x00), borderWidth(0.f), borderThickness(0.f), updateCooldown(0.f)//, lastEventState(false)
{}
WidgetBoard::~WidgetBoard() {}
//

//  Public functions
void WidgetBoard::initialize(const float& bThickness, const float& bWidth, const uint8_t& corner)
{
	colors[State::CURRENT] = colors[(State)(configuration & (uint8_t)State::STATE_MASK)];
	positions[State::CURRENT] = positions[(State)(configuration & (uint8_t)State::STATE_MASK)];
	sizes[State::CURRENT] = sizes[(State)(configuration & (uint8_t)State::STATE_MASK)];

	borderThickness = bThickness;
	borderWidth = bWidth;
	cornerConfiguration = corner;

	batchList.push_back(DrawBatch());	//	BATCH_INDEX_BORDER
	batchList.push_back(DrawBatch());	//	BATCH_INDEX_CENTER
	updateBuffers(true);

	if(configuration & (uint8_t)WidgetVirtual::OrphanFlags::RESPONSIVE)
	{
		initializeVBO(BATCH_INDEX_BORDER, GL_DYNAMIC_DRAW);
		initializeVBO(BATCH_INDEX_CENTER, GL_DYNAMIC_DRAW);
	}
	else
	{
		initializeVBO(BATCH_INDEX_BORDER, GL_STATIC_DRAW);
		initializeVBO(BATCH_INDEX_CENTER, GL_STATIC_DRAW);
	}
	initializeVAOs();
}
void WidgetBoard::draw(Shader* s, uint8_t& stencilMask, const mat4f& model)
{
	//	texture related stuff
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
	glm::vec4 color(colors[State::CURRENT].x, colors[State::CURRENT].y, colors[State::CURRENT].z, 0.5f * colors[State::CURRENT].w);
	loc = s->getUniformLocation("color");
	if (loc >= 0) glUniform4fv(loc, 1, &color.x);

	glBindVertexArray(batchList[BATCH_INDEX_CENTER].vao);
	glDrawElements(GL_TRIANGLES, (int)batchList[BATCH_INDEX_CENTER].faces.size(), GL_UNSIGNED_SHORT, NULL);
}
void WidgetBoard::update(const float& elapseTime)
{
	State s = (State)(configuration & (uint8_t)State::STATE_MASK);
	colors[State::CURRENT] = 0.9f * colors[State::CURRENT] + 0.1f * colors[s];
	positions[State::CURRENT] = positions[s];
	sizes[State::CURRENT] = sizes[s];

	//	update buffers if needed
	if (configuration & (uint8_t)WidgetVirtual::OrphanFlags::RESPONSIVE)
	{
		updateCooldown += elapseTime;
		if (configuration & (uint8_t)WidgetVirtual::OrphanFlags::NEED_UPDATE && updateCooldown > 500.f)
		{
			updateBuffers();
			updateVBOs();
			configuration &= ~(uint8_t)WidgetVirtual::OrphanFlags::NEED_UPDATE;
			updateCooldown = 0.f;
		}
	}
}
bool WidgetBoard::mouseEvent(const mat4f& base, const vec4f& ray, const float& parentscale, const bool& clicked)
{
	if ((configuration & (uint8_t)BoardFlags::CAN_BE_ACTIVATED))
	{
		if (clicked && WidgetVirtual::intersect(base, ray))
		{
			setState(State::ACTIVE);
			return true;
		}
	}
	return false;
}
//

//	Protected functions
void WidgetBoard::updateBuffers(const bool& firstInit)
{

	DrawBatch border, center;
	vec4f dimension = vec4f(0.5f * sizes[State::CURRENT].x - borderThickness, 0.f, 0.5f * sizes[State::CURRENT].y - borderThickness, 1.f);
	const float pi = glm::pi<float>();
	unsigned int borderCornerIndex;

	//	center vertex
	center.vertices.push_back(vec4f(0.f, 0.f, 0.f, 1.f));
	center.faces.push_back(0);

	//	Top
	if (cornerConfiguration & TOP_LEFT)
	{
		center.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, dimension.z, 1.f));
		center.faces.push_back(1);

		//	vertex 0
		border.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, dimension.z, 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, dimension.z + borderThickness, 1.f));

		//	begin triangle 1
		border.faces.push_back(0); border.faces.push_back(1);
	}
	else
	{
		center.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z, 1.f));
		center.faces.push_back(1);

		//	vertex 0
		border.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z, 1.f));
		border.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z + borderThickness, 1.f));

		//	begin triangle 1
		border.faces.push_back(0); border.faces.push_back(1);
	}
	if (cornerConfiguration & TOP_RIGHT)
	{
		//	center
		center.vertices.push_back(vec4f(dimension.x - borderWidth, 0.f, dimension.z, 1.f));
		center.faces.push_back(2);

		center.vertices.push_back(vec4f(dimension.x, 0.f, dimension.z - borderWidth, 1.f));
		center.faces.push_back(0); center.faces.push_back(2); center.faces.push_back(3);

		//	vertex 2
		border.vertices.push_back(vec4f(dimension.x - borderWidth, 0.f, dimension.z, 1.f));
		border.vertices.push_back(vec4f(dimension.x - borderWidth, 0.f, dimension.z + borderThickness, 1.f));
		border.vertices.push_back(vec4f(dimension.x - borderWidth + borderThickness * sin(pi / 8), 0.f, dimension.z + borderThickness * cos(pi / 8), 1.f));

		//	vertex 5
		border.vertices.push_back(vec4f(dimension.x - borderWidth + borderThickness * sin(pi / 4), 0.f, dimension.z + borderThickness * cos(pi / 4), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness * sin(pi / 4), 0.f, dimension.z - borderWidth + borderThickness * cos(pi / 4), 1.f));
		border.vertices.push_back(vec4f(dimension.x, 0.f, dimension.z - borderWidth, 1.f));

		//	vertex 8
		border.vertices.push_back(vec4f(dimension.x + borderThickness * sin(3 * pi / 8), 0.f, dimension.z - borderWidth + borderThickness * cos(3 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness, 0.f, dimension.z - borderWidth, 1.f));

		//	finish last block triangle
		border.faces.push_back(2);

		//	finish top rectangle
		border.faces.push_back(1); border.faces.push_back(2); border.faces.push_back(3);

		//	semi disk
		border.faces.push_back(2); border.faces.push_back(3); border.faces.push_back(4);
		border.faces.push_back(2); border.faces.push_back(4); border.faces.push_back(5);

		//	corner
		border.faces.push_back(2); border.faces.push_back(5); border.faces.push_back(7);
		border.faces.push_back(5); border.faces.push_back(6); border.faces.push_back(7);

		//	semi disk
		borderCornerIndex = 7;
		border.faces.push_back(7); border.faces.push_back(6); border.faces.push_back(8);
		border.faces.push_back(7); border.faces.push_back(8); border.faces.push_back(9);
	}
	else
	{
		center.vertices.push_back(vec4f(dimension.x, 0.f, dimension.z, 1.f));
		center.faces.push_back(2);

		//	vertex 2
		border.vertices.push_back(vec4f(dimension.x, 0.f, dimension.z, 1.f));
		border.vertices.push_back(vec4f(dimension.x, 0.f, dimension.z + borderThickness, 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness * sin(pi / 8), 0.f, dimension.z + borderThickness * cos(pi / 8), 1.f));

		//	vertex 5
		border.vertices.push_back(vec4f(dimension.x + borderThickness * sin(pi / 4), 0.f, dimension.z + borderThickness * cos(pi / 4), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness * sin(3 * pi / 8), 0.f, dimension.z + borderThickness * cos(3 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness, 0.f, dimension.z, 1.f));

		//	finish last block triangle
		border.faces.push_back(2);

		//	finish top rectangle
		border.faces.push_back(1); border.faces.push_back(2); border.faces.push_back(3);

		//	semi disk
		borderCornerIndex = 2;
		border.faces.push_back(2); border.faces.push_back(3); border.faces.push_back(4);
		border.faces.push_back(2); border.faces.push_back(4); border.faces.push_back(5);
		border.faces.push_back(2); border.faces.push_back(5); border.faces.push_back(6);
		border.faces.push_back(2); border.faces.push_back(6); border.faces.push_back(7);
	}

	//	Right
	if (cornerConfiguration & BOTTOM_RIGHT)
	{
		//	center
		unsigned int index = (int)center.vertices.size() - 1;
		center.vertices.push_back(vec4f(dimension.x, 0.f, -dimension.z + borderWidth, 1.f));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(vec4f(dimension.x - borderWidth, 0.f, -dimension.z, 1.f));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(dimension.x, 0.f, -dimension.z + borderWidth, 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness, 0.f, -dimension.z + borderWidth, 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(dimension.x + borderThickness* sin(5 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(5 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness* sin(6 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(6 * pi / 8), 1.f));

		//	right rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(dimension.x - borderWidth, 0.f, -dimension.z, 1.f));
		border.vertices.push_back(vec4f(dimension.x - borderWidth + borderThickness* sin(6 * pi / 8), 0.f, -dimension.z + borderThickness * cos(6 * pi / 8), 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(dimension.x - borderWidth + borderThickness* sin(7 * pi / 8), 0.f, -dimension.z + borderThickness * cos(7 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x - borderWidth, 0.f, -dimension.z - borderThickness, 1.f));

		//	bottom right corner
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index);			border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);		border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index + 2);			border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index + 3);			border.faces.push_back(index + 4);
	}
	else
	{
		//	center
		unsigned int index = (int)center.vertices.size() - 1;
		center.vertices.push_back(vec4f(dimension.x, 0.f, -dimension.z, 1.f));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(dimension.x, 0.f, -dimension.z, 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness, 0.f, -dimension.z, 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(dimension.x + borderThickness* sin(5 * pi / 8), 0.f, -dimension.z + borderThickness * cos(5 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness* sin(6 * pi / 8), 0.f, -dimension.z + borderThickness * cos(6 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x + borderThickness* sin(7 * pi / 8), 0.f, -dimension.z + borderThickness * cos(7 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(dimension.x, 0.f, -dimension.z - borderThickness, 1.f));

		//	right rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 4);		border.faces.push_back(index + 5);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 5);		border.faces.push_back(index + 6);
	}

	//	Bottom
	if (cornerConfiguration & BOTTOM_LEFT)
	{
		//	center
		unsigned int index = (int)center.vertices.size() - 1;
		center.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, -dimension.z, 1.f));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(vec4f(-dimension.x, 0.f, -dimension.z + borderWidth, 1.f));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, -dimension.z, 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, -dimension.z - borderThickness, 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(-dimension.x + borderWidth + borderThickness* sin(9 * pi / 8), 0.f, -dimension.z + borderThickness * cos(9 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderWidth + borderThickness* sin(10 * pi / 8), 0.f, -dimension.z + borderThickness * cos(10 * pi / 8), 1.f));

		//	bottom rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(-dimension.x, 0.f, -dimension.z + borderWidth, 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(10 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(10 * pi / 8), 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(11 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(11 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x - borderThickness, 0.f, -dimension.z + borderWidth, 1.f));

		//	bottom left corner
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index);			border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);		border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index + 2);			border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index + 3);			border.faces.push_back(index + 4);
	}
	else
	{
		//	center
		unsigned int index = (int)center.vertices.size() - 1;
		center.vertices.push_back(vec4f(-dimension.x, 0.f, -dimension.z, 1.f));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(-dimension.x, 0.f, -dimension.z, 1.f));
		border.vertices.push_back(vec4f(-dimension.x, 0.f, -dimension.z - borderThickness, 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(9 * pi / 8), 0.f, -dimension.z + borderThickness * cos(9 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(10 * pi / 8), 0.f, -dimension.z + borderThickness * cos(10 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(11 * pi / 8), 0.f, -dimension.z + borderThickness * cos(11 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x - borderThickness, 0.f, -dimension.z, 1.f));

		//	bottom rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 4);		border.faces.push_back(index + 5);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 5);		border.faces.push_back(index + 6);
	}

	//	Left
	if (cornerConfiguration & TOP_LEFT)
	{
		//	center
		unsigned int index = (int)center.vertices.size() - 1;
		center.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z - borderWidth, 1.f));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, dimension.z, 1.f));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z - borderWidth, 1.f));
		border.vertices.push_back(vec4f(-dimension.x - borderThickness, 0.f, dimension.z - borderWidth, 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(13 * pi / 8), 0.f, dimension.z - borderWidth + borderThickness * cos(13 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(14 * pi / 8), 0.f, dimension.z - borderWidth + borderThickness * cos(14 * pi / 8), 1.f));

		//	left rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, dimension.z, 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderWidth + borderThickness* sin(14 * pi / 8), 0.f, dimension.z + borderThickness * cos(14 * pi / 8), 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(-dimension.x + borderWidth + borderThickness* sin(15 * pi / 8), 0.f, dimension.z + borderThickness * cos(15 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderWidth, 0.f, dimension.z + borderThickness, 1.f));

		//	top left corner
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index);			border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);		border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index + 2);			border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);	border.faces.push_back(index + 3);			border.faces.push_back(index + 4);
	}
	else
	{
		//	center
		unsigned int index = (int)center.vertices.size() - 1;
		center.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z, 1.f));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		//	vertex index + 1
		index = (int)border.vertices.size() - 1;
		border.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z, 1.f));
		border.vertices.push_back(vec4f(-dimension.x - borderThickness, 0.f, dimension.z, 1.f));

		//	vertex index + 3
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(13 * pi / 8), 0.f, dimension.z + borderThickness * cos(13 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(14 * pi / 8), 0.f, dimension.z + borderThickness * cos(14 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x + borderThickness* sin(15 * pi / 8), 0.f, dimension.z + borderThickness * cos(15 * pi / 8), 1.f));
		border.vertices.push_back(vec4f(-dimension.x, 0.f, dimension.z + borderThickness, 1.f));

		//	bottom rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 4);		border.faces.push_back(index + 5);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 5);		border.faces.push_back(index + 6);
	}

	//	end
	if (firstInit)
	{
		for (unsigned int i = 0; i < border.vertices.size(); i++)
			border.textures.push_back(vec2f(0.f, 0.f));
		batchList[BATCH_INDEX_BORDER].textures.swap(border.textures);

		for (unsigned int i = 0; i < center.vertices.size(); i++)
			center.textures.push_back(vec2f(0.f, 0.f));
		batchList[BATCH_INDEX_CENTER].textures.swap(center.textures);
	}

	batchList[BATCH_INDEX_BORDER].vertices.swap(border.vertices);
	batchList[BATCH_INDEX_BORDER].faces.swap(border.faces);
	batchList[BATCH_INDEX_CENTER].vertices.swap(center.vertices);
	batchList[BATCH_INDEX_CENTER].faces.swap(center.faces);
}
void WidgetBoard::updateVBOs()
{
	for (unsigned int i = 0; i < batchList.size(); i++)
	{
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].verticesBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].vertices.size() * sizeof(vec4f), batchList[i].vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].textures.size() * sizeof(vec2f), batchList[i].textures.data());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[i].faces.size() * sizeof(unsigned short), batchList[i].faces.data());
	}
}
//