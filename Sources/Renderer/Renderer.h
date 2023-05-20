#pragma once

#include <GL/glew.h>


#include <glm/glm.hpp>

#include <Utiles/Mutex.h>
#include <Core/RenderContext.h>
#include <Utiles/Singleton.h>
#include <World/World.h>

#include "CameraComponent.h"

#define MAX_LIGHT_COUNT 128

class Shader;
class Mesh;
class LightComponent;

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

			INSTANCE_DRAWABLE_BB,
			INSTANCE_ANIMATABLE_BB,
		};
		enum class RenderOption
		{
			DEFAULT,
			BOUNDING_BOX,
			WIREFRAME,
			NORMALS
		};
		struct GlobalMatrices
		{
			mat4f view;
			mat4f projection;
			vec4f cameraPosition;
		};
		struct EnvironementLighting
		{
			vec4f m_backgroundColor;
			vec4f m_ambientColor;
			vec4f m_directionalLightDirection;
			vec4f m_directionalLightColor;
		};
		struct Light
		{
			vec4f m_position;
			vec4f m_direction;
			vec4f m_color;
			float m_range;
			float m_intensity;
			float m_inCutOff;
			float m_outCutOff;
		};
		//

		//  Public functions
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const vec4f& color = vec4f(0.4f, 0.2f, 0.1f, 1.f));
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

		void setEnvBackgroundColor(vec4f color);
		void setEnvAmbientColor(vec4f color);
		void setEnvDirectionalLightDirection(vec4f direction);
		void setEnvDirectionalLightColor(vec4f color);

		void addLight(LightComponent* _light);

		CameraComponent* getCamera();
		World* getWorld();
		RenderContext* getContext();
		Shader* getShader(ShaderIdentifier id);
		bool isGridVisible();
		unsigned int getNbDrawnInstances() const;
		unsigned int getNbDrawnTriangles() const;
		RenderOption getRenderOption() const;

		vec4f getEnvBackgroundColor() const;
		vec4f getEnvAmbientColor() const;
		vec4f getEnvDirectionalLightDirection() const;
		vec4f getEnvDirectionalLightColor() const;
		//

		//	Render function
		void drawObject(Entity* object);
		void drawMap(Map* map, Shader* s = nullptr);
		// 
		
		//	Debug
		void drawImGui(World& world);
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
			vec4f p;
			explicit EntityCompareDistance(const vec4f& cameraPosition) : p(cameraPosition) {};
			bool operator()(Entity* a, Entity* b) const
			{
				const float d1 = vec4f::dot(a->getWorldPosition() - p, a->getWorldPosition() - p);
				const float d2 = vec4f::dot(b->getWorldPosition() - p, b->getWorldPosition() - p);
				return d1 < d2;
			}
		};
		//

		//	Protected functions
		void loadModelMatrix(Shader* shader, const mat4f* model, const mat4f* rotMatrix, const int& modelSize = 1);
		void loadVAO(const GLuint& vao);

		void drawInstancedObject(Shader* s, Mesh* m, std::vector<mat4f>& models, std::vector<mat4f>& normalMatrices);

		void initGlobalUniformBuffers();
		void updateGlobalUniformBuffers();
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
		vec4f m_gridColor;
		unsigned int instanceDrawn, trianglesDrawn;
		Shader* lastShader;
		GLuint lastVAO;

		std::vector<std::pair<uint64_t, Entity*>> renderQueue;

		EnvironementLighting m_envLighting;

		GLuint m_globalMatricesID, m_environementLightingID, m_lightsID;
		GlobalMatrices m_globalMatrices;
		EnvironementLighting m_environementLighting;
		Light m_lights[MAX_LIGHT_COUNT];
		int m_lightCount;
		std::vector<LightComponent*> m_lightComponents;

#ifdef USE_IMGUI
		bool m_drawLightDirection = false;
#endif //USE_IMGUI
		//
};

