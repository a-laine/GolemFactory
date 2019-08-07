#include "Debug.h"
#include "Resources/Shader.h"

// Static initialization
glm::vec3 Debug::color = glm::vec3(0.f);
glm::mat4 Debug::view = glm::mat4(1.f);
glm::mat4 Debug::projection = glm::mat4(1.f);
//

//  Default
Debug::Debug() : renderer(nullptr), pointMesh(nullptr), cubeMesh(nullptr), sphereMesh(nullptr), pointShader(nullptr), segmentShader(nullptr), defaultShader(nullptr), wiredShader(nullptr)
{}
Debug::~Debug()
{
	ResourceManager::getInstance()->release(pointMesh);
	ResourceManager::getInstance()->release(cubeMesh);
	ResourceManager::getInstance()->release(sphereMesh);
	ResourceManager::getInstance()->release(pointShader);
	ResourceManager::getInstance()->release(segmentShader);
	ResourceManager::getInstance()->release(defaultShader);
	ResourceManager::getInstance()->release(wiredShader);
}
//

//	Public functions
void Debug::initialize(const std::string& pointMeshName, const std::string& cubeMeshName, const std::string& sphereMeshName, const std::string& pointShaderName, const std::string& segmentShaderName, const std::string& defaultShaderName, const std::string& wiredShaderName)
{
	renderer = Renderer::getInstance();
	pointMesh = ResourceManager::getInstance()->getResource<Mesh>(pointMeshName);
	cubeMesh = ResourceManager::getInstance()->getResource<Mesh>(cubeMeshName);
	sphereMesh = ResourceManager::getInstance()->getResource<Mesh>(sphereMeshName);
	pointShader = ResourceManager::getInstance()->getResource<Shader>(pointShaderName);
	segmentShader = ResourceManager::getInstance()->getResource<Shader>(segmentShaderName);
	defaultShader = ResourceManager::getInstance()->getResource<Shader>(defaultShaderName);
	wiredShader = ResourceManager::getInstance()->getResource<Shader>(wiredShaderName);
}
//

//  Draw functions
void Debug::drawPoint(const glm::vec3& p)
{
	if (This->renderer && This->pointShader && This->pointMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->pointShader, &glm::translate(glm::mat4(1.f), p)[0][0], &view[0][0], &projection[0][0]);
		if (!This->pointShader) return;

		//	override mesh color
		int loc = This->pointShader->getUniformLocation("overrideColor");
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
void Debug::drawCube(const glm::mat4& transform, const glm::vec3& size)
{
	if (This->renderer && This->defaultShader && This->cubeMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->defaultShader, &glm::scale(transform, size)[0][0], &view[0][0], &projection[0][0]);
		if (!This->defaultShader) return;

		//	override mesh color
		int loc = This->defaultShader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->cubeMesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)This->cubeMesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
void Debug::drawSphere(const glm::vec3& center, const float& radius)
{
	if (This->renderer && This->defaultShader && This->sphereMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->defaultShader, &glm::scale(glm::translate(glm::mat4(1.f), center), glm::vec3(radius))[0][0], &view[0][0], &projection[0][0]);
		if (!This->defaultShader) return;

		//	override mesh color
		int loc = This->defaultShader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->sphereMesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)This->sphereMesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
void Debug::drawLine(const glm::vec3& point1, const glm::vec3& point2)
{
	if (This->renderer && This->segmentShader && This->pointMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->segmentShader, &glm::translate(glm::mat4(1.f), point1)[0][0], &view[0][0], &projection[0][0]);
		if (!This->segmentShader) return;

		int loc = This->segmentShader->getUniformLocation("vector");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&(point2 - point1)[0]);

		//	override mesh color
		loc = This->segmentShader->getUniformLocation("overrideColor");
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
void Debug::drawMesh(const Mesh* const mesh, const glm::mat4& transform)
{
	if (This->renderer && This->defaultShader && mesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->defaultShader, &transform[0][0], &view[0][0], &projection[0][0]);
		if (!This->defaultShader) return;

		//	override mesh color
		int loc = This->defaultShader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(mesh->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}


void Debug::drawWiredCube(const glm::mat4& transform, const glm::vec3& size)
{
	if (This->renderer && This->wiredShader && This->cubeMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->wiredShader, &glm::scale(transform, size)[0][0], &view[0][0], &projection[0][0]);
		if (!This->wiredShader) return;

		//	override mesh color
		int loc = This->wiredShader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->cubeMesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)This->cubeMesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
void Debug::drawWiredSphere(const glm::vec3& center, const float& radius)
{
	if (This->renderer && This->wiredShader && This->sphereMesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->wiredShader, &glm::translate(glm::mat4(1.f), center)[0][0], &view[0][0], &projection[0][0]);
		if (!This->wiredShader) return;

		//	override mesh color
		int loc = This->wiredShader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->sphereMesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)This->sphereMesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
void Debug::drawWiredMesh(const Mesh* const mesh, const glm::mat4& transform)
{
	if (This->renderer && This->wiredShader && mesh)
	{
		//	Get shader and prepare matrix
		This->renderer->loadMVPMatrix(This->wiredShader, &transform[0][0], &view[0][0], &projection[0][0]);
		if (!This->wiredShader) return;

		//	override mesh color
		int loc = This->wiredShader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform3fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(mesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)mesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform3fv(loc, 1, (float*)&glm::vec3(-1.f, 0.f, 0.f)[0]);
	}
	else
	{

	}
}
//

