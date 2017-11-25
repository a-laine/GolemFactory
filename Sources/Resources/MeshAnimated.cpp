#include "MeshAnimated.h"
#include "Utiles/ToolBox.h"

#include <fstream>
#include <sstream>


//  Default
MeshAnimated::MeshAnimated(const std::string& path, const std::string& meshName) : Mesh(meshName)
{
	//	open file
	std::string tmpExtension = Mesh::extension;
	if (meshName.find(Mesh::extension) != std::string::npos)
		tmpExtension = "";
	std::ifstream file(path + meshName + tmpExtension);
	if (!file.good())
	{
		if (logVerboseLevel >= ResourceVirtual::ERRORS)
			std::cerr << "ERROR : loading mesh : " << meshName << " : fail to open file" << std::endl;
		return;
	}

	//	initialization
	int lineIndex = 0;
	std::string line;
	std::vector<glm::vec3> tmpv, tmpvn, tmpc, tmpw;
	std::vector<glm::ivec3> tmpb;

	//	loading
	bool errorOccured = false;
	while (!file.eof())
	{
		std::getline(file, line);
		lineIndex++;

		if (line.empty()) continue;
		ToolBox::clearWhitespace(line);

		if (line.substr(0, 2) == "v ")
		{
			//	try to push a new vertex to vertex array
			std::istringstream iss(line.substr(2));
			glm::vec3 v;
			iss >> v.x; iss >> v.y; iss >> v.z;
			if (iss.fail()) printErrorLog(meshName, lineIndex, errorOccured);
			tmpv.push_back(v);
		}
		else if (line.substr(0, 3) == "vn ")
		{
			//	try to push a new vertex normales to normales array
			std::istringstream iss(line.substr(2));
			glm::vec3 vn;
			iss >> vn.x; iss >> vn.y; iss >> vn.z;
			if (iss.fail()) printErrorLog(meshName, lineIndex, errorOccured);
			tmpvn.push_back(vn);
		}
		else if (line.substr(0, 2) == "c ")
		{
			//	try to push a new vertex color to colors array
			std::istringstream iss(line.substr(2));
			glm::vec3 c;
			iss >> c.x; iss >> c.y; iss >> c.z;
			if (iss.fail()) printErrorLog(meshName, lineIndex, errorOccured);
			tmpc.push_back(c);
		}
		else if (line.substr(0, 2) == "w ")
		{
			//	try to push a new vertex weight list to weights list array
			std::istringstream iss(line.substr(2));
			glm::vec3 w;
			iss >> w.x; iss >> w.y; iss >> w.z;
			if (iss.fail()) printErrorLog(meshName, lineIndex, errorOccured);
			tmpw.push_back(w);
		}
		else if (line.substr(0, 2) == "b ")
		{
			//	try to push a new vertex bones list to bones list array
			std::istringstream iss(line.substr(2));
			glm::ivec3 b;
			iss >> b.x; iss >> b.y; iss >> b.z;
			if (iss.fail()) printErrorLog(meshName, lineIndex, errorOccured);
			tmpb.push_back(b);
		}
		else if (line.substr(0, 2) == "f ")
		{
			gfvertex_extended v1, v2, v3;
			if (sscanf_s(line.c_str(), "f %i//%i/%i/%i/%i %i//%i/%i/%i/%i %i//%i/%i/%i/%i",
				&v1.v, &v1.vn, &v1.c, &v1.w, &v1.b,
				&v2.v, &v2.vn, &v2.c, &v2.w, &v2.b,
				&v3.v, &v3.vn, &v3.c, &v3.w, &v3.b) == 15)
			{
				//	check if requested indexes are present in arrays
				int outrange = 0;
				if (v1.v<0 || v1.v >= (int)tmpv.size()) outrange++;
				if (v2.v<0 || v2.v >= (int)tmpv.size()) outrange++;
				if (v3.v<0 || v3.v >= (int)tmpv.size()) outrange++;

				if (v1.vn<0 || v1.vn >= (int)tmpvn.size()) outrange++;
				if (v2.vn<0 || v2.vn >= (int)tmpvn.size()) outrange++;
				if (v3.vn<0 || v3.vn >= (int)tmpvn.size()) outrange++;

				if (v1.c<0 || v1.c >= (int)tmpc.size()) outrange++;
				if (v2.c<0 || v2.c >= (int)tmpc.size()) outrange++;
				if (v3.c<0 || v3.c >= (int)tmpc.size()) outrange++;

				if (v1.w<0 || v1.w >= (int)tmpw.size()) outrange++;
				if (v2.w<0 || v2.w >= (int)tmpw.size()) outrange++;
				if (v3.w<0 || v3.w >= (int)tmpw.size()) outrange++;

				if (v1.b<0 || v1.b >= (int)tmpb.size()) outrange++;
				if (v2.b<0 || v2.b >= (int)tmpb.size()) outrange++;
				if (v3.b<0 || v3.b >= (int)tmpb.size()) outrange++;

				//	push vertex attributes in arrays, print errors if not
				if (outrange) printErrorLog(meshName, lineIndex, errorOccured);
				else
				{
					faces.push_back((unsigned short)vertices.size());
					faces.push_back((unsigned short)vertices.size() + 1);
					faces.push_back((unsigned short)vertices.size() + 2);

					vertices.push_back(tmpv[v1.v]);		vertices.push_back(tmpv[v2.v]);		vertices.push_back(tmpv[v3.v]);
					normales.push_back(tmpvn[v1.vn]);	normales.push_back(tmpvn[v2.vn]);	normales.push_back(tmpvn[v3.vn]);
					colors.push_back(tmpc[v1.c]);		colors.push_back(tmpc[v2.c]);		colors.push_back(tmpc[v3.c]);
					weights.push_back(tmpw[v1.w]);		weights.push_back(tmpw[v2.w]);		weights.push_back(tmpw[v3.w]);
					bones.push_back(tmpb[v1.b]);		bones.push_back(tmpb[v2.b]);		bones.push_back(tmpb[v3.b]);
				}
			}
			else printErrorLog(meshName, lineIndex, errorOccured);
		}
	}
	
	//	end
	file.close();
	configuration = WELL_LOADED | HAS_SKELETON;
	computeBoundingBox();
	initializeVBO();
	initializeVAO();
}
MeshAnimated::MeshAnimated(const std::string& meshName, const bool& isAnimable, const std::vector<glm::vec3>& verticesArray, const std::vector<glm::vec3>& normalesArray,
	const std::vector<glm::vec3>& colorArray, const std::vector<glm::ivec3>& bonesArray, const std::vector<glm::vec3>& weightsArray,
	const std::vector<unsigned short>& facesArray)
	: Mesh(meshName), bones(bonesArray), weights(weightsArray)
{
	configuration = WELL_LOADED | HAS_SKELETON;
	if (isAnimable) configuration |= IS_ANIMABLE;

	vertices = verticesArray;
	normales = normalesArray;
	colors = colorArray;
	faces = facesArray;

	computeBoundingBox();
	initializeVBO();
	initializeVAO();
}
MeshAnimated::~MeshAnimated()
{
	weights.clear();
	bones.clear();
	glDeleteBuffers(1, &bonesBuffer);
	glDeleteBuffers(1, &weightsBuffer);
}
//


//	Public functions
void MeshAnimated::initializeVBO()
{
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normales.size() * sizeof(glm::vec3), normales.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &bonesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
	glBufferData(GL_ARRAY_BUFFER, bones.size() * sizeof(glm::ivec3), bones.data(), GL_STATIC_DRAW);
	
	glGenBuffers(1, &weightsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, weightsBuffer);
	glBufferData(GL_ARRAY_BUFFER, weights.size() * sizeof(glm::vec3), weights.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned short), faces.data(), GL_STATIC_DRAW);
}
void MeshAnimated::initializeVAO()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
	glVertexAttribIPointer(3, 3, GL_INT, 0, NULL);
	
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, weightsBuffer);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);
}

void MeshAnimated::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_SHORT, NULL);
}
//


//	Set / get functions
bool MeshAnimated::isValid() const
{
	if (!Mesh::isValid()) return false;
	else return glIsBuffer(bonesBuffer) && glIsBuffer(weightsBuffer);
}
//
