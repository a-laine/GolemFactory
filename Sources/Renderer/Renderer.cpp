#include "Renderer.h"

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
#include <Utiles/Debug.h>


#define BATCH_SIZE 32

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

	defaultShader[GRID] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE] = nullptr;
	defaultShader[INSTANCE_DRAWABLE] = nullptr;
	defaultShader[INSTANCE_ANIMATABLE_BB] = nullptr;
	defaultShader[INSTANCE_DRAWABLE_BB] = nullptr;
	defaultShader[HUD] = nullptr;

	drawGrid = true;

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
void Renderer::initGlobalUniformBuffers()
{
	// global matrices and camera position
	m_globalMatrices.view = mat4f::identity;
	m_globalMatrices.projection = mat4f::identity;
	m_globalMatrices.cameraPosition = vec4f(0, 0, 0, 1);

	glGenBuffers(1, &m_globalMatricesID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_globalMatrices), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_globalMatricesID, 0, sizeof(m_globalMatrices));
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// environment lighting settings
	m_environementLighting.m_directionalLightDirection = vec4f(-10, -20, 10, 0);
	m_environementLighting.m_directionalLightColor = vec4f(0.15, 0.15, 0.15, 1.0);
	m_environementLighting.m_ambientColor = vec4f(0.05, 0.05, 0.05, 1.0);

	glGenBuffers(1, &m_environementLightingID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_environementLighting), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_environementLightingID, 0, sizeof(m_environementLighting));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void Renderer::updateGlobalUniformBuffers()
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_globalMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_globalMatrices), &m_globalMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, m_environementLightingID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_environementLighting), &m_environementLighting);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void Renderer::render(CameraComponent* renderCam)
{
	//	clear previous states
	trianglesDrawn = 0;
	instanceDrawn = 0;
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
	glEnable(GL_CULL_FACE); 
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	//	draw grid
	Shader* shader = defaultShader[GRID];
	if (drawGrid && shader && glIsVertexArray(gridVAO))
	{
		shader->enable();
		loadMVPMatrix(shader, &mat4f::identity[0][0], quatf::identity);
		int loc = shader->getUniformLocation("overrideColor");
		if (loc >= 0) glUniform4fv(loc, 1, &m_gridColor[0]);

		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_INT, NULL);

		if (loc >= 0) glUniform4fv(loc, 1, (float*)&vec4f(-1.f, 0.f, 0.f, 1.f)[0]);
	}

	//	get instance list
	float camFovVert = camera->getVerticalFieldOfView();
	vec4f camPos, camFwd, camUp, camRight;
	camera->getFrustrum(camPos, camFwd, camRight, camUp);
	FrustrumSceneQuerry sceneTest(camPos, camFwd, camUp, -camRight, camFovVert, context->getViewportRatio());
	VirtualEntityCollector collector;
	collector.m_flags = (uint64_t)Entity::Flags::Fl_Drawable;
	collector.m_exclusionFlags = (uint64_t)Entity::Flags::Fl_Hide;
	world->getSceneManager().getEntities(&sceneTest, &collector);

	//	sort
	renderQueue.clear();
	for (Entity* object : collector.getResult())
	{
		DrawableComponent* comp = object->getComponent<DrawableComponent>();
		if (!comp || !comp->isValid()) continue;
#ifdef USE_IMGUI
		if (!comp->visible()) continue;
#endif
		uint32_t queue = comp->getShader()->getRenderQueue();

		vec4f v = object->getWorldPosition() - camPos;
		uint16_t d = (uint16_t)vec4f::dot(v, v);
		if (queue >= 3000)
		{
			//compute 2's complement of d
			d = ~d; 
			d++;
		}

		uint32_t hash = (queue << 16) | d;
		renderQueue.push_back({ hash, object});
	}
	std::sort(renderQueue.begin(), renderQueue.end(), [](std::pair<uint32_t, Entity*> a, std::pair<uint32_t, Entity*> b) {return a.first < b.first; });

	//	draw instance list
	for (const auto& it : renderQueue)
	{
		//	try to do dynamic batching
		Entity* object = it.second;
		DrawableComponent* comp = object->getComponent<DrawableComponent>();
		if (GLEW_VERSION_1_3 && comp->getShader()->getInstanciable())
		{
			shader = comp->getShader()->getInstanciable();
			Mesh* m = comp->getMesh();
			std::vector<mat4f>& batch = groupBatches[shader][m];
			batch.push_back(object->getWorldTransformMatrix());
			if (batch.size() >= BATCH_SIZE)
			{
				drawInstancedObject(shader, m, batch);
				batch.clear();
			}
		}
		else // simple draw
		{
			std::vector<Entity*>& batch = simpleBatches[comp->getShader()];
			batch.push_back(object);
			if (batch.size() >= BATCH_SIZE)
			{
				for (unsigned int i = 0; i < BATCH_SIZE; i++)
					drawObject(batch[i]);
				batch.clear();
			}
		}
	}

	//	draw residual objects in batches
	for (auto it = simpleBatches.begin(); it != simpleBatches.end(); ++it)
	{
		std::vector<Entity*>& batch = it->second;
		for (unsigned int i = 0; i < batch.size(); i++)
			drawObject(batch[i]);
		batch.clear();
	}
	for (auto it = groupBatches.begin(); it != groupBatches.end(); ++it)
	{
		shader = it->first;
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			Mesh* m = it2->first;
			if(!groupBatches[shader][m].empty())
				drawInstancedObject(shader, m, groupBatches[shader][m]);
			groupBatches[shader][m].clear();
		}
	}
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
						if (!defaultShader[HUD]) shader = (*it2)->getShader();
						loadMVPMatrix(shader, &mat4f::translate(model, (*it2)->getPosition())[0][0], quatf::identity);
						if (!shader) continue;

						//	Draw
						(*it2)->draw(shader, stencilMask, model);
						instanceDrawn++;
						trianglesDrawn += (*it2)->getNumberFaces();
					}
				}
			}
		}
	}
	glDisable(GL_BLEND);
}
//


//	Protected functions
void Renderer::loadMVPMatrix(Shader* shader, const float* model, const quatf rotation, const int& modelSize)
{
	if (shader == lastShader && shader)
	{
		int loc = shader->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, modelSize, GL_FALSE, model);

		if (modelSize == 1)
		{
			loc = shader->getUniformLocation("normalMatrix");
			if (loc >= 0)
			{
				mat4f rotationMatrix = mat4f(rotation);
				glUniformMatrix4fv(loc, modelSize, GL_FALSE, &rotationMatrix[0][0]);
			}
		}
	}
	else if (shader)
	{
		shader->enable();
		int loc = shader->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, modelSize, GL_FALSE, model);

		if (modelSize == 1)
		{
			loc = shader->getUniformLocation("normalMatrix");
			if (loc >= 0)
			{
				mat4f rotationMatrix = mat4f(rotation);
				glUniformMatrix4fv(loc, modelSize, GL_FALSE, &rotationMatrix[0][0]);
			}
		}
		lastShader = shader;
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
unsigned int Renderer::getNbDrawnTriangles() const { return trianglesDrawn; }
Renderer::RenderOption Renderer::getRenderOption() const { return renderOption; }


void Renderer::setEnvAmbientColor(vec4f color) { m_environementLighting.m_ambientColor = color; }
void Renderer::setEnvDirectionalLightDirection(vec4f direction) { m_environementLighting.m_directionalLightDirection = direction; }
void Renderer::setEnvDirectionalLightColor(vec4f color) { m_environementLighting.m_directionalLightColor = color; }
vec4f Renderer::getEnvAmbientColor() const { return m_environementLighting.m_ambientColor; }
vec4f Renderer::getEnvDirectionalLightDirection() const { return m_environementLighting.m_directionalLightDirection; }
vec4f Renderer::getEnvDirectionalLightColor() const { return m_environementLighting.m_directionalLightColor; }
//

//	Debug
void Renderer::drawImGui(World& world)
{
#ifdef USE_IMGUI
	ImGui::Begin("Rendering setings");
	ImGui::PushID(this);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Environement lighting");
	ImGui::DragFloat3("Directional light direction", &m_environementLighting.m_directionalLightDirection[0], 0.01f);
	ImGui::ColorEdit3("Directional light color", &m_environementLighting.m_directionalLightColor[0]);
	ImGui::ColorEdit3("Ambient color", &m_environementLighting.m_ambientColor[0]);
	ImGui::Checkbox("Draw light direction", &m_drawLightDirection);

	const char* renderOptions[] = { "Default", "Bounding box", "Wireframe", "Normals" };
	static int renderOptionsCurrentIdx = (int)renderOption;
	const char* renderOptionPreviewValue = renderOptions[renderOptionsCurrentIdx];
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
#endif // USE_IMGUI
}
//