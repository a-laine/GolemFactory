#include "MeshSaver.h"


//	Public functions
void MeshSaver::save(Mesh* mesh, const std::string& resourcesPath, std::string fileName, vec4f scaleModifier)
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
void MeshSaver::saveStatic(Mesh* mesh, std::ofstream& file, vec4f scaleModifier)
{
	//	initialize buffers
	std::set<vec4> vertices;
	std::set<vec4> normales;
	std::set<vec4> uvs;
	float truncature = 0.001f;

	//	compression
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec4(getTruncatedAlias(mesh->vertices[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->normals.size(); i++)
		normales.insert(vec4(getTruncatedAlias(mesh->normals[i], truncature)));
	for (unsigned int i = 0; i < mesh->uvs.size(); i++)
		uvs.insert(vec4(getTruncatedAlias(mesh->uvs[i], truncature)));

	//	finish header
	file << "# Vertex count : " << vertices.size() << std::endl;
	file << "# Normals count : " << normales.size() << std::endl;
	file << "# Uvs count : " << uvs.size() << std::endl;
	file << "# Faces count : " << mesh->faces.size() << std::endl;
	file << "# Static mesh" << std::endl << std::endl << std::endl;


	//	vertices
	file << "#  positions" << std::endl;
	for (std::set<vec4>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		file << "v " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	normales
	file << "#  normales" << std::endl;
	for (std::set<vec4>::iterator it = normales.begin(); it != normales.end(); ++it)
		file << "vn " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	colors
	file << "#  uvs" << std::endl;
	for (std::set<vec4>::iterator it = uvs.begin(); it != uvs.end(); ++it)
		file << "uv " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	faces
	file << "#  triangles" << std::endl;
	for (unsigned int i = 0; i < mesh->faces.size(); i += 3)
	{
		file << "f ";
		for (int j = 0; j < 3; j++)
		{
			//	get attributes proxy
			vec4f v = getTruncatedAlias(mesh->vertices[mesh->faces[i + j]], truncature);
			vec4f vn = getTruncatedAlias(mesh->normals[mesh->faces[i + j]], truncature);
			vec4f c = getTruncatedAlias(mesh->uvs[mesh->faces[i + j]], truncature);

			//	search vertex index
			int index = 0;
			for (std::set<vec4>::iterator it = vertices.begin(); it != vertices.end(); ++it, index++)
				if (it->x == v.x && it->y == v.y && it->z == v.z) break;
			file << index << "//"; // double because of texture not yet supported

			// search normal index
			index = 0;
			for (std::set<vec4>::iterator it = normales.begin(); it != normales.end(); ++it, index++)
				if (it->x == vn.x && it->y == vn.y && it->z == vn.z) break;
			file << index << '/';

			// search color index
			index = 0;
			for (std::set<vec4>::iterator it = uvs.begin(); it != uvs.end(); ++it, index++)
				if (it->x == c.x && it->y == c.y && it->z == c.z) break;
			file << index << ' ';
		}
		file << std::endl;
	}
	file << std::endl;
}
void MeshSaver::saveAnimated(Mesh* mesh, std::ofstream& file, vec4f scaleModifier)
{
	//	initialize buffers
	std::set<vec4> vertices;
	std::set<vec4> normales;
	std::set<vec4> uvs;
	std::set<vec4> weights;
	std::set<ivec4> bones;
	float truncature = 0.001f;
	//glm::vec3 u;

	//	compression
	for (unsigned int i = 0; i < mesh->vertices.size(); i++)
		vertices.insert(vec4(getTruncatedAlias(mesh->vertices[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->normals.size(); i++)
		normales.insert(vec4(getTruncatedAlias(mesh->normals[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->uvs.size(); i++)
		uvs.insert(vec4(getTruncatedAlias(mesh->uvs[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->weights.size(); i++)
		weights.insert(vec4(getTruncatedAlias(mesh->weights[i] * scaleModifier, truncature)));
	for (unsigned int i = 0; i < mesh->bones.size(); i++)
		bones.insert(ivec4(mesh->bones[i]));

	//	finish header
	file << "# Vertex count : " << vertices.size() << std::endl;
	file << "# Normals count : " << normales.size() << std::endl;
	file << "# Uvs count : " << uvs.size() << std::endl;
	file << "# Weights count : " << weights.size() << std::endl;
	file << "# Bones count : " << bones.size() << std::endl;
	file << "# Faces count : " << mesh->faces.size() << std::endl;
	file << "# Animatable mesh" << std::endl << std::endl << std::endl;


	//	vertices
	file << "#  positions" << std::endl;
	for (std::set<vec4>::iterator it = vertices.begin(); it != vertices.end(); ++it)
		file << "v " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	normales
	file << "#  normales" << std::endl;
	for (std::set<vec4>::iterator it = normales.begin(); it != normales.end(); ++it)
		file << "vn " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	colors
	file << "#  uvs" << std::endl;
	for (std::set<vec4>::iterator it = uvs.begin(); it != uvs.end(); ++it)
		file << "uv " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	weights
	file << "#  weights" << std::endl;
	for (std::set<vec4>::iterator it = weights.begin(); it != weights.end(); ++it)
		file << "w " << it->x << ' ' << it->y << ' ' << it->z << std::endl;
	file << std::endl;

	//	bones
	file << "#  bones" << std::endl;
	for (std::set<ivec4>::iterator it = bones.begin(); it != bones.end(); ++it)
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
			vec4f v = getTruncatedAlias(mesh->vertices[mesh->faces[i + j]], truncature);
			vec4f vn = getTruncatedAlias(mesh->normals[mesh->faces[i + j]], truncature);
			vec4f c = getTruncatedAlias(mesh->uvs[mesh->faces[i + j]], truncature);
			vec4f w = getTruncatedAlias(mesh->weights[mesh->faces[i + j]], truncature);
			vec4i b = mesh->bones[mesh->faces[i + j]];

			//	search vertex index
			int index = 0;
			for (std::set<vec4>::iterator it = vertices.begin(); it != vertices.end(); ++it, index++)
				if (it->x == v.x && it->y == v.y && it->z == v.z) break;
			file << index << "//"; // double because of texture not yet supported

			// search normal index
			index = 0;
			for (std::set<vec4>::iterator it = normales.begin(); it != normales.end(); ++it, index++)
				if (it->x == vn.x && it->y == vn.y && it->z == vn.z) break;
			file << index << '/';

			// search color index
			index = 0;
			for (std::set<vec4>::iterator it = uvs.begin(); it != uvs.end(); ++it, index++)
				if (it->x == c.x && it->y == c.y && it->z == c.z) break;
			file << index << '/';

			// search weights index
			index = 0;
			for (std::set<vec4>::iterator it = weights.begin(); it != weights.end(); ++it, index++)
				if (it->x == w.x && it->y == w.y && it->z == w.z) break;
			file << index << '/';

			// search bones index
			index = 0;
			for (std::set<ivec4>::iterator it = bones.begin(); it != bones.end(); ++it, index++)
				if (it->x == b.x && it->y == b.y && it->z == b.z) break;
			file << index << ' ';
		}
		file << std::endl;
	}
	file << std::endl;
}

vec4f MeshSaver::getTruncatedAlias(vec4f original, const float& truncature)
{
	const float invTroncature = 1.f / truncature;
	float x = (int)(original.x * invTroncature + 0.5f * truncature) * truncature;
	float y = (int)(original.y * invTroncature + 0.5f * truncature) * truncature;
	float z = (int)(original.z * invTroncature + 0.5f * truncature) * truncature;
	float w = (int)(original.w * invTroncature + 0.5f * truncature) * truncature;
	return vec4f(x, y, z, w);
}
//