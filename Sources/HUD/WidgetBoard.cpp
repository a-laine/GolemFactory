#include "WidgetBoard.h"

//  Default
WidgetBoard::WidgetBoard(const uint8_t& config, const std::string& shaderName) : WidgetVirtual(config, shaderName) {}
WidgetBoard::~WidgetBoard() {}
//

//  Public functions
void WidgetBoard::initialize(const float& borderThickness, const float& borderWidth, const uint8_t& corner)
{
	//	init
	drawBatch border;
	drawBatch center;
	border.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	center.color = glm::vec4(1.f, 1.f, 1.f, 0.5f);
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
	/**/if (corner & BOTTOM_RIGHT)
	{
		//	center
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(dimension.x, 0.f, -dimension.z + borderWidth));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(glm::vec3(dimension.x - borderWidth, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);
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



		//	right rectangle
		border.faces.push_back(borderCornerIndex);  border.faces.push_back(index);		border.faces.push_back(index + 1);
		border.faces.push_back(index);				border.faces.push_back(index + 1);	border.faces.push_back(index + 2);

		//	semi disk
		borderCornerIndex = index + 1;
	}

	//	Bottom
	/**/if (corner & BOTTOM_LEFT)
	{
		//	center
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z + borderWidth));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);
	}
	/**/else
	{
		//	center
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, -dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);
	}

	//	Left
	/**/if (corner & TOP_LEFT)
	{
		//	center
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z - borderWidth));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);

		center.vertices.push_back(glm::vec3(-dimension.x + borderWidth, 0.f, dimension.z));
		center.faces.push_back(0); center.faces.push_back(index + 1); center.faces.push_back(index + 2);
	}
	/**/else
	{
		//	center
		unsigned int index = center.vertices.size() - 1;
		center.vertices.push_back(glm::vec3(-dimension.x, 0.f, dimension.z));
		center.faces.push_back(0); center.faces.push_back(index); center.faces.push_back(index + 1);
	}

	//	Debug log
	std::cout << "center vertices : " << center.vertices.size() << std::endl;
	std::cout << "center faces : " << center.faces.size() << std::endl;
	std::cout << "center textures : " << center.textures.size() << std::endl << std::endl;

	std::cout << "border vertices : " << border.vertices.size() << std::endl;
	std::cout << "border faces : " << border.faces.size() << std::endl;
	std::cout << "border textures : " << border.textures.size() << std::endl << std::endl;

	//	end
	for (unsigned int i = 0; i < border.vertices.size(); i++)
		border.textures.push_back(glm::vec2(0.f, 0.f));
	batchList.push_back(border);

	for (unsigned int i = 0; i < center.vertices.size(); i++)
		center.textures.push_back(glm::vec2(0.f, 0.f));
	batchList.push_back(center);

	initializeVBOs();
	initializeVAOs();
}
//

//  Set/get functions
void WidgetBoard::setColor(const glm::vec4& c)
{
	if (!batchList.empty())
	{
		batchList[0].color = c;
		batchList[1].color = glm::vec4(c.x, c.y, c.z, 0.5f * c.w);
	}
}
//