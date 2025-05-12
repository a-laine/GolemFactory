#pragma once

#include <GL/glew.h>

#include "Math/TMath.h"

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
#define CullingModeMask (1ULL << 61)

class Shader;
class Material;
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
		enum ShaderIdentifier : uint8_t
		{	
			HUD = 1,
			GRID,

			INSTANCE_DRAWABLE_BB,
			INSTANCE_ANIMATABLE_BB,

			DEFAULT
		};
		enum class RenderOption : uint8_t
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
		
		enum ShadingConfig : uint8_t
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
		struct Batch
		{
			Mesh* mesh;
			Material* material;
			Shader* shader;
			DrawableComponent* constantDataReference;
			std::vector<mat4f> matrices;

			unsigned short maxInstanceCount;
			unsigned short dataSize;
			unsigned short instanceCount;

			bool clockwise;
			bool pushMatrices;
			vec4f* instanceDatas;
		};
		struct DrawElement
		{
			uint64_t hash;
			Entity* entity;
			Material* material;
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
		void initializeSkybox(const std::string& textureName);
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
		void setEnvAmbientColor(vec3f color);
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
		vec3f getEnvAmbientColor() const;
		vec4f getEnvDirectionalLightDirection() const;
		vec4f getEnvDirectionalLightColor() const;

		void fullScreenDraw(const Texture* texture, Shader* shader = nullptr, float alpha = 1.f, bool bindIntoImage = false);
		//

		//	Render function
		void bindMaterial(Material* _material, Shader* _shader);
		void loadMatrices(Shader* _shader, float* _instanceMatrices, unsigned short _instanceCount = 1);
		void loadInstanceDatas(Shader* _shader, vec4f* _instanceDatas, unsigned short _dataSize, unsigned short _instanceCount = 1);
		void drawObject(Entity* object, Shader* forceShader = nullptr);
		void loadVAO(const GLuint& vao);
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
		 
		//	Protected functions
		void loadGlobalUniforms(Shader* shader);

		void drawInstancedObject(Material* material, Shader* _shader, Mesh* _mesh, float* _matrices, vec4f* _instanceDatas,
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
		void AtmosphericScattering();
		//void DynamicBatching();
		void OcclusionCulling();
		void ShadowCasting();
		void CreateBatches(std::vector<DrawElement>& _queue, int _shadowMode);
		//

		//  Attributes

		#pragma region General_Attributes

				CameraComponent* camera;
				World* world;
				RenderContext* context;
				std::map<ShaderIdentifier, Shader*> defaultShader;
				TerrainVirtualTexture* m_terrainVirtualTexture;

				std::vector<DrawElement> renderQueue;
				FrustrumSceneQuerry sceneQuery;
				VirtualEntityCollector collector;
				float m_frustrumFar = 10000.f;
				int m_queryMaxDepth = 1000;
				RenderOption renderOption; 
		#pragma endregion

		#pragma region Grid

				bool m_drawGrid;
				unsigned int vboGridSize;
				GLuint gridVAO, vertexbuffer, arraybuffer, colorbuffer, normalbuffer;
				vec4f m_gridColor;
		#pragma endregion

		#pragma region OpenGL_States

				GLuint lastVAO;
				Shader* lastShader;
				Material* lastMaterial;
				Skeleton* lastSkeleton;

				Shader* fullscreenTriangle;

				#ifdef USE_IMGUI
					Shader* occlusionResultDraw;
				#endif

				GLuint fullscreenVAO;
				bool m_cwFrontFace;
				bool shaderJustActivated;
				std::vector<Texture*> m_bindedTextures;
				int m_bindedMaxShadowCascade;
				int m_bindedOmniLayer;

		#pragma endregion

		#pragma region Batching_Instancing

				struct BatchKey
				{
					Mesh* m_mesh;
					Shader* m_shader;
					Material* m_material;
					int8_t m_cw;
					
					BatchKey(Mesh* ms, Shader* s, Material* ma, bool cw) : m_mesh(ms), m_shader(s), m_material(ma), m_cw(cw ? 1 : -1) {};
					inline bool operator<(const BatchKey& other) const 
					{
						if (m_shader != other.m_shader) return (intptr_t)m_shader < (intptr_t)other.m_shader;
						if (m_material != other.m_material) return (intptr_t)m_material < (intptr_t)other.m_material;
						if (m_mesh != other.m_mesh) return (intptr_t)m_mesh < (intptr_t)other.m_mesh;
						return m_cw < other.m_cw;
					}
				};

				std::map<BatchKey, Batch*> batchOpened;
				std::vector<Batch*> batchFreePool;
				std::vector<Batch*> batchClosedPool;

				GLuint m_globalMatricesID;
				GlobalMatrices m_globalMatrices;
				GLint m_maxUniformSize;
				bool m_enableInstancing = true;
				bool m_hasShadowCaster = false;
		#pragma endregion

		#pragma region Occlusion_Culling

				bool m_enableOcclusionCulling = true;
				vec2i m_occlusionBufferSize;
				std::vector<std::pair<float, OccluderComponent*>> m_occluders;
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
					vec3f m_ambientColor;
					float m_fogDensity;
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
				Texture m_lightClusterTexture;
				Shader* m_lightClustering;
				Texture* m_skyboxTexture;
				Mesh* m_skyboxMesh;
				Material* m_skyboxMaterial;
				Shader* m_atmosphericScattering;
				bool m_enableAtmosphericScattering;
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
				std::vector<DrawElement> shadowQueue;
				std::vector<LightComponent*> shadowOmniCaster;
				BoxSceneQuerry omniLightQuery;
				VirtualEntityCollector omniLightCollector;
				OrientedBox shadowAreaBoxes[4];
				float shadowAreaMargin;
				float shadowAreaMarginLightDirection;
				int shadowOmniLayerUniform;
				int shadowCascadeMax;
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

				Texture* m_terrainMaterialCollection;
				std::vector<TerrainMaterial> m_terrainMaterialInfos;
				std::vector<std::string> m_terrainMaterialNames;
				GLuint m_terrainMaterialCollectionID;
		#pragma endregion

#ifdef USE_IMGUI
		float m_directionalLightDebugRaySpacing = 5.f;
		float m_directionalLightDebugRayYoffset = 10.f;

		bool m_drawLightDirection = false;
		bool m_lightFrustrumCulling = true;
		bool m_drawClusters = false;
		bool m_drawOcclusionBuffer = false;
#endif //USE_IMGUI
		//
};

