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
		enum OrphanFlags
		{
			VISIBLE = 1 << 4,
			RESPONSIVE = 1 << 5,
			NEED_UPDATE = 1<<6
		};
		enum State
		{
			DEFAULT = 0 << 0,
			HOVER = 1 << 0,
			ACTIVE = 2 << 0,
			STATE_MASK = 0x03 << 0,
			
			CURRENT = 3 << 0,
			ALL = 4 << 0
		};
		struct DrawBatch
		{
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
		virtual void draw(Shader* s, uint8_t& stencilMask);
		virtual void update(const float& elapseTime);
		virtual bool intersect(const glm::mat4& base, const glm::vec3& ray, const glm::vec3 origin, glm::vec3& result);
		virtual bool mouseEvent(const glm::vec3& eventLocation, const bool& clicked);

		virtual void setString(const std::string& s);
		virtual std::string getString() const;
		virtual std::stringstream* getStream();
		//

		//  Set/get functions
		virtual void setState(State state);
		virtual void setSize(const glm::vec2& s, const State& state = CURRENT);
		virtual void setPosition(const glm::vec3& p, const State& state = CURRENT);
		virtual void setColor(const glm::vec4& c, const State& state = CURRENT);
		void setVisibility(const bool& visible);
		void setResponsive(const bool& responsive);
		void setTexture(const std::string& textureName);
		void setShader(const std::string& shaderName);


		WidgetType getType() const;
		State getState() const;
		glm::vec2 getSize(State state = CURRENT);
		glm::vec3 getPosition(State state = CURRENT);
		glm::vec4 getColor(const unsigned int& index, State state = CURRENT);
		Shader* getShader() const;
		Texture* getTexture() const;
		bool isVisible() const;
		bool isResponsive() const;
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
		uint8_t configuration, lastConfiguration;
		std::map<State, glm::vec2> sizes;
		std::map<State, glm::vec3> positions;
		std::map<State, glm::vec4> colors;

		std::vector<DrawBatch> batchList;

		Shader* shader;
		Texture* texture;
		//
};