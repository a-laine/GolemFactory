#pragma once

#include <fstream>

#include <glm/glm.hpp>

#include "../WidgetVirtual.h"
#include "../WidgetContainer.h"

#include "../WidgetLabel.h"
#include "../WidgetImage.h"
#include "../WidgetConsole.h"
#include "../WidgetBoard.h"
#include "../WidgetRadioButton.h"


#include "Utiles/Parser/Variant.h"

class WidgetSaver
{
	public:
		//	Public functions
		static Variant serialize(WidgetVirtual* w, const std::map<std::string, WidgetVirtual*>& association);
		//

	protected:
		//	Protected functions
		static void serializeBoard(WidgetBoard* v, Variant& root);
		static void serializeImage(WidgetImage* v, Variant& root);
		static void serializeLabel(WidgetLabel* v, Variant& root);
		static void serializeConsole(WidgetConsole* v, Variant& root);
		static void serializeRadioButton(WidgetRadioButton* v, Variant& root);
		//
};
