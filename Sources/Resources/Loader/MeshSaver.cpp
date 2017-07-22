#include "MeshSaver.h"

//  Default
MeshSaver::MeshSaver()
{

}
MeshSaver::~MeshSaver()
{

}
//

//	Public functions
void MeshSaver::save(Mesh* mesh)
{
	MeshAnimated* m = dynamic_cast<MeshAnimated*>(mesh);
	if (m) save(m);
	else
	{
		//	vertices compression
		for (unsigned int i = 0; i < mesh->vertices.size(); i++)
			vertices.insert(vec3(mesh->vertices[i]));

		//	normales compression
		for (unsigned int i = 0; i < mesh->normales.size(); i++)
			normales.insert(vec3(mesh->normales[i]));

		//	colors compression
		for (unsigned int i = 0; i < mesh->colors.size(); i++)
			colors.insert(vec3(mesh->colors[i]));
	}
}
void MeshSaver::save(MeshAnimated* mesh)
{
	//	vertices compression
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec3(mesh->vertices[i]));
	std::cout << std::endl << "vertices compression : " << vertices.size() << " -> -" << mesh->vertices.size() - vertices.size() << std::endl;

	//	normales compression
	for (unsigned int i = 0; i < mesh->normales.size(); i++)
		normales.insert(vec3(mesh->normales[i]));
	std::cout << "normales compression : " << normales.size() << " -> -" << mesh->normales.size() - normales.size() << std::endl;

	//	colors compression
	for (unsigned int i = 0; i < mesh->colors.size(); i++)
		colors.insert(vec3(mesh->colors[i]));
	std::cout << "colors compression : " << colors.size() << " -> -" << mesh->colors.size() - colors.size() << std::endl;

	//	weights compression
	for (unsigned int i = 0; i < mesh->weights.size(); i++)
		weights.insert(vec3(mesh->weights[i]));
	std::cout << "weights compression : " << weights.size() << " -> -" << mesh->weights.size() - weights.size() << std::endl;

	//	bones compression
	for (unsigned int i = 0; i < mesh->bones.size(); i++)
		bones.insert(ivec3(mesh->bones[i]));
	std::cout << "bones compression : " << bones.size() << " -> -" << mesh->bones.size() - bones.size() << std::endl << std::endl;
}
//