#pragma once

#include <GL/glew.h>


#include <glm/glm.hpp>

#include <Utiles/Mutex.h>
#include <Core/RenderContext.h>
#include <Utiles/Singleton.h>
#include <World/World.h>

#include "CameraComponent.h"


class Shader;
class Mesh;

class Renderer : public Singleton<Renderer>
{
	friend class Singleton<Renderer>;
	friend class Debug;

	public:
		//	Miscellaneous
		enum ShaderIdentifier
		{	
			HUD = 1,
			GRID,

			INSTANCE_DRAWABLE,
			INSTANCE_DRAWABLE_BB,
			INSTANCE_DRAWABLE_WIRED,

			INSTANCE_ANIMATABLE,
			INSTANCE_ANIMATABLE_BB,
			INSTANCE_ANIMATABLE_WIRED
		};
		enum RenderOption
		{
			DEFAULT,
			BOUNDING_BOX,
			WIREFRAME
		};
		//

		//  Public functions
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const glm::vec3& color = glm::vec3(0.4f, 0.2f, 0.1f));
		//void initi
		void render(CameraComponent* renderCam);
		void renderHUD();
		//

		//  Set/get functions
		void setCamera(CameraComponent* cam);
		void setWorld(World* currentWorld);
		void setContext(RenderContext* ctx);
		void setShader(ShaderIdentifier id, Shader* s);
		void setGridVisible(bool enable);
		void setRenderOption(const RenderOption& option);
		
		CameraComponent* getCamera();
		World* getWorld();
		RenderContext* getContext();
		Shader* getShader(ShaderIdentifier id);
		bool isGridVisible();
		unsigned int getNbDrawnInstances() const;
		unsigned int getNbDrawnTriangles() const;
		RenderOption getRenderOption() const;
		//

		//	Render function
		void drawObject(Entity* object, const float* view, const float* projection);
		void drawMap(Map* map, const float* view, const float* projection, Shader* s = nullptr);
		//

		Shader* normalViewer;

	private:
		//  Default
		Renderer();
		~Renderer();
		//

		//	Miscellaneous
		struct EntityCompareDistance
		{
			glm::vec3 p;
			explicit EntityCompareDistance(const glm::vec3& cameraPosition) : p(cameraPosition) {};
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
		CameraComponent* camera;
		World* world;
		RenderContext* context;
		std::map<ShaderIdentifier, Shader*> defaultShader;
		RenderOption renderOption;

		bool drawGrid;
		unsigned int vboGridSize;
		GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;
		unsigned int instanceDrawn, trianglesDrawn;
		Shader* lastShader;
		GLuint lastVAO;

		std::map<Shader*, std::vector<Entity*> > simpleBatches;
		std::map<Shader*, std::map<Mesh*, std::vector<glm::mat4> > > groupBatches;
		//
};

