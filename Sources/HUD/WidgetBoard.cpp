#include "WidgetBoard.h"

#define BATCH_INDEX_BORDER 0
#define BATCH_INDEX_CENTER 1

//  Default
WidgetBoard::WidgetBoard(const uint8_t& config, const std::string& shaderName) : 
	WidgetVirtual(WidgetVirtual::BOARD, config | NEED_UPDATE, shaderName), updateCooldown(0.f)
{}
WidgetBoard::~WidgetBoard() {}
//

//  Public functions
void WidgetBoard::initialize(const float& bThickness, const float& bWidth, const uint8_t& corner)
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
	updateBuffers(true);

	if(configuration & RESPONSIVE) 
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
void WidgetBoard::draw(Shader* s, uint8_t& stencilMask)
{
	//	texture related stuff
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
}
void WidgetBoard::update(const float& elapseTime)
{
	State s = (State)(configuration & STATE_MASK);
	colors[CURRENT] = 0.9f * colors[CURRENT] + 0.1f * colors[s];
	positions[CURRENT] = positions[s];
	sizes[CURRENT] = sizes[s];
	lastConfiguration = configuration;

	//	update buffers if needed
	if (configuration & RESPONSIVE)
	{
		updateCooldown += elapseTime;
		if (configuration & NEED_UPDATE && updateCooldown > 500.f)
		{
			updateBuffers();
			updateVBOs();
			configuration &= ~NEED_UPDATE;
			updateCooldown = 0.f;
		}
	}
}
//

//	Protected functions
void WidgetBoard::updateBuffers(const bool& firstInit)
{

	DrawBatch border, center;
	glm::vec3 dimension = glm::vec3(0.5f * sizes[CURRENT].x - borderThickness, 0.f, 0.5f * sizes[CURRENT].y - borderThickness);
	const float pi = glm::pi<float>();
	unsigned int borderCornerIndex;

	//	center vertex
	center.vertices.push_back(glm::vec3(0.f, 0.f, 0.f));
	center.faces.push_back(0);

	//	Top
	if (cornerConfiguration & TOP_LEFT)
	{
		center.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z));
		center.faces.push_back(1);

		//	vertex 0
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z));
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z + borderThickness));

		//	begin triangle 1
		border.faces.push_back(0); border.faces.push_back(1);
	}
	else
	{
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z));
		center.faces.push_back(1);

		//	vertex 0
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z));
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z + borderThickness));

		//	begin triangle 1
		border.faces.push_back(0); border.faces.push_back(1);
	}
	if (cornerConfiguration & TOP_RIGHT)
	{
		//	center
		center.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, dimension.z));
		center.faces.push_back(2);

		center.vertices.push_back(glm::vec3(dimension.x, 0.f, dimension.z - borderWidth));
		center.faces.push_back(0); center.faces.push_back(2); center.faces.push_back(3);

		//	vertex 2
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, dimension.z));
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, dimension.z + borderThickness));
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth + borderThickness * sin(pi / 8), 0.f, dimension.z + borderThickness * cos(pi / 8)));

		//	vertex 5
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth + borderThickness * sin(pi / 4), 0.f, dimension.z + borderThickness * cos(pi / 4)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness * sin(pi / 4), 0.f, dimension.z - borderWidth + borderThickness * cos(pi / 4)));
		border.vertices.push_back(glm::vec3(dimension.x, 0.f, dimension.z - borderWidth));

		//	vertex 8
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness * sin(3 * pi / 8), 0.f, dimension.z - borderWidth + borderThickness * cos(3 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness, 0.f, dimension.z - borderWidth));

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
		center.vertices.push_back(glm::vec3(dimension.x, 0.f, dimension.z));
		center.faces.push_back(2);

		//	vertex 2
		border.vertices.push_back(glm::vec3(dimension.x, 0.f, dimension.z));
		border.vertices.push_back(glm::vec3(dimension.x, 0.f, dimension.z + borderThickness));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness * sin(pi / 8), 0.f, dimension.z + borderThickness * cos(pi / 8)));

		//	vertex 5
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness * sin(pi / 4), 0.f, dimension.z + borderThickness * cos(pi / 4)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness * sin(3 * pi / 8), 0.f, dimension.z + borderThickness * cos(3 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness, 0.f, dimension.z));

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
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(dimension.x, 0.f, -dimension.z + borderWidth));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(dimension.x, 0.f, -dimension.z + borderWidth));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness, 0.f, -dimension.z + borderWidth));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness* sin(5 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(5 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness* sin(6 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(6 * pi / 8)));

		//	right rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, -dimension.z));
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth + borderThickness* sin(6 * pi / 8), 0.f, -dimension.z + borderThickness * cos(6 * pi / 8)));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth + borderThickness* sin(7 * pi / 8), 0.f, -dimension.z + borderThickness * cos(7 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, -dimension.z - borderThickness));

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
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(dimension.x, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(dimension.x, 0.f, -dimension.z));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness, 0.f, -dimension.z));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness* sin(5 * pi / 8), 0.f, -dimension.z + borderThickness * cos(5 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness* sin(6 * pi / 8), 0.f, -dimension.z + borderThickness * cos(6 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness* sin(7 * pi / 8), 0.f, -dimension.z + borderThickness * cos(7 * pi / 8)));
		border.vertices.push_back(glm::vec3(dimension.x, 0.f, -dimension.z - borderThickness));

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
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z + borderWidth));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, -dimension.z));
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, -dimension.z - borderThickness));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth + borderThickness* sin(9 * pi / 8), 0.f, -dimension.z + borderThickness * cos(9 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth + borderThickness* sin(10 * pi / 8), 0.f, -dimension.z + borderThickness * cos(10 * pi / 8)));

		//	bottom rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z + borderWidth));
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(10 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(10 * pi / 8)));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(11 * pi / 8), 0.f, -dimension.z + borderWidth + borderThickness * cos(11 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x - borderThickness, 0.f, -dimension.z + borderWidth));

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
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z));
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z - borderThickness));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(9 * pi / 8), 0.f, -dimension.z + borderThickness * cos(9 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(10 * pi / 8), 0.f, -dimension.z + borderThickness * cos(10 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(11 * pi / 8), 0.f, -dimension.z + borderThickness * cos(11 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x - borderThickness, 0.f, -dimension.z));

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
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z - borderWidth));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z - borderWidth));
		border.vertices.push_back(glm::vec3(-dimension.x - borderThickness, 0.f, dimension.z - borderWidth));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(13 * pi / 8), 0.f, dimension.z - borderWidth + borderThickness * cos(13 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(14 * pi / 8), 0.f, dimension.z - borderWidth + borderThickness * cos(14 * pi / 8)));

		//	left rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 2);		border.faces.push_back(index + 3);
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index + 3);		border.faces.push_back(index + 4);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z));
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth + borderThickness* sin(14 * pi / 8), 0.f, dimension.z + borderThickness * cos(14 * pi / 8)));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth + borderThickness* sin(15 * pi / 8), 0.f, dimension.z + borderThickness * cos(15 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z + borderThickness));

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
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		//	vertex index + 1
		index = border.vertices.size() - 1;
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z));
		border.vertices.push_back(glm::vec3(-dimension.x - borderThickness, 0.f, dimension.z));

		//	vertex index + 3
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(13 * pi / 8), 0.f, dimension.z + borderThickness * cos(13 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(14 * pi / 8), 0.f, dimension.z + borderThickness * cos(14 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x + borderThickness* sin(15 * pi / 8), 0.f, dimension.z + borderThickness * cos(15 * pi / 8)));
		border.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z + borderThickness));

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
			border.textures.push_back(glm::vec2(0.f, 0.f));
		batchList[BATCH_INDEX_BORDER].textures.swap(border.textures);

		for (unsigned int i = 0; i < center.vertices.size(); i++)
			center.textures.push_back(glm::vec2(0.f, 0.f));
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
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].vertices.size() * sizeof(glm::vec3), batchList[i].vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, batchList[i].texturesBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, batchList[i].textures.size() * sizeof(glm::vec2), batchList[i].textures.data());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchList[i].facesBuffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, batchList[i].faces.size() * sizeof(unsigned short), batchList[i].faces.data());
	}
}
//