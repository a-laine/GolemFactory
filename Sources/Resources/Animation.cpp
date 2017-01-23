#include "Animation.h"
#include "Utiles/Parser/Reader.h"

//  Static attributes
std::string Animation::extension = ".animation";
//

//  Default
Animation::Animation(const std::string& animationName, const std::vector<KeyFrame>& animations)
	: ResourceVirtual(animationName, ResourceVirtual::ANIMATION), timeLine(animations)
{}

Animation::Animation(const std::string& path, const std::string& animationName) : ResourceVirtual(animationName, ResourceVirtual::ANIMATION)
{
	std::cerr << "Animation loading in GF format not supported yet" << std::endl;
}
Animation::~Animation()
{

}
//

//	Public functions
bool Animation::isValid() const
{
	return timeLine.empty();
}
//