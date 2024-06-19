#pragma once

#include <GL/glew.h>


#include "Math/TMath.h"
//#include <glm/glm.hpp>

#include <Utiles/Mutex.h>
#include <Core/RenderContext.h>
#include <Utiles/Singleton.h>
#include <World/World.h>
#include <Utiles/ProfilerConfig.h>
#include <Resources/Texture.h>
#include <Scene/FrustrumSceneQuerry.h>

#define MAX_LIGHT_COUNT 255
#define CLUSTER_MAX_LIGHT 16
#define CLUSTER_SIZE_X 64
#define CLUSTER_SIZE_Y 36
#define CLUSTER_SIZE_Z 128
#define MAX_OMNILIGHT_SHADOW_COUNT 8
#define MAX_TERRAIN_MATERIAL 128

#define TransparentMask (1ULL << 63)
#define FaceCullingMask (1ULL << 62)

class Shader;
class Mesh;
class LightComponent;
class OccluderComponent;
class CameraComponent;
class DrawableComponent;

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

			DEFAULT
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
		
		enum ShadingConfig
		{
			eUseLightClustering = 0,
			eLightCountHeatmap = 1,
			eUseShadow = 2,
			eDrawShadowCascades = 3,
			eOmniShadowPass = 4
		};

		struct ModelMatrix
		{
			mat4f modelMatrix;
			mat4f normalMatrix;
		};
		struct Batch;
		struct DrawElement
		{
			uint64_t hash;
			Entity* entity;
			Shader* shader;
			Mesh* mesh;
			Batch* batch;
		};
		//

		//  Public functions
		void initializeConstants();
		void initializeGrid(const unsigned int& gridSize, const float& elementSize = 1.f, const vec4f& color = vec4f(0.4f, 0.2f, 0.1f, 1.f));
		void initializeLightClusterBuffer(int width, int height, int depth);
		void initializeOcclusionBuffers(int width, int height);
		void initializeShadows(int cascadesWidth, int cascadesHeight, int omniWidth, int omniHeight);
		void initializeTerrainMaterialCollection(const std::string& textureName);
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
		void setVirtualTexture(TerrainVirtualTexture* virtualTexture);

		void setEnvBackgroundColor(vec4f color);
		void setEnvAmbientColor(vec4f color);
		void setEnvDirectionalLightDirection(vec4f direction);
		void setEnvDirectionalLightColor(vec4f color);
		void incrementShaderAnimatedTime(float time);

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
		void loadInstanceMatrices(Shader* _shader, float* _instanceMatrices, unsigned short _instanceCount = 1);
		void loadInstanceDatas(Shader* _shader, vec4f* _instanceDatas, unsigned short _dataSize, unsigned short _instanceCount = 1);
		void drawObject(Entity* object, Shader* forceShader = nullptr);
		// 
		
		//	Debug
		void drawImGui(World& world);

		void initializeOverviewRenderer(int width, int height);
		GLuint renderMeshOverview(Mesh* mesh, float angle0, float angle1, float zoom);
		vec2i getOverviewTextureSize() const;
		//

		Shader* normalViewer;

	private:
		//  Default
		Renderer();
		~Renderer();
		//

		//	Miscellaneous
		struct Batch
		{
			Shader* shader;
			Mesh* mesh;
			unsigned short instanceCount;

			bool pushMatrices;
			DrawableComponent* constantDataReference;
			std::vector<mat4f> matrices;

			unsigned short maxInstanceCount;
			unsigned short dataSize;
			vec4f* instanceDatas;
		};
		/*struct ShadowDrawElement
		{
			float distance;
			Entity* entity;
			Batch* batch;
		};*/
		//

		//	Protected functions
		void loadGlobalUniforms(Shader* shader);
		void loadVAO(const GLuint& vao);

		void drawInstancedObject(Shader* _shader, Mesh* _mesh, float* _matrices, vec4f* _instanceDatas,
			unsigned short _dataSize, unsigned short _instanceCount, DrawableComponent* _constantDataRef);

		void initGlobalUniformBuffers();
		void updateShadowCascadeMatrices(CameraComponent* renderCam, float viewportRatio);
		void updateGlobalUniformBuffers();

		void computeOmniShadowProjection(LightComponent* light, int omniIndex);
		//

		// Render stages
		void CollectEntitiesBindLights();
		void CollectTerrainQueueData();
		void LightClustering();
		void DynamicBatching();
		void OcclusionCulling();
		void ShadowCasting();
		//

		//  Attributes

		#pragma region General_Attributes

				CameraComponent* camera;
				World* world;
				RenderContext* context;
				std::map<ShaderIdentifier, Shader*> defaultShader;
				RenderOption renderOption; 
				TerrainVirtualTexture* m_terrainVirtualTexture;

				std::vector<DrawElement> renderQueue;
				FrustrumSceneQuerry sceneQuery;
				VirtualEntityCollector collector;
		#pragma endregion

		#pragma region Grid

				bool drawGrid;
				unsigned int vboGridSize;
				GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;
				vec4f m_gridColor;
		#pragma endregion

		#pragma region OpenGL_States

				GLuint lastVAO, fullscreenVAO;
				Shader* lastShader;
				Skeleton* lastSkeleton;
				bool shaderJustActivated;
				Shader* fullscreenTriangle;

				#ifdef USE_IMGUI
					Shader* occlusionResultDraw;
				#endif
		#pragma endregion

		#pragma region Batching_Instancing

				bool m_enableInstancing = true;
				bool m_hasInstancingShaders = false;
				bool m_hasShadowCaster = false;
				std::map<std::pair<Shader*, Mesh*>, Batch*> batchOpened;
				std::vector<Batch*> batchFreePool;
				std::vector<Batch*> batchClosedPool;

				GLuint m_globalMatricesID;
				GlobalMatrices m_globalMatrices;
				GLint m_maxUniformSize;
		#pragma endregion

		#pragma region Occlusion_Culling

				bool m_enableOcclusionCulling = true;
				std::vector<std::pair<float, OccluderComponent*>> m_occluders;
				vec2i m_occlusionBufferSize;
				float* m_occlusionCenterX = nullptr;
				float* m_occlusionCenterY = nullptr;
				float* m_occlusionDepth = nullptr;
				std::vector<vec4f> occluderScreenVertices;

				#ifdef USE_IMGUI
					Texture occlusionTexture;
				#endif

				unsigned int occluderTriangles, occluderRasterizedTriangles, occluderPixelsTest, occlusionCulledInstances;
				float m_OcclusionElapsedTime, m_OcclusionAvgTime;
		#pragma endregion

		#pragma region Lights_And_LightClustering

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
					unsigned int m_shadingConfiguration;
					float m_clusterDepthScale;
					float m_clusterDepthBias;
					float m_near;
					float m_far;
					float m_tanFovX;
					float m_tanFovY;

					Light m_lights[MAX_LIGHT_COUNT];
				};

				GLuint m_environementLightingID, m_lightsID, m_clustersID;
				SceneLights m_sceneLights;
				EnvironementLighting m_environementLighting;
				Texture lightClusterTexture;
				Shader* lightClustering;
		#pragma endregion

		#pragma region Shadows
				struct OmniShadows
				{
					int m_omniShadowLightIndexes[MAX_OMNILIGHT_SHADOW_COUNT];
					float m_omniShadowLightNear[MAX_OMNILIGHT_SHADOW_COUNT];
					mat4f m_shadowOmniProjections[6 * MAX_OMNILIGHT_SHADOW_COUNT];

				};

				bool m_shadowStableFit = true;
				GLuint m_omniShadowsID, m_ShadowCascadeFBO, m_ShadowOmniFBO;
				OmniShadows m_OmniShadows;
				Texture shadowCascadeTexture;
				Texture shadowOmniTextures;
				//std::vector<ShadowDrawElement> shadowCascadeQueue;
				std::vector<DrawElement> shadowCascadeQueue;
				std::vector<LightComponent*> shadowOmniCaster;
				BoxSceneQuerry omniLightQuery;
				VirtualEntityCollector omniLightCollector;
				OrientedBox shadowAreaBoxes[4];
				float shadowAreaMargin;
				float shadowAreaMarginLightDirection;
				int shadowOmniLayerUniform;
		#pragma endregion

		#pragma region Metrics_And_Debug
				struct DebugShaderUniform
				{
					vec4f vertexNormalColor;
					vec4f faceNormalColor;
					float wireframeEdgeFactor;
					float occlusionResultDrawAlpha;
					float occlusionResultCuttoff;
					float animatedTime;
				};

				unsigned int m_timerQueryID;
				unsigned int instanceDrawn, drawCalls, shadowDrawCalls, trianglesDrawn;
				float m_GPUelapsedTime, m_GPUavgTime;
				DebugShaderUniform m_debugShaderUniform;
				GLuint m_DebugShaderUniformID, overviewFBO;
				Texture overviewTexture;
				Texture overviewDepth;
		#pragma endregion

		#pragma region Terrain_Global_Attributes
				struct TerrainMaterial
				{
					int m_albedo;
					int m_normal;
					float m_tiling;
					float m_metalic;
				};

				GLuint m_terrainMaterialCollectionID;
				Texture* m_terrainMaterialCollection;
				std::vector<TerrainMaterial> m_terrainMaterialInfos;
				std::vector<std::string> m_terrainMaterialNames;
		#pragma endregion

#ifdef USE_IMGUI
		bool m_drawLightDirection = false;
		bool m_lightFrustrumCulling = true;
		bool m_drawClusters = false;
		bool m_drawOcclusionBuffer = false;
#endif //USE_IMGUI
		//
};

