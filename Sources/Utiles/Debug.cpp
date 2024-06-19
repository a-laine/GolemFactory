#include "Debug.h"
#include <Resources/Shader.h>
#include <Renderer/DrawableComponent.h>

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
Debug::Debug() : renderer(nullptr),  cubeMesh(nullptr), sphereMesh(nullptr), capsuleMesh(nullptr), 
	 defaultShader(nullptr), wiredShader(nullptr), debug(nullptr), textureReinterpreter(nullptr)
{
	glGenFramebuffers(1, &textureReinterpreterFBO);
}
Debug::~Debug()
{
	ResourceManager::getInstance()->release(cubeMesh);
	ResourceManager::getInstance()->release(sphereMesh);
	ResourceManager::getInstance()->release(capsuleMesh);

	ResourceManager::getInstance()->release(defaultShader);
	ResourceManager::getInstance()->release(wiredShader);
	ResourceManager::getInstance()->release(debug);

	for (auto& it : This->vertexScratchBuffers)
	{
		glDeleteBuffers(1, &it.vbo);
	}
}
//

//	Public functions
void Debug::initialize(const std::string& cubeMeshName, const std::string& sphereMeshName, const std::string& capsuleMeshName, 
	const std::string& defaultShaderName, const std::string& wiredShaderName, const std::string& multiplePrimitiveShaderName,
	const std::string& textureReinterpreterShaderName)
{
	renderer = Renderer::getInstance();
	cubeMesh = ResourceManager::getInstance()->getResource<Mesh>(cubeMeshName);
	sphereMesh = ResourceManager::getInstance()->getResource<Mesh>(sphereMeshName);
	capsuleMesh = ResourceManager::getInstance()->getResource<Mesh>(capsuleMeshName);

	defaultShader = ResourceManager::getInstance()->getResource<Shader>(defaultShaderName);
	wiredShader = defaultShader->getVariant(Shader::computeVariantCode(false, 0, true));// ResourceManager::getInstance()->getResource<Shader>(wiredShaderName);
	debug = ResourceManager::getInstance()->getResource<Shader>(multiplePrimitiveShaderName);

	textureReinterpreter = ResourceManager::getInstance()->getResource<Shader>(textureReinterpreterShaderName);
}
void Debug::setDepthTest(bool enable)
{
	if (enable)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}
void Debug::setFaceCulling(bool enable)
{
	if (enable)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
}
void Debug::setBlending(bool enable)
{
	if (enable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void Debug::drawPoint(const vec4f& p)
{
	Vertex point = {p, color};
	drawMultiplePrimitive(&point, 1, mat4f::identity, GL_POINTS);
}
void Debug::drawLine(const vec4f& point1, const vec4f& point2)
{
	Vertex vertices[2];
	vertices[0] = { point1, color }; vertices[1] = { point2, color };
	drawMultiplePrimitive(vertices, 2, mat4f::identity, GL_LINES);
}
void Debug::drawLineCube(const mat4f& transform, const vec4f& size)
{
	vec4f x = size.x * vec4f(transform[0]);
	vec4f y = size.y * vec4f(transform[1]);
	vec4f z = size.z * vec4f(transform[2]);
	vec4f center = vec4f(transform[3]);

	Vertex vertices[24];
	vertices[0] = { center + x + y + z, color }; vertices[1] = { center + x + y - z, color };
	vertices[2] = { center + x - y + z, color }; vertices[3] = { center + x - y - z, color };
	vertices[4] = { center - x + y + z, color }; vertices[5] = { center - x + y - z, color };
	vertices[6] = { center - x - y + z, color }; vertices[7] = { center - x - y - z, color };

	vertices[8] = { center + x + y + z, color }; vertices[9] = { center + x - y + z, color };
	vertices[10] = { center + x + y - z, color }; vertices[11] = { center + x - y - z, color };
	vertices[12] = { center - x + y + z, color }; vertices[13] = { center - x - y + z, color };
	vertices[14] = { center - x + y - z, color }; vertices[15] = { center - x - y - z, color };

	vertices[16] = { center + x + y + z, color }; vertices[17] = { center - x + y + z, color };
	vertices[18] = { center + x + y - z, color }; vertices[19] = { center - x + y - z, color };
	vertices[20] = { center + x - y + z, color }; vertices[21] = { center - x - y + z, color };
	vertices[22] = { center + x - y - z, color }; vertices[23] = { center - x - y - z, color };

	drawMultiplePrimitive(vertices, 24, mat4f::identity, GL_LINES);
}
void Debug::drawLineCapsule(const vec4f& point1, const vec4f& point2, const float& radius)
{
	constexpr unsigned int quadrature = 32;
	constexpr unsigned int cylinderFaces = quadrature;
	constexpr float stepAngle = 2.f * PI / quadrature;
	constexpr unsigned int quarterQuadrature = quadrature / 4;

	vec4f axis = point2 - point1;
	float height = 0.5f * axis.getNorm();
	axis.normalize();
	vec4f axis_n0 = (std::abs(axis.x) > std::abs(axis.z) ? vec4f(-axis.y, axis.x, 0, 0) : vec4f(0, -axis.z, axis.y, 0)).getNormal();
	vec4f axis_n1 = vec4f::cross(axis, axis_n0);

	std::vector<Vertex> vertices;
	vertices.reserve(6 * quadrature + 8 * quadrature + quarterQuadrature);
	for (int i = 0; i < quadrature; i++)
	{
		float a1 = i * stepAngle;
		float a2 = ((i + 1) % quadrature) * stepAngle;
		float rca1 = radius * cos(a1);
		float rsa1 = radius * sin(a1);
		float rca2 = radius * cos(a2);
		float rsa2 = radius * sin(a2);

		Vertex c0 = { point1 + rca1 * axis_n0 + rsa1 * axis_n1, Debug::color };
		Vertex c1 = { point1 + rca2 * axis_n0 + rsa2 * axis_n1, Debug::color };
		Vertex c2 = { point2 + rca1 * axis_n0 + rsa1 * axis_n1, Debug::color };
		Vertex c3 = { point2 + rca2 * axis_n0 + rsa2 * axis_n1, Debug::color };

		vertices.push_back(c0); vertices.push_back(c1);
		vertices.push_back(c2); vertices.push_back(c3);
		vertices.push_back(c0); vertices.push_back(c2);

		for (int j = 0; j < quarterQuadrature; j++)
		{
			float b1 = j * stepAngle;
			float b2 = ((j + 1) % quadrature) * stepAngle;
			float rcb1 = cos(b1);
			float rsb1 = sin(b1);
			float rcb2 = cos(b2);
			float rsb2 = sin(b2);

			Vertex c4 = { point1 + (rcb1 * rca1) * axis_n0 + (rcb1 * rsa1) * axis_n1 - (radius * rsb1) * axis, Debug::color };
			Vertex c6 = { point1 + (rcb2 * rca1) * axis_n0 + (rcb2 * rsa1) * axis_n1 - (radius * rsb2) * axis, Debug::color };
			Vertex c7 = { point1 + (rcb2 * rca2) * axis_n0 + (rcb2 * rsa2) * axis_n1 - (radius * rsb2) * axis, Debug::color };

			vertices.push_back(c6); vertices.push_back(c7);
			vertices.push_back(c4); vertices.push_back(c6);

			Vertex c8 = { point2 + (rcb1 * rca1) * axis_n0 + (rcb1 * rsa1) * axis_n1 + (radius * rsb1) * axis, Debug::color };
			Vertex c10 = { point2 + (rcb2 * rca1) * axis_n0 + (rcb2 * rsa1) * axis_n1 + (radius * rsb2) * axis, Debug::color };
			Vertex c11 = { point2 + (rcb2 * rca2) * axis_n0 + (rcb2 * rsa2) * axis_n1 + (radius * rsb2) * axis, Debug::color };

			vertices.push_back(c10); vertices.push_back(c11);
			vertices.push_back(c8);  vertices.push_back(c10);
		}
	}
	drawMultiplePrimitive(vertices.data(), (unsigned int)vertices.size(), mat4f::identity, GL_LINES);
}
//

// real draw functions
const vec4f defaultColorUniform = vec4f(-1.f, 0.f, 0.f, 1.f);
void Debug::capsule(const vec4f& point1, const vec4f& point2, const float& radius, Shader* shader)
{
	constexpr unsigned int quadrature = 32;										// capsule mesh was generated using this value
	constexpr unsigned int cylinderFaces = 6 * quadrature;						// number of faces on cylinder part
	constexpr unsigned int hemisphereFaces = 6 * quadrature * quadrature / 4;	// number of faces on one spherical part

	if (This->renderer && shader && This->capsuleMesh)
	{
		// prepare transform
		vec4f center = 0.5f * (point1 + point2);
		mat4f base = mat4f::identity;// = mat4f::translate(mat4f::identity, center);
		//vec4f tmp = base[1];
		//base[1] = base[2];
		//base[2] = tmp;
		mat4f rotMat = mat4f::identity;
		vec4f axis = (point1 - point2).getNormal();
		vec4f v = vec4f::cross(vec4f(0, 0, 1, 0), axis);
		quatf orientation = quatf::identity;
		if (v != vec4f::zero)
		{
			float angle = asinf(v.getNorm());
			orientation = quatf(angle, (vec3f)v.getNormal());
			base = mat4f::rotate(mat4f::identity, orientation);
			rotMat = base;
		}
		base[3] = center;

		//	Get shader and prepare matrix
		Renderer::ModelMatrix modelMatrix = { base, rotMat };
		This->renderer->loadInstanceMatrices(shader, (float*)&modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, (float*)&color);

		//	draw meshes
		float length = 0.5f * (point1 - point2).getNorm();
		This->renderer->loadVAO(This->capsuleMesh->getVAO());

		mat4f model = mat4f::scale(base, vec4f(radius, radius, length, 1.f));
		modelMatrix.modelMatrix = model;
		This->renderer->loadInstanceMatrices(shader, (float*)&modelMatrix);
		glDrawElements(GL_TRIANGLES, cylinderFaces, GL_UNSIGNED_SHORT, NULL);

		model = mat4f::translate(base, vec4f(0, 0, length, 0.f));
		model = mat4f::scale(model, vec4f(radius, radius, radius, 1.f));
		modelMatrix.modelMatrix = model;
		This->renderer->loadInstanceMatrices(shader, (float*)&modelMatrix);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)(cylinderFaces * sizeof(unsigned short)));

		model = mat4f::translate(base, vec4f(0, 0, -length, 0.f));
		model = mat4f::scale(model, vec4f(radius, radius, radius, 1.f));
		modelMatrix.modelMatrix = model;
		This->renderer->loadInstanceMatrices(shader, (float*)&modelMatrix);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)((cylinderFaces + hemisphereFaces) * sizeof(unsigned short)));

		if (loc >= 0) glUniform4fv(loc, 1, &defaultColorUniform[0]);
	}
}
void Debug::mesh(const Mesh* const mesh, const mat4f& transform, Shader* shader)
{
	if (This->renderer && shader && mesh)
	{
		//	Get shader and prepare matrix
		mat4f orientation = mat4f(quatf(transform));
		Renderer::ModelMatrix modelMatrix = { transform, orientation };
		This->renderer->loadInstanceMatrices(shader, (float*)&modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) 
			glUniform4fv(loc, 1, (float*)&color);

		//	Draw mesh
		//glBindVertexArray(mesh->getVAO());
		This->renderer->loadVAO(mesh->getVAO());
		glDrawElements(GL_TRIANGLES, mesh->getNumberIndices(), mesh->getIndicesType(), NULL);

		if (loc >= 0) glUniform4fv(loc, 1, &defaultColorUniform[0]);
	}
}
void Debug::drawMultiplePrimitive(const Vertex* vertices, const unsigned int& verticesCount, const mat4f& model, unsigned int drawMode)
{
	if (!This->debug || !This->renderer)
		return;

	Renderer::ModelMatrix modelMatrix = { model, model };
	This->renderer->loadInstanceMatrices(This->debug, (float*)&modelMatrix);

	constexpr size_t vboSize = sizeof(Vertex) * 4096;
	uint8_t* startPtr = (uint8_t*)vertices;
	uint8_t* endPtr = startPtr + sizeof(Vertex) * verticesCount;

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
		int range = (int)std::min((size_t)(endPtr - startPtr), vboSize - buffer->offset);

		glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
		glBufferSubData(GL_ARRAY_BUFFER, buffer->offset, range, startPtr);

		glBindVertexArray(buffer->vao);
		glDrawArrays(drawMode, (int)(buffer->offset / sizeof(Vertex)), (int)(range / sizeof(Vertex)));

		startPtr += range;
		buffer->offset += range;
	}
}


void Debug::reinterpreteTexture(const Texture* in, Texture* out, float layer)
{
	if (!This->renderer || !This->textureReinterpreter)
		return;

	// resize out
	if (out->size.x != in->size.x || out->size.y != in->size.y)
	{
		out->size.x = in->size.x;
		out->size.y = in->size.y;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, out->getTextureId());
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, in->size.x, in->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	// bing framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, This->textureReinterpreterFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, out->getTextureId(), 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Debug::reinterpreteTexture" << std::endl;
		return;
	}

	// draw
	glViewport(0, 0, in->size.x, in->size.y);
	This->renderer->lastShader = This->textureReinterpreter;
	This->renderer->lastVAO = This->renderer->fullscreenVAO;
	This->textureReinterpreter->enable();

	if (in->m_internalFormat == GL_DEPTH_COMPONENT32F)
	{
		if (in->size.z > 0)
		{
			if (in->m_type == GL_TEXTURE_2D_ARRAY)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, in->getTextureId());

				int loc = This->textureReinterpreter->getUniformLocation("type");
				if (loc >= 0)
					glUniform1f(loc, 0);
			}
			else if (in->m_type == GL_TEXTURE_CUBE_MAP_ARRAY)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, in->getTextureId());

				int loc = This->textureReinterpreter->getUniformLocation("type");
				if (loc >= 0)
					glUniform1f(loc, 1);
			}
		}
		else
		{
			if (in->m_type == GL_TEXTURE_2D)
			{
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, in->getTextureId());

				int loc = This->textureReinterpreter->getUniformLocation("type");
				if (loc >= 0)
					glUniform1f(loc, 2);
			}
		}
	}
	else if (in->m_internalFormat == GL_RGBA16UI)
	{
		if (in->m_type == GL_TEXTURE_2D)
		{

			glActiveTexture(GL_TEXTURE0);
			glBindImageTexture(0, in->getTextureId(), 0, GL_TRUE, 0, GL_READ_ONLY, in->m_internalFormat);

			int loc = This->textureReinterpreter->getUniformLocation("type");
			if (loc >= 0)
				glUniform1f(loc, 3);
		}
	}
	else if (in->m_type == GL_TEXTURE_2D_ARRAY)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, in->getTextureId());

		int loc = This->textureReinterpreter->getUniformLocation("type");
		if (loc >= 0)
			glUniform1f(loc, 4);
	}

	int loc = This->textureReinterpreter->getUniformLocation("layer");
	if (loc >= 0)
		glUniform1f(loc, layer);


	glBindVertexArray(This->renderer->lastVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// end
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
//

void Debug::clearVBOs()
{
	for (auto& it : This->vertexScratchBuffers)
		it.offset = 0;
}