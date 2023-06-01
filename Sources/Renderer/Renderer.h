#pragma once

#include <GL/glew.h>


#include <glm/glm.hpp>

#include <Utiles/Mutex.h>
#include <Core/RenderContext.h>
#include <Utiles/Singleton.h>
#include <World/World.h>

#include "CameraComponent.h"

#define MAX_LIGHT_COUNT 128
#define CLUSTER_MAX_LIGHT 16
#define CLUSTER_SIZE_X 64
#define CLUSTER_SIZE_Y 36
#define CLUSTER_SIZE_Z 128

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
		struct SceneLights
		{
			int m_lightCount;
			unsigned int m_shadingConfiguration;;
			float m_clusterDepthScale;
			float m_clusterDepthBias;
			float m_near;
			float m_far;
			float m_tanFovX;
			float m_tanFovY;
			Light m_lights[MAX_LIGHT_COUNT];
		};
		//

		//  Public functions
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const vec4f& color = vec4f(0.4f, 0.2f, 0.1f, 1.f));
		void initializeLightClusterBuffer(int width, int height, int depth);
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

		CameraComponent* getCamera();
		World* getWorld();
		RenderContext* getContext();
		Shader* getShader(ShaderIdentifier id);
		bool isGridVisible();
		unsigned int getNbDrawnInstances() const;
		unsigned int getNbDrawCalls() const;
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
		Shader* lightClustering;

	private:
		//  Default
		Renderer();
		~Renderer();
		//

		//	Miscellaneous
		struct ModelMatrix
		{
			mat4f model;
			mat4f normalMatrix;
		};
		struct Batch
		{
			Shader* shader;
			Mesh* mesh;
			std::vector<ModelMatrix> models;
		};
		struct DrawElement
		{
			uint64_t hash;
			Entity* entity;
			Batch* batch;
		};
		//

		//	Protected functions
		//void loadModelMatrix(Shader* shader, const mat4f* model, const mat4f* rotMatrix, const int& modelSize = 0);
		void loadModelMatrix(Shader* shader, const ModelMatrix* model, const int& modelSize = 1);
		void loadVAO(const GLuint& vao);

		void drawInstancedObject(Shader* s, Mesh* m, std::vector<ModelMatrix>& models);

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
		unsigned int instanceDrawn, drawCalls, trianglesDrawn;
		Shader* lastShader;
		GLuint lastVAO;

		bool m_enableInstancing = true;
		std::vector<DrawElement> renderQueue;
		std::map<std::pair<Shader*, Mesh*>, Batch*> batchOpened;
		std::vector<Batch*> batchFreePool;
		std::vector<Batch*> batchClosedPool;

		// global params
		GLuint m_globalMatricesID, m_environementLightingID, m_lightsID, m_clustersID;
		GlobalMatrices m_globalMatrices;
		EnvironementLighting m_environementLighting;
		SceneLights m_sceneLights;

		// light clustering param
		Texture lightClusterTexture;

#ifdef USE_IMGUI
		bool m_drawLightDirection = false;
		bool m_lightFrustrumCulling = true;
		bool m_drawClusters = false;

		vec4f* clustersMin = nullptr;
		vec4f* clustersMax = nullptr;
#endif //USE_IMGUI
		//
};

