#pragma once

#include <GL/glew.h>


#include <glm/glm.hpp>

#include <Utiles/Mutex.h>
#include <Core/RenderContext.h>
#include <Utiles/Singleton.h>
#include <World/World.h>

#include <Scene/FrustrumSceneQuerry.h>

#define MAX_LIGHT_COUNT 255
#define CLUSTER_MAX_LIGHT 16
#define CLUSTER_SIZE_X 64
#define CLUSTER_SIZE_Y 36
#define CLUSTER_SIZE_Z 128

class Shader;
class Mesh;
class LightComponent;
class OccluderComponent;
class CameraComponent;
//class Debug::Vertex;

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

			mat4f m_shadowCascadeProjections[4];
			vec4f m_shadowFarPlanes;
			float m_shadowBlendMargin;
		};
		enum ShadingConfig
		{
			eUseLightClustering = 0,
			eLightCountHeatmap = 1,
			eUseShadow = 2,
			eDrawShadowCascades = 3,
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
		struct DebugShaderUniform
		{
			vec4f vertexNormalColor;
			vec4f faceNormalColor;
			float wireframeEdgeFactor;
			float occlusionResultDrawAlpha;
			float occlusionResultCuttoff;
		};
		//

		//  Public functions
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const vec4f& color = vec4f(0.4f, 0.2f, 0.1f, 1.f));
		void initializeLightClusterBuffer(int width, int height, int depth);
		void initializeOcclusionBuffers(int width, int height);
		void initializeShadowCascades(int width, int height);
		void render(CameraComponent* renderCam);
		void renderHUD();
		void swap();
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

		double getElapsedTime() const;
		double getAvgElapsedTime() const;

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

		void fullScreenDraw(const Texture* texture, Shader* shader = nullptr, float alpha = 1.f, bool bindIntoImage = false);
		//

		//	Render function
		void drawObject(Entity* object, Shader* forceShader = nullptr);
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
		struct ShadowDrawElement
		{
			float distance;
			Entity* entity;
			Batch* batch;
		};
		//

		//	Protected functions
		void loadModelMatrix(Shader* shader, const ModelMatrix* model, const int& modelSize = 1);
		void loadGlobalUniforms(Shader* shader);
		void loadVAO(const GLuint& vao);

		void drawInstancedObject(Shader* s, Mesh* m, std::vector<ModelMatrix>& models);

		void initGlobalUniformBuffers();
		void updateShadowCascadeMatrices(CameraComponent* renderCam, float viewportRatio);
		void updateGlobalUniformBuffers();
		//

		// Render stages
		void CollectEntitiesBindLights();
		void LightClustering();
		void DynamicBatching();
		void OcclusionCulling();
		void ShadowCasting();
		//

		//  Attributes
		CameraComponent* camera;
		World* world;
		RenderContext* context;
		std::map<ShaderIdentifier, Shader*> defaultShader;
		RenderOption renderOption;

		// grid and usefull opengl
		bool drawGrid;
		unsigned int vboGridSize;
		GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;
		vec4f m_gridColor;
		unsigned int instanceDrawn, drawCalls, trianglesDrawn;
		GLuint lastVAO, fullscreenVAO;
		Shader* lastShader;
		bool shaderJustActivated;
		Shader* fullscreenTriangle;
		Shader* occlusionResultDraw;

		// render queue - queries - collector
		std::vector<DrawElement> renderQueue;
		FrustrumSceneQuerry sceneQuery;
		VirtualEntityCollector collector;

		// batching - instancing
		bool m_enableInstancing = true;
		bool m_hasInstancingShaders = false;
		bool m_hasShadowCaster = false;
		std::map<std::pair<Shader*, Mesh*>, Batch*> batchOpened;
		std::vector<Batch*> batchFreePool;
		std::vector<Batch*> batchClosedPool;

		// occlusion
		bool m_enableOcclusionCulling = true;
		std::vector<std::pair<float, OccluderComponent*>> m_occluders;
		vec2i m_occlusionBufferSize;
		float* m_occlusionCenterX = nullptr;
		float* m_occlusionCenterY = nullptr;
		float* m_occlusionDepth = nullptr;
		std::vector<vec4f> occluderScreenVertices;
		uint32_t* m_occlusionDepthColor = nullptr;
		Texture occlusionTexture;
		unsigned int occluderTriangles, occluderRasterizedTriangles, occluderPixelsTest, occlusionCulledInstances;

		// global params
		GLuint m_globalMatricesID, m_environementLightingID, m_lightsID, m_clustersID, m_DebugShaderUniformID, m_ShadowFBO;
		GlobalMatrices m_globalMatrices;
		EnvironementLighting m_environementLighting;
		SceneLights m_sceneLights;
		DebugShaderUniform m_debugShaderUniform;

		// light clustering param and shadows
		bool m_shadowStableFit = true;
		Texture lightClusterTexture;
		Texture shadowCascadeTexture;
		std::vector<ShadowDrawElement> shadowQueue;
		OrientedBox shadowAreaBoxes[4];
		float shadowAreaMargin;
		float shadowAreaMarginLightDirection;

		// timing
		unsigned int m_timerQueryID;
		float m_GPUelapsedTime, m_GPUavgTime;
		float m_OcclusionElapsedTime, m_OcclusionAvgTime;


#ifdef USE_IMGUI
		bool m_drawLightDirection = false;
		bool m_lightFrustrumCulling = true;
		bool m_drawClusters = false;
		bool m_drawOcclusionBuffer = false;
#endif //USE_IMGUI
		//
};

