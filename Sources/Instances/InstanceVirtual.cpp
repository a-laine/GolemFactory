#include "InstanceVirtual.h"

//  Default
InstanceVirtual::InstanceVirtual() : position(0,0,0), size(1,1,1), bbsize(0, 0, 0)
{
	count = 0;
}
InstanceVirtual::~InstanceVirtual() {}
//


//	Set/Get functions
void InstanceVirtual::setPosition(glm::vec3 p) { position = p; }
void InstanceVirtual::setSize(glm::vec3 s) { size = s; }

glm::vec3 InstanceVirtual::getPosition() { return position; }
glm::vec3 InstanceVirtual::getSize() { return size; }
glm::vec3 InstanceVirtual::getBBSize() { return size; }
//
