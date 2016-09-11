#include "Skeleton.h"

//  Static attributes
std::string Skeleton::extension = ".skeleton";
//

//	Default
Skeleton::Skeleton(std::string path, std::string skeletonName) : ResourceVirtual(skeletonName, ResourceVirtual::MESH), configuration(0x00)
{
	// TODO:
	//	skeleton loading from .gfSkeleton file
	//	json format parsing
}
Skeleton::~Skeleton()
{

}
//

//	Public functions
void Skeleton::addBone(const Bone& b)
{
	boneList.push_back(b);
}
void Skeleton::addBone(unsigned int parent, unsigned int nbChildren, unsigned int* children)
{
	Bone b;
	b.parent = parent;
	for (unsigned int i = 0; i < nbChildren; i++)
		b.children.push_back(children[i]);
	b.children.shrink_to_fit();
	addBone(b);
}

bool Skeleton::isValid() const
{
	return (configuration & VALID);
}
//