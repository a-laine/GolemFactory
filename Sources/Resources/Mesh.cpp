#include "Mesh.h"
#include "Loader/MeshLoader.h"
#include "Loader/AssimpLoader.h"

#include <Utiles/Assert.hpp>


//  Static attributes
char const * const Mesh::directory = "Meshes/";
char const * const Mesh::extension = ".mesh";
std::string Mesh::defaultName;
//

//  Default
Mesh::Mesh(const std::string& meshName)
    : ResourceVirtual(meshName)
    , boundingBox(vec4f::zero, vec4f::zero)
    , vao(0), verticesBuffer(0), uvsBuffer(0), normalsBuffer(0), facesBuffer(0)
    , BBoxVao(0), vBBoxBuffer(0), fBBoxBuffer(0)
    , weightsBuffer(0), bonesBuffer(0)
    , wBBoxBuffer(0), bBBoxBuffer(0)
{}
Mesh::~Mesh()
{
	clear();
}
//


//	Public functions
void Mesh::initialize(const std::vector<vec4f>& verticesArray, const std::vector<vec4f>& normalsArray,
    const std::vector<vec4f>& uvArray, const std::vector<unsigned short>& facesArray,
    const std::vector<vec4i>& bonesArray, const std::vector<vec4f>& weightsArray)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    if (verticesArray.size() > 65000)
    {
        std::cout << ".............................................." << verticesArray.size() << std::endl;
    }

    vertices = verticesArray;
    normals = normalsArray;
    uvs = uvArray;
    faces = facesArray;
    bones = bonesArray;
    weights = weightsArray;

    computeBoundingBox();
    initializeVBO();
    initializeVAO();

    if(!glIsBuffer(verticesBuffer) || !glIsBuffer(normalsBuffer) || !glIsBuffer(uvsBuffer) || !glIsBuffer(facesBuffer) || !glIsVertexArray(vao) ||
        (hasSkeleton() && (!glIsBuffer(bonesBuffer) || !glIsBuffer(weightsBuffer))))
    {
        clear();
        state = INVALID;
    }
    else
        state = VALID;
}

void Mesh::initialize(std::vector<vec4f>* verticesArray, std::vector<vec4f>* normalsArray,
    std::vector<vec4f>* uvArray, std::vector<unsigned short>* facesArray,
    std::vector<vec4i>* bonesArray, std::vector<vec4f>* weightsArray)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    vertices = *verticesArray;
    normals = *normalsArray;
    uvs = *uvArray;
    faces = *facesArray;
    bones = *bonesArray;
    weights = *weightsArray;

    computeBoundingBox();
    initializeVBO();
    initializeVAO();

    if(!glIsBuffer(verticesBuffer) || !glIsBuffer(normalsBuffer) || !glIsBuffer(uvsBuffer) || !glIsBuffer(facesBuffer) || !glIsVertexArray(vao) ||
        (hasSkeleton() && (!glIsBuffer(bonesBuffer) || !glIsBuffer(weightsBuffer))))
    {
        clear();
        state = INVALID;
    }
    else
        state = VALID;
}

void Mesh::computeBoundingBox()
{
    if (vertices.empty())
    {
        boundingBox.min = boundingBox.max = vec4f(0, 0, 0, 1);
    }
    else
    {
	    boundingBox.min = vec4f(std::numeric_limits<float>::max());
	    boundingBox.max = vec4f(-std::numeric_limits<float>::max());
    }

	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		boundingBox.min = vec4f::min(boundingBox.min, vertices[i]);
		boundingBox.max = vec4f::max(boundingBox.max, vertices[i]);
	}

	//	bbox vertex array
	vBBox.push_back(boundingBox.min);
	vBBox.push_back(vec4f(boundingBox.min.x, boundingBox.min.y, boundingBox.max.z, 1.f));
	vBBox.push_back(vec4f(boundingBox.min.x, boundingBox.max.y, boundingBox.min.z, 1.f));
	vBBox.push_back(vec4f(boundingBox.min.x, boundingBox.max.y, boundingBox.max.z, 1.f));
	vBBox.push_back(vec4f(boundingBox.max.x, boundingBox.min.y, boundingBox.min.z, 1.f));
	vBBox.push_back(vec4f(boundingBox.max.x, boundingBox.min.y, boundingBox.max.z, 1.f));
	vBBox.push_back(vec4f(boundingBox.max.x, boundingBox.max.y, boundingBox.min.z, 1.f));
	vBBox.push_back(boundingBox.max);

	//	bbox faces array (please don't change order it's important)
	fBBox.push_back(4); fBBox.push_back(0); fBBox.push_back(6);
	fBBox.push_back(2); fBBox.push_back(0); fBBox.push_back(6);
	fBBox.push_back(2); fBBox.push_back(0); fBBox.push_back(3);
	fBBox.push_back(1); fBBox.push_back(0); fBBox.push_back(3);
	fBBox.push_back(6); fBBox.push_back(2); fBBox.push_back(7);
	fBBox.push_back(3); fBBox.push_back(2); fBBox.push_back(7);
	fBBox.push_back(6); fBBox.push_back(4); fBBox.push_back(7);
	fBBox.push_back(5); fBBox.push_back(4); fBBox.push_back(7);
	fBBox.push_back(4); fBBox.push_back(0); fBBox.push_back(5);
	fBBox.push_back(1); fBBox.push_back(0); fBBox.push_back(5);
	fBBox.push_back(5); fBBox.push_back(1); fBBox.push_back(7);
	fBBox.push_back(3); fBBox.push_back(1); fBBox.push_back(7);

    if(hasSkeleton())
    {
        for(unsigned int i = 0; i < vBBox.size(); i++)
        {
            bBBox.push_back(vec4i::zero);
            wBBox.push_back(vec4f(1, 0, 0, 0));
        }
    }
}
void Mesh::initializeVBO()
{
	//	mesh
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec4f), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec4f), normals.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &uvsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvsBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec4f), uvs.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &facesBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(unsigned short), faces.data(), GL_STATIC_DRAW);

	//	bounding box
	glGenBuffers(1, &vBBoxBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBBoxBuffer);
	glBufferData(GL_ARRAY_BUFFER, vBBox.size() * sizeof(vec4f), vBBox.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &fBBoxBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBBoxBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fBBox.size() * sizeof(unsigned short), fBBox.data(), GL_STATIC_DRAW);

    if(hasSkeleton())
    {
        //	mesh
        glGenBuffers(1, &bonesBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
        glBufferData(GL_ARRAY_BUFFER, bones.size() * sizeof(vec4i), bones.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &weightsBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, weightsBuffer);
        glBufferData(GL_ARRAY_BUFFER, weights.size() * sizeof(vec4f), weights.data(), GL_STATIC_DRAW);

        //	bounding box
        glGenBuffers(1, &bBBoxBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, bBBoxBuffer);
        glBufferData(GL_ARRAY_BUFFER, bBBox.size() * sizeof(vec4i), bBBox.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &wBBoxBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, wBBoxBuffer);
        glBufferData(GL_ARRAY_BUFFER, wBBox.size() * sizeof(vec4f), wBBox.data(), GL_STATIC_DRAW);
    }
}
void Mesh::initializeVAO()
{
	//	mesh
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvsBuffer);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    if(hasSkeleton())
    {
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
        glVertexAttribIPointer(3, 4, GL_INT, 0, NULL);

        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, weightsBuffer);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
	glBindVertexArray(0);

	//	bounding box
	glGenVertexArrays(1, &BBoxVao);
	glBindVertexArray(BBoxVao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vBBoxBuffer);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    if(hasSkeleton())
    {
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, bBBoxBuffer);
        glVertexAttribIPointer(3, 4, GL_INT, 0, NULL);

        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, wBBoxBuffer);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBBoxBuffer);
	glBindVertexArray(0);
}

const GLuint Mesh::getVAO() const { return vao;}
const GLuint Mesh::getBBoxVAO() const { return BBoxVao; }
//


//  Set/get functions
unsigned int Mesh::getNumberVertices() const { return (unsigned int)vertices.size(); }
unsigned int Mesh::getNumberFaces() const { return (unsigned int)faces.size() / 6; }
const std::vector<vec4f>* Mesh::getBBoxVertices() const { return &vBBox; }
const std::vector<unsigned short>* Mesh::getBBoxFaces() const { return &fBBox; }
const std::vector<vec4f>* Mesh::getVertices() const { return &vertices; }
const std::vector<vec4f>* Mesh::getNormals() const { return &normals; }
const std::vector<unsigned short>* Mesh::getFaces() const { return &faces; }
const std::vector<vec4i>* Mesh::getBones() const { return &bones; }
const std::vector<vec4f>* Mesh::getWeights() const { return &weights; }
const AxisAlignedBox& Mesh::getBoundingBox() const { return boundingBox; };

bool Mesh::hasSkeleton() const { return !bones.empty() && !weights.empty(); }

std::string Mesh::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
std::string Mesh::getIdentifier() const
{
    return getIdentifier(name);
}

std::string Mesh::getLoaderId(const std::string& resourceName) const
{
    size_t ext = resourceName.find_last_of('.');
    if(ext != std::string::npos && resourceName.substr(ext) != Mesh::extension)
        return "assimp";
    else
        return extension;
}

const std::string& Mesh::getDefaultName() { return defaultName; }
void Mesh::setDefaultName(const std::string& name) { defaultName = name; }
//

void Mesh::clear()
{
    //	delete mesh attributes
    vertices.clear();
    normals.clear();
    uvs.clear();
    faces.clear();

    glDeleteBuffers(1, &verticesBuffer);
    glDeleteBuffers(1, &normalsBuffer);
    glDeleteBuffers(1, &uvsBuffer);
    glDeleteBuffers(1, &facesBuffer);
    glDeleteVertexArrays(1, &vao);

	verticesBuffer = 0;
	normalsBuffer = 0;
    uvsBuffer = 0;
	facesBuffer = 0;
	vao = 0;

    //	delete bounding box attributes
    vBBox.clear();
    fBBox.clear();

    glDeleteBuffers(1, &vBBoxBuffer);
    glDeleteBuffers(1, &fBBoxBuffer);
    glDeleteVertexArrays(1, &BBoxVao);

	vBBoxBuffer = 0;
	fBBoxBuffer = 0;
	BBoxVao = 0;

    if(hasSkeleton())
    {
        weights.clear();
        bones.clear();
        glDeleteBuffers(1, &bonesBuffer);
        glDeleteBuffers(1, &weightsBuffer);

		bonesBuffer = 0;
		weightsBuffer = 0;
    }
}


