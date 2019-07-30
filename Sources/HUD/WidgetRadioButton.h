#pragma once

#include "WidgetLabel.h"

class WidgetRadioButton : public WidgetLabel
{
	friend class WidgetSaver;

	public:
		//  Default
		WidgetRadioButton(const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetRadioButton();
		//

		//	Public functions
		void update(const float& elapseTime) override;
		void initialize(const std::string& txt, uint8_t textConfig = AlignmentMode::LEFT) override;
		void draw(Shader* s, uint8_t& stencilMask, const glm::mat4& model) override;
		bool intersect(const glm::mat4& base, const glm::vec3& ray) override;
		bool mouseEvent(const glm::mat4& base, const glm::vec3& ray, const float& parentscale, const bool& clicked) override;
		//

		//	Set / get functions
		void setTextureOn(const std::string& name);
		void setTextureOff(const std::string& name);

		void setBoolean(const bool& b) override;
		bool getBoolean() const override;
		//

	protected:
		//	Protected functions
		void updateBuffers() override;
		//

		//  Attributes
		Texture* onTexture;
		Texture* offTexture;

		bool checked;
		bool lastEventState;
		//
};