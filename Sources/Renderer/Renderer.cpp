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


#define BATCH_SIZE 32


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
void Renderer::initializeGrid(const unsigned int& gridSize,const float& elementSize, const glm::vec3& color)
{
	if (glIsVertexArray(gridVAO)) return;

	defaultShader[GRID] = ResourceManager::getInstance()->getResource<Shader>("wired");

	//	generate grid vertex buffer
	float* vertexBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	float* colorBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	float* normalBufferGrid = new float[3 * (gridSize + 1)*(gridSize + 1)];
	for (unsigned int i = 0; i < gridSize + 1; i++)
		for (unsigned int j = 0; j < gridSize + 1; j++)
		{
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = elementSize*i - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = elementSize*j - (gridSize * elementSize) / 2;
			vertexBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 0;

			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = 0.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = 0.f;
			normalBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = 1.f;

			colorBufferGrid[3 * (i*(gridSize + 1) + j) + 0] = color.x;
			colorBufferGrid[3 * (i*(gridSize + 1) + j) + 1] = color.y;
			colorBufferGrid[3 * (i*(gridSize + 1) + j) + 2] = color.z;
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

	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * (gridSize + 1)*(gridSize + 1) * sizeof(float), colorBufferGrid, GL_STATIC_DRAW);

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
	delete[] colorBufferGrid;
	delete[] normalBufferGrid;
	delete[] indexBufferGrid;
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
	glm::mat4 view = renderCam->getGlobalViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(renderCam->getVerticalFieldOfView(context->getViewportRatio())), context->getViewportRatio(), 0.1f, 1500.f);
	
	//	opengl state
	glEnable(GL_DEPTH_TEST);

	//	draw grid
	Shader* shader = defaultShader[GRID];
	if (drawGrid && shader && glIsVertexArray(gridVAO))
	{
		shader->enable();
		loadMVPMatrix(shader, &glm::mat4(1.0)[0][0], &view[0][0], &projection[0][0]);
		glBindVertexArray(gridVAO);
		glDrawElements(GL_TRIANGLES, vboGridSize, GL_UNSIGNED_INT, NULL);
	}

	//	get instance list
	float camFovVert = camera->getVerticalFieldOfView(context->getViewportRatio());
	glm::vec3 camPos, camFwd, camUp, camRight;
	camera->getFrustrum(camPos, camFwd, camUp, camRight);
	//std::vector<Entity*> instanceList;
	FrustrumSceneQuerry sceneTest(camPos, camFwd, camUp, -camRight, camFovVert / 1.6f, camFovVert / 1.6f);
	VirtualEntityCollector collector;
	world->getSceneManager().getEntities(&sceneTest, &collector);

	EntityCompareDistance compare(camPos);
	std::sort(collector.getResult().begin(), collector.getResult().end(), compare);

	//	draw instance list
	for (Entity* object : collector.getResult())
	{
		DrawableComponent* comp = object->getComponent<DrawableComponent>();
		if(!comp || !comp->isValid()) continue;

		//	try to do dynamic batching
		if (GLEW_VERSION_1_3 && comp->getShader()->getInstanciable())
		{
			shader = comp->getShader()->getInstanciable();
			Mesh* m = comp->getMesh();
			std::vector<glm::mat4>& batch = groupBatches[shader][m];
			batch.push_back(object->getTransformMatrix());
			if (batch.size() >= BATCH_SIZE)
			{
				drawInstancedObject(shader, m, batch, &view[0][0], &projection[0][0]);
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
					drawObject(batch[i], &view[0][0], &projection[0][0]);
				batch.clear();
			}
		}
	}

	//	draw residual objects in batches
	for (auto it = simpleBatches.begin(); it != simpleBatches.end(); ++it)
	{
		std::vector<Entity*>& batch = it->second;
		for (unsigned int i = 0; i < batch.size(); i++)
			drawObject(batch[i], &view[0][0], &projection[0][0]);
		batch.clear();
	}
	for (auto it = groupBatches.begin(); it != groupBatches.end(); ++it)
	{
		shader = it->first;
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			Mesh* m = it2->first;
			if(!groupBatches[shader][m].empty())
				drawInstancedObject(shader, m, groupBatches[shader][m], &view[0][0], &projection[0][0]);
			groupBatches[shader][m].clear();
		}
	}
}
void Renderer::renderHUD()
{
	if (!context) return;

	// bind matrix
	glm::mat4 view = glm::eulerAngleZX(glm::pi<float>(), glm::pi<float>()*0.5f);
	view[3] = glm::vec4(0.f, 0.f, -DISTANCE_HUD_CAMERA, 1.f);
	glm::mat4 projection = glm::perspective(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION), context->getViewportRatio(), 0.1f, 1500.f);

	//	change opengl states
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

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
				glm::mat4 model = it->second[i]->getModelMatrix();
				std::vector<WidgetVirtual*>& list = it->second[i]->getChildrenList();
				for (std::vector<WidgetVirtual*>::iterator it2 = list.begin(); it2 != list.end(); ++it2)
				{
					if ((*it2)->isVisible())	// widget visible
					{
						//	Get shader and prepare matrix
						Shader* shader = nullptr;
						if (!defaultShader[HUD]) shader = (*it2)->getShader();
						loadMVPMatrix(shader, &glm::translate(model, (*it2)->getPosition())[0][0], &view[0][0], &projection[0][0]);
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
void Renderer::loadMVPMatrix(Shader* shader, const float* model, const float* view, const float* projection, const int& modelSize)
{
	if (shader == lastShader && shader)
	{
		int loc = shader->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, modelSize, GL_FALSE, model);
	}
	else if (shader)
	{
		shader->enable();
		int loc = shader->getUniformLocation("model");
		if (loc >= 0) glUniformMatrix4fv(loc, modelSize, GL_FALSE, model);
		loc = shader->getUniformLocation("view");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, view);
		loc = shader->getUniformLocation("projection");
		if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, projection);
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


/*void Renderer::addDrawShapeDefinition(Shape::ShapeType type, Mesh* mesh, Shader* shader)
{
	drawShapeDefinition[type] = std::pair<Mesh*, Shader*>(mesh, shader);
}*/
//
