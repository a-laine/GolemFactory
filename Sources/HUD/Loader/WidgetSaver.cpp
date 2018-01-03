#include "WidgetSaver.h"
#include "Utiles/ToolBox.h"


//	Public functions
Variant WidgetSaver::serialize(WidgetVirtual* w, const std::map<std::string, WidgetVirtual*>& association)
{
	Variant rootVariant;   rootVariant.createMap();
	rootVariant.insert("jointList", Variant::MapType());
	rootVariant.insert("order", Variant::ArrayType());

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
		rootVariant.insert("sizeAll", ToolBox::getFromVec3(w->positions[WidgetVirtual::DEFAULT]));
	else
	{
		rootVariant.insert("sizeDefault", ToolBox::getFromVec3(w->positions[WidgetVirtual::DEFAULT]));
		rootVariant.insert("sizeHover", ToolBox::getFromVec3(w->positions[WidgetVirtual::HOVER]));
		rootVariant.insert("sizeActive", ToolBox::getFromVec3(w->positions[WidgetVirtual::ACTIVE]));
		rootVariant.insert("sizeCurrent", ToolBox::getFromVec3(w->positions[WidgetVirtual::CURRENT]));
	}

	//	write colors
	if (w->colors[WidgetVirtual::DEFAULT] == w->colors[WidgetVirtual::HOVER] && w->colors[WidgetVirtual::DEFAULT] == w->colors[WidgetVirtual::ACTIVE] && w->colors[WidgetVirtual::DEFAULT] == w->colors[WidgetVirtual::CURRENT])
		rootVariant.insert("sizeAll", ToolBox::getFromVec4(w->colors[WidgetVirtual::DEFAULT]));
	else
	{
		rootVariant.insert("sizeDefault", ToolBox::getFromVec4(w->colors[WidgetVirtual::DEFAULT]));
		rootVariant.insert("sizeHover", ToolBox::getFromVec4(w->colors[WidgetVirtual::HOVER]));
		rootVariant.insert("sizeActive", ToolBox::getFromVec4(w->colors[WidgetVirtual::ACTIVE]));
		rootVariant.insert("sizeCurrent", ToolBox::getFromVec4(w->colors[WidgetVirtual::CURRENT]));
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

	//	end
	return rootVariant;
}
//


//	Protected functions
void WidgetSaver::serializeBoard(WidgetBoard* v, Variant& root)
{

}
void WidgetSaver::serializeImage(WidgetImage* v, Variant& root)
{

}
void WidgetSaver::serializeLabel(WidgetLabel* v, Variant& root)
{

}
void WidgetSaver::serializeConsole(WidgetConsole* v, Variant& root)
{
	//	special board attributes
	root.insert("cornerConfig", Variant((int)v->cornerConfiguration));
	root.insert("borderWidth", Variant(v->borderWidth));
	root.insert("borderThickness", Variant(v->borderThickness));
	root.insert("margin", Variant(v->margin));
	root.insert("sizeChar", Variant(v->sizeChar));

	//	write font
	if (v->font) root.insert("font", Variant(v->font->name));

	//	write text string
	std::string txt = v->text;
	for (std::string::size_type i = 0; i != std::string::npos;)
	{
		i = txt.find("\n", i);
		if (i != std::string::npos)
		{
			txt.replace(i, 2, "\\n");
			i += 3;
		}
	}
	root.insert("text", Variant(txt));
}
void WidgetSaver::serializeRadioButton(WidgetRadioButton* v, Variant& root)
{

}
//








