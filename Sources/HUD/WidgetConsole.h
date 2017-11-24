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
		void update(const float& elapseTime);
		void initialize(const float& borderThickness, const float& borderWidth, const uint8_t& corner = 0x00);
		void draw(Shader* s, uint8_t& stencilMask);
		bool intersect(const glm::mat4& base, const glm::vec3& ray, const glm::vec3 origin, glm::vec3& result);
		//

		//	Set / get functions
		void setFont(const std::string& fontName);
		void setSizeChar(const float& f);

		std::stringstream* getStream();
		Font* getFont() const;
		float getSizeChar() const;
		//

	protected:
		//	Protected functions
		void updateBuffers(const bool& firstInit = false);
		//

		//  Attributes
		Font* font;
		std::stringstream text;
		std::vector<float> linesLength;
		float sizeChar;
		//
};