#include "MeshSaver.h"


//	Public functions
void MeshSaver::save(Mesh* mesh, const std::string& resourcesPath, std::string fileName, glm::vec3 scaleModifier)
{
	//	initialize fileName
	if (fileName.empty())
		fileName = mesh->name;
	if (fileName.find_first_of('.') != std::string::npos)
		fileName = fileName.substr(0, fileName.find_first_of('.'));

	//	create and initialize file
	std::ofstream file(resourcesPath + fileName + Mesh::extension, std::ofstream::out);
	file << "# File : " << fileName << std::endl;
	file << "# Format : gfmesh, for Golem Factory engines" << std::endl;

	if (mesh->hasSkeleton()) saveAnimated(mesh, file, scaleModifier);
	else saveStatic(mesh, file, scaleModifier);

	//	end
	file.close();
}
//

//	Protected functions
void MeshSaver::saveStatic(Mesh* mesh, std::ofstream& file, glm::vec3 scaleModifier)
{
	//	initialize buffers
	std::set<vec3> vertices;
	std::set<vec3> normales;
	std::set<vec3> colors;
	float truncature = 0.001f;
	glm::vec3 u;

	//	compression
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec3(getTruncatedAlias(mesh->vertices[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->normals.size(); i++)
		normales.insert(vec3(getTruncatedAlias(mesh->normals[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->colors.size(); i++)
		colors.insert(vec3(getTruncatedAlias(mesh->colors[i] * scaleModifier, truncature)));

	//	finish header
	file << "# Vertex count : " << vertices.size() << std::endl;
	file << "# Normals count : " << normales.size() << std::endl;
	file << "# Colors count : " << colors.size() << std::endl;
	file << "# Faces count : " << mesh->faces.size() << std::endl;
	file << "# Static mesh" << std::endl << std::endl << std::endl;


	//	vertices
	file << "#  positions" << std::endl;
	for (std::set<vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		file << "v " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	normales
	file << "#  normales" << std::endl;
	for (std::set<vec3>::iterator it = normales.begin(); it != normales.end(); ++it)
		file << "vn " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	colors
	file << "#  colors" << std::endl;
	for (std::set<vec3>::iterator it = colors.begin(); it != colors.end(); ++it)
		file << "c " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	faces
	file << "#  triangles" << std::endl;
	for (unsigned int i = 0; i < mesh->faces.size(); i += 3)
	{
		file << "f ";
		for (int j = 0; j < 3; j++)
		{
			//	get attributes proxy
			glm::vec3 v = getTruncatedAlias(mesh->vertices[mesh->faces[i + j]], truncature);
			glm::vec3 vn = getTruncatedAlias(mesh->normals[mesh->faces[i + j]], truncature);
			glm::vec3 c = getTruncatedAlias(mesh->colors[mesh->faces[i + j]], truncature);

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
	//	initialize buffers
	std::set<vec3> vertices;
	std::set<vec3> normales;
	std::set<vec3> colors;
	std::set<vec3> weights;
	std::set<ivec3> bones;
	float truncature = 0.001f;
	glm::vec3 u;

	//	compression
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec3(getTruncatedAlias(mesh->vertices[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->normals.size(); i++)
		normales.insert(vec3(getTruncatedAlias(mesh->normals[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->colors.size(); i++)
		colors.insert(vec3(getTruncatedAlias(mesh->colors[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->weights.size(); i++)
		weights.insert(vec3(getTruncatedAlias(mesh->weights[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->bones.size(); i++)
		bones.insert(ivec3(mesh->bones[i]));

	//	finish header
	file << "# Vertex count : " << vertices.size() << std::endl;
	file << "# Normals count : " << normales.size() << std::endl;
	file << "# Colors count : " << colors.size() << std::endl;
	file << "# Weights count : " << weights.size() << std::endl;
	file << "# Bones count : " << bones.size() << std::endl;
	file << "# Faces count : " << mesh->faces.size() << std::endl;
	file << "# Animatable mesh" << std::endl << std::endl << std::endl;


	//	vertices
	file << "#  positions" << std::endl;
	for (std::set<vec3>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		file << "v " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	normales
	file << "#  normales" << std::endl;
	for (std::set<vec3>::iterator it = normales.begin(); it != normales.end(); ++it)
		file << "vn " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	colors
	file << "#  colors" << std::endl;
	for (std::set<vec3>::iterator it = colors.begin(); it != colors.end(); ++it)
		file << "c " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	weights
	file << "#  weights" << std::endl;
	for (std::set<vec3>::iterator it = weights.begin(); it != weights.end(); ++it)
		file << "w " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	bones
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
			//	get attributes proxy
			glm::vec3 v = getTruncatedAlias(mesh->vertices[mesh->faces[i + j]], truncature);
			glm::vec3 vn = getTruncatedAlias(mesh->normals[mesh->faces[i + j]], truncature);
			glm::vec3 c = getTruncatedAlias(mesh->colors[mesh->faces[i + j]], truncature);
			glm::vec3 w = getTruncatedAlias(mesh->weights[mesh->faces[i + j]], truncature);
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

glm::vec3 MeshSaver::getTruncatedAlias(glm::vec3 original, const float& truncature)
{
	float x = (int)(original.x / truncature + truncature / 2) * truncature;
	float y = (int)(original.y / truncature + truncature / 2) * truncature;
	float z = (int)(original.z / truncature + truncature / 2) * truncature;
	return glm::vec3(x, y, z);
}
//