#pragma once

#include <HUD/Layer.h>
#include <HUD/WidgetVirtual.h>
#include <HUD/WidgetLabel.h>
#include <HUD/WidgetImage.h>
#include <HUD/WidgetConsole.h>
#include <HUD/WidgetBoard.h>
#include <HUD/WidgetRadioButton.h>

#include <Utiles/Parser/Variant.h>


class WidgetLoader
{
	public:
		//	Public functions
		static WidgetVirtual* deserialize(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static Layer* deserialize(const Variant& variant, const std::string& variantName, std::map<std::string, WidgetVirtual*>& association, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		//

	protected:
		//	Protected functions
		static WidgetBoard* deserializeBoard(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static WidgetImage* deserializeImage(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static WidgetLabel* deserializeLabel(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static WidgetConsole* deserializeConsole(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static WidgetRadioButton* deserializeRadioButton(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);

		static void tryLoadSizes(WidgetVirtual* w, const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static void tryLoadPositions(WidgetVirtual* w, const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);
		static void tryLoadColors(WidgetVirtual* w, const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount);

		static void printError(const std::string& varName, const std::string& error, std::string& errorHeader, const std::string& errorIndent);
		static bool tryExtractVector(const Variant& variant, float* vector, const int& vectorSize, std::string& error);
		//
};


