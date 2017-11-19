#include "WidgetBoard.h"

//  Default
WidgetBoard::WidgetBoard(const uint8_t& config, const std::string& shaderName) : WidgetVirtual(WidgetVirtual::BOARD, config, shaderName) {}
WidgetBoard::~WidgetBoard() {}
//

//  Public functions
void WidgetBoard::initialize(const float& borderThickness, const float& borderWidth, const uint8_t& corner)
{
	//	init
	drawBatch border;   border.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	drawBatch center;   center.color = glm::vec4(1.f, 1.f, 1.f, 0.5f);
	
	glm::vec3 dimension = glm::vec3(0.5f * size.x - borderThickness, 0.f, 0.5f * size.y - borderThickness);
	const float pi = glm::pi<float>();
	unsigned int borderCornerIndex;

	//	center vertex
	center.vertices.push_back(glm::vec3(0.f, 0.f, 0.f));
	center.faces.push_back(0);

	//	Top
	if (corner & TOP_LEFT)
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
	if (corner & TOP_RIGHT)
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
		border.vertices.push_back(glm::vec3(dimension.x + borderThickness * sin(3*pi / 8), 0.f, dimension.z + borderThickness * cos(3*pi / 8)));
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
	if (corner & BOTTOM_RIGHT)
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
	if (corner & BOTTOM_LEFT)
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
	if (corner & TOP_LEFT)
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
	
	//	fill texture coord to dummy value
	for (unsigned int i = 0; i < border.vertices.size(); i++)
		border.textures.push_back(glm::vec2(0.f, 0.f));
	batchList.push_back(border);
	for (unsigned int i = 0; i < center.vertices.size(); i++)
		center.textures.push_back(glm::vec2(0.f, 0.f));
	batchList.push_back(center);

	//	end
	initializeVBOs();
	initializeVAOs();
}
//

//  Set/get functions
void WidgetBoard::setColor(const glm::vec4& c)
{
	if (batchList.size() >= 2)
	{
		batchList[0].color = c;
		batchList[1].color = glm::vec4(c.x, c.y, c.z, 0.5f * c.w);
	}
}
//