#include "Debug.h"
#include <Resources/ResourceManager.h>
#include <Renderer/Renderer.h>
#include <Renderer/DrawableComponent.h>
#include <Resources/Shader.h>
#include <Resources/Mesh.h>

#include <Renderer/GLDebugger.h>

//#include <glm/gtx/vector_angle.hpp>


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
	glGenFramebuffers(1, &textureReinterpreterFBO);			CheckGLError("Debug init", "glGenFramebuffers");
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
		glDeleteBuffers(1, &it.vbo);						CheckGLError("Debug shut", "glDeleteBuffers");
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
	{
		glEnable(GL_DEPTH_TEST);			CheckGLError("Debug change depthTest", "glEnable(GL_DEPTH_TEST)");
	}
	else
	{
		glDisable(GL_DEPTH_TEST);			CheckGLError("Debug change depthTest", "glDisable(GL_DEPTH_TEST)");
	}
}
void Debug::setFaceCulling(bool enable)
{
	if (enable)
	{
		glEnable(GL_CULL_FACE);				CheckGLError("Debug change culling", "glEnable(GL_CULL_FACE)");
	}
	else
	{
		glDisable(GL_CULL_FACE);			CheckGLError("Debug change culling", "glDisable(GL_CULL_FACE)");
	}
	glCullFace(GL_BACK);					CheckGLError("Debug change culling", "glCullFace(GL_BACK)");
	glFrontFace(GL_CW);						CheckGLError("Debug change culling", "glFrontFace(GL_CW)");
}
void Debug::setBlending(bool enable)
{
	if (enable)
	{
		glEnable(GL_BLEND);					CheckGLError("Debug change blending", "glEnable(GL_BLEND)");
	}
	else
	{
		glDisable(GL_BLEND);				CheckGLError("Debug change blending", "glDisable(GL_BLEND)");
	}
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
	constexpr unsigned int quadratureModulo = quadrature - 1;
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
		float a2 = ((i + 1) & quadratureModulo) * stepAngle;
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
			float b2 = ((j + 1) & quadratureModulo) * stepAngle;
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
void Debug::drawLineSphere(const vec4f& center, const float& radius, vec4f upaxis)
{
	constexpr unsigned int quadrature = 32;
	constexpr unsigned int quadratureModulo = quadrature - 1;
	constexpr float stepAngle = 2.f * PI / quadrature;
	constexpr unsigned int quarterQuadrature = quadrature / 4;

	vec4f axis = upaxis;
	axis.normalize();
	vec4f axis_n0 = (std::abs(axis.x) > std::abs(axis.z) ? vec4f(-axis.y, axis.x, 0, 0) : vec4f(0, -axis.z, axis.y, 0)).getNormal();
	vec4f axis_n1 = vec4f::cross(axis, axis_n0);

	std::vector<Vertex> vertices;
	vertices.reserve(6 * quadrature + 8 * quadrature + quarterQuadrature);
	for (int i = 0; i < quadrature; i++)
	{
		float a1 = i * stepAngle;
		float a2 = ((i + 1) & quadratureModulo) * stepAngle;
		float rca1 = radius * cos(a1);
		float rsa1 = radius * sin(a1);
		float rca2 = radius * cos(a2);
		float rsa2 = radius * sin(a2);

		for (int j = 0; j < quarterQuadrature; j++)
		{
			float b1 = j * stepAngle;
			float b2 = ((j + 1) & quadratureModulo) * stepAngle;
			float rcb1 = cos(b1);
			float rsb1 = sin(b1);
			float rcb2 = cos(b2);
			float rsb2 = sin(b2);

			Vertex c4 = { center + (rcb1 * rca1) * axis_n0 + (rcb1 * rsa1) * axis_n1 - (radius * rsb1) * axis, Debug::color };
			Vertex c6 = { center + (rcb2 * rca1) * axis_n0 + (rcb2 * rsa1) * axis_n1 - (radius * rsb2) * axis, Debug::color };
			Vertex c7 = { center + (rcb2 * rca2) * axis_n0 + (rcb2 * rsa2) * axis_n1 - (radius * rsb2) * axis, Debug::color };

			vertices.push_back(c6); vertices.push_back(c7);
			vertices.push_back(c4); vertices.push_back(c6);

			Vertex c8 = { center + (rcb1 * rca1) * axis_n0 + (rcb1 * rsa1) * axis_n1 + (radius * rsb1) * axis, Debug::color };
			Vertex c10 = { center + (rcb2 * rca1) * axis_n0 + (rcb2 * rsa1) * axis_n1 + (radius * rsb2) * axis, Debug::color };
			Vertex c11 = { center + (rcb2 * rca2) * axis_n0 + (rcb2 * rsa2) * axis_n1 + (radius * rsb2) * axis, Debug::color };

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
		mat4f base = mat4f::identity;
		mat4f rotMat = mat4f::identity;
		vec4f axis = (point1 - point2).getNormal();
		vec4f v = vec4f::cross(vec4f(0, 0, 1, 0), axis);
		quatf orientation = quatf::identity;
		if (v != vec4f::zero)
		{
			float angle = asinf(v.getNorm());
			orientation = quatf(angle, v.getNormal().xyz());
			base = mat4f::rotate(mat4f::identity, orientation);
			rotMat = base;
		}
		base[3] = center;

		//	Get shader and prepare matrix
		Renderer::ModelMatrix modelMatrix = { base, rotMat };
		This->renderer->bindMaterial(nullptr, shader);
		This->renderer->loadMatrices(shader, (float*)&modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) 
		{
			glUniform4fv(loc, 1, (float*)&color); CheckGLError("Debug set color", "glUniform4fv()");
		}

		//	draw meshes
		float length = 0.5f * (point1 - point2).getNorm();
		This->renderer->loadVAO(This->capsuleMesh->getVAO());

		mat4f model = mat4f::scale(base, vec4f(radius, radius, length, 1.f));
		modelMatrix.modelMatrix = model;
		This->renderer->bindMaterial(nullptr, shader);
		This->renderer->loadMatrices(shader, (float*)&modelMatrix);
		glDrawElements(GL_TRIANGLES, cylinderFaces, GL_UNSIGNED_SHORT, NULL);
		CheckGLError("Debug draw", "glDrawElements()");

		model = mat4f::translate(base, vec4f(0, 0, length, 0.f));
		model = mat4f::scale(model, vec4f(radius, radius, radius, 1.f));
		modelMatrix.modelMatrix = model;
		This->renderer->bindMaterial(nullptr, shader);
		This->renderer->loadMatrices(shader, (float*)&modelMatrix);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)(cylinderFaces * sizeof(unsigned short)));
		CheckGLError("Debug draw", "glDrawElements()");

		model = mat4f::translate(base, vec4f(0, 0, -length, 0.f));
		model = mat4f::scale(model, vec4f(radius, radius, radius, 1.f));
		modelMatrix.modelMatrix = model;
		This->renderer->bindMaterial(nullptr, shader);
		This->renderer->loadMatrices(shader, (float*)&modelMatrix);
		glDrawElements(GL_TRIANGLES, hemisphereFaces, GL_UNSIGNED_SHORT, (void*)((cylinderFaces + hemisphereFaces) * sizeof(unsigned short)));
		CheckGLError("Debug draw", "glDrawElements()");

		if (loc >= 0) 
		{
			glUniform4fv(loc, 1, &defaultColorUniform[0]); CheckGLError("Debug set color", "glUniform4fv()");
		}
	}
}
void Debug::mesh(const Mesh* const mesh, const mat4f& transform, Shader* shader)
{
	if (This->renderer && shader && mesh)
	{
		//	Get shader and prepare matrix
		mat4f orientation = mat4f(transform.extractRotation());
		Renderer::ModelMatrix modelMatrix = { transform, orientation };
		This->renderer->bindMaterial(nullptr, shader);
		This->renderer->loadMatrices(shader, (float*)&modelMatrix);

		//	override mesh color
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) 
		{
			glUniform4fv(loc, 1, (float*)&color); CheckGLError("Debug set color", "glUniform4fv()");
		}

		//	Draw mesh
		//glBindVertexArray(mesh->getVAO());
		This->renderer->loadVAO(mesh->getVAO());
		glDrawElements(GL_TRIANGLES, mesh->getNumberIndices(), mesh->getIndicesType(), NULL);

		if (loc >= 0) 
		{
			glUniform4fv(loc, 1, &defaultColorUniform[0]); CheckGLError("Debug set color", "glUniform4fv()");
		}
	}
}
void Debug::drawMultiplePrimitive(const Vertex* vertices, const unsigned int& verticesCount, const mat4f& model, unsigned int drawMode)
{
	if (!This->debug || !This->renderer)
		return;

	Renderer::ModelMatrix modelMatrix = { model, model };
	This->renderer->bindMaterial(nullptr, This->debug);
	This->renderer->loadMatrices(This->debug, (float*)&modelMatrix);

	constexpr size_t vboSize = sizeof(Vertex) * 4096;
	uint8_t* startPtr = (uint8_t*)vertices;
	uint8_t* endPtr = startPtr + sizeof(Vertex) * verticesCount;
	int neededSize = sizeof(Vertex);
	switch (drawMode)
	{
		case GL_POINTS: case GL_POINT: neededSize = 1 * sizeof(Vertex); break;
		case GL_LINES: case GL_LINE: neededSize = 2 * sizeof(Vertex); break;
		case GL_LINE_LOOP: neededSize = verticesCount * sizeof(Vertex); break;
		case GL_TRIANGLES: neededSize = 3 * sizeof(Vertex); break;
		default:
			GF_ASSERT_MSG(false, "Invalid primitive type");
			break;
	}

	GF_ASSERT_MSG(neededSize < vboSize, "Too many primitive, cannot fit in one scartch buffer page ! (try split it)");

	while (startPtr < endPtr)
	{
		VertexVBO* buffer = nullptr;
		for (auto& it : This->vertexScratchBuffers)
		{
			if (it.offset < vboSize && vboSize > neededSize + it.offset)
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

			glGenBuffers(1, &buffer->vbo);										CheckGLError("extendScratchBuffer", "glGenBuffers()");
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);							CheckGLError("extendScratchBuffer", "glBindBuffer()");
			glBufferData(GL_ARRAY_BUFFER, vboSize, nullptr, GL_DYNAMIC_DRAW);	CheckGLError("extendScratchBuffer", "glBufferData()");

			glGenVertexArrays(1, &buffer->vao);									CheckGLError("extendScratchBuffer", "glGenVertexArrays()");
			glBindVertexArray(buffer->vao);										CheckGLError("extendScratchBuffer", "glBindVertexArray()");

			glEnableVertexAttribArray(0);										CheckGLError("extendScratchBuffer", "glEnableVertexAttribArray()");
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);							CheckGLError("extendScratchBuffer", "glBindBuffer()");
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL); CheckGLError("extendScratchBuffer", "glVertexAttribPointer()");

			glEnableVertexAttribArray(1);										CheckGLError("extendScratchBuffer", "glEnableVertexAttribArray()");
			glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);							CheckGLError("extendScratchBuffer", "glBindBuffer()");
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec4f)); CheckGLError("extendScratchBuffer", "glVertexAttribPointer()");
			glBindBuffer(GL_ARRAY_BUFFER, 0);									CheckGLError("extendScratchBuffer", "glBindBuffer()");
			glBindVertexArray(0);												CheckGLError("extendScratchBuffer", "glBindVertexArray()");
		}

		glBindVertexArray(0);													CheckGLError("debug multiDraw", "glBindVertexArray()");

		int range = (int)std::min((size_t)(endPtr - startPtr), vboSize - buffer->offset);
		range = ((int)(range / neededSize)) * neededSize;
		GF_ASSERT_MSG(range > 0, "Error");

		glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);								CheckGLError("debug multiDraw", "glBindBuffer()");
		glBufferSubData(GL_ARRAY_BUFFER, buffer->offset, range, startPtr);		CheckGLError("debug multiDraw", "glBufferSubData()");
		glBindBuffer(GL_ARRAY_BUFFER, 0);										CheckGLError("debug multiDraw", "glBindBuffer()");

		glBindVertexArray(buffer->vao);											CheckGLError("debug multiDraw", "glBindVertexArray()");
		glDrawArrays(drawMode, (int)(buffer->offset / sizeof(Vertex)), (int)(range / sizeof(Vertex))); CheckGLError("debug multiDraw", "glDrawArrays()");
		glBindVertexArray(0);													CheckGLError("debug multiDraw", "glBindVertexArray()");

		startPtr += range;
		buffer->offset += range;
	}
	GF_ASSERT_MSG(startPtr == endPtr, "what?");
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

		glActiveTexture(GL_TEXTURE0);						CheckGLError("reinterpreteTexture", "glActiveTexture()");
		glBindTexture(GL_TEXTURE_2D, out->getTextureId());	CheckGLError("reinterpreteTexture", "glBindTexture()");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, in->size.x, in->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		CheckGLError("reinterpreteTexture", "glTexImage2D()");
	}

	// bing framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, This->textureReinterpreterFBO);					CheckGLError("reinterpreteTexture", "glBindFramebuffer()");
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, out->getTextureId(), 0);	CheckGLError("reinterpreteTexture", "glFramebufferTexture()");
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);														CheckGLError("reinterpreteTexture", "glDrawBuffers()");
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Debug::reinterpreteTexture" << std::endl;
		return;
	}

	// draw
	glViewport(0, 0, in->size.x, in->size.y);											CheckGLError("reinterpreteTexture", "glViewport()");
	This->renderer->lastShader = This->textureReinterpreter;
	This->renderer->lastVAO = This->renderer->fullscreenVAO;
	This->textureReinterpreter->enable();

	if (in->m_internalFormat == GL_DEPTH_COMPONENT32F)
	{
		if (in->size.z > 0)
		{
			if (in->m_type == GL_TEXTURE_2D_ARRAY)
			{
				glActiveTexture(GL_TEXTURE0);											CheckGLError("reinterpreteTexture", "glActiveTexture()");
				glBindTexture(GL_TEXTURE_2D_ARRAY, in->getTextureId());					CheckGLError("reinterpreteTexture", "glBindTexture()");

				int loc = This->textureReinterpreter->getUniformLocation("type");
				if (loc >= 0)
				{
					glUniform1f(loc, 0);												CheckGLError("reinterpreteTexture", "glUniform1f(type)");
				}
			}
			else if (in->m_type == GL_TEXTURE_CUBE_MAP_ARRAY)
			{
				glActiveTexture(GL_TEXTURE1);											CheckGLError("reinterpreteTexture", "glActiveTexture()");
				glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, in->getTextureId());			CheckGLError("reinterpreteTexture", "glBindTexture()");

				int loc = This->textureReinterpreter->getUniformLocation("type");
				if (loc >= 0)
				{
					glUniform1f(loc, 1);												CheckGLError("reinterpreteTexture", "glUniform1f(type)");
				}
			}
		}
		else
		{
			if (in->m_type == GL_TEXTURE_2D)
			{
				glActiveTexture(GL_TEXTURE2);											CheckGLError("reinterpreteTexture", "glActiveTexture()");
				glBindTexture(GL_TEXTURE_2D, in->getTextureId());						CheckGLError("reinterpreteTexture", "glBindTexture()");

				int loc = This->textureReinterpreter->getUniformLocation("type");
				if (loc >= 0)
				{
					glUniform1f(loc, 2);												CheckGLError("reinterpreteTexture", "glUniform1f(type)");
				}
			}
		}
	}
	else if (in->m_internalFormat == GL_RGBA16UI)
	{
		if (in->m_type == GL_TEXTURE_2D)
		{
			glActiveTexture(GL_TEXTURE0);												CheckGLError("reinterpreteTexture", "glActiveTexture()");
			glBindImageTexture(0, in->getTextureId(), 0, GL_TRUE, 0, GL_READ_ONLY, in->m_internalFormat); CheckGLError("reinterpreteTexture", "glBindImageTexture()");

			int loc = This->textureReinterpreter->getUniformLocation("type");
			if (loc >= 0)
			{
				glUniform1f(loc, 3);													CheckGLError("reinterpreteTexture", "glUniform1f(type)");
			}
		}
	}
	else if (in->m_type == GL_TEXTURE_2D_ARRAY)
	{
		glActiveTexture(GL_TEXTURE0);													CheckGLError("reinterpreteTexture", "glActiveTexture()");
		glBindTexture(GL_TEXTURE_2D_ARRAY, in->getTextureId());							CheckGLError("reinterpreteTexture", "glBindTexture()");

		int loc = This->textureReinterpreter->getUniformLocation("type");
		if (loc >= 0)
		{
			glUniform1f(loc, 4);														CheckGLError("reinterpreteTexture", "glUniform1f(type)");
		}
	}
	else if (in->m_type == GL_TEXTURE_CUBE_MAP)
	{
		glActiveTexture(GL_TEXTURE3);													CheckGLError("reinterpreteTexture", "glActiveTexture()");
		glBindTexture(GL_TEXTURE_CUBE_MAP, in->getTextureId());							CheckGLError("reinterpreteTexture", "glBindTexture()");

		int loc = This->textureReinterpreter->getUniformLocation("type");
		if (loc >= 0)
		{
			glUniform1f(loc, 5);														CheckGLError("reinterpreteTexture", "glUniform1f(type)");
		}
	}

	int loc = This->textureReinterpreter->getUniformLocation("layer");
	if (loc >= 0)
	{
		glUniform1f(loc, layer);														CheckGLError("reinterpreteTexture", "glUniform1f(layer)");
	}


	glBindVertexArray(This->renderer->lastVAO);											CheckGLError("reinterpreteTexture", "glBindVertexArray()");
	glDrawArrays(GL_TRIANGLES, 0, 3);													CheckGLError("reinterpreteTexture", "glDrawArrays()");

	// end
	glBindFramebuffer(GL_FRAMEBUFFER, 0);												CheckGLError("reinterpreteTexture", "glBindFramebuffer(0)");
}
//

void Debug::clearVBOs()
{
	for (auto& it : This->vertexScratchBuffers)
		it.offset = 0;
}