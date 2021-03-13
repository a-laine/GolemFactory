#pragma once

#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <Resources/ResourceManager.h>
#include <Resources/Mesh.h>
#include <Resources/Texture.h>
#include <Resources/Shader.h>


class WidgetVirtual
{
	friend class WidgetSaver;

	public:
		//  Miscellaneous
		enum class WidgetType
		{
			VIRTUAL = 0,
			BOARD,
			CONSOLE,
			IMAGE,
			RADIO_BUTTON,
			LABEL
		};
		enum class OrphanFlags : uint8_t
		{
			VISIBLE = 1 << 4,
			RESPONSIVE = 1 << 5,
			NEED_UPDATE = 1<<6,
			SPECIAL = 1 << 7
		};
		enum class State : uint8_t
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
			GLuint vao = 0;
			GLuint verticesBuffer = 0;
			GLuint texturesBuffer = 0;
			GLuint facesBuffer = 0;

			std::vector<glm::vec3> vertices;
			std::vector<glm::vec2> textures;
			std::vector<unsigned short> faces;
		};
		//

		//  Default
		WidgetVirtual(const WidgetType& t = WidgetType::VIRTUAL, const uint8_t& config = (uint8_t)OrphanFlags::VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetVirtual();
		//

		//	Public functions
		virtual void draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model);
		virtual void update(const float& elapseTime);
		virtual bool intersect(const glm::mat4& base, const glm::vec3& ray);
		virtual bool mouseEvent(const glm::mat4& base, const glm::vec3& ray, const float& parentscale, const bool& clicked);

		virtual void setBoolean(const bool& b);
		virtual void setString(const std::string& s);
		virtual void append(const std::string& s);
		virtual std::string getString() const;
		virtual bool getBoolean() const;
		//

		//  Set/get functions
		virtual void setState(State state);
		virtual void setSize(const glm::vec2& s, const State& state = State::CURRENT);
		virtual void setPosition(const glm::vec3& p, const State& state = State::CURRENT);
		virtual void setColor(const glm::vec4& c, const State& state = State::CURRENT);
		void setVisibility(const bool& visible);
		void setResponsive(const bool& responsive);
		void setTexture(const std::string& textureName);
		void setShader(const std::string& shaderName);
		void setConfiguration(const uint8_t& config);


		WidgetType getType() const;
		State getState() const;
		glm::vec2 getSize(State state = State::CURRENT);
		glm::vec3 getPosition(State state = State::CURRENT);
		glm::vec4 getColor(const unsigned int& index, State state = State::CURRENT);
		Shader* getShader() const;
		Texture* getTexture() const;
		bool isVisible() const;
		bool isResponsive() const;
		virtual unsigned int getNumberFaces() const;
		std::vector<WidgetVirtual*>& getChildrenList();
		//

		//	Hierarchy modifiers
		virtual void addChild(WidgetVirtual* w);
		virtual bool removeChild(WidgetVirtual* w);
		//

	protected:
		//	Protected functions
		void drawClippingShape(const unsigned int& batchIndex, const bool& enableClipping, Shader* s, uint8_t& stencilMask);
		void initializeVBO(const unsigned int& batchIndex, int VBOtype = GL_STATIC_DRAW);
		void initializeVAOs();

		void indentLine(std::ostream& out, const int& i) const;
		//

		//  Attributes
		WidgetType type;
		uint8_t configuration;
		std::map<State, glm::vec2> sizes;
		std::map<State, glm::vec3> positions;
		std::map<State, glm::vec4> colors;

		std::vector<DrawBatch> batchList;
		std::vector<WidgetVirtual*> children;

		Shader* shader;
		Texture* texture;
		//
};