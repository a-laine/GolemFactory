#pragma once

#include "WidgetVirtual.h"
#include <Resources/Font.h>

class WidgetLabel : public WidgetVirtual
{
	friend class WidgetSaver;

	public:
		//	Miscellaneous
		enum AlignmentMode
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
		enum FlagsConfig
		{
			ITALIC = 1 << 4,
			CLIPPING = 1 << 5
		};
		//

		//  Default
		WidgetLabel(const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetLabel();
		//

		//	Public functions
		virtual void update(const float& elapseTime) override;
		virtual void initialize(const std::string& txt, uint8_t textConfiguration = AlignmentMode::CENTER);
		virtual void draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model) override;
		virtual bool intersect(const glm::mat4& base, const glm::vec3& ray) override;
		
		void setString(const std::string& newText) override;
		std::string getString() const override;
		void append(const std::string& s) override;
		//

		//	Set / get functions
		void setFont(const std::string& fontName);
		void setSizeChar(const float& f);

		
		Font* getFont() const;
		float getSizeChar() const;
		//

	protected:
		//	Protected functions
		void initVBOtext();
		virtual void updateBuffers();
		void updateVBOs();
		void parseText();
		glm::vec2 getLineOrigin(const unsigned int& lineIndex, const uint8_t& textConfiguration);
		//

		//  Attributes
		Font* font;
		std::string text;
		std::vector<float> linesLength;
		uint8_t textConfiguration;
		float sizeChar;
		float updateCooldown;
		//
};