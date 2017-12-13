#pragma once


#include "WidgetBoard.h"

class WidgetConsole : public WidgetBoard
{
	public:
		//  Default
		WidgetConsole(const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetConsole();
		//

		//	Public functions
		void serialize(std::ostream& out, const int& indentation, std::string name, int& number);
		void update(const float& elapseTime);
		void initialize(const float& borderThickness, const float& borderWidth, const uint8_t& corner = 0x00);
		void draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model);
		bool intersect(const glm::mat4& base, const glm::vec3& ray);
		bool mouseEvent(const glm::mat4& base, const glm::vec3& ray, const float& parentscale, const bool& clicked);
		//

		//	Set / get functions
		void setFont(const std::string& fontName);
		void setSizeChar(const float& f);
		void setMargin(const float& f);
		void append(const std::string& s);

		Font* getFont() const;
		float getSizeChar() const;
		float getMargin() const;
		//

	protected:
		//	Protected functions
		void initVBOtext();
		void updateBuffers(const bool& firstInit = false);
		void updateTextBuffer();
		void parseText();
		//

		//  Attributes
		Font* font;
		std::string text;
		std::vector<float> linesLength;
		float sizeChar;
		float margin;
		float firstCursory;
		float elevator, elevatorLength, elevatorRange;
		//
};