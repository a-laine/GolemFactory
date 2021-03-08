#include "Debug.h"
#include <Resources/Shader.h>

#include <glm/gtx/vector_angle.hpp>


// Static initialization
glm::vec3 Debug::color = glm::vec3(0.f);
glm::mat4 Debug::view = glm::mat4(1.f);
glm::mat4 Debug::projection = glm::mat4(1.f);

const glm::vec3 Debug::black = glm::vec3(0.f, 0.f, 0.f);
const glm::vec3 Debug::white = glm::vec3(1.f, 1.f, 1.f);
const glm::vec3 Debug::magenta = glm::vec3(1.f, 0.f, 1.f);
const glm::vec3 Debug::orange = glm::vec3(0.992f, 0.415f, 0.008f);
const glm::vec3 Debug::grey = glm::vec3(0.2f, 0.2f, 0.2f);
const glm::vec3 Debug::red = glm::vec3(1.f, 0.f, 0.f);
const glm::vec3 Debug::green = glm::vec3(0.f, 1.f, 0.f);
const glm::vec3 Debug::blue = glm::vec3(0.f, 0.f, 1.f);

const glm::vec3 Debug::darkBlue = glm::vec3(0.103f, 0.103f, 0.403f);
const glm::vec3 Debug::darkGreen = glm::vec3(0.f, 0.25f, 0.f);

//

//  Default
Debug::Debug() : renderer(nullptr), pointMesh(nullptr), cubeMesh(nullptr), sphereMesh(nullptr), capsuleMesh(nullptr), pointShader(nullptr), segmentShader(nullptr), defaultShader(nullptr), wiredShader(nullptr)
{}
Debug::~Debug()
{
	ResourceManager::getInstance()->release(pointMesh);
	ResourceManager::getInstance()->release(cubeMesh);
	ResourceManager::getInstance()->release(sphereMesh);
	ResourceManager::getInstance()->release(capsuleMesh);

	ResourceManager::getInstance()->release(pointShader);
	ResourceManager::getInstance()->release(segmentShader);
	ResourceManager::getInstance()->release(defaultShader);
	ResourceManager::getInstance()->release(wiredShader);
}
//

//	Public functions
void Debug::initialize(const std::string& pointMeshName, const std::string& cubeMeshName, const std::string& sphereMeshName, const std::string& capsuleMeshName, const std::string& pointShaderName, const std::string& segmentShaderName, const std::string& defaultShaderName, const std::string& wiredShaderName)
{
	renderer = Renderer::getInstance();
	pointMesh = ResourceManager::getInstance()->getResource<Mesh>(pointMeshName);
	cubeMesh = ResourceManager::getInstance()->getResource<Mesh>(cubeMeshName);
	sphereMesh = ResourceManager::getInstance()->getResource<Mesh>(sphereMeshName);
	capsuleMesh = ResourceManager::getInstance()->getResource<Mesh>(capsuleMeshName);

	pointShader = ResourceManager::getInstance()->getResource<Shader>(pointShaderName);
	segmentShader = ResourceManager::getInstance()->getResource<Shader>(segmentShaderName);
	defaultShader = ResourceManager::getInstance()->getResource<Shader>(defaultShaderName);
	wiredShader = ResourceManager::getInstance()->getResource<Shader>(wiredShaderName);
}
//

// real draw functions
void Debug::point(const glm::vec3& p, Shader* shader)
{
	if (This->renderer && shader && This->pointMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(shader, &glm::translate(glm::mat4(1.f), p)[0][0], &view[0][0], &projection[0][0]);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->pointMesh->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
void Debug::line(const glm::vec3& point1, const glm::vec3& point2, Shader* shader)
{
	if (This->renderer && shader && This->pointMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(shader, &glm::translate(glm::mat4(1.f), point1)[0][0], &view[0][0], &projection[0][0]);

		int loc = shader->getUniformLocation("vector");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&(point2 - point1)[0]);

		//	override mesh color
		loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->pointMesh->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
void Debug::capsule(const glm::vec3& point1, const glm::vec3& point2, const float& radius, Shader* shader)
{
	constexpr unsigned int quadrature = 32;										// capsule mesh was generated using this value
	constexpr unsigned int cylinderFaces = 6 * quadrature;						// number of faces on cylinder part
	constexpr unsigned int hemisphereFaces = 6 * quadrature * quadrature / 4;	// number of faces on one spherical part

	if (This->renderer && shader && This->capsuleMesh)
	{
		// prepare transform
		glm::vec3 center = 0.5f * (point1 + point2);
		glm::mat4 base = glm::translate(glm::mat4(1.f), center);
		glm::vec3 v = glm::cross(glm::vec3(0, 0, 1), point1 - point2);
		if (v != glm::vec3(0.f))
			base = base * glm::rotate(glm::angle(glm::vec3(0, 0, 1), glm::normalize(point1 - point2)), glm::normalize(v));
		else
			base = base * glm::rotate(glm::angle(glm::vec3(0, 0, 1), glm::normalize(point1 - point2)), glm::vec3(1, 0, 0));

		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(shader, &base[0][0], &view[0][0], &projection[0][0]);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	draw meshes
		float length = 0.5f * glm::length(point1 - point2);
		This->renderer->loadVAO(This->capsuleMesh->getVAO());

		glm::mat4 model = glm::scale(base, glm::vec3(radius, radius, length));
		This->renderer->loadMVPMatrix(shader, &model[0][0], &view[0][0], &projection[0][0]);
		glDrawElements(GL_TRIANGLES, cylinderFaces, GL_UNSIGNED_SHORT, NULL);

		model = glm::translate(base, glm::vec3(0, 0, length));
		model = glm::scale(model, glm::vec3(radius, radius, radius));
		This->renderer->loadMVPMatrix(shader, &model[0][0], &view[0][0], &projection[0][0]);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)(cylinderFaces * sizeof(unsigned short)));

		model = glm::translate(base, glm::vec3(0, 0, -length));
		model = glm::scale(model, glm::vec3(radius, radius, radius));
		This->renderer->loadMVPMatrix(shader, &model[0][0], &view[0][0], &projection[0][0]);
		glDrawElements(GL_TRIANGLES, (int)This->capsuleMesh->getFaces()->size(), GL_UNSIGNED_SHORT, (void*)((hemisphereFaces + cylinderFaces) * sizeof(unsigned short)));
	}
	else
	{

	}
}
void Debug::mesh(const Mesh* const mesh, const glm::mat4& transform, Shader* shader)
{
	if (This->renderer && shader && mesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(shader, &transform[0][0], &view[0][0], &projection[0][0]);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(mesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)mesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		//if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
//