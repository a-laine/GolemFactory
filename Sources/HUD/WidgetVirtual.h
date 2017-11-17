#pragma once

#include <iostream>

#include <glm/glm.hpp>

#include "Resources/ResourceManager.h"

class WidgetVirtual
{
	public:
		//  Miscellaneous
		enum OriginPosition
		{
			MIDDLE_V = 0 << 0,
			TOP = 1 << 0,
			BOTTOM = 2 << 0,

			MIDDLE_H = 0 << 2,
			LEFT = 1 << 2,
			RIGHT = 2 << 2,

			CENTER = MIDDLE_H | MIDDLE_V,
			HORIZONTAL_MASK = 0x0C,
			VERTICAL_MASK = 0x03,
		};
		enum State
		{
			NONE = 0 << 4,
			UNDER_CURSOR = 1 << 4,
			SELECTED = 2 << 4,
			STATE_MASK = 0x03 << 4
		};
		enum Flags
		{
			VISIBLE = 1 << 6,
			ACTIVE = 1 << 7
		};
		struct drawBatch
		{
			glm::vec4 color;
			GLuint  vao,
				verticesBuffer,
				texturesBuffer,
				facesBuffer;
			std::vector<glm::vec3> vertices;
			std::vector<glm::vec2> textures;
			std::vector<unsigned int> faces;
		};
		//

		//  Default
		WidgetVirtual(const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetVirtual();
		//

		//	Public functions
		virtual void initializeVBO(const unsigned int& index);
		virtual void initializeVAO(const unsigned int& index);

		virtual void draw(const unsigned int& index);
		virtual void update(const float& elapseTime);
		//

		//  Set/get functions
		virtual void setSize(const glm::vec2& s);
		virtual void setPosition(const glm::vec2& p);
		virtual void setOrigin(const uint8_t& origin);
		void setVisibility(const bool& visible);
		void setActive(const bool& active);

		glm::vec2 getSize() const;
		glm::vec2 getPosition() const;
		glm::vec4* getColor(const unsigned int& index);
		uint8_t getOriginPosition() const;
		bool isVisible() const;
		bool isActive() const;
		uint8_t getState() const;
		unsigned int getBatchListSize() const;
		Shader* getShader() const;
		//

	protected:
		//  Attributes
		uint8_t configuration;
		glm::vec2 position;
		glm::vec2 size;

		std::vector<drawBatch> batchList;

		Shader* shader;
		//
};