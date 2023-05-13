#pragma once


#include "WidgetBoard.h"
#include <Resources/Font.h>

class WidgetConsole : public WidgetBoard
{
	friend class WidgetSaver;

	public:
		//  Default
		WidgetConsole(const uint8_t& config = (uint8_t)WidgetVirtual::OrphanFlags::VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetConsole();
		//

		//	Public functions
		void update(const float& elapseTime) override;
		void initialize(const float& borderThickness, const float& borderWidth, const uint8_t& corner = 0x00) override;
		void draw(Shader* s, uint8_t& stencilMask, const mat4f& model) override;
		bool intersect(const mat4f& base, const vec4f& ray) override;
		bool mouseEvent(const mat4f& base, const vec4f& ray, const float& parentscale, const bool& clicked) override;
		//

		//	Set / get functions
		void setFont(const std::string& fontName);
		void setSizeChar(const float& f);
		void setMargin(const float& f);
		void append(const std::string& s) override;

		Font* getFont() const;
		float getSizeChar() const;
		float getMargin() const;
		//

	protected:
		//	Protected functions
		void initVBOtext();
		void updateBuffers(const bool& firstInit = false) override;
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

		enum class ConsoleFlags : uint8_t
		{
			NEED_TEXT_UPDATE = (uint8_t)WidgetVirtual::OrphanFlags::SPECIAL
		};
};