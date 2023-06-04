#include "Debug.h"
#include <Resources/Shader.h>

#include <glm/gtx/vector_angle.hpp>


// Static initialization
vec4f Debug::color = vec4f(0.f, 0.f, 0.f, 1.f);
mat4f Debug::view = mat4f::identity;
mat4f Debug::projection = mat4f(1.f);
float Debug::viewportRatio = 1.f;

const vec4f Debug::black = vec4f(0.f, 0.f, 0.f, 1.f);
const vec4f Debug::white = vec4f(1.f, 1.f, 1.f, 1.f);
const vec4f Debug::magenta = vec4f(1.f, 0.f, 1.f, 1.f);
const vec4f Debug::orange = vec4f(0.992f, 0.415f, 0.008f, 1.f);
const vec4f Debug::grey = vec4f(0.2f, 0.2f, 0.2f, 1.f);
const vec4f Debug::red = vec4f(1.f, 0.f, 0.f, 1.f);
const vec4f Debug::green = vec4f(0.f, 1.f, 0.f, 1.f);
const vec4f Debug::blue = vec4f(0.f, 0.f, 1.f, 1.f);
const vec4f Debug::yellow = vec4f(1.f, 1.f, 0.f, 1.f);

const vec4f Debug::darkBlue = vec4f(0.103f, 0.103f, 0.403f, 1.f);
const vec4f Debug::darkGreen = vec4f(0.f, 0.25f, 0.f, 1.f);

//

//  Default
Debug::Debug() : renderer(nullptr), pointMesh(nullptr), cubeMesh(nullptr), sphereMesh(nullptr), capsuleMesh(nullptr), pointShader(nullptr), 
	segmentShader(nullptr), defaultShader(nullptr), wiredShader(nullptr), multipleSegmentShader(nullptr)
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
	ResourceManager::getInstance()->release(multipleSegmentShader);

	for (auto& it : This->vertexScratchBuffers)
	{
		glDeleteBuffers(1, &it.vbo);
	}
}
//

//	Public functions
void Debug::initialize(const std::string& pointMeshName, const std::string& cubeMeshName, const std::string& sphereMeshName, 
	const std::string& capsuleMeshName, const std::string& pointShaderName, const std::string& segmentShaderName, 
	const std::string& defaultShaderName, const std::string& wiredShaderName, const std::string& multipleSegmentShaderName)
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
	multipleSegmentShader = ResourceManager::getInstance()->getResource<Shader>(multipleSegmentShaderName);
}
void Debug::setDepthTest(bool enable)
{
	if (enable)
		glEnable(GL_DEPTH_TEST);// glDepthFunc(GL_LESS);
	else
		glDisable(GL_DEPTH_TEST);//glDepthFunc(GL_ALWAYS);
}
void Debug::drawLineCube(const mat4f& transform, const vec4f& size)
{
	vec4f x = size.x * vec4f(transform[0]);
	vec4f y = size.y * vec4f(transform[1]);
	vec4f z = size.z * vec4f(transform[2]);
	vec4f center = vec4f(transform[3]);

	std::vector<Vertex> vertices;
	vertices.reserve(24);
	vertices.push_back({ center + x + y + z, color }); vertices.push_back({ center + x + y - z, color });
	vertices.push_back({ center + x - y + z, color }); vertices.push_back({ center + x - y - z, color });
	vertices.push_back({ center - x + y + z, color }); vertices.push_back({ center - x + y - z, color });
	vertices.push_back({ center - x - y + z, color }); vertices.push_back({ center - x - y - z, color });

	vertices.push_back({ center + x + y + z, color }); vertices.push_back({ center + x - y + z, color });
	vertices.push_back({ center + x + y - z, color }); vertices.push_back({ center + x - y - z, color });
	vertices.push_back({ center - x + y + z, color }); vertices.push_back({ center - x - y + z, color });
	vertices.push_back({ center - x + y - z, color }); vertices.push_back({ center - x - y - z, color });

	vertices.push_back({ center + x + y + z, color }); vertices.push_back({ center - x + y + z, color });
	vertices.push_back({ center + x + y - z, color }); vertices.push_back({ center - x + y - z, color });
	vertices.push_back({ center + x - y + z, color }); vertices.push_back({ center - x - y + z, color });
	vertices.push_back({ center + x - y - z, color }); vertices.push_back({ center - x - y - z, color });

	drawMultipleLines(vertices, mat4f::identity);
}
//

// real draw functions
void Debug::point(const vec4f& p, Shader* shader)
{
	if (This->renderer && shader && This->pointMesh)
	{
		//	Get shader and prepare matrix
		mat4f m = mat4f::translate(mat4f::identity, p);
		Renderer::ModelMatrix modelMatrix = { m, mat4f::identity };
		This->renderer->loadModelMatrix(shader, &modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->pointMesh->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}
	else
	{

	}
}
void Debug::line(const vec4f& point1, const vec4f& point2, Shader* shader)
{
	if (This->renderer && shader && This->pointMesh)
	{
		//	Get shader and prepare matrix
		mat4f m = mat4f::translate(mat4f::identity, point1);
		Renderer::ModelMatrix modelMatrix = { m, mat4f::identity };
		This->renderer->loadModelMatrix(shader, &modelMatrix);

		int loc = shader->getUniformLocation("vector");
		if (loc >= 0) glUniform4fv(loc, 1, (float*)&(point2 - point1)[0]);

		//	override mesh color
		loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(This->pointMesh->getVAO());
		glDrawElements(GL_POINTS, 1, GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}
}
void Debug::capsule(const vec4f& point1, const vec4f& point2, const float& radius, Shader* shader)
{
	constexpr unsigned int quadrature = 32;										// capsule mesh was generated using this value
	constexpr unsigned int cylinderFaces = 6 * quadrature;						// number of faces on cylinder part
	constexpr unsigned int hemisphereFaces = 6 * quadrature * quadrature / 4;	// number of faces on one spherical part

	if (This->renderer && shader && This->capsuleMesh)
	{
		// prepare transform
		vec4f center = 0.5f * (point1 + point2);
		mat4f base = mat4f::translate(mat4f::identity, center);
		mat4f rotMat = mat4f::identity;
		vec4f v = vec4f::cross(vec4f(0, 0, 1, 0), point1 - point2);
		quatf orientation = quatf::identity;
		if (v != vec4f::zero)
		{
			float angle = asinf(v.getNorm() / (point1 - point2).getNorm());
			orientation = quatf(angle, (vec3f)v.getNormal());
			base = mat4f::rotate(base, orientation);
			rotMat = mat4f::rotate(rotMat, orientation);
		}

		//	Get shader and prepare matrix
		Renderer::ModelMatrix modelMatrix = { base, rotMat };
		This->renderer->loadModelMatrix(shader, &modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, (float*)&color);

		//	draw meshes
		float length = 0.5f * (point1 - point2).getNorm();
		This->renderer->loadVAO(This->capsuleMesh->getVAO());

		mat4f model = mat4f::scale(base, vec4f(radius, radius, length, 1.f));
		This->renderer->loadModelMatrix(shader, &modelMatrix);
		glDrawElements(GL_TRIANGLES, cylinderFaces, GL_UNSIGNED_SHORT, NULL);

		model = mat4f::translate(base, vec4f(0, 0, length, 0.f));
		model = mat4f::scale(model, vec4f(radius, radius, radius, 1.f));
		This->renderer->loadModelMatrix(shader, &modelMatrix);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)(cylinderFaces * sizeof(unsigned short)));

		model = mat4f::translate(base, vec4f(0, 0, -length, 0.f));
		model = mat4f::scale(model, vec4f(radius, radius, radius, 1.f));
		This->renderer->loadModelMatrix(shader, &modelMatrix);
		glDrawElements(GL_TRIANGLES, (int)This->capsuleMesh->getFaces()->size(), GL_UNSIGNED_SHORT, (void*)((hemisphereFaces + cylinderFaces) * sizeof(unsigned short)));

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}
}
void Debug::mesh(const Mesh* const mesh, const mat4f& transform, Shader* shader)
{
	if (This->renderer && shader && mesh)
	{
		//	Get shader and prepare matrix
		mat4f orientation = mat4f(quatf(transform));
		Renderer::ModelMatrix modelMatrix = { transform, orientation };
		This->renderer->loadModelMatrix(shader, &modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) 
			glUniform4fv(loc, 1, (float*)&color);

		//	Draw mesh
		This->renderer->loadVAO(mesh->getVAO());
		glDrawElements(GL_TRIANGLES, (int)mesh->getFaces()->size(), GL_UNSIGNED_SHORT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}
}
void Debug::drawMultipleLines(const std::vector<Vertex>& points, const mat4f& model)
{
	if (!This->multipleSegmentShader || !This->renderer)
		return;

	Renderer::ModelMatrix modelMatrix = { model, model };
	This->renderer->loadModelMatrix(This->multipleSegmentShader, &modelMatrix);

	constexpr size_t vboSize = sizeof(Vertex) * 4096;
	uint8_t* startPtr = (uint8_t*)points.data();
	uint8_t* endPtr = (uint8_t*)points.data() + sizeof(Vertex) * points.size();

	while (startPtr < endPtr)
	{
		VertexVBO* buffer = nullptr;
		for (auto& it : This->vertexScratchBuffers)
		{
			if (it.offset < vboSize)
			{
				buffer = &it;
				break;
			}
		}
		if (!buffer)
		{
			This->vertexScratchBuffers.emplace_back();
			buffer = &This->vertexScratchBuffers.back();
			buffer->offset = 0;

			glGenBuffers(1, &buffer->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
			glBufferData(GL_ARRAY_BUFFER, vboSize, nullptr, GL_DYNAMIC_DRAW);

			glGenVertexArrays(1, &buffer->vao);
			glBindVertexArray(buffer->vao);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);

			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec4f));
		}

		glBindVertexArray(0);
		int range = std::min((size_t)(endPtr - startPtr), vboSize - buffer->offset);

		glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
		glBufferSubData(GL_ARRAY_BUFFER, buffer->offset, range, startPtr);

		glBindVertexArray(buffer->vao);
		glDrawArrays(GL_LINES, (int)(buffer->offset / sizeof(Vertex)), (int)(range / sizeof(Vertex)));

		startPtr += range;
		buffer->offset += range;
	}
}
//

void Debug::clearVBOs()
{
	for (auto& it : This->vertexScratchBuffers)
		it.offset = 0;
}