#pragma once

#include "WidgetLabel.h"

class WidgetRadioButton : public WidgetLabel
{
	public:
		//  Default
		WidgetRadioButton(const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetRadioButton();
		//

		//	Public functions
		void serialize(std::ostream& out, const int& indentation, std::string name, int& number);
		void update(const float& elapseTime);
		void initialize(const std::string& txt, uint8_t textConfig = AlignmentMode::LEFT);
		void draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model);
		bool intersect(const glm::mat4& base, const glm::vec3& ray);
		bool mouseEvent(const glm::mat4& base, const glm::vec3& ray, const float& parentscale, const bool& clicked);
		//

		//	Set / get functions
		void setTextureOn(const std::string& name);
		void setTextureOff(const std::string& name);

		void setBoolean(const bool& b);
		bool getBoolean() const;
		//

	protected:
		//	Protected functions
		void updateBuffers();
		//

		//  Attributes
		Texture* onTexture;
		Texture* offTexture;

		bool checked;
		bool lastEventState;
		//
};