#pragma once

#include <glm/glm.hpp>

#include "Singleton.h"
#include <Resources/ResourceManager.h>
#include <Renderer/Renderer.h>


class Debug : public Singleton<Debug>
{
	friend class Singleton<Debug>;
	friend class Renderer;

	public:
		// Structs
		struct Vertex
		{
			vec4f m_position;
			vec4f m_color;
		};
		//
		

		// Public functions
		void initialize(const std::string& cubeMeshName, const std::string& sphereMeshName, const std::string& capsuleMeshName, 
			const std::string& defaultShaderName, const std::string& wiredShaderName, const std::string& multiplePrimitiveShaderName,
			const std::string& textureReinterpreterShaderName);
		//

		//  Draw functions
		static void setDepthTest(bool enable);
		static void setFaceCulling(bool enable);
		static void setBlending(bool enable);

		static void drawPoint(const vec4f& p);
		static void drawCube(const mat4f& transform, const vec4f& size) { Debug::mesh(This->cubeMesh, mat4f::scale(transform, size), This->defaultShader); };
		static void drawSphere(const vec4f& center, const float& radius) { Debug::mesh(This->sphereMesh, mat4f::scale(mat4f::translate(mat4f::identity, center), vec4f(radius)), This->defaultShader); };
		static void drawLine(const vec4f& point1, const vec4f& point2);
		static void drawCapsule(const vec4f& point1, const vec4f& point2, const float& radius) { Debug::capsule(point1, point2, radius, This->defaultShader); };
		static void drawMesh(const Mesh* const mesh, const mat4f& transform) { Debug::mesh(mesh, transform, This->defaultShader); };

		static void drawMultiplePrimitive(const Vertex* vertices, const int& verticesCount, const mat4f& model, int drawMode);

		static void drawWiredCube(const mat4f& transform, const vec4f& size) { Debug::mesh(This->cubeMesh, mat4f::scale(transform, size), This->wiredShader); };
		static void drawWiredCube(const mat4f& transform, const vec4f& min, const vec4f& max)
		{
			Debug::mesh(This->cubeMesh, mat4f::scale(mat4f::translate(mat4f::identity, 0.5f * (max + min)) * transform, 0.5f * (max - min)), This->wiredShader);
		};
		static void drawWiredSphere(const vec4f& center, const float& radius) { Debug::mesh(This->sphereMesh, mat4f::scale(mat4f::translate(mat4f::identity, center), vec4f(radius)), This->wiredShader); };
		static void drawWiredCapsule(const vec4f& point1, const vec4f& point2, const float& radius) { Debug::capsule(point1, point2, radius, This->wiredShader); };
		static void drawWiredMesh(const Mesh* const mesh, const mat4f& transform) { Debug::mesh(mesh, transform, This->wiredShader); };

		static void drawLineCube(const mat4f& transform, const vec4f& size);
		static void drawLineCube(const mat4f& transform, const vec4f& min, const vec4f& max)
		{
			Debug::drawLineCube(transform * mat4f::translate(mat4f::identity, 0.5f * (max + min)), 0.5f * (max - min));
		}
		static void drawLineCapsule(const vec4f& point1, const vec4f& point2, const float& radius);

		static void reinterpreteTexture(const Texture* in, Texture* out, float layer = 0.f);
		//

		//	Log fuctions
		static void log(const std::string& message);
		static void logWarning(const std::string& message);
		static void logError(const std::string& message);
		//

		// Attributes
		static vec4f color;
		static mat4f view;
		static mat4f projection;
		static float viewportRatio;

		static const vec4f black;
		static const vec4f white;
		static const vec4f magenta;
		static const vec4f orange;
		static const vec4f grey;
		static const vec4f red;
		static const vec4f green;
		static const vec4f blue;
		static const vec4f yellow;

		static const vec4f darkBlue;
		static const vec4f darkGreen;
		//

	private:
		//  Default
		Debug();  //!< Default constructor.
		~Debug(); //!< Default destructor.
		//

		// real draw functions
		//static void point(const vec4f& p, Shader* shader);
		//static void line(const vec4f& point1, const vec4f& point2, Shader* shader);
		static void capsule(const vec4f& point1, const vec4f& point2, const float& radius, Shader* shader);
		static void mesh(const Mesh* const mesh, const mat4f& transform, Shader* shader);
		//

		//
		void clearVBOs();
		//

		// Attributes
		Renderer* renderer;

		Mesh* cubeMesh;
		Mesh* sphereMesh;
		Mesh* capsuleMesh;

		Shader* defaultShader;
		Shader* wiredShader;
		Shader* debug;

		struct VertexVBO
		{
			GLuint vbo, vao;
			int offset;
		};
		std::list<VertexVBO> vertexScratchBuffers;


		Shader* textureReinterpreter;
		GLuint textureReinterpreterFBO;
		//
};
