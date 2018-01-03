#include "WidgetSaver.h"
#include "Utiles/ToolBox.h"


//	Public functions
Variant WidgetSaver::serialize(WidgetVirtual* w, std::map<std::string, WidgetVirtual*>& association, int& unknownIndex)
{
	Variant rootVariant;   rootVariant.createMap();

	//	write type
	switch (w->type)
	{
		case WidgetVirtual::VIRTUAL:		rootVariant.insert("type", Variant("VIRTUAL"));		break;
		case WidgetVirtual::BOARD:			rootVariant.insert("type", Variant("BOARD"));		break;
		case WidgetVirtual::IMAGE:			rootVariant.insert("type", Variant("IMAGE"));		break;
		case WidgetVirtual::LABEL:			rootVariant.insert("type", Variant("LABEL"));		break;
		case WidgetVirtual::CONSOLE:		rootVariant.insert("type", Variant("CONSOLE"));		break;
		case WidgetVirtual::RADIO_BUTTON:	rootVariant.insert("type", Variant("RADIO_BUTTON"));break;
		default:							rootVariant.insert("type", Variant("UNKNOWN"));		break;
	}

	//	write configuration
	rootVariant.insert("config", Variant((int)w->configuration));

	//	write sizes
	if (w->sizes[WidgetVirtual::DEFAULT] == w->sizes[WidgetVirtual::HOVER] && w->sizes[WidgetVirtual::DEFAULT] == w->sizes[WidgetVirtual::ACTIVE] && w->sizes[WidgetVirtual::DEFAULT] == w->sizes[WidgetVirtual::CURRENT])
		rootVariant.insert("sizeAll", ToolBox::getFromVec2(w->sizes[WidgetVirtual::DEFAULT]));
	else
	{
		rootVariant.insert("sizeDefault", ToolBox::getFromVec2(w->sizes[WidgetVirtual::DEFAULT]));
		rootVariant.insert("sizeHover", ToolBox::getFromVec2(w->sizes[WidgetVirtual::HOVER]));
		rootVariant.insert("sizeActive", ToolBox::getFromVec2(w->sizes[WidgetVirtual::ACTIVE]));
		rootVariant.insert("sizeCurrent", ToolBox::getFromVec2(w->sizes[WidgetVirtual::CURRENT]));
	}

	//	write positions
	if (w->positions[WidgetVirtual::DEFAULT] == w->positions[WidgetVirtual::HOVER] && w->positions[WidgetVirtual::DEFAULT] == w->positions[WidgetVirtual::ACTIVE] && w->positions[WidgetVirtual::DEFAULT] == w->positions[WidgetVirtual::CURRENT])
		rootVariant.insert("positionAll", ToolBox::getFromVec3(w->positions[WidgetVirtual::DEFAULT]));
	else
	{
		rootVariant.insert("positionDefault", ToolBox::getFromVec3(w->positions[WidgetVirtual::DEFAULT]));
		rootVariant.insert("positionHover", ToolBox::getFromVec3(w->positions[WidgetVirtual::HOVER]));
		rootVariant.insert("positionActive", ToolBox::getFromVec3(w->positions[WidgetVirtual::ACTIVE]));
		rootVariant.insert("positionCurrent", ToolBox::getFromVec3(w->positions[WidgetVirtual::CURRENT]));
	}

	//	write colors
	if (w->colors[WidgetVirtual::DEFAULT] == w->colors[WidgetVirtual::HOVER] && w->colors[WidgetVirtual::DEFAULT] == w->colors[WidgetVirtual::ACTIVE] && w->colors[WidgetVirtual::DEFAULT] == w->colors[WidgetVirtual::CURRENT])
		rootVariant.insert("colorAll", ToolBox::getFromVec4(w->colors[WidgetVirtual::DEFAULT]));
	else
	{
		rootVariant.insert("colorDefault", ToolBox::getFromVec4(w->colors[WidgetVirtual::DEFAULT]));
		rootVariant.insert("colorHover", ToolBox::getFromVec4(w->colors[WidgetVirtual::HOVER]));
		rootVariant.insert("colorActive", ToolBox::getFromVec4(w->colors[WidgetVirtual::ACTIVE]));
		rootVariant.insert("colorCurrent", ToolBox::getFromVec4(w->colors[WidgetVirtual::CURRENT]));
	}

	//	write shader and texture name
	if (w->shader && w->shader->name != "defaultWidget")
		rootVariant.insert("shader", Variant(w->shader->name));
	if (w->texture)
		rootVariant.insert("texture", Variant(w->texture->name));

	//	derivate type
	switch (w->type)
	{
		case WidgetVirtual::BOARD:
			if (WidgetBoard* board = dynamic_cast<WidgetBoard*>(w))
				serializeBoard(board, rootVariant);
			break;
		case WidgetVirtual::IMAGE:
			if (WidgetImage* image = dynamic_cast<WidgetImage*>(w))
				serializeImage(image, rootVariant);
			break;
			break;
		case WidgetVirtual::LABEL:
			if (WidgetLabel* label = dynamic_cast<WidgetLabel*>(w))
				serializeLabel(label, rootVariant);
			break;
		case WidgetVirtual::CONSOLE:
			if (WidgetConsole* console = dynamic_cast<WidgetConsole*>(w))
				serializeConsole(console, rootVariant);
			break;
		case WidgetVirtual::RADIO_BUTTON:
			if (WidgetRadioButton* radiobutton = dynamic_cast<WidgetRadioButton*>(w))
				serializeRadioButton(radiobutton, rootVariant);
			break;
		default: break;
	}

	//	children
	if (!w->children.empty())
	{
		rootVariant.insert("children", Variant::MapType());
		for (unsigned int i = 0; i < w->children.size(); ++i)
		{
			std::string name = "unknown_" + std::to_string(++unknownIndex);
			for (std::map<std::string, WidgetVirtual*>::iterator it = association.begin(); it != association.end(); it++)
			{
				if (w->children[i] == it->second)
				{
					name = it->first;
					break;
				}
			}
			Variant child = serialize(w->children[i], association, unknownIndex);
			rootVariant.getMap()["children"].insert(name, child);
		}
	}

	//	end
	return rootVariant;
}
Variant WidgetSaver::serialize(Layer* l, std::map<std::string, WidgetVirtual*>& association, int& unknownIndex)
{
	Variant rootVariant;   rootVariant.createMap();

	//	write configuration
	rootVariant.insert("config", Variant((int)l->configuration));
	rootVariant.insert("screenPosition", ToolBox::getFromVec3(l->screenPosition));
	rootVariant.insert("targetPosition", ToolBox::getFromVec3(l->targetPosition));
	rootVariant.insert("eulerAngle", ToolBox::getFromVec3(l->eulerAngle));
	rootVariant.insert("size", Variant(l->size));

	//	children
	if (!l->children.empty())
	{
		rootVariant.insert("children", Variant::MapType());
		for (unsigned int i = 0; i < l->children.size(); ++i)
		{
			std::string name = "unknown_" + std::to_string(++unknownIndex);
			for (std::map<std::string, WidgetVirtual*>::iterator it = association.begin(); it != association.end(); it++)
			{
				if (l->children[i] == it->second)
				{
					name = it->first;
					break;
				}
			}
			Variant child = serialize(l->children[i], association, unknownIndex);
			rootVariant.getMap()["children"].insert(name, child);
		}
	}

	//	end
	return rootVariant;
}
//


//	Protected functions
void WidgetSaver::serializeBoard(WidgetBoard* w, Variant& root)
{
	root.insert("cornerConfig", Variant((int)w->cornerConfiguration));
	root.insert("borderWidth", Variant(w->borderWidth));
	root.insert("borderThickness", Variant(w->borderThickness));
}
void WidgetSaver::serializeImage(WidgetImage* w, Variant& root)
{

}
void WidgetSaver::serializeLabel(WidgetLabel* w, Variant& root)
{
	//	special board attributes
	root.insert("sizeChar", Variant(w->sizeChar));
	root.insert("textConfiguration", Variant((int)w->textConfiguration));
	if (w->font) root.insert("font", Variant(w->font->name));
	root.insert("text", formatText(w->text));
}
void WidgetSaver::serializeConsole(WidgetConsole* w, Variant& root)
{
	//	special board attributes
	root.insert("cornerConfig", Variant((int)w->cornerConfiguration));
	root.insert("borderWidth", Variant(w->borderWidth));
	root.insert("borderThickness", Variant(w->borderThickness));
	root.insert("margin", Variant(w->margin));
	root.insert("sizeChar", Variant(w->sizeChar));
	if (w->font) root.insert("font", Variant(w->font->name));
	root.insert("text", formatText(w->text));
}
void WidgetSaver::serializeRadioButton(WidgetRadioButton* w, Variant& root)
{
	//	special board attributes
	root.insert("sizeChar", Variant(w->sizeChar));
	root.insert("textConfiguration", Variant((int)w->textConfiguration));
	if (w->font) root.insert("font", Variant(w->font->name));
	root.insert("text", formatText(w->text));
	if (w->onTexture) root.insert("onTexture", Variant(w->onTexture->name));
	if (w->offTexture) root.insert("offTexture", Variant(w->offTexture->name));
}

Variant WidgetSaver::formatText(std::string txt)
{
	for (std::string::size_type i = 0; i != std::string::npos;)
	{
		i = txt.find("\n", i);
		if (i != std::string::npos)
		{
			txt.replace(i, 1, "\\n");
			i += 2;
		}
	}
	return Variant(txt);
}
//








