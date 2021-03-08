#pragma once

#include <fstream>

#include <glm/glm.hpp>

#include <HUD/Layer.h>
#include <HUD/WidgetVirtual.h>
#include <HUD/WidgetLabel.h>
#include <HUD/WidgetImage.h>
#include <HUD/WidgetConsole.h>
#include <HUD/WidgetBoard.h>
#include <HUD/WidgetRadioButton.h>

#include <Utiles/Parser/Variant.h>

class WidgetSaver
{
	public:
		//	Public functions
		static Variant serialize(WidgetVirtual* w, std::map<std::string, WidgetVirtual*>& association, int& unknownIndex);
		static Variant serialize(Layer* w, std::map<std::string, WidgetVirtual*>& association, int& unknownIndex);
		//

	protected:
		//	Protected functions
		static void serializeBoard(WidgetBoard* v, Variant& root);
		static void serializeImage(WidgetImage* v, Variant& root);
		static void serializeLabel(WidgetLabel* v, Variant& root);
		static void serializeConsole(WidgetConsole* v, Variant& root);
		static void serializeRadioButton(WidgetRadioButton* v, Variant& root);

		static Variant formatText(std::string txt);
		//
};
