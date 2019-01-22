#include "MeshSaver.h"


//	Public functions
void MeshSaver::save(Mesh* mesh, const std::string& resourcesPath, std::string fileName, glm::vec3 scaleModifier)
{
	//	initialize fileName
	if (fileName.empty())
		fileName = mesh->name;
	if (fileName.find_last_of('/') != std::string::npos)
		fileName = fileName.substr(fileName.find_last_of('/') + 1);
	if (fileName.find_first_of('.') != std::string::npos)
		fileName = fileName.substr(0, fileName.find_first_of('.'));

	//	create and initialize file
	std::ofstream file(resourcesPath + "Meshes/" + fileName + Mesh::extension, std::ofstream::out);
	file << "# File : " << fileName << std::endl;
	file << "# Format : gfmesh, for Golem Factory engines" << std::endl;
	file << "# Vertex count : " << mesh->vertices.size() << std::endl;
	file << "# Faces count : " << mesh->faces.size() << std::endl;

	if (mesh->hasSkeleton()) saveAnimated(mesh, file, scaleModifier);
	else saveStatic(mesh, file, scaleModifier);

	//	end
	file.close();
}
//

//	Protected functions
void MeshSaver::saveStatic(Mesh* mesh, std::ofstream& file, glm::vec3 scaleModifier)
{
	//	finish header
	file << "# Static mesh" << std::endl << std::endl << std::endl;

	//	initialize buffers
	std::set<vec3> vertices;
	std::set<vec3> normales;
	std::set<vec3> colors;
	std::vector<unsigned int> faces;
	float truncature = 0.001f;

	//	vertices compression & write
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec3(mesh->vertices[i]));
	file << "#  positions" << std::endl;
	for (std::set<vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		file << "v " << (int)(it->x * scaleModifier.x / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->y * scaleModifier.y / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->z * scaleModifier.z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	normales compression & write
	for (unsigned int i = 0; i < mesh->normals.size(); i++)
		normales.insert(vec3(mesh->normals[i]));
	file << "#  normales" << std::endl;
	for (std::set<vec3>::iterator it = normales.begin(); it != normales.end(); ++it)
		file << "vn " << (int)(it->x / truncature + truncature / 2) * truncature
			 << ' ' <<   (int)(it->y / truncature + truncature / 2) * truncature
			 << ' ' <<   (int)(it->z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	colors compression & write
	for (unsigned int i = 0; i < mesh->colors.size(); i++)
		colors.insert(vec3(mesh->colors[i]));
	file << "#  colors" << std::endl;
	for (std::set<vec3>::iterator it = colors.begin(); it != colors.end(); ++it)
		file << "c " << (int)(it->x / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->y / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	faces
	file << "#  triangles" << std::endl;
	for (unsigned int i = 0; i < mesh->faces.size(); i += 3)
	{
		file << "f ";
		for (int j = 0; j < 3; j++)
		{
			glm::vec3 v = mesh->vertices[mesh->faces[i + j]];
			glm::vec3 vn = mesh->normals[mesh->faces[i + j]];
			glm::vec3 c = mesh->colors[mesh->faces[i + j]];

			//	search vertex index
			int index = 0;
			for (std::set<vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it, index++)
				if (it->x == v.x && it->y == v.y && it->z == v.z) break;
			file << index << "//"; // double because of texture not yet supported

			// search normal index
			index = 0;
			for (std::set<vec3>::iterator it = normales.begin(); it != normales.end(); ++it, index++)
				if (it->x == vn.x && it->y == vn.y && it->z == vn.z) break;
			file << index << '/';

			// search color index
			index = 0;
			for (std::set<vec3>::iterator it = colors.begin(); it != colors.end(); ++it, index++)
				if (it->x == c.x && it->y == c.y && it->z == c.z) break;
			file << index << ' ';
		}
		file << std::endl;
	}
	file << std::endl;
}
void MeshSaver::saveAnimated(Mesh* mesh, std::ofstream& file, glm::vec3 scaleModifier)
{
	//	finish header
	file << "# Animatable mesh : contain bones and / or weights" << std::endl << std::endl << std::endl;

	//	initialize buffers
	std::set<vec3> vertices;
	std::set<vec3> normales;
	std::set<vec3> colors;
	std::set<vec3> weights;
	std::set<ivec3> bones;
	std::vector<unsigned int> faces;
	float truncature = 0.001f;

	//	vertices compression & write
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec3(mesh->vertices[i]));
	file << "#  positions" << std::endl;
	for (std::set<vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		file << "v " << (int)(it->x * scaleModifier.x / truncature + truncature/2) * truncature
			 << ' ' <<  (int)(it->y * scaleModifier.y / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->z * scaleModifier.z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	normales compression & write
	for (unsigned int i = 0; i < mesh->normals.size(); i++)
		normales.insert(vec3(mesh->normals[i]));
	file << "#  normales" << std::endl;
	for (std::set<vec3>::iterator it = normales.begin(); it != normales.end(); ++it)
		file << "vn " << (int)(it->x / truncature + truncature / 2) * truncature
			 << ' ' <<   (int)(it->y / truncature + truncature / 2) * truncature
			 << ' ' <<   (int)(it->z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	colors compression & write
	for (unsigned int i = 0; i < mesh->colors.size(); i++)
		colors.insert(vec3(mesh->colors[i]));
	file << "#  colors" << std::endl;
	for (std::set<vec3>::iterator it = colors.begin(); it != colors.end(); ++it)
		file << "c " << (int)(it->x / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->y / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	weights compression & write
	for (unsigned int i = 0; i < mesh->weights.size(); i++)
		weights.insert(vec3(mesh->weights[i]));
	file << "#  weights" << std::endl;
	for (std::set<vec3>::iterator it = weights.begin(); it != weights.end(); ++it)
		file << "w " << (int)(it->x / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->y / truncature + truncature / 2) * truncature
			 << ' ' <<  (int)(it->z / truncature + truncature / 2) * truncature << std::endl;
	file << std::endl;

	//	bones compression & write
	for (unsigned int i = 0; i < mesh->bones.size(); i++)
		bones.insert(ivec3(mesh->bones[i]));
	file << "#  bones" << std::endl;
	for (std::set<ivec3>::iterator it = bones.begin(); it != bones.end(); ++it)
		file << "b " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	faces
	file << "#  triangles" << std::endl;
	for (unsigned int i = 0; i < mesh->faces.size(); i += 3)
	{
		file << "f ";
		for (int j = 0; j < 3; j++)
		{
			glm::vec3 v = mesh->vertices[mesh->faces[i + j]];
			glm::vec3 vn = mesh->normals[mesh->faces[i + j]];
			glm::vec3 c = mesh->colors[mesh->faces[i + j]];
			glm::vec3 w = mesh->weights[mesh->faces[i + j]];
			glm::ivec3 b = mesh->bones[mesh->faces[i + j]];

			//	search vertex index
			int index = 0;
			for (std::set<vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it, index++)
				if (it->x == v.x && it->y == v.y && it->z == v.z) break;
			file << index << "//"; // double because of texture not yet supported

			// search normal index
			index = 0;
			for (std::set<vec3>::iterator it = normales.begin(); it != normales.end(); ++it, index++)
				if (it->x == vn.x && it->y == vn.y && it->z == vn.z) break;
			file << index << '/';

			// search color index
			index = 0;
			for (std::set<vec3>::iterator it = colors.begin(); it != colors.end(); ++it, index++)
				if (it->x == c.x && it->y == c.y && it->z == c.z) break;
			file << index << '/';

			// search weights index
			index = 0;
			for (std::set<vec3>::iterator it = weights.begin(); it != weights.end(); ++it, index++)
				if (it->x == w.x && it->y == w.y && it->z == w.z) break;
			file << index << '/';

			// search bones index
			index = 0;
			for (std::set<ivec3>::iterator it = bones.begin(); it != bones.end(); ++it, index++)
				if (it->x == b.x && it->y == b.y && it->z == b.z) break;
			file << index << ' ';
		}
		file << std::endl;
	}
	file << std::endl;
}
//