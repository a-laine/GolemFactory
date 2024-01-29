#include "Mesh.h"
#include "Loader/MeshLoader.h"
#include "Loader/AssimpLoader.h"
#include "Skeleton.h"

#ifdef USE_IMGUI
    #include <Renderer/Renderer.h>
#endif

#include <Utiles/Assert.hpp>
#include <Utiles/ConsoleColor.h>


//  Static attributes
char const * const Mesh::directory = "Meshes/";
char const * const Mesh::extension = ".mesh";
std::string Mesh::defaultName;
//

//  Default
Mesh::Mesh(const std::string& meshName)
    : ResourceVirtual(meshName, ResourceVirtual::ResourceType::MESH)
    , boundingBox(vec4f::zero, vec4f::zero)
    , vao(0), verticesBuffer(0), uvsBuffer(0), normalsBuffer(0), facesBuffer(0)
    , faceType(GL_UNSIGNED_BYTE), faceIndicesCount(0), faceIndicesElementSize(0), faceIndicesArray(nullptr)
    , BBoxVao(0), vBBoxBuffer(0), fBBoxBuffer(0)
    , weightsBuffer(0), bonesBuffer(0)
    , wBBoxBuffer(0), bBBoxBuffer(0)
{

}
Mesh::~Mesh()
{
	clear();
}
//


//	Public functions
void Mesh::initialize(const std::vector<vec4f>& verticesArray, const std::vector<vec4f>& normalsArray,
    const std::vector<vec4f>& uvArray, const std::vector<unsigned int>& facesArray,
    const std::vector<vec4i>& bonesArray, const std::vector<vec4f>& weightsArray)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    /*if (verticesArray.size() > 65000)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : Mesh::initialize : " << name << 
            " : indice overflow, too much vertices (" << verticesArray.size() << ")" << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }*/

    vertices = verticesArray;
    normals = normalsArray;
    uvs = uvArray;
    bones = bonesArray;
    weights = weightsArray;

    if (verticesArray.size() < 0xFF)
    {
        faceType = GL_UNSIGNED_BYTE;
        faceIndicesElementSize = sizeof(uint8_t);
        faceIndicesCount = facesArray.size();
        faceIndicesArray = new uint8_t[faceIndicesCount];
        for (unsigned int i = 0; i < faceIndicesCount; i++)
            faceIndicesArray[i] = (uint8_t)facesArray[i];
    }
    else if (verticesArray.size() < 0xFFFF)
    {
        faceType = GL_UNSIGNED_SHORT;
        faceIndicesElementSize = sizeof(uint16_t);
        faceIndicesCount = facesArray.size();
        faceIndicesArray = (uint8_t*)(new uint16_t[faceIndicesCount]);
        uint16_t* castedArray = (uint16_t*)faceIndicesArray;
        for (unsigned int i = 0; i < faceIndicesCount; i++)
            castedArray[i] = (uint16_t)facesArray[i];
    }
    else
    {
        faceType = GL_UNSIGNED_INT;
        faceIndicesElementSize = sizeof(uint32_t);
        faceIndicesCount = facesArray.size();
        faceIndicesArray = (uint8_t*)(new uint32_t[faceIndicesCount]);
        uint32_t* castedArray = (uint32_t*)faceIndicesArray;
        for (unsigned int i = 0; i < faceIndicesCount; i++)
            castedArray[i] = facesArray[i];
    }

    computeBoundingBox();
    initializeVBO();
    initializeVAO();
    checkValidity();
}

void Mesh::initialize(std::vector<vec4f>* verticesArray, std::vector<vec4f>* normalsArray,
    std::vector<vec4f>* uvArray, std::vector<unsigned int>* facesArray,
    std::vector<vec4i>* bonesArray, std::vector<vec4f>* weightsArray)
{
    GF_ASSERT(state == INVALID);
    state = LOADING;

    /*if (verticesArray->size() > 65000)
    {
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : Mesh::initialize : " << name <<
            " : indice overflow, too much vertices (" << verticesArray->size() << ")" << std::flush;
        std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
    }*/

    vertices = *verticesArray;
    normals = *normalsArray;
    uvs = *uvArray;
    bones = *bonesArray;
    weights = *weightsArray;

    if (verticesArray->size() < 0xFF)
    {
        faceType = GL_UNSIGNED_BYTE;
        faceIndicesElementSize = sizeof(uint8_t);
        faceIndicesCount = facesArray->size();
        faceIndicesArray = new uint8_t[faceIndicesCount];
        for (unsigned int i = 0; i < faceIndicesCount; i++)
            faceIndicesArray[i] = (uint8_t)(*facesArray)[i];
    }
    else if (verticesArray->size() < 0xFFFF)
    {
        faceType = GL_UNSIGNED_SHORT;
        faceIndicesElementSize = sizeof(uint16_t);
        faceIndicesCount = facesArray->size();
        faceIndicesArray = (uint8_t*)(new uint16_t[faceIndicesCount]);
        uint16_t* castedArray = (uint16_t*)faceIndicesArray;
        for (unsigned int i = 0; i < faceIndicesCount; i++)
            castedArray[i] = (uint16_t)(*facesArray)[i];
    }
    else
    {
        faceType = GL_UNSIGNED_INT;
        faceIndicesElementSize = sizeof(uint32_t);
        faceIndicesCount = facesArray->size();
        faceIndicesArray = (uint8_t*)(new uint32_t[faceIndicesCount]);
        uint32_t* castedArray = (uint32_t*)faceIndicesArray;
        for (unsigned int i = 0; i < faceIndicesCount; i++)
            castedArray[i] = (*facesArray)[i];
    }

    computeBoundingBox();
    initializeVBO();
    initializeVAO();
    checkValidity();
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

    if (!normals.empty())
    {
	    glGenBuffers(1, &normalsBuffer);
	    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec4f), normals.data(), GL_STATIC_DRAW);
    }

    if (!uvs.empty())
    {
	    glGenBuffers(1, &uvsBuffer);
	    glBindBuffer(GL_ARRAY_BUFFER, uvsBuffer);
	    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec4f), uvs.data(), GL_STATIC_DRAW);
    }

    glGenBuffers(1, &facesBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndicesCount * faceIndicesElementSize, faceIndicesArray, GL_STATIC_DRAW);

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

    if (!normals.empty())
    {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    if (!uvs.empty())
    {
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvsBuffer);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    if (hasSkeleton())
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

void  Mesh::setBoneNames(std::vector<std::string>& names)
{
    boneNames.swap(names);
}
void Mesh::retargetSkin(const Skeleton* skeleton)
{
    if (!skeleton || !hasSkeleton())
        return;

    bool canRemap = true;
    bool needRemap = false;
    const std::vector<Skeleton::Bone>& skelBones = skeleton->getBones();
    std::vector<std::string> newBoneNames;
    newBoneNames.assign(skelBones.size(), "-unused-");
    std::map<int, int> remapTable;

    for (int i = 0; i < boneNames.size(); i++)
    {
        if (i < skelBones.size() && boneNames[i] == skelBones[i].name)
        {
            remapTable[i] = skelBones[i].id;
            newBoneNames[i] = skelBones[i].name;
        }
        else
        {
            needRemap = true;
            int targetIndex = -1;
            for (int j = 0; j < skelBones.size(); j++)
            {
                if (boneNames[i] == skelBones[j].name)
                {
                    targetIndex = j;
                    newBoneNames[j] = skelBones[j].name;
                    break;
                }
            }

            canRemap = targetIndex >= 0;
            remapTable[i] = targetIndex;
        }
    }

    if (!canRemap || !needRemap)
        return;

    boneNames.swap(newBoneNames);

    for (int i = 0; i < bones.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (bones[i][j] < 0)
                break;
            bones[i][j] = remapTable[bones[i][j]];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, bonesBuffer);
    glBufferData(GL_ARRAY_BUFFER, bones.size() * sizeof(vec4i), bones.data(), GL_STATIC_DRAW);
}

const GLuint Mesh::getVAO() const { return vao;}
const GLuint Mesh::getBBoxVAO() const { return BBoxVao; }
//


//  Set/get functions
unsigned int Mesh::getNumberVertices() const { return (unsigned int)vertices.size(); }
unsigned int Mesh::getNumberFaces() const { return faceIndicesCount / 3; }
unsigned int Mesh::getNumberIndices() const { return faceIndicesCount; }
unsigned int Mesh::getFaceIndiceAt(unsigned int i) const
{
    if (faceType == GL_UNSIGNED_BYTE)
        return faceIndicesArray[i];
    if (faceType == GL_UNSIGNED_SHORT)
        return ((uint16_t*)faceIndicesArray)[i];
    else
        return ((uint32_t*)faceIndicesArray)[i];
}
const std::vector<vec4f>* Mesh::getBBoxVertices() const { return &vBBox; }
const std::vector<unsigned short>* Mesh::getBBoxFaces() const { return &fBBox; }
const std::vector<vec4f>* Mesh::getVertices() const { return &vertices; }
const std::vector<vec4f>* Mesh::getNormals() const { return &normals; }
const uint8_t* Mesh::getFaces() const { return faceIndicesArray; }
GLuint Mesh::getIndicesType() const { return faceType; }
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
        return "assimpMesh";
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

    faceIndicesCount = 0;
    if (faceIndicesArray)
    {
        delete[] faceIndicesArray;
        faceIndicesArray = nullptr;
    }

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

void Mesh::checkValidity()
{
    if (!glIsBuffer(verticesBuffer) || !glIsBuffer(facesBuffer) || !glIsVertexArray(vao))
    {
        clear();
        state = INVALID;
    }
    else if (hasSkeleton() && (!glIsBuffer(bonesBuffer) || !glIsBuffer(weightsBuffer)))
    {
        clear();
        state = INVALID;
    }
    else if (!normals.empty() && !glIsBuffer(normalsBuffer))
    {
        clear();
        state = INVALID;
    }
    else if (!uvs.empty() && !glIsBuffer(uvsBuffer))
    {
        clear();
        state = INVALID;
    }
    else
        state = VALID;
}

void Mesh::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Mesh infos");
    ImGui::Text("Vertices count : %d", getNumberVertices());
    ImGui::Text("Faces count : %d", getNumberFaces());
    if (faceType == GL_UNSIGNED_BYTE)
        ImGui::Text("indiceType : BYTE", getNumberFaces());
    if (faceType == GL_UNSIGNED_SHORT)
        ImGui::Text("indiceType : SHORT", getNumberFaces());
    else
        ImGui::Text("indiceType : INT", getNumberFaces());
    ImGui::Spacing();

    // overview
    overviewAngle0 += overviewAngleSpeed;
    if (overviewAngle0 > PI)
        overviewAngle0 -= 2 * PI;

    GLuint texid = Renderer::getInstance()->renderMeshOverview(this, overviewAngle0, overviewAngle1);
    vec2i texSize = Renderer::getInstance()->getOverviewTextureSize();

    if (!boneNames.empty())
    {
        ImGui::TextColored(ResourceVirtual::titleColorDraw, "Skin mapping");
        for (int i = 0; i < boneNames.size(); i++)
            ImGui::Text("%d : %s", i, boneNames[i].c_str());
        ImGui::Spacing();
    }

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Overview");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

    float ratio = (ImGui::GetContentRegionAvail().x - 5) / texSize.x;
    ImGui::ImageButton((void*)texid, ImVec2(texSize.x * ratio, texSize.y * ratio), ImVec2(0, 0), ImVec2(1, 1), -1, ImColor(25, 25, 25, 255), ImColor(255,255,255,255));
    ImGui::PopStyleColor(3);

    ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
    ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
    ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);
    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            clickPos = mousePositionRelative;
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            overviewAngle0 = 0.02f * (mousePositionRelative.x - clickPos.x);
            overviewAngle1 = 0.02f * (mousePositionRelative.y - clickPos.y);
            overviewAngle1 = clamp(overviewAngle1, -1.2f, 1.2f);
        }
    }
    ImGui::SliderFloat("Rotation speed", &overviewAngleSpeed, 0.f, 0.1f);

#endif
}