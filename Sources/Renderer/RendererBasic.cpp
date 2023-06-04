#include "Renderer.h"
#include "CameraComponent.h"
#include "imgui_internal.h"

#include <iostream>
#include <sstream>

#include <EntityComponent/Entity.hpp>
#include <HUD/WidgetManager.h>
#include <Scene/FrustrumSceneQuerry.h>
#include <Scene/FrustrumEntityCollector.h>
#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Renderer/DrawableComponent.h>
#include <Animation/SkeletonComponent.h>
#include <Renderer/Lighting/LightComponent.h>
#include <Utiles/Debug.h>




#define MAX_INSTANCE 32

#ifdef USE_IMGUI
bool RenderingWindowEnable = true;
#endif


//  Default
Renderer::Renderer() : 
	normalViewer(nullptr), renderOption(RenderOption::DEFAULT),
	vboGridSize(0), gridVAO(0), vertexbuffer(0), arraybuffer(0), colorbuffer(0), normalbuffer(0),
	instanceDrawn(0), trianglesDrawn(0), lastShader(nullptr)
{
	context = nullptr;
	camera = nullptr;
	world = nullptr;
	m_OcclusionElapsedTime = m_OcclusionAvgTime = 0;

	defaultShader[GRID] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE_BB] = nullptr;
	defaultShader[INSTANCE_DRAWABLE_BB] = nullptr;
	defaultShader[HUD] = nullptr;

	lightClustering = nullptr;

	drawGrid = true;
	batchFreePool.reserve(512);

	glGenQueries(1, &m_timerQueryID);

	initGlobalUniformBuffers();
	updateGlobalUniformBuffers();
}
Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &gridVAO);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &arraybuffer);

	glDeleteBuffers(1, &m_environementLightingID);
	glDeleteBuffers(1, &m_lightsID);
	glDeleteBuffers(1, &m_globalMatricesID);

	if (m_occlusionDepth)
	{
		delete[] m_occlusionDepth;
		delete[] m_occlusionCenterX;
		delete[] m_occlusionCenterY;
		delete[] m_occlusionDepthColor;
	}

}
//

//  Public functions
void Renderer::initializeGrid(const unsigned int& gridSize,const float& elementSize, const vec4f& color)
{
	if (glIsVertexArray(gridVAO)) return;

	defaultShader[GRID] = ResourceManager::getInstance()->getResource<Shader>("wired");
	m_gridColor = color;

	//	generate grid vertex buffer
	float* vertexBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	float* normalBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	for (unsigned int i = 0; i < gridSize + 1; i++)
		for (unsigned int j = 0; j < gridSize + 1; j++)
		{
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = elementSize*i - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = 0;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = elementSize*j - (gridSize * elementSize) / 2;

			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = 0.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = 1.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 0.f;
		}

	uint32_t* indexBufferGrid = new uint32_t[6 * gridSize*gridSize];
	for (unsigned int i = 0; i < gridSize; i++)
		for (unsigned int j = 0; j < gridSize; j++)
		{
			indexBufferGrid[6 * (i*gridSize + j) + 0] = i*(gridSize + 1) + j + (gridSize + 1);
			indexBufferGrid[6 * (i*gridSize + j) + 1] = i*(gridSize + 1) + j;
			indexBufferGrid[6 * (i*gridSize + j) + 2] = i*(gridSize + 1) + j + 1;

			indexBufferGrid[6 * (i*gridSize + j) + 3] = i*(gridSize + 1) + j + (gridSize + 1);
			indexBufferGrid[6 * (i*gridSize + j) + 4] = i*(gridSize + 1) + j + (gridSize + 1) + 1;
			indexBufferGrid[6 * (i*gridSize + j) + 5] = i*(gridSize + 1) + j + 1;
		}
	
	vboGridSize = 6 * gridSize * gridSize;

	//	initialize VBO
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * (gridSize + 1)*(gridSize + 1) * sizeof(float), vertexBufferGrid, GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * (gridSize + 1)*(gridSize + 1) * sizeof(float), normalBufferGrid, GL_STATIC_DRAW);

	glGenBuffers(1, &arraybuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * gridSize*gridSize * sizeof(unsigned int), indexBufferGrid, GL_STATIC_DRAW);

	//	initialize VAO
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arraybuffer);
	glBindVertexArray(0);

	//	free grid resources
	delete[] vertexBufferGrid;
	delete[] normalBufferGrid;
	delete[] indexBufferGrid;
}
void Renderer::initializeLightClusterBuffer(int width, int height, int depth)
{
	vec3i imageSize = vec3i(width, height, depth);

	const int length = imageSize.x * imageSize.y * imageSize.z * 4;
	uint32_t* buffer = new uint32_t[length];
	for (int i = 0; i < imageSize.x; i++)
		for (int j = 0; j < imageSize.y; j++)
			for (int k = 0; k < imageSize.z; k++)
			{
				int id = i * imageSize.y * imageSize.z * 4 + j * imageSize.z * 4 + k * 4;

				buffer[id] = 0xFFFFFFFF;
				buffer[id + 1] = 0xFFFFFFFF;
				buffer[id + 2] = 0xFFFFFFFF;
				buffer[id + 3] = 0xFFFFFFFF;
			}

	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_3D | (uint8_t)TC::MIN_NEAREST | (uint8_t)TC::MAG_NEAREST | (uint8_t)TC::WRAP_CLAMP;
	lightClusterTexture.initialize("lightClusterTexture", imageSize, buffer, config, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
	ResourceManager::getInstance()->addResource(&lightClusterTexture);
	delete[] buffer;

	m_sceneLights.m_near = 2.f;
	m_sceneLights.m_far = 1500.f;
	float logRatio = imageSize.z / log(m_sceneLights.m_far / m_sceneLights.m_near);
	m_sceneLights.m_clusterDepthScale = logRatio;
	m_sceneLights.m_clusterDepthBias = logRatio * log(m_sceneLights.m_near);
	m_sceneLights.m_shadingConfiguration = 0x01;
}
void Renderer::initializeOcclusionBuffers(int width, int height)
{
	m_occlusionBufferSize = vec2i(width, height);
	int size = m_occlusionBufferSize.x * m_occlusionBufferSize.y;
	m_occlusionDepth = new float[size];
	m_occlusionCenterX = new float[size];
	m_occlusionCenterY = new float[size];
	m_occlusionDepthColor = new uint32_t[size];

	constexpr float depthMax = std::numeric_limits<float>::min();
	for (int i = 0; i < m_occlusionBufferSize.x; i++)
		for (int j = 0; j < m_occlusionBufferSize.y; j++)
		{
			int id = j * m_occlusionBufferSize.x + i;
			m_occlusionDepth[id] = depthMax;
			m_occlusionCenterX[id] = (float)(i + 0.5f) / m_occlusionBufferSize.x;
			m_occlusionCenterY[id] = (float)(j + 0.5f) / m_occlusionBufferSize.y;
			m_occlusionDepthColor[id] = 0;
		}

	using TC = Texture::TextureConfiguration;
	uint8_t config = (uint8_t)TC::TEXTURE_2D | (uint8_t)TC::MIN_NEAREST | (uint8_t)TC::MAG_NEAREST | (uint8_t)TC::WRAP_CLAMP;
	occlusionTexture.initialize("occlusionTexture", vec3i(m_occlusionBufferSize.x, m_occlusionBufferSize.y, 0), 
		m_occlusionDepthColor, config, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	ResourceManager::getInstance()->addResource(&occlusionTexture);

	occlusionResultDraw = ResourceManager::getInstance()->getResource<Shader>("occlusionResult");
}
void Renderer::initGlobalUniformBuffers()
{
	// full screen quad 
	glCreateVertexArrays(1, &fullscreenVAO);
	glBindVertexArray(fullscreenVAO);
	glBindVertexArray(0);

	fullscreenTriangle = ResourceManager::getInstance()->getResource<Shader>("fullscreenTriangle");

	// global matrices and camera position
	m_globalMatrices.view = mat4f::identity;
	m_globalMatrices.projection = mat4f::identity;
	m_globalMatrices.cameraPosition = vec4f(0, 0, 0, 1);

	glGenBuffers(1, &m_globalMatricesID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_globalMatrices), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_globalMatricesID, 0, sizeof(m_globalMatrices));

	// environment lighting settings
	m_environementLighting.m_directionalLightDirection = vec4f(-10, -20, 10, 0);
	m_environementLighting.m_directionalLightColor = vec4f(0.15, 0.15, 0.15, 1.0);
	m_environementLighting.m_ambientColor = vec4f(0.05, 0.05, 0.05, 1.0);

	glGenBuffers(1, &m_environementLightingID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_environementLighting), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_environementLightingID, 0, sizeof(m_environementLighting));

	// lights data
	glGenBuffers(1, &m_lightsID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_sceneLights), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, m_lightsID, 0, sizeof(m_sceneLights));

	// lights data
	m_debugShaderUniform.vertexNormalColor = vec4f(0.0, 0.0, 1.0, 0.1);
	m_debugShaderUniform.faceNormalColor = vec4f(0.0, 1.0, 1.0, 0.1);
	m_debugShaderUniform.wireframeEdgeFactor = 0.4f;
	m_debugShaderUniform.occlusionResultCuttoff = 50;
	m_debugShaderUniform.occlusionResultDrawAlpha = 0.8f;

	glGenBuffers(1, &m_DebugShaderUniformID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_DebugShaderUniformID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_debugShaderUniform), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 3, m_DebugShaderUniformID, 0, sizeof(m_debugShaderUniform));

	// end
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void Renderer::updateGlobalUniformBuffers()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_globalMatrices), &m_globalMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_environementLighting), &m_environementLighting);

	glBindBuffer(GL_UNIFORM_BUFFER, m_DebugShaderUniformID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_debugShaderUniform), &m_debugShaderUniform);
}


void Renderer::render(CameraComponent* renderCam)
{
	//	clear previous states
	glBeginQuery(GL_TIME_ELAPSED, m_timerQueryID);
	trianglesDrawn = 0;
	instanceDrawn = 0;
	drawCalls = 0;
	occlusionCulledInstances = 0;
	lastShader = nullptr;
	lastVAO = 0;
	if (!context || !camera || !world || !renderCam) return;

	//	bind matrix
	m_globalMatrices.view = renderCam->getViewMatrix();
	m_globalMatrices.projection = mat4f::perspective(renderCam->getVerticalFieldOfView(), context->getViewportRatio(), 0.1f, 1500.f);
	m_globalMatrices.cameraPosition = renderCam->getPosition();
	updateGlobalUniformBuffers();
	
	//	opengl state
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	//	draw grid
	Shader* shader = defaultShader[GRID];
	if (drawGrid && shader && glIsVertexArray(gridVAO))
	{
		shader->enable();
		ModelMatrix modelMatrix = {mat4f::identity, mat4f::identity};
		loadModelMatrix(shader, &modelMatrix);
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, &m_gridColor[0]);

		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_INT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}

	m_sceneLights.m_tanFovY = tan(0.5f * camera->getVerticalFieldOfView());
	m_sceneLights.m_tanFovX = m_sceneLights.m_tanFovY * context->getViewportRatio();
	CollectEntitiesBindLights();


	if (lightClustering)
		LightClustering();
	if (m_enableOcclusionCulling && !m_occluders.empty() && m_occlusionDepth)
		OcclusionCulling();
	else
	{
		m_OcclusionAvgTime = 0.f;
		m_OcclusionElapsedTime = 0.f;
	}

	//	sort
	uint64_t transparentMask = 1ULL << 63;
	uint64_t faceCullingMask = 1ULL << 62;
	uint64_t compareMask = ~(transparentMask | faceCullingMask); // don't use the states bits for comparing entities
	std::sort(renderQueue.begin(), renderQueue.end(), [compareMask](DrawElement& a, DrawElement& b)
		{
			return (compareMask & a.hash) < (compareMask & b.hash);
		});

	// batching
	if (m_enableInstancing && m_hasInstancingShaders)
		DynamicBatching();

	// state tracking
	bool blendingEnabled = false;
	bool faceCullingEnabled = true;
	const auto SetBlending = [&blendingEnabled](bool state)
	{
		if (!blendingEnabled && state)
			glEnable(GL_BLEND);
		else if (blendingEnabled && !state)
			glDisable(GL_BLEND);
		blendingEnabled = state;
	};
	const auto SetCulling = [&faceCullingEnabled](bool state)
	{
		if (!faceCullingEnabled && state)
			glEnable(GL_CULL_FACE);
		else if (faceCullingEnabled && !state)
			glDisable(GL_CULL_FACE);
		faceCullingEnabled = state;
	};

	//	draw instance list
	for (const auto& it : renderQueue)
	{
		// skip batched entities
		if (!it.entity)
			continue;

		//	opengl states managing
		SetBlending(it.hash & transparentMask);
		SetCulling(it.hash & faceCullingMask);

		if (it.batch)
			drawInstancedObject(it.batch->shader, it.batch->mesh, it.batch->models);
		else
			drawObject(it.entity);
	}

	glEndQuery(GL_TIME_ELAPSED);
}
void Renderer::renderHUD()
{
	if (!context) return;

	// bind matrix
	m_globalMatrices.view = mat4f::eulerAngleZX((float)PI, (float)PI * 0.5f);
	m_globalMatrices.view[3] = vec4f(0.f, 0.f, -DISTANCE_HUD_CAMERA, 1.f);
	m_globalMatrices.projection = mat4f::perspective((float)DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION, context->getViewportRatio(), 0.1f, 1500.f);
	updateGlobalUniformBuffers();

	//	change opengl states
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	uint8_t stencilMask = 0x00;

	//	draw all widget in hudList
	std::map<std::string, std::vector<Layer*>>& wList = WidgetManager::getInstance()->hudList;
	for (std::map<std::string, std::vector<Layer*> >::iterator it = wList.begin(); it != wList.end(); ++it)
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i]->isVisible())		// layer visible
			{
				mat4f model = it->second[i]->getModelMatrix();
				std::vector<WidgetVirtual*>& list = it->second[i]->getChildrenList();
				for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
				{
					if ((*it2)->isVisible())	// widget visible
					{
						//	Get shader and prepare matrix
						Shader* shader = nullptr;
						if (!defaultShader[HUD]) 
							shader = (*it2)->getShader();

						mat4f m = mat4f::translate(model, (*it2)->getPosition());
						ModelMatrix modelMatrix = { m, model };
						loadModelMatrix(shader, &modelMatrix);
						if (!shader)
							continue;

						//	Draw
						(*it2)->draw(shader, stencilMask, model);
						drawCalls;
						instanceDrawn++;
						trianglesDrawn += (*it2)->getNumberFaces();
					}
				}
			}
		}
	}
	glDisable(GL_BLEND);
}
void Renderer::swap()
{
	int stopTimerAvailable = 0;
	while (!stopTimerAvailable) 
		glGetQueryObjectiv(m_timerQueryID, GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);

	GLuint64 elapsedGPUtimer;
	glGetQueryObjectui64v(m_timerQueryID, GL_QUERY_RESULT, &elapsedGPUtimer);
	m_GPUelapsedTime = (float)(elapsedGPUtimer) * 1E-06;
	m_GPUavgTime = 0.95f * m_GPUavgTime + 0.05f * m_GPUelapsedTime;

	Debug::getInstance()->clearVBOs();
}
//


//	Protected functions
void Renderer::loadModelMatrix(Shader* shader, const ModelMatrix* model, const int& modelSize)
{
	if (shader)
	{
		if (shader != lastShader)
		{
			shader->enable();

			GLint lightClusterLocation = glGetUniformLocation(shader->getProgram(), "lightClusters");
			if (lightClusterLocation >= 0)
			{
				glUniform1i(lightClusterLocation, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindImageTexture(0, lightClusterTexture.getTextureId(), 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32UI);
			}
		}
		lastShader = shader;

		int loc = shader->getUniformLocation("matrixArray");
		if (loc >= 0)
			glUniformMatrix4fv(loc, 2 * modelSize, GL_FALSE, (const float*)model);
	}
	else  lastShader = nullptr;
}
void Renderer::loadVAO(const GLuint& vao)
{
	if (vao != lastVAO)
	{
		glBindVertexArray(vao);
		lastVAO = vao;
	}
}
//

//  Set/get functions
void Renderer::setCamera(CameraComponent* cam) { camera = cam; }
void Renderer::setWorld(World* currentWorld) { world = currentWorld; }
void Renderer::setContext(RenderContext* ctx) { context = ctx; }
void Renderer::setShader(ShaderIdentifier id, Shader* s)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if(it != defaultShader.end()) ResourceManager::getInstance()->release(defaultShader[id]);

	if (s) defaultShader[id] = ResourceManager::getInstance()->getResource<Shader>(s);
	else defaultShader[id] = nullptr;
}
void Renderer::setGridVisible(bool enable) { drawGrid = enable; }
void Renderer::setRenderOption(const RenderOption& option) { renderOption = option; }


CameraComponent* Renderer::getCamera() { return camera; }
World* Renderer::getWorld() { return world; }
RenderContext* Renderer::getContext() { return context; }
Shader* Renderer::getShader(ShaderIdentifier id)
{
	std::map<ShaderIdentifier, Shader*>::iterator it = defaultShader.find(id);
	if (it != defaultShader.end()) return defaultShader[id];
	else return nullptr;
}
bool Renderer::isGridVisible() { return drawGrid; }
unsigned int Renderer::getNbDrawnInstances() const { return instanceDrawn; }
unsigned int Renderer::getNbDrawCalls() const { return drawCalls; }
unsigned int Renderer::getNbDrawnTriangles() const { return trianglesDrawn; }
Renderer::RenderOption Renderer::getRenderOption() const { return renderOption; }


double Renderer::getElapsedTime() const { return m_GPUelapsedTime; }
double Renderer::getAvgElapsedTime() const { return m_GPUavgTime; }

void Renderer::setEnvBackgroundColor(vec4f color)
{ 
	m_environementLighting.m_backgroundColor = color;
	glClearColor(m_environementLighting.m_backgroundColor.x, m_environementLighting.m_backgroundColor.y, m_environementLighting.m_backgroundColor.z, m_environementLighting.m_backgroundColor.w);
}
void Renderer::setEnvAmbientColor(vec4f color) { m_environementLighting.m_ambientColor = color; }
void Renderer::setEnvDirectionalLightDirection(vec4f direction) { m_environementLighting.m_directionalLightDirection = direction; }
void Renderer::setEnvDirectionalLightColor(vec4f color) { m_environementLighting.m_directionalLightColor = color; }
vec4f Renderer::getEnvBackgroundColor() const { return m_environementLighting.m_backgroundColor; }
vec4f Renderer::getEnvAmbientColor() const { return m_environementLighting.m_ambientColor; }
vec4f Renderer::getEnvDirectionalLightDirection() const { return m_environementLighting.m_directionalLightDirection; }
vec4f Renderer::getEnvDirectionalLightColor() const { return m_environementLighting.m_directionalLightColor; }

//

//	Debug
void Renderer::drawImGui(World& world)
{
#ifdef USE_IMGUI
	ImVec4 titleColor = ImVec4(1, 1, 0, 1);

	ImGui::Begin("Rendering setings");
	ImGui::PushID(this);
	ImGui::TextColored(titleColor, "Environement lighting");
	ImGui::DragFloat3("Directional light direction", &m_environementLighting.m_directionalLightDirection[0], 0.01f);
	ImGui::ColorEdit3("Directional light color", &m_environementLighting.m_directionalLightColor[0]);
	ImGui::ColorEdit3("Ambient color", &m_environementLighting.m_ambientColor[0]);
	if (ImGui::ColorEdit3("Background color", &m_environementLighting.m_backgroundColor[0]))
		glClearColor(m_environementLighting.m_backgroundColor.x, m_environementLighting.m_backgroundColor.y, m_environementLighting.m_backgroundColor.z, m_environementLighting.m_backgroundColor.w);
	ImGui::Checkbox("Draw light direction", &m_drawLightDirection);

	const char* renderOptions[] = { "Default", "Bounding box", "Wireframe", "Normals" };
	static int renderOptionsCurrentIdx = (int)renderOption;
	const char* renderOptionPreviewValue = renderOptions[renderOptionsCurrentIdx];


	ImGui::Spacing();
	ImGui::TextColored(titleColor, "Rendering parameters");
	ImGui::Checkbox("Instancing enabled", &m_enableInstancing);
	if (ImGui::BeginCombo("Render option", renderOptionPreviewValue))
	{
		for (int n = 0; n < IM_ARRAYSIZE(renderOptions); n++)
		{
			const bool is_selected = (renderOptionsCurrentIdx == n);
			if (ImGui::Selectable(renderOptions[n], is_selected))
			{
				renderOptionsCurrentIdx = n;
				renderOption = (RenderOption)n;
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("Light frustrum culling", &m_lightFrustrumCulling);

	const auto& CheckboxFlag = [](const char* label, unsigned int& bitfield, int flag, unsigned int mask = 0xFFFFFFFF)
	{
		bool b = bitfield & (1 << flag);
		if (ImGui::Checkbox(label, &b))
		{
			bitfield &= mask;
			if (b)
				bitfield |= 1 << flag;
			else	
				bitfield &= ~(1 << flag);
		}
	};
	CheckboxFlag("Do light clustering", m_sceneLights.m_shadingConfiguration, 0);
	CheckboxFlag("Draw light count heatmap", m_sceneLights.m_shadingConfiguration, 1);

	ImGui::Checkbox("Occlusion culling", &m_enableOcclusionCulling);
	ImGui::Checkbox("Draw light clusters", &m_drawClusters);
	ImGui::Checkbox("Draw occlusion buffer", &m_drawOcclusionBuffer);

	if (!m_drawOcclusionBuffer)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	ImGui::SliderFloat("Occlusion buffer alpha", &m_debugShaderUniform.occlusionResultDrawAlpha, 0, 1);
	ImGui::SliderFloat("Occlusion  gradiant cutoff", &m_debugShaderUniform.occlusionResultCuttoff, 1.f, m_sceneLights.m_far, "%.2f", ImGuiSliderFlags_Logarithmic);

	if (!m_drawOcclusionBuffer)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::Spacing();
	ImGui::TextColored(titleColor, "Debug shader uniforms");
	ImGui::ColorEdit3("Vertex normal color", &m_debugShaderUniform.vertexNormalColor[0]);
	ImGui::ColorEdit3("Face normal color", &m_debugShaderUniform.faceNormalColor[0]);
	ImGui::SliderFloat("Vertex normal length", &m_debugShaderUniform.vertexNormalColor[3], 0.f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Face normal length", &m_debugShaderUniform.faceNormalColor[3], 0.f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Wireframe edge factor", &m_debugShaderUniform.wireframeEdgeFactor, 0.f, 1.f, "%.3f", ImGuiSliderFlags_Logarithmic);

	ImGui::Spacing();
	int columnWidth = 250;
	ImGui::TextColored(titleColor, "DrawInfos");
	ImGui::Text("Entities : %d", instanceDrawn);				ImGui::SameLine(columnWidth); ImGui::Text("Occluder triangles : %d", occluderTriangles);
	ImGui::Text("Drawcalls : %d", drawCalls);					ImGui::SameLine(columnWidth); ImGui::Text("Occlusion rasterized triangles : %d", occluderRasterizedTriangles);
	ImGui::Text("Triangles : %d", trianglesDrawn);				ImGui::SameLine(columnWidth); ImGui::Text("Occlusion pixel test : %d", occluderPixelsTest);
	ImGui::Text("Lights : %d", m_sceneLights.m_lightCount);		ImGui::SameLine(columnWidth); ImGui::Text("Occlusion skipped : %d", occlusionCulledInstances);
	ImGui::Text("Occlusion time : %d ms (%d)", (int)m_OcclusionAvgTime, (int)m_OcclusionElapsedTime);
	
	ImGui::PopID();
	ImGui::End();

	if (m_drawLightDirection)
	{
		vec4f d = m_environementLighting.m_directionalLightDirection;
		d.normalize();

		Debug::color = m_environementLighting.m_directionalLightColor;
		for (int i = -50; i <= 50; i++)
			for (int j = -50; j <= 50; j++)
			{
				vec4f p = vec4f(5 * i, 0, 5 * j, 1.f);
				Debug::drawLine(p, p - 100.f * d);
			}
	}

	CameraComponent* mainCamera = world.getMainCamera();
	if (m_drawClusters)
	{
		static std::vector<Debug::Vertex> clusterLines;
		if (clusterLines.empty())
		{
			vec4f color = Debug::white;
			vec3i imageSize = lightClusterTexture.size;
			int clusterCount = imageSize.x * imageSize.y * imageSize.z;
			int rightClusterCount = imageSize.y * imageSize.z;
			int topClusterCount = imageSize.x * imageSize.z;

			int a = imageSize.x;
			int b = imageSize.y;
			int c = imageSize.z;
			int size = 2 * (3 * a * b * c + 4 * a * b + 2 * b * c + 2 * a * c + c);
			clusterLines.reserve(size);

			for (int i = 0; i < imageSize.x; i++)
				for (int j = 0; j < imageSize.y; j++)
					for (int k = 0; k < imageSize.z; k++)
					{
						vec3f invImageSize = vec3f(1.0 / imageSize.x, 1.0 / imageSize.y, 1.0 / imageSize.z);
						float nearFarRatio = m_sceneLights.m_far / m_sceneLights.m_near;
						float maxDepthBound = -pow(nearFarRatio, (k + 1) * invImageSize.z) * m_sceneLights.m_near;
						float minDepthBound;
						if (k == 0) minDepthBound = 1.0;
						else minDepthBound = -pow(nearFarRatio, k * invImageSize.z) * m_sceneLights.m_near;

						vec2f frustrumSize = vec2f(-m_sceneLights.m_tanFovX * maxDepthBound, -m_sceneLights.m_tanFovY * maxDepthBound);
						vec2f cellSize = 2.f * vec2f(frustrumSize.x * invImageSize.x, frustrumSize.y * invImageSize.y);

						vec2f s = vec2f(i * cellSize.x, j * cellSize.y) - frustrumSize;
						vec4f m = vec4f(s.x, s.y, minDepthBound, 1);
						vec4f M = vec4f(s.x + cellSize.x, s.y + cellSize.y, maxDepthBound, 1);

						clusterLines.push_back({ m, color }); clusterLines.push_back({ vec4f(M.x, m.y, m.z, 1.f) , color });
						clusterLines.push_back({ m, color }); clusterLines.push_back({ vec4f(m.x, M.y, m.z, 1.f) , color });
						clusterLines.push_back({ m, color }); clusterLines.push_back({ vec4f(m.x, m.y, M.z, 1.f) , color });

						if (j == imageSize.y - 1)
						{
							clusterLines.push_back({ vec4f(m.x, M.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, m.z, 1.f) , color });
							clusterLines.push_back({ vec4f(m.x, M.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(m.x, M.y, M.z, 1.f) , color });

							if (i == imageSize.x - 1)
							{
								 clusterLines.push_back({ vec4f(M.x, M.y, m.z, 1.f), color }); 
								 clusterLines.push_back({ M , color });
							}
						}
						if (i == imageSize.x - 1)
						{
							clusterLines.push_back({ vec4f(M.x, m.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, m.z, 1.f) , color });
							clusterLines.push_back({ vec4f(M.x, m.y, m.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, m.y, M.z, 1.f) , color });
						}

						if (k == imageSize.z - 1)
						{
							clusterLines.push_back({ vec4f(m.x, m.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, m.y, M.z, 1.f) , color });
							clusterLines.push_back({ vec4f(m.x, m.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(m.x, M.y, M.z, 1.f) , color });
							clusterLines.push_back({ vec4f(m.x, M.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, M.z, 1.f) , color });
							clusterLines.push_back({ vec4f(M.x, m.y, M.z, 1.f), color }); clusterLines.push_back({ vec4f(M.x, M.y, M.z, 1.f) , color });
						}
					}
		}

		Debug::drawMultipleLines(clusterLines, mainCamera->getModelMatrix());
	}

	if (m_drawOcclusionBuffer)
	{
		fullScreenDraw(&occlusionTexture, occlusionResultDraw);
	}
#endif // USE_IMGUI
}
//