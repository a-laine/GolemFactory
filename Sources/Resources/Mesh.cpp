#include "Mesh.h"

//  Default
Mesh::Mesh(std::string path,std::string meshName) :
    ResourceVirtual(path,meshName,ResourceVirtual::MESH)
{
    configuration = 0x00;
    std::string file = path + meshName;

    if(MeshLoader::loadMesh(file,data,faces,sizeofVertex,offsetNormal,offsetTexture,offsetWeight,offsetColor))
        return;

    data.shrink_to_fit();
    faces.shrink_to_fit();
    configuration |= VALID;
}
Mesh::~Mesh()
{
    data.clear();
    faces.clear();
}

bool Mesh::isValid() const{return configuration&VALID;}
//

//  Set/get functions
void Mesh::addAttribute(AttributeType t,int l,int s,void* d)
{
    Attribute a;
        a.type = t;
        a.location = l;
        a.size = s;
        a.data = d;
    attributeSummary.push_back(a);
}

unsigned int Mesh::getNumberVertices() const{return data.size()/sizeofVertex;}
unsigned int Mesh::getNumberFaces() const{return faces.size();}
//
