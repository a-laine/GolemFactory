#pragma once


#include "WidgetVirtual.h"

class WidgetImage : public WidgetVirtual
{
	friend class WidgetSaver;

	public:
		//  Default
		WidgetImage(const std::string& textureName= "default", const uint8_t& config = VISIBLE, const std::string& shaderName = "defaultWidget");
		virtual ~WidgetImage();
		//

		//	Public functions
		void initialize();
		//
};