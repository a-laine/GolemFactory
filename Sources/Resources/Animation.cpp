#include "Animation.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Animation::extension = ".anim";
//


//  Default
Animation::Animation(std::string path, std::string animationName) : ResourceVirtual(animationName, ResourceVirtual::ANIMATION)
{
	//  Initialization
	Variant v; Variant* tmp = NULL;
	std::string tmpName;

	try
	{
		Reader::parseFile(v, path + animationName + extension);
		tmp = &(v.getMap().begin()->second);
	}
	catch (std::exception&) { return; }
	Variant& animMap = *tmp;

	std::cout << "toto" << std::endl;
}
Animation::~Animation()
{

}

bool Animation::isValid() const
{
	return false;
}
//