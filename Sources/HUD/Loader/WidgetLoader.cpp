#include "WidgetLoader.h"

#include <Utiles/Parser/Reader.h>
#include <Utiles/ConsoleColor.h>


//	Public functions
WidgetVirtual* WidgetLoader::deserialize(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	if (variant.getType() != Variant::MAP)
	{
		errorCount++;
		printError(variantName, "is not a map", errorHeader, errorIndent);
		return nullptr;
	}

	try
	{
		std::string type = variant["type"].toString();
		WidgetVirtual* widget = nullptr;

		if (type == "LABEL")
			widget = deserializeLabel(variant, variantName, errorHeader, errorIndent, errorCount);
		else if (type == "BOARD")
			widget = deserializeBoard(variant, variantName, errorHeader, errorIndent, errorCount);
		else if (type == "IMAGE")
			widget = deserializeImage(variant, variantName, errorHeader, errorIndent, errorCount);
		else if (type == "CONSOLE")
			widget = deserializeConsole(variant, variantName, errorHeader, errorIndent, errorCount);
		else if (type == "RADIO_BUTTON")
			widget = deserializeRadioButton(variant, variantName, errorHeader, errorIndent, errorCount);
		else
			printError(variantName, "has unknown type", errorHeader, errorIndent);

		return widget;
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has no string field \"type\"", errorHeader, errorIndent);
	}

	return nullptr;
}
Layer* WidgetLoader::deserialize(const Variant& variant, const std::string& variantName, std::map<std::string, WidgetVirtual*>& association, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	if (variant.getType() != Variant::MAP)
	{
		errorCount++;
		printError(variantName, "is nor a map", errorHeader, errorIndent);
		return nullptr;
	}

	Layer* layer = new Layer();
	try // layer.size
	{
		const Variant& v = variant["size"];
		if (v.getType() == Variant::DOUBLE)
			layer->setSize((float)v.toDouble());
		else if (v.getType() == Variant::FLOAT)
			layer->setSize(v.toFloat());
		else if (v.getType() == Variant::INT)
			layer->setSize((float)v.toInt());
		else
		{
			errorCount++;
			printError(variantName, "has none float field \"size\"", errorHeader, errorIndent);
			layer->setSize(1.f);
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none field \"size\"", errorHeader, errorIndent);
	}

	try // layer.config
	{
		layer->setConfiguration((uint8_t)variant["config"].toInt());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"config\"", errorHeader, errorIndent);
	}

	vec4f vector = vec4f::zero;
	std::string error;
	try // layer.screenPosition
	{
		if (tryExtractVector(variant["screenPosition"], &vector.x, 3, error))
			layer->setScreenPosition(vector);
		else
		{
			errorCount++;
			printError(variantName + ".screenPosition ", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none array field \"screenPosition\"", errorHeader, errorIndent);
	}

	try // layer.targetPosition
	{
		if (tryExtractVector(variant["targetPosition"], &vector.x, 3, error))
			layer->setTargetPosition(vector);
		else
		{
			errorCount++;
			printError(variantName + ".targetPosition ", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none array field \"targetPosition\"", errorHeader, errorIndent);
	}

	try // layer.eulerAngle
	{
		if (tryExtractVector(variant["eulerAngle"], &vector.x, 3, error))
			layer->setOrientation(vector.x, vector.y, vector.z);
		else
			printError(variantName + ".eulerAngle ", error, errorHeader, errorIndent);
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none array field \"eulerAngle\"", errorHeader, errorIndent);
	}

	const Variant* children = nullptr;
	try // layer.children
	{
		children = &variant["children"];
		if (children->getType() != Variant::MAP)
		{
			errorCount++;
			printError(variantName + ".children", "is not a map", errorHeader, errorIndent);
			children = nullptr;
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none map field \"children\"", errorHeader, errorIndent);
	}
	if (!children)
	{
		delete layer;
		return nullptr;
	}

	std::vector<std::string> names;
	bool hasAssociation = false;
	for (auto it = children->getMap().begin(); it != children->getMap().end(); it++)
	{
		const Variant& child = it->second;
		const std::string& name = it->first;
		
		if (child.getType() != Variant::MAP)
		{
			errorCount++;
			printError(variantName + "." + name, "is not a map", errorHeader, errorIndent);
			continue;
		}

		WidgetVirtual* widget = deserialize(child, variantName + "." + name, errorHeader, errorIndent, errorCount);

		if (widget && name.find("unknown_") == std::string::npos)
		{
			association[name] = widget;
			hasAssociation = true;
		}

		if (widget)
			layer->addChild(widget);
	}
	if (hasAssociation)
	{
		std::stable_sort(layer->getChildrenList().begin(), layer->getChildrenList().end(), [](const WidgetVirtual* a, const WidgetVirtual* b)
			{
				return ((int)a->getType() < (int)b->getType());
			});
	}

	if (errorHeader.empty() && errorCount != 0)
	{
		for (unsigned int i = 0; i < names.size(); i++)
			association.erase(names[i]);
		std::vector<WidgetVirtual*>& children = layer->getChildrenList();
		for (unsigned int i = 0; i < children.size(); i++)
			delete children[i];
		delete layer;
		return nullptr;
	}

	return layer;
}
//

//	Protected functions
WidgetBoard* WidgetLoader::deserializeBoard(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	WidgetBoard* board = new WidgetBoard();

	try // board.config
	{
		board->setConfiguration((uint8_t)variant["config"].toInt());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"config\"", errorHeader, errorIndent);
	}

	tryLoadSizes(board, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadPositions(board, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadColors(board, variant, variantName, errorHeader, errorIndent, errorCount);

	uint8_t cornerConfig = 0;
	float borderWidth = 0.f;
	float borderThickness = 0.f;

	try // board.cornerConfig
	{
		cornerConfig = (uint8_t)variant["cornerConfig"].toInt();
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"cornerConfig\"", errorHeader, errorIndent);
	}

	try // board.borderThickness
	{
		const Variant& v = variant["borderThickness"];
		if (v.getType() == Variant::DOUBLE)
			borderThickness = (float)v.toDouble();
		else if (v.getType() == Variant::FLOAT)
			borderThickness = v.toFloat();
		else if (v.getType() == Variant::INT)
			borderThickness = (float)v.toInt();
		else
			printError(variantName, "has none float field \"borderThickness\"", errorHeader, errorIndent);
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"borderThickness\"", errorHeader, errorIndent);
	}

	try // board.borderWidth
	{
		const Variant& v = variant["borderWidth"];
		if (v.getType() == Variant::DOUBLE)
			borderWidth = (float)v.toDouble();
		else if (v.getType() == Variant::FLOAT)
			borderWidth = v.toFloat();
		else if (v.getType() == Variant::INT)
			borderWidth = (float)v.toInt();
		else
			printError(variantName, "has none float field \"borderWidth\"", errorHeader, errorIndent);
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"borderWidth\"", errorHeader, errorIndent);
	}

	board->initialize(borderThickness, borderWidth, cornerConfig);

	return board;
}
WidgetImage* WidgetLoader::deserializeImage(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	WidgetImage* image = new WidgetImage();

	try // image.config
	{
		image->setConfiguration((uint8_t)variant["config"].toInt());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"config\"", errorHeader, errorIndent);
	}

	tryLoadSizes(image, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadPositions(image, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadColors(image, variant, variantName, errorHeader, errorIndent, errorCount);

	try // image.texture
	{
		image->setTexture(variant["texture"].toString());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none string field \"texture\"", errorHeader, errorIndent);
	}

	image->initialize();

	return image;
}
WidgetLabel* WidgetLoader::deserializeLabel(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	WidgetLabel* label = new WidgetLabel();

	try // label.config
	{
		label->setConfiguration((uint8_t)variant["config"].toInt());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"config\"", errorHeader, errorIndent);
	}

	try // label.sizeChar
	{
		const Variant& v = variant["sizeChar"];
		if (v.getType() == Variant::DOUBLE)
			label->setSizeChar((float)v.toDouble());
		else if (v.getType() == Variant::FLOAT)
			label->setSizeChar(v.toFloat());
		else if (v.getType() == Variant::INT)
			label->setSizeChar((float)v.toInt());
		else
		{
			errorCount++;
			printError(variantName, "has none float field \"sizeChar\"", errorHeader, errorIndent);
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none float field \"sizeChar\"", errorHeader, errorIndent);
	}

	try // label.font
	{
		label->setFont(variant["font"].toString());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none string field \"font\"", errorHeader, errorIndent);
	}

	tryLoadSizes(label, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadPositions(label, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadColors(label, variant, variantName, errorHeader, errorIndent, errorCount);

	uint8_t textConfig = 0;
	try // label.textConfiguration
	{
		textConfig = variant["textConfiguration"].toInt();
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"textConfiguration\"", errorHeader, errorIndent);
	}
	try // label.text
	{
		label->initialize(variant["text"].toString(), textConfig);
	}
	catch (const std::exception&)
	{
		label->initialize("", textConfig);
		printError(variantName, "has invalid or none string field \"text\"", errorHeader, errorIndent);
	}

	return label;
}
WidgetConsole* WidgetLoader::deserializeConsole(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	WidgetConsole* console = new WidgetConsole();

	try // console.config
	{
		console->setConfiguration((uint8_t)variant["config"].toInt());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"config\"", errorHeader, errorIndent);
	}

	tryLoadSizes(console, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadPositions(console, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadColors(console, variant, variantName, errorHeader, errorIndent, errorCount);

	try // console.sizeChar
	{
		const Variant& v = variant["sizeChar"];
		if (v.getType() == Variant::DOUBLE)
			console->setSizeChar((float)v.toDouble());
		else if (v.getType() == Variant::FLOAT)
			console->setSizeChar(v.toFloat());
		else if (v.getType() == Variant::INT)
			console->setSizeChar((float)v.toInt());
		else
		{
			errorCount++;
			printError(variantName, "has none float field \"sizeChar\"", errorHeader, errorIndent);
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none float field \"sizeChar\"", errorHeader, errorIndent);
	}

	try // console.margin
	{
		const Variant& v = variant["margin"];
		if (v.getType() == Variant::DOUBLE)
			console->setMargin((float)v.toDouble());
		else if (v.getType() == Variant::FLOAT)
			console->setMargin(v.toFloat());
		else if (v.getType() == Variant::INT)
			console->setMargin((float)v.toInt());
		else
		{
			printError(variantName, "has none float field \"margin\"", errorHeader, errorIndent);
			console->setMargin(0.1f);
		}
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none float field \"margin\"", errorHeader, errorIndent);
	}

	try // console.font
	{
		console->setFont(variant["font"].toString());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none string field \"font\"", errorHeader, errorIndent);
	}

	uint8_t cornerConfig = 0;
	float borderWidth = 0.1f;
	float borderThickness = 0.02f;

	try // console.cornerConfig
	{
		cornerConfig = (uint8_t)variant["cornerConfig"].toInt();
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"cornerConfig\"", errorHeader, errorIndent);
	}

	try // console.borderThickness
	{
		const Variant& v = variant["borderThickness"];
		if (v.getType() == Variant::DOUBLE)
			borderThickness = (float)v.toDouble();
		else if (v.getType() == Variant::FLOAT)
			borderThickness = v.toFloat();
		else if (v.getType() == Variant::INT)
			borderThickness = (float)v.toInt();
		else
			printError(variantName, "has none float field \"borderThickness\"", errorHeader, errorIndent);
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"borderThickness\"", errorHeader, errorIndent);
	}

	try // console.borderWidth
	{
		const Variant& v = variant["borderWidth"];
		if (v.getType() == Variant::DOUBLE)
			borderWidth = (float)v.toDouble();
		else if (v.getType() == Variant::FLOAT)
			borderWidth = v.toFloat();
		else if (v.getType() == Variant::INT)
			borderWidth = (float)v.toInt();
		else
			printError(variantName, "has none float field \"borderWidth\"", errorHeader, errorIndent);
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"borderWidth\"", errorHeader, errorIndent);
	}

	try // console.text
	{
		const Variant& v = variant["text"];
		std::string msg = v.toString();
		if (!msg.empty())
			console->append(msg);
	}
	catch (const std::exception&) {}

	console->initialize(borderThickness, borderWidth, cornerConfig);

	return console;
}
WidgetRadioButton* WidgetLoader::deserializeRadioButton(const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	WidgetRadioButton* button = new WidgetRadioButton();

	try // button.config
	{
		button->setConfiguration((uint8_t)variant["config"].toInt());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none int field \"config\"", errorHeader, errorIndent);
	}

	tryLoadSizes(button, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadPositions(button, variant, variantName, errorHeader, errorIndent, errorCount);
	tryLoadColors(button, variant, variantName, errorHeader, errorIndent, errorCount);

	try // button.sizeChar
	{
		const Variant& v = variant["sizeChar"];
		if (v.getType() == Variant::DOUBLE)
			button->setSizeChar((float)v.toDouble());
		else if (v.getType() == Variant::FLOAT)
			button->setSizeChar(v.toFloat());
		else if (v.getType() == Variant::INT)
			button->setSizeChar((float)v.toInt());
		else
		{
			errorCount++;
			printError(variantName, "has none float field \"sizeChar\"", errorHeader, errorIndent);
		}
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none float field \"sizeChar\"", errorHeader, errorIndent);
	}

	try // button.onTexture
	{
		button->setTextureOn(variant["onTexture"].toString());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none string field \"onTexture\"", errorHeader, errorIndent);
	}

	try // button.offTexture
	{
		button->setTextureOff(variant["offTexture"].toString());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none string field \"offTexture\"", errorHeader, errorIndent);
	}

	try // button.font
	{
		button->setFont(variant["font"].toString());
	}
	catch (const std::exception&)
	{
		errorCount++;
		printError(variantName, "has invalid or none string field \"font\"", errorHeader, errorIndent);
	}

	uint8_t textConfig = 0;
	std::string text = "";

	try // button.text
	{
		text = variant["text"].toString();
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none string field \"text\"", errorHeader, errorIndent);
	}

	try // button.textConfiguration
	{
		textConfig = (uint8_t)variant["textConfiguration"].toInt();
	}
	catch (const std::exception&)
	{
		printError(variantName, "has invalid or none int field \"textConfiguration\"", errorHeader, errorIndent);
	}

	button->initialize(text, textConfig);

	return button;
}



void WidgetLoader::tryLoadSizes(WidgetVirtual* w, const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	vec2f size = vec2f::zero;
	std::string error;
	bool oneLoaded = false;

	try // label.sizeActive
	{
		if (tryExtractVector(variant["sizeActive"], &size.x, 2, error))
		{
			w->setSize(size, WidgetVirtual::State::ACTIVE);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".sizeActive", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}
	
	try // label.sizeCurrent
	{
		if (tryExtractVector(variant["sizeCurrent"], &size.x, 2, error))
		{
			w->setSize(size, WidgetVirtual::State::CURRENT);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".sizeCurrent", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.sizeDefault
	{
		if (tryExtractVector(variant["sizeDefault"], &size.x, 2, error))
		{
			w->setSize(size, WidgetVirtual::State::DEFAULT);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".sizeDefault", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.sizeHover
	{
		if (tryExtractVector(variant["sizeHover"], &size.x, 2, error))
		{
			w->setSize(size, WidgetVirtual::State::HOVER);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".sizeHover", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.sizeAll
	{
		if (tryExtractVector(variant["sizeAll"], &size.x, 2, error))
		{
			if (oneLoaded)
				printError(variantName + ".sizeAll", "another size tag was found (all will be erase)", errorHeader, errorIndent);

			w->setSize(size, WidgetVirtual::State::ALL);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".sizeAll", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	if (!oneLoaded)
		printError(variantName, "no size tag was found", errorHeader, errorIndent);
}
void WidgetLoader::tryLoadPositions(WidgetVirtual* w, const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	vec4f position = vec4f::zero;
	std::string error;
	bool oneLoaded = false;

	try // label.positionActive
	{
		if (tryExtractVector(variant["positionActive"], &position.x, 3, error))
		{
			w->setPosition(position, WidgetVirtual::State::ACTIVE);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".positionActive", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.positionCurrent
	{
		if (tryExtractVector(variant["positionCurrent"], &position.x, 3, error))
		{
			w->setPosition(position, WidgetVirtual::State::CURRENT);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".positionCurrent", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.positionDefault
	{
		if (tryExtractVector(variant["positionDefault"], &position.x, 3, error))
		{
			w->setPosition(position, WidgetVirtual::State::DEFAULT);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".positionDefault", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.positionHover
	{
		if (tryExtractVector(variant["positionHover"], &position.x, 3, error))
		{
			w->setPosition(position, WidgetVirtual::State::HOVER);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".positionHover", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.positionAll
	{
		if (tryExtractVector(variant["positionAll"], &position.x, 3, error))
		{
			if (oneLoaded)
				printError(variantName + ".positionAll", "another position tag was found (all will be erase)", errorHeader, errorIndent);

			w->setPosition(position, WidgetVirtual::State::ALL);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".positionAll", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	if (!oneLoaded)
	{
		errorCount++;
		printError(variantName, "no position tag was found", errorHeader, errorIndent);
	}
}
void WidgetLoader::tryLoadColors(WidgetVirtual* w, const Variant& variant, const std::string& variantName, std::string& errorHeader, const std::string& errorIndent, int& errorCount)
{
	vec4f color = vec4f::zero;
	std::string error;
	bool oneLoaded = false;

	try // label.colorActive
	{
		if (tryExtractVector(variant["colorActive"], &color.x, 4, error))
		{
			w->setColor(color, WidgetVirtual::State::ACTIVE);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".colorActive", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.colorCurrent
	{
		if (tryExtractVector(variant["colorCurrent"], &color.x, 4, error))
		{
			w->setColor(color, WidgetVirtual::State::CURRENT);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".colorCurrent", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.colorDefault
	{
		if (tryExtractVector(variant["colorDefault"], &color.x, 4, error))
		{
			w->setColor(color, WidgetVirtual::State::DEFAULT);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".colorDefault", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.colorHover
	{
		if (tryExtractVector(variant["colorHover"], &color.x, 4, error))
		{
			w->setColor(color, WidgetVirtual::State::HOVER);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".colorHover", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	try // label.colorAll
	{
		if (tryExtractVector(variant["colorAll"], &color.x, 4, error))
		{
			if (oneLoaded)
				printError(variantName + ".colorAll", "another color tag was found (all will be erase)", errorHeader, errorIndent);

			w->setColor(color, WidgetVirtual::State::ALL);
			oneLoaded = true;
		}
		else
		{
			errorCount++;
			printError(variantName + ".colorAll", error, errorHeader, errorIndent);
		}
	}
	catch (const std::exception&) {}

	if (!oneLoaded)
		printError(variantName, "no color tag was found", errorHeader, errorIndent);
}



void WidgetLoader::printError(const std::string& varName, const std::string& error, std::string& errorHeader, const std::string& errorIndent)
{
	if (!errorHeader.empty())
	{
		std::cerr << ConsoleColor::getColorString(ConsoleColor::Color::RED) << errorHeader << std::flush;
		std::cerr << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

		errorHeader.clear();
	}
	std::cerr << errorIndent << varName << " " << error << std::endl;
}
bool WidgetLoader::tryExtractVector(const Variant& variant, float* vector, const int& vectorSize, std::string& error)
{
	if (variant.getType() != Variant::ARRAY)
	{
		error = "is not an array";
		return false;
	}

	Variant::ArrayType& array = variant.getArray();
	if (array.size() != vectorSize)
	{
		error = "has a wrong size";
		return false;
	}

	for (int i = 0; i < vectorSize; i++)
	{
		if (array[i].getType() == Variant::DOUBLE)
			vector[i] = (float)array[i].toDouble();
		else if (array[i].getType() == Variant::FLOAT)
			vector[i] = array[i].toFloat();
		else if(array[i].getType() == Variant::INT)
			vector[i] = (float)array[i].toInt();
		else
		{
			error = "contain not float value " + std::to_string((int)array[i].getType());
			return false;
		}
	}
	return true;
}

//
