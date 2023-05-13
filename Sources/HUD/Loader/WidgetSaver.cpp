#include "WidgetSaver.h"
#include <Utiles/ToolBox.h>


//	Public functions
Variant WidgetSaver::serialize(WidgetVirtual* w, std::map<std::string, WidgetVirtual*>& association, int& unknownIndex)
{
	Variant rootVariant;   rootVariant.createMap();

	//	write type
	switch (w->type)
	{
		case WidgetVirtual::WidgetType::VIRTUAL:		rootVariant.insert("type", Variant("VIRTUAL"));		break;
		case WidgetVirtual::WidgetType::BOARD:			rootVariant.insert("type", Variant("BOARD"));		break;
		case WidgetVirtual::WidgetType::IMAGE:			rootVariant.insert("type", Variant("IMAGE"));		break;
		case WidgetVirtual::WidgetType::LABEL:			rootVariant.insert("type", Variant("LABEL"));		break;
		case WidgetVirtual::WidgetType::CONSOLE:		rootVariant.insert("type", Variant("CONSOLE"));		break;
		case WidgetVirtual::WidgetType::RADIO_BUTTON:	rootVariant.insert("type", Variant("RADIO_BUTTON"));break;
		default:							rootVariant.insert("type", Variant("UNKNOWN"));		break;
	}

	//	write configuration
	rootVariant.insert("config", Variant((int)w->configuration));

	//	write sizes
	const vec2f& s = w->sizes[WidgetVirtual::State::DEFAULT];
	if (s == w->sizes[WidgetVirtual::State::HOVER] && s == w->sizes[WidgetVirtual::State::ACTIVE] && s == w->sizes[WidgetVirtual::State::CURRENT])
		rootVariant.insert("sizeAll", ToolBox::getFromVec2f(s));
	else
	{
		rootVariant.insert("sizeDefault", ToolBox::getFromVec2f(s));
		rootVariant.insert("sizeHover", ToolBox::getFromVec2f(w->sizes[WidgetVirtual::State::HOVER]));
		rootVariant.insert("sizeActive", ToolBox::getFromVec2f(w->sizes[WidgetVirtual::State::ACTIVE]));
		rootVariant.insert("sizeCurrent", ToolBox::getFromVec2f(w->sizes[WidgetVirtual::State::CURRENT]));
	}

	//	write positions
	const vec4f& p = w->positions[WidgetVirtual::State::DEFAULT];
	if (p == w->positions[WidgetVirtual::State::HOVER] && p == w->positions[WidgetVirtual::State::ACTIVE] && p == w->positions[WidgetVirtual::State::CURRENT])
		rootVariant.insert("positionAll", ToolBox::getFromVec4f(p));
	else
	{
		rootVariant.insert("positionDefault", ToolBox::getFromVec4f(p));
		rootVariant.insert("positionHover", ToolBox::getFromVec4f(w->positions[WidgetVirtual::State::HOVER]));
		rootVariant.insert("positionActive", ToolBox::getFromVec4f(w->positions[WidgetVirtual::State::ACTIVE]));
		rootVariant.insert("positionCurrent", ToolBox::getFromVec4f(w->positions[WidgetVirtual::State::CURRENT]));
	}

	//	write colors
	const vec4f& c = w->colors[WidgetVirtual::State::DEFAULT];
	if (c == w->colors[WidgetVirtual::State::HOVER] && c == w->colors[WidgetVirtual::State::ACTIVE] && c == w->colors[WidgetVirtual::State::CURRENT])
		rootVariant.insert("colorAll", ToolBox::getFromVec4f(c));
	else
	{
		rootVariant.insert("colorDefault", ToolBox::getFromVec4f(c));
		rootVariant.insert("colorHover", ToolBox::getFromVec4f(w->colors[WidgetVirtual::State::HOVER]));
		rootVariant.insert("colorActive", ToolBox::getFromVec4f(w->colors[WidgetVirtual::State::ACTIVE]));
		rootVariant.insert("colorCurrent", ToolBox::getFromVec4f(w->colors[WidgetVirtual::State::CURRENT]));
	}

	//	write shader and texture name
	if (w->shader && w->shader->name != "defaultWidget")
		rootVariant.insert("shader", Variant(w->shader->name));
	if (w->texture)
		rootVariant.insert("texture", Variant(w->texture->name));

	//	derivate type
	switch (w->type)
	{
		case WidgetVirtual::WidgetType::BOARD:
			serializeBoard(static_cast<WidgetBoard*>(w), rootVariant);
			break;
		case WidgetVirtual::WidgetType::IMAGE:
			serializeImage(static_cast<WidgetImage*>(w), rootVariant);
			break;
		case WidgetVirtual::WidgetType::LABEL:
			serializeLabel(static_cast<WidgetLabel*>(w), rootVariant);
			break;
		case WidgetVirtual::WidgetType::CONSOLE:
			serializeConsole(static_cast<WidgetConsole*>(w), rootVariant);
			break;
		case WidgetVirtual::WidgetType::RADIO_BUTTON:
			serializeRadioButton(static_cast<WidgetRadioButton*>(w), rootVariant);
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
			for (std::map<std::string, WidgetVirtual*>::iterator it = association.begin(); it != association.end(); ++it)
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
	rootVariant.insert("screenPosition", ToolBox::getFromVec4f(l->screenPosition));
	rootVariant.insert("targetPosition", ToolBox::getFromVec4f(l->targetPosition));
	rootVariant.insert("eulerAngle", ToolBox::getFromVec3f(l->eulerAngle));
	rootVariant.insert("size", Variant(l->size));

	//	children
	if (!l->children.empty())
	{
		rootVariant.insert("children", Variant::MapType());
		for (unsigned int i = 0; i < l->children.size(); ++i)
		{
			std::string name = "unknown_" + std::to_string(++unknownIndex);
			for (std::map<std::string, WidgetVirtual*>::iterator it = association.begin(); it != association.end(); ++it)
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








