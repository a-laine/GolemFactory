#pragma once

#include <GL/glew.h>


#include <glm/glm.hpp>

#include "Utiles/Mutex.h"

#include <GLFW/glfw3.h>

#include "Utiles/Camera.h"
#include "Utiles/Singleton.h"
#include "World/World.h"


class Shader;
class Mesh;

class Renderer : public Singleton<Renderer>
{
	friend class Singleton<Renderer>;

	public:
		//	Miscellaneous
		enum ShaderIdentifier
		{
			INSTANCE_DRAWABLE = 1,
			INSTANCE_ANIMATABLE,
			HUD,
			GRID,

			INSTANCE_DRAWABLE_BB,
			INSTANCE_ANIMATABLE_BB
		};
		enum RenderOption
		{
			DEFAULT,
			BOUNDING_BOX
		};
		//

		//  Public functions
		void initGLEW(int verbose = 1);
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const glm::vec3& color = glm::vec3(0.4f, 0.2f, 0.1f));
		void render(Camera* renderCam);
		void renderHUD(Camera* renderCam);
		//

		//  Set/get functions
		void setCamera(Camera* cam);
		void setWorld(World* currentWorld);
		void setWindow(GLFWwindow* win);
		void setShader(ShaderIdentifier id, Shader* s);
		void setGridVisible(bool enable);
		void setRenderOption(const RenderOption& option);
		
		Camera* getCamera();
		World* getWorld();
		GLFWwindow* getWindow();
		Shader* getShader(ShaderIdentifier id);
		bool isGridVisible();
		unsigned int getNbDrawnInstances() const;
		unsigned int getNbDrawnTriangles() const;
		RenderOption getRenderOption() const;
		//

		//	Render function
		void drawObject(Entity* object, const float* view, const float* projection);
		//

	private:
		//  Default
		Renderer();
		~Renderer();
		//

		//	Miscellaneous
		struct EntityCompareDistance
		{
			glm::vec3 p;
			EntityCompareDistance(const glm::vec3& cameraPosition) : p(cameraPosition) {};
			bool operator()(Entity* a, Entity* b) const
			{
				const float d1 = glm::dot(a->getPosition() - p, a->getPosition() - p);
				const float d2 = glm::dot(b->getPosition() - p, b->getPosition() - p);
				return d1 < d2;
			}
		};
		//

		//	Protected functions
		void loadMVPMatrix(Shader* shader, const float* model, const float* view, const float* projection, const int& modelSize = 1);
		void loadVAO(const GLuint& vao);
		void drawInstancedObject(Shader* s, Mesh* m, std::vector<glm::mat4>& models, const float* view, const float* projection);
		//

		//  Attributes
		GLFWwindow* window;
		Camera* camera;
		World* world;
		std::map<ShaderIdentifier, Shader*> defaultShader;
		RenderOption renderOption;
		int openglVersionA, openglVersionB, openglVersionC;

		bool drawGrid;
		unsigned int vboGridSize;
		GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;

		unsigned int instanceDrawn, trianglesDrawn;
		double dummy;
		Shader* lastShader;
		GLuint lastVAO;

		std::map<Shader*, std::vector<Entity*> > simpleBatches;
		std::map<Shader*, std::map<Mesh*, std::vector<glm::mat4> > > groupBatches;
		//
};

