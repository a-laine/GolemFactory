#pragma once

#include <iostream>

#include <glm/glm.hpp>

#include "Resources/ResourceManager.h"

class WidgetVirtual
{
	public:
		//  Miscellaneous
		enum WidgetType
		{
			VIRTUAL = 0,
			BOARD,
			IMAGE,
			LABEL
		};
		enum Flags
		{
			VISIBLE = 1 << 6
		};
		struct DrawBatch
		{
			glm::vec4 color;
			GLuint  vao,
				verticesBuffer,
				texturesBuffer,
				facesBuffer;
			std::vector<glm::vec3> vertices;
			std::vector<glm::vec2> textures;
			std::vector<unsigned short> faces;
		};
		//

		//  Default
		WidgetVirtual(const WidgetType& t = VIRTUAL, const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetVirtual();
		//

		//	Public functions
		virtual void initialize(int VBOtype = GL_STATIC_DRAW);
		virtual void draw(Shader* s, uint8_t& stencilMask);
		virtual void update(const float& elapseTime);
		virtual bool intersect(const glm::mat4& base, const glm::vec3& ray, const glm::vec3 origin, glm::vec3& result);

		virtual void setString(const std::string& s);
		virtual std::string getString();
		virtual std::stringstream* getStream();
		//

		//  Set/get functions
		virtual void setSize(const glm::vec2& s);
		virtual void setPosition(const glm::vec3& p);
		void setVisibility(const bool& visible);
		void setTexture(const std::string& textureName);
		void setShader(const std::string& shaderName);


		WidgetType getType() const;
		glm::vec3 getPosition() const;
		glm::vec4* getColor(const unsigned int& index);
		Shader* getShader() const;
		Texture* getTexture() const;
		bool isVisible() const;
		virtual unsigned int getNumberFaces() const;
		//

	protected:
		//	Protected functions
		void drawClippingShape(const unsigned int& batchIndex, const bool& enableClipping, Shader* s, uint8_t& stencilMask);
		void initializeVBOs(int VBOtype = GL_STATIC_DRAW);
		void initializeVAOs();
		//

		//  Attributes
		WidgetType type;
		uint8_t configuration;
		glm::vec3 position;
		glm::vec2 size;

		std::vector<DrawBatch> batchList;

		Shader* shader;
		Texture* texture;
		
		static float const PICKING_MARGIN;
		//
};