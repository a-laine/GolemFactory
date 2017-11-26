#include "Mesh.h"
#include "Utiles/ToolBox.h"

#include <fstream>
#include <sstream>

//  Static attributes
std::string Mesh::extension = ".mesh";
//

//  Default
Mesh::Mesh(const std::string& meshName) : ResourceVirtual(meshName, ResourceVirtual::MESH), configuration(0x00) {}
Mesh::Mesh(const std::string& path, const std::string& meshName) : ResourceVirtual(meshName, ResourceVirtual::MESH), configuration(0x00)
{
	//	open file
	std::string tmpExtension = Mesh::extension;
	if (meshName.find(Mesh::extension) != std::string::npos)
		tmpExtension = "";
	std::ifstream file(path + meshName + tmpExtension);
	if (!file.good())
	{
		if (logVerboseLevel > 0)
			std::cerr << "ERROR : loading mesh : " << meshName << " : fail to open file" << std::endl;
		return;
	}

	
	//	initialization
	int lineIndex = 0;
	std::string line;
	std::vector<glm::vec3> tmpv, tmpvn, tmpc;

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
			//	try to push a new vertex normal to normales array
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
		else if (line.substr(0, 2) == "f ")
		{
			gfvertex v1, v2, v3;
			if (sscanf_s(line.c_str(), "f %i//%i/%i %i//%i/%i %i//%i/%i",
				&v1.v, &v1.vn, &v1.c,
				&v2.v, &v2.vn, &v2.c,
				&v3.v, &v3.vn, &v3.c) == 9)
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
				}
			}
			else printErrorLog(meshName, lineIndex, errorOccured);
		}
	}

	//	end
	file.close();
	configuration = WELL_LOADED;
	computeBoundingBox();
	initializeVBO();
	initializeVAO();
}
Mesh::Mesh(const std::string& meshName, const std::vector<glm::vec3>& verticesArray, const std::vector<glm::vec3>& normalesArray, const std::vector<glm::vec3>& colorArray, const std::vector<unsigned short>& facesArray)
	: ResourceVirtual(meshName, ResourceVirtual::MESH), configuration(WELL_LOADED), vertices(verticesArray), normales(normalesArray), colors(colorArray), faces(facesArray)
{
	computeBoundingBox();
	initializeVBO();
	initializeVAO();
}
Mesh::~Mesh()
{
	//	delete mesh attributes
	vertices.clear();
	normales.clear();
	colors.clear();
	faces.clear();

	glDeleteBuffers(1, &verticesBuffer);
	glDeleteBuffers(1, &normalsBuffer);
	glDeleteBuffers(1, &colorsBuffer);
	glDeleteBuffers(1, &facesBuffer);
	glDeleteVertexArrays(1, &vao);

	//	delete bounding box attributes
	vBBOx.clear();
	fBBOx.clear();

	glDeleteBuffers(1, &vBBOBuffer);
	glDeleteBuffers(1, &fBBOBuffer);
	glDeleteVertexArrays(1, &BBOvao);
}
//


//	Public functions
void Mesh::computeBoundingBox()
{
	aabb_min = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	aabb_max = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		aabb_min.x = std::min(aabb_min.x, vertices[i].x);
		aabb_max.x = std::max(aabb_max.x, vertices[i].x);

		aabb_min.y = std::min(aabb_min.y, vertices[i].y);
		aabb_max.y = std::max(aabb_max.y, vertices[i].y);

		aabb_min.z = std::min(aabb_min.z, vertices[i].z);
		aabb_max.z = std::max(aabb_max.z, vertices[i].z);
	}

	//	bbox vertex array
	vBBOx.push_back(aabb_min);
	vBBOx.push_back(glm::vec3(aabb_min.x, aabb_min.y, aabb_max.z));
	vBBOx.push_back(glm::vec3(aabb_min.x, aabb_max.y, aabb_min.z));
	vBBOx.push_back(glm::vec3(aabb_min.x, aabb_max.y, aabb_max.z));
	vBBOx.push_back(glm::vec3(aabb_max.x, aabb_min.y, aabb_min.z));//4
	vBBOx.push_back(glm::vec3(aabb_max.x, aabb_min.y, aabb_max.z));
	vBBOx.push_back(glm::vec3(aabb_max.x, aabb_max.y, aabb_min.z));
	vBBOx.push_back(aabb_max);

	//	bbox faces array (please don't change order it's important)
	fBBOx.push_back(4); fBBOx.push_back(0); fBBOx.push_back(6);
	fBBOx.push_back(2); fBBOx.push_back(0); fBBOx.push_back(6);
	fBBOx.push_back(2); fBBOx.push_back(0); fBBOx.push_back(3);
	fBBOx.push_back(1); fBBOx.push_back(0); fBBOx.push_back(3);
	fBBOx.push_back(6); fBBOx.push_back(2); fBBOx.push_back(7);
	fBBOx.push_back(3); fBBOx.push_back(2); fBBOx.push_back(7);
	fBBOx.push_back(6); fBBOx.push_back(4); fBBOx.push_back(7);
	fBBOx.push_back(5); fBBOx.push_back(4); fBBOx.push_back(7);
	fBBOx.push_back(4); fBBOx.push_back(0); fBBOx.push_back(5);
	fBBOx.push_back(1); fBBOx.push_back(0); fBBOx.push_back(5);
	fBBOx.push_back(5); fBBOx.push_back(1); fBBOx.push_back(7);
	fBBOx.push_back(3); fBBOx.push_back(1); fBBOx.push_back(7);
}
void Mesh::initializeVBO()
{
	//	mesh
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normales.size() * sizeof(glm::vec3), normales.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &colorsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorsBuffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned short), faces.data(), GL_STATIC_DRAW);

	//	bounding box
	glGenBuffers(1, &vBBOBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBBOBuffer);
	glBufferData(GL_ARRAY_BUFFER, vBBOx.size() * sizeof(glm::vec3), vBBOx.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &fBBOBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBBOBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fBBOx.size() * sizeof(unsigned short), fBBOx.data(), GL_STATIC_DRAW);
}
void Mesh::initializeVAO()
{
	//	mesh
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);

	//	bounding box
	glGenVertexArrays(1, &BBOvao);
	glBindVertexArray(BBOvao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vBBOBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBBOBuffer);
	glBindVertexArray(0);
}

void Mesh::draw(const RenderOption& option)
{
	switch (option)
	{
		case BOUNDING_BOX:
			glBindVertexArray(BBOvao);
			glDrawElements(GL_TRIANGLES, fBBOx.size(), GL_UNSIGNED_SHORT, NULL);
			break;

		default:
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_SHORT, NULL);
			break;
	}
}
//


//  Set/get functions
unsigned int Mesh::getNumberVertices() const { return vertices.size(); }
unsigned int Mesh::getNumberFaces() const { return faces.size(); }
const std::vector<glm::vec3>* Mesh::getBBoxVertices() const { return &vBBOx; }
const std::vector<unsigned short>* Mesh::getBBoxFaces() const { return &fBBOx; }
const std::vector<glm::vec3>* Mesh::getVertices() const { return &vertices; }
const std::vector<unsigned short>* Mesh::getFaces() const { return &faces; }

bool Mesh::hasSkeleton() const { return (configuration & HAS_SKELETON) != 0; }
bool Mesh::isValid() const
{
	if (configuration & WELL_LOADED)
		return glIsBuffer(verticesBuffer) && glIsBuffer(normalsBuffer) && glIsBuffer(colorsBuffer) && glIsBuffer(facesBuffer) && glIsVertexArray(vao);
	else return false;
}
bool Mesh::isAnimable() const { return (configuration & IS_ANIMABLE) != 0; }
bool Mesh::isFromGolemFactoryFormat() const
{
	if (name.find_last_of('.') == std::string::npos) return true;
	return (name.substr(name.find_last_of('.')) == Mesh::extension);
}
//