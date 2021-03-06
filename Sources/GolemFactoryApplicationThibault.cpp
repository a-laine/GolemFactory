// Golem Factory 2.0.cpp: define console application entry point
//



#include <iostream>
#include <list>
#include <time.h>
#include <sys/types.h>

#include "Utiles/System.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Utiles/ToolBox.h"
#include "HUD/WidgetManager.h"
#include "Events/EventHandler.h"
#include "Renderer/Renderer.h"
#include "Renderer/DrawableComponent.h"
#include "Animation/Animator.h"
#include "Generators/HouseGenerator.h"
#include "Resources/Loader/SkeletonSaver.h"
#include "Animation/AnimationComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Application.h"
#include "Events/EventEnum.h"

#include <Resources/Texture.h>
#include <Resources/Font.h>
#include <Resources/Shader.h>
#include <Resources/Mesh.h>
#include <Resources/Skeleton.h>
#include <Resources/Animation.h>
#include <Resources/Loader/AnimationLoader.h>
#include <Resources/Loader/FontLoader.h>
#include <Resources/Loader/AssimpLoader.h>
#include <Resources/Loader/MeshLoader.h>
#include <Resources/Loader/ShaderLoader.h>
#include <Resources/Loader/SkeletonLoader.h>
#include <Resources/Loader/ImageLoader.h>
#include <Resources/Loader/TextureLoader.h>

#include "Physics/Physics.h"
#include "Physics/RigidBody.h"

#include "Resources/Loader/MeshSaver.h"
#include "Scene/RayEntityCollector.h"
#include "Scene/RaySceneQuerry.h"

#include "Utiles/Debug.h"
#include "Physics/GJK.h"

#define GRID_SIZE 512
#define GRID_ELEMENT_SIZE 1.f
#define DEBUG_LEVEL 0



//	global attributes
RenderContext* context = nullptr;
World world;
Entity* avatar = nullptr;
Entity* freeflyCamera = nullptr;
Entity* frustrumCamera = nullptr;
Shader* normalShader = nullptr;

double completeTime = 16.;
double averageCompleteTime = 16.;

float avatarZeroHeight;
glm::vec3 avatarspeed(0.f);

CameraComponent* currentCamera = nullptr;
struct {
	float radius = 2.f;
	glm::vec3 target;
} cameraInfos;
//


// prototypes
void initializeForestScene(bool emptyPlace = false);
void initializePhysicsScene(int testCase = -1);
std::string checkResourcesDirectory();

void initManagers();
void picking();
void events();
void updates(float elapseTime);
//


bool wiredhull = true;
Entity* testEntity = nullptr;

// program
int main()
{
	std::cout << "Application start" << std::endl;
	Application application;
	context = application.createWindow("Thibault test", 1600, 900);
	context->makeCurrent();
	context->setVSync(true);
	application.initGLEW(1);
	initManagers();

	//	Collision test
		WidgetManager::getInstance()->setActiveHUD("debug");
		world.getEntityFactory().createObject("cube", [](Entity* object) // ground collider
		{
			object->setTransformation(glm::vec3(0.f, 0.f, -10.f), glm::vec3(1000, 1000, 10), glm::fquat());
			object->removeComponent(object->getComponent<DrawableComponent>());
		});

		normalShader = ResourceManager::getInstance()->getResource<Shader>("normalViewer");

	//	Test scene
		Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("wired"));
		//initializeForestScene(false);
		//initializePhysicsScene(0);
		Renderer::getInstance()->setGridVisible(false);
		if (world.getMap().loadFromHeightmap(ResourceManager::getInstance()->getRepository() + "/Textures/", "mountains512.png"))
		{
			std::cout << "Map loaded !" << std::endl;
		}
		else
		{
			std::cout << "ERROR in map loading" << std::endl;
		}
		/*avatar = world.getEntityFactory().createObject("peasant", [](Entity* object)
		{
            DrawableComponent* drawable = object->getComponent<DrawableComponent>();
			float scale = 1.7f / (drawable->getMeshBBMax() - drawable->getMeshBBMin()).z;
			glm::vec3 pos = glm::vec3(-10.f, 0.f, -scale * drawable->getMeshBBMin().z);
			//drawable->setShader(nullptr);
			object->setTransformation(pos, glm::vec3(scale), glm::toQuat(glm::rotate(glm::pi<float>() / 2.f + atan2(1.f, 0.f), glm::vec3(0.f, 0.f, 1.f))));

			SkeletonComponent* skeletonComp = object->getComponent<SkeletonComponent>();
			cameraInfos.target = skeletonComp->getJointPosition("Head");

			CameraComponent* tbCam = new CameraComponent(true);
			tbCam->setPosition(cameraInfos.target - glm::vec3(0, 1, 0));
			tbCam->lookAt(cameraInfos.target, cameraInfos.radius / scale);
			object->addComponent(tbCam);

			//currentCamera = tbCam;
			avatarZeroHeight = object->getPosition().z;
		});*/

		freeflyCamera = world.getEntityFactory().createObject([](Entity* object)
		{
			object->setPosition(glm::vec3(0, 0, 5));
			object->setShape(new Sphere());
			CameraComponent* ffCam = new CameraComponent(true);
			ffCam->lookAt(glm::vec3(0, 1, 0));
			object->addComponent(ffCam);
			
			currentCamera = ffCam;
		});

		frustrumCamera = world.getEntityFactory().createObject([](Entity* object)
		{
			object->setShape(new Sphere());
			CameraComponent* cam = new CameraComponent(true);
			object->addComponent(cam);
			Renderer::getInstance()->setCamera(cam);
		});

		WidgetManager::getInstance()->setBoolean("BBpicking", false);
		WidgetManager::getInstance()->setBoolean("wireframe", false);
		Renderer::getInstance()->normalViewer = ResourceManager::getInstance()->getResource<Shader>("normalViewer");

		//Entity* testTree = world.getEntityFactory().createObject("tree", glm::vec3(5, 0, 0), glm::vec3(1), glm::rotate(glm::quat(), glm::radians((rand() % 3600) / 10.f), glm::vec3(0, 0, 1)));
		//Collision::debugUnitaryTest(2, nullptr);
		

	// init loop time tracking
	double averageTime = 0;
	long samples = 0;
	double elapseTime = 16.;
	double dummy = 0;

	//	game loop
	std::cout << "game loop initiated" << std::endl;
	while (!application.shouldExit())
	{
		// begin loop
		double startTime = glfwGetTime();
		
		//  handle events
		events();
		updates((float)elapseTime);

		//	physics
		/*std::vector<UserEventType> v;
		EventHandler::getInstance()->getFrameEvent(v);
		for (unsigned int i = 0; i < v.size(); i++)
		{
			//	debug
			if (v[i] == SLOT8)
			{
				world.getPhysics().stepSimulation(0.016, &world.getSceneManager());
			}
		}*/
		world.getPhysics().stepSimulation(0.016f, &world.getSceneManager());

		// Render scene & picking
		if (WidgetManager::getInstance()->getBoolean("BBrendering"))
			Renderer::getInstance()->setRenderOption(Renderer::RenderOption::BOUNDING_BOX);
		else if (WidgetManager::getInstance()->getBoolean("wireframe"))
			Renderer::getInstance()->setRenderOption(Renderer::RenderOption::WIREFRAME);
		else Renderer::getInstance()->setRenderOption(Renderer::RenderOption::DEFAULT);
		Renderer::getInstance()->render(currentCamera);
		/*{
			glm::mat4 view = currentCamera->getGlobalViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(currentCamera->getVerticalFieldOfView(context->getViewportRatio())), context->getViewportRatio(), 0.1f, 1500.f);
		}*/
		 
		Debug::view = currentCamera->getGlobalViewMatrix();
		Debug::projection = glm::perspective(glm::radians(currentCamera->getVerticalFieldOfView(context->getViewportRatio())), context->getViewportRatio(), 0.1f, 30000.f);  //far = 1500.f
		Renderer::getInstance()->drawMap(world.getMapPtr(), &Debug::view[0][0], &Debug::projection[0][0], normalShader);

		/*Debug::color = Debug::black;
		float o = 0.2f;
		Debug::drawLine(glm::vec3(0), glm::vec3(1, 0, o)); Debug::drawLine(glm::vec3(1, 0, o), glm::vec3(0, 0, 2 * o));
		Debug::drawLine(glm::vec3(0, 0, 2 * o), glm::vec3(0, 1, 3 * o)); Debug::drawLine(glm::vec3(0, 1, 3 * o), glm::vec3(0, 0, 4 * o));*/

		//const Capsule* shape1 = static_cast<const Capsule*>(avatar->getGlobalBoundingShape());
		
		//const Hull* shape2 = static_cast<const Hull*>(testEntity->getGlobalBoundingShape());
		//Hull shape1 = Hull(shape2->mesh, glm::scale(avatar->getMatrix(), glm::vec3(5.f)));

		//Capsule s1(shape1->p1, shape1->p2 + 0.5f * currentCamera->getRight(), shape1->radius);
		//auto u = shape2->toAxisAlignedBox();
		//Capsule s2(u.min, u.max, 1.f);

		/*if(Collision::collide(shape1, *shape2))
			Debug::color = Debug::red;
		else Debug::color = Debug::white;*/

		/*Debug::drawWiredMesh(shape1.mesh, shape1.base);
		Debug::drawWiredMesh(shape2->mesh, shape2->base);
		//Debug::drawWiredCapsule(s1.p1, s1.p2, s1.radius);
		//Debug::drawWiredCapsule(s2.p1, s2.p2, s2.radius);
				
		Debug::color = Debug::white;
		Intersection::Contact contact = Intersection::intersect(shape1, *shape2);
		Debug::color = Debug::red;
		Debug::drawLine(contact.contactPointA, contact.contactPointB);

		Debug::color = Debug::blue;
		Debug::drawLine(contact.contactPointA, contact.contactPointA + 0.15f * contact.normalA);
		Debug::drawLine(contact.contactPointB, contact.contactPointB + 0.15f * contact.normalB);*/
		

		// 
		picking();
		Renderer::getInstance()->renderHUD();

		//	clear garbages
		world.clearGarbage();
		ResourceManager::getInstance()->clearGarbage();

		// End loop
		completeTime = 1000.0*(glfwGetTime() - startTime);
		context->swapBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		elapseTime = 1000.0*(glfwGetTime() - startTime);
	}

	//	end
	std::cout << "ending game" << std::endl;
	world.clearGarbage();
	ResourceManager::getInstance()->clearGarbage();
	return 0;
}
//


//	functions implementation
void initializeForestScene(bool emptyPlace)
{
	// blue sky & green grass!!
	glClearColor(0.6f, 0.85f, 0.91f, 0.f);
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));

	// init instance placement
	std::string objectType;
	float sDispersion;
	float sOffset;
	HouseGenerator hg;
	srand(3);// (unsigned int)time(NULL));

	//	center tree in place
	if (!emptyPlace)
	{
		//world.getEntityFactory().createObject("tree", glm::vec3(0), glm::vec3(5.f, 5.f, 5.f));
		//world.getEntityFactory().createObject("rock", glm::vec3(0), glm::vec3(50.f, 50.f, 50.f));
		//world.getEntityFactory().createObject([&hg](Entity* house) { hg.getHouse(house, 0, 100, 100); });

		unsigned int N = 7;
		unsigned int N2 = 2;
		for (unsigned int j = 0; j < N2; j++)
		{
			for (unsigned int i = 0; i < N; i++)
			{
				glm::vec3 p = glm::vec3(5.f * cos((float)i / N * 2.f * glm::pi<float>()), 5.f * sin((float)i / N * 2.f * glm::pi<float>()), 2.f*(j + 1));
				p += N2 * 0.5f * glm::normalize(p);
				world.getEntityFactory().createObject("sphere", [&p, &j](Entity* object)
				{
					//auto q = glm::normalize(glm::fquat(0.01f *(rand() % 100), 0.01f *(rand() % 100), 0.01f *(rand() % 100), 0.01f *(rand() % 100)));
					auto q = glm::fquat(0, 0, 0, 1);
					object->setTransformation(p, glm::vec3(1.f), q);
					RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
					rb->setMass(1.f);
					rb->setGravityFactor(1.f);
					//rb->setAngularVelocity(glm::vec3(0,1,0));
					object->addComponent(rb);
				});
			}
		}
		return;

		/*testEntity = world.getEntityFactory().createObject("rock", [](Entity* object)
		{
			object->getComponent<DrawableComponent>()->setShader(ResourceManager::getInstance()->getResource<Shader>("default"));
			object->setTransformation(glm::vec3(0.f, 0.f, 0.f), glm::vec3(5.f), glm::fquat(0,0,0,1));
			RigidBody* rb = new RigidBody(RigidBody::DYNAMIC, RigidBody::CONTINUOUS);
			rb->setMass(1.f);
			rb->setGravityFactor(1.f);
			rb->setAngularVelocity(glm::vec3(1.f, 0, 0));
			
			//object->addComponent(rb);
			object->getComponent<DrawableComponent>()->setShader(nullptr);
		});*/
	}

	// village
	int vilageHouseCount = 0;
	float villageRadius[] = {20.f, 35.f, 50.f};
	std::vector<glm::vec3> houseCircle;
	for (int i = 0; i < 3; i++)
	{
		int houseCount = 5 + i * 7;
		vilageHouseCount += houseCount;
		float angleOffset = 6.28f * ((rand() % 100) / 100.f);

		for (int j = 0; j < houseCount; j++)
		{
			float radius = villageRadius[i] + 3.f * (((rand() % 100) / 50.f) - 1.f);
			float angle = angleOffset + 6.28f * j / houseCount + ((((rand() % 100) / 50.f) - 1.f)) / houseCount;
			
			glm::mat4 a(1.0);
			if(rand() % 2)
				a = glm::rotate(a, 3.14f + angle + 0.4f * ((((rand() % 100) / 50.f) - 1.f)), glm::vec3(0, 0, 1));
			else
				a = glm::rotate(a, angle + 0.4f * ((((rand() % 100) / 50.f) - 1.f)), glm::vec3(0, 0, 1));
			
			world.getEntityFactory().createObject([radius, angle, &houseCircle, &hg](Entity* house)
			{
                hg.getHouse(house, rand(), 20, 30);
				glm::vec3 p = glm::vec3(radius * cos(angle), radius * sin(angle), house->getGlobalBoundingShape()->toSphere().radius);
				for (unsigned int k = 0; k < houseCircle.size(); k++)
				{
					float delta = glm::length(glm::vec3(houseCircle[k].x - p.x, houseCircle[k].y - p.y, 0.f));
					if (delta < houseCircle[k].z + p.z - 0.5f)
					{
						p += (1.f + houseCircle[k].z + p.z - delta) * glm::normalize(glm::vec3(p.x - houseCircle[k].x, p.y - houseCircle[k].y, 0.f));
						k = 0;
					}
				}

				houseCircle.push_back(p);
				house->setPosition(glm::vec3(p.x, p.y, 0.f));
				house->setOrientation(glm::toQuat(glm::rotate(glm::mat4(1.0), 1.57f + angle, glm::vec3(0, 0, 1))));
			});
		}
	}


	// forest
	for (int i = 0; i < GRID_SIZE; i++)
		for (int j = 0; j < GRID_SIZE; j++)
		{
			glm::vec3 p(GRID_ELEMENT_SIZE*i - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + GRID_ELEMENT_SIZE * ((rand() % 10) / 20.f - 0.25f),
				GRID_ELEMENT_SIZE*j - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + GRID_ELEMENT_SIZE * ((rand() % 10) / 20.f - 0.25f),
				0);

			if(glm::length(p) < villageRadius[2] + 5.f * GRID_ELEMENT_SIZE)
				continue;

			int r = rand() % 100;
			if (r < 20)
			{
				objectType = "rock";
				sDispersion = 0.4f;
				sOffset = 1.f;
			}
			else if (r < 80)
			{
				objectType = "tree";
				sDispersion = 0.3f;
				sOffset = 2.0f;
			}
			else continue;

			float s = sOffset + sDispersion * ((rand() % 100) / 50.f - 1.f);
			glm::quat a = glm::rotate(glm::quat(), glm::radians((rand() % 3600) / 10.f), glm::vec3(0, 0, 1));

			world.getEntityFactory().createObject(objectType, p, glm::vec3(s, s, s), a);
		}
	
	//	debug
	if (DEBUG_LEVEL)
	{
		std::cout << "Instance count : " << world.getObjectCount() << std::endl;
		std::cout << "House count : " << vilageHouseCount  << std::endl;
	}
}
void initializePhysicsScene(int testCase)
{
	glClearColor(0.6f, 0.85f, 0.91f, 0.f);
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));

	if (testCase == 0)
	{
		unsigned int N = 20;
		for (unsigned int i = 0; i < N; i++)
		{
			for (unsigned int j = 0; j < N; j++)
			{
				float r = 1.f;// 0.02f *(rand() % 100) + 0.3f;
				glm::vec3 p = glm::vec3(5.f * (i - N / 2.f), 5.f * (j - N / 2.f), r);
				world.getEntityFactory().createObject("sphere", [&p, &r](Entity* object)
				{
					object->setTransformation(p, glm::vec3(r), glm::fquat(0, 0, 0, 1));
					RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
					rb->setMass(3.f * r);
					rb->setBouncyness(0.1f);
					rb->setFriction(0.f);
					rb->setGravityFactor(1.f);
					rb->setVelocity(glm::vec3(0.1f *((rand() % 200) - 100), 0.1f *((rand() % 200) - 100), 0));
					object->addComponent(rb);
				});
			}
		}
	}
}
std::string checkResourcesDirectory()
{
	//	check for home repository
	std::string directory = "C:/Users/Thibault/Documents/Github/GolemFactory/Resources/";
	if (ToolBox::isPathExist(directory)) return directory;

	//	check for work repository
	directory = "C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/";
	if (ToolBox::isPathExist(directory)) return directory;
	
	//	return the default resource path for portable applications
	std::cout << "FATAL WARRNING : Fail to find ressource repo" << std::endl;
	return "Resources/";
}


void initManagers()
{
	std::string resourceRepository = checkResourcesDirectory();
	if (DEBUG_LEVEL) std::cout << "Found resources folder at : " << resourceRepository << std::endl;

	// Init Event handler
	EventHandler::getInstance()->addWindow(context->getParentWindow());
	EventHandler::getInstance()->setRepository(resourceRepository);
	EventHandler::getInstance()->loadKeyMapping("RPG Key mapping", "");
	EventHandler::getInstance()->setCursorMode(false);
	EventHandler::getInstance()->setResizeCallback(WidgetManager::resizeCallback);

	// Init Resources manager
	ResourceVirtual::logVerboseLevel = ResourceVirtual::VerboseLevel::ALL;
	ResourceManager::getInstance()->setRepository(resourceRepository);
    Texture::setDefaultName("10points.png");
    Font::setDefaultName("Comic Sans MS");
    Shader::setDefaultName("default");
    Mesh::setDefaultName("cube2.obj");
    Skeleton::setDefaultName("human");
    Animation::setDefaultName("human");
    ResourceManager::getInstance()->addNewResourceLoader(".animation", new AnimationLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".font", new FontLoader());
    ResourceManager::getInstance()->addNewResourceLoader("assimp", new AssimpLoader(AssimpLoader::ResourceType::MESH));
    ResourceManager::getInstance()->addNewResourceLoader(".mesh", new MeshLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".shader", new ShaderLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".skeleton", new SkeletonLoader());
    ResourceManager::getInstance()->addNewResourceLoader("image", new ImageLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".texture", new TextureLoader());

	// Init world
	const glm::vec3 worldHalfSize = glm::vec3(GRID_SIZE*GRID_ELEMENT_SIZE, GRID_SIZE*GRID_ELEMENT_SIZE, 64) * 0.5f;
	const glm::vec3 worldPos = glm::vec3(0, 0, worldHalfSize.z - 5);
	NodeVirtual::debugWorld = &world;
	world.getSceneManager().init(worldPos - worldHalfSize, worldPos + worldHalfSize, glm::ivec3(4, 4, 1), 3);
	world.setMaxObjectCount(4000000);
	world.getEntityFactory().createObject([](Entity* object)
	{
		glm::vec3 s = glm::vec3(0.5f*GRID_SIZE*GRID_ELEMENT_SIZE, 0.5f*GRID_SIZE*GRID_ELEMENT_SIZE, 1.f);
		object->setShape(new AxisAlignedBox(-s, s));
		object->setPosition(glm::vec3(0.f, 0.f, -1.f));
	});
	world.getMap().setShader(ResourceManager::getInstance()->getResource<Shader>("map"));

	//	Renderer
	Renderer::getInstance()->setContext(context);
	Renderer::getInstance()->setWorld(&world);
	Renderer::getInstance()->initializeGrid(GRID_SIZE, GRID_ELEMENT_SIZE, glm::vec3(24 / 255.f, 202 / 255.f, 230 / 255.f));	// blue tron
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_DRAWABLE, ResourceManager::getInstance()->getResource<Shader>("default"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_DRAWABLE_BB, ResourceManager::getInstance()->getResource<Shader>("wired"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_ANIMATABLE_BB, ResourceManager::getInstance()->getResource<Shader>("skeletonBB"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_DRAWABLE_WIRED, ResourceManager::getInstance()->getResource<Shader>("wired"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_ANIMATABLE_WIRED, ResourceManager::getInstance()->getResource<Shader>("wiredSkinning"));
	
	// Debug
	Debug::getInstance()->initialize("Shapes/point", "Shapes/box", "Shapes/sphere", "Shapes/capsule", "Shapes/point", "Shapes/segment", "default", "wired");

	// Animator
	Animator::getInstance();

	//	HUD
	WidgetManager::getInstance()->setInitialViewportRatio(context->getViewportRatio());
	WidgetManager::getInstance()->loadHud("default");
}
void picking()
{
	glm::vec3 cameraPos = currentCamera->getGlobalPosition();
	glm::vec3 cameraForward = currentCamera->getForward(); // no rotations

	RaySceneQuerry test(cameraPos, cameraForward, 10000);
	RayEntityCollector collector(cameraPos, cameraForward, 10000);
	world.getSceneManager().getEntities(&test, &collector);

	//std::sort(collector.getSortedResult().begin(), collector.getSortedResult().end(), [](std::pair<float, unsigned int> a, std::pair<float, unsigned int> b) { return a.first > b.first; });

	if (!collector.getSortedResult().empty())
	{
		std::sort(collector.getSortedResult().begin(), collector.getSortedResult().end(), [](std::pair<float, unsigned int> a, std::pair<float, unsigned int> b) { return a.first > b.first; });
		float distance = collector.getSortedResult().front().first;
		Entity* entity = collector.getResult()[collector.getSortedResult().front().second];

		std::string type;
		AnimationComponent* compAnim = entity->getComponent<AnimationComponent>();
		DrawableComponent* compDraw = entity->getComponent<DrawableComponent>();
		if(compAnim)       type = "animatable";
		else if(compDraw)  type = "drawable";
		else               type = "empty entity";
		glm::vec3 p = cameraPos + distance * cameraForward;
		
		WidgetManager::getInstance()->setString("interaction", "Distance : " + ToolBox::to_string_with_precision(distance, 5) +
			" m\nPosition : (" + ToolBox::to_string_with_precision(p.x, 5) + " , " + ToolBox::to_string_with_precision(p.y, 5) + " , " + ToolBox::to_string_with_precision(p.z, 5) +
			")\nInstance on ray : " + std::to_string(collector.getResult().size()) +
			"\nFirst instance pointed id : " + std::to_string(entity->getId()) +
			"\n  type : " + type);

		if (WidgetManager::getInstance()->getBoolean("BBpicking"))
		{
			Renderer::RenderOption option = Renderer::getInstance()->getRenderOption();
			Renderer::getInstance()->setRenderOption(option == Renderer::RenderOption::DEFAULT ? Renderer::RenderOption::BOUNDING_BOX : Renderer::RenderOption::DEFAULT);
			glm::mat4 view = currentCamera->getGlobalViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(currentCamera->getVerticalFieldOfView(context->getViewportRatio())), context->getViewportRatio(), 0.1f, 1500.f);

			for (auto it = collector.getResult().begin(); it != collector.getResult().end(); ++it)
				Renderer::getInstance()->drawObject((*it), &view[0][0], &projection[0][0]);
			Renderer::getInstance()->setRenderOption(option);
		}
	}
	else
	{
		WidgetManager::getInstance()->setString("interaction", "Distance : (inf)\nPosition : ()\nInstance on ray : 0\nFirst instance pointed id : (null)\n ");
	}
}
void events()
{
	EventHandler::getInstance()->handleEvent();
	std::vector<UserEventType> v;
	EventHandler::getInstance()->getFrameEvent(v);

	bool isTrackBall = avatar != nullptr && currentCamera->getParentEntity() == avatar;

	for (unsigned int i = 0; i < v.size(); i++)
	{
		//	debug
		/*if (v[i] == SLOT8) GJK::max_iteration++;
		else if (v[i] == SLOT9) GJK::max_iteration--;

		//	micselenious
		else */if (v[i] == QUIT) glfwSetWindowShouldClose(context->getParentWindow(), GL_TRUE);
		else if (v[i] == CHANGE_CURSOR_MODE) EventHandler::getInstance()->setCursorMode(!EventHandler::getInstance()->getCursorMode());
		else if (v[i] == ACTION && avatar != nullptr)
		{
			if (isTrackBall)
			{
				CameraComponent* tbCam = avatar->getComponent<CameraComponent>();
				CameraComponent* ffCam = freeflyCamera->getComponent<CameraComponent>();
				freeflyCamera->setPosition(tbCam->getGlobalPosition());
				ffCam->setOrientation(tbCam->getOrientation()); // free rotations
				currentCamera = ffCam;
				world.updateObject(freeflyCamera);
			}
			else
			{
				CameraComponent* tbCam = avatar->getComponent<CameraComponent>();
				currentCamera = tbCam;
			}

			isTrackBall = currentCamera->getParentEntity() == avatar;
		}

		//	avatar related
		else if (v[i] == SLOT1 && avatar) Animator::getInstance()->launchAnimation(avatar, "hello");
		else if (v[i] == SLOT2 && avatar) Animator::getInstance()->launchAnimation(avatar, "yes");
		else if (v[i] == SLOT3 && avatar) Animator::getInstance()->launchAnimation(avatar, "no");
		else if (v[i] == FORWARD || v[i] == BACKWARD || v[i] == LEFT || v[i] == RIGHT)
		{
			if (EventHandler::getInstance()->isActivated(v[i]) && isTrackBall && avatar)
			{
				if (EventHandler::getInstance()->isActivated(RUN)) Animator::getInstance()->launchAnimation(avatar, "run");
				else Animator::getInstance()->launchAnimation(avatar, "walk");
			}
			else if (!EventHandler::getInstance()->isActivated(FORWARD) && !EventHandler::getInstance()->isActivated(BACKWARD) && 
				     !EventHandler::getInstance()->isActivated(LEFT) && !EventHandler::getInstance()->isActivated(RIGHT) && avatar)
			{
				Animator::getInstance()->stopAnimation(avatar, "run");
				Animator::getInstance()->stopAnimation(avatar, "walk");
			}
		}
		else if (v[i] == RUN && avatar)
		{
			if (EventHandler::getInstance()->isActivated(RUN))
			{
				if (EventHandler::getInstance()->isActivated(FORWARD) && isTrackBall)
					Animator::getInstance()->launchAnimation(avatar, "run");
				else
					Animator::getInstance()->stopAnimation(avatar, "run");
			}
			else
			{
				Animator::getInstance()->stopAnimation(avatar, "run");
				if (EventHandler::getInstance()->isActivated(FORWARD) && isTrackBall)
					Animator::getInstance()->launchAnimation(avatar, "walk");
			}
		}

		//	debug action
		else if (v[i] == HELP) WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "help" ? "" : "help"));
		else if (v[i] == F9)   WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "debug" ? "" : "debug"));
		else if (v[i] == F10)  WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "rendering" ? "" : "rendering"));


		else if (v[i] == F4)   WidgetManager::getInstance()->setBoolean("wireframe", !WidgetManager::getInstance()->getBoolean("wireframe"));
		else if (v[i] == F5)
		{
			wiredhull = !wiredhull;
			Renderer::getInstance()->setShader(Renderer::INSTANCE_DRAWABLE_WIRED, ResourceManager::getInstance()->getResource<Shader>(wiredhull ? "wired" : "default"));
		}
	}
}
void updates(float elapseTime)
{
	//	animate avatar
	if(avatar)
		Animator::getInstance()->animate(avatar, elapseTime);

	//	Compute HUD picking parameters
	if (EventHandler::getInstance()->getCursorMode())
	{
		glm::vec2 cursor = EventHandler::getInstance()->getCursorNormalizedPosition();
		glm::mat4 projection = glm::perspective(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION), context->getViewportRatio(), 0.1f, 1500.f);
		glm::vec4 ray_eye = glm::inverse(projection) * glm::vec4(cursor.x, cursor.y, -1.f, 1.f);
		WidgetManager::getInstance()->setPickingParameters(
			glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -DISTANCE_HUD_CAMERA)) * glm::eulerAngleZX(glm::pi<float>(), glm::pi<float>()*0.5f),
			glm::normalize(glm::vec3(ray_eye.x, ray_eye.y, ray_eye.z)));// ,
			//currentCamera->getGlobalPosition());
	}
	else
	{
		WidgetManager::getInstance()->setPickingParameters(glm::mat4(1.f), glm::vec3(0.f));// , currentCamera->getGlobalPosition());
	}

	//	Update widgets
	averageCompleteTime = 0.99f * averageCompleteTime + 0.01f * completeTime;
	WidgetManager::getInstance()->setString("runtime speed",
		"FPS : " + std::to_string((int)(1000.f / completeTime)) + "\navg : " + std::to_string((int)(1000.f / averageCompleteTime)) +
		"\n\nTime : " + ToolBox::to_string_with_precision(completeTime) + " ms\navg : " + ToolBox::to_string_with_precision(averageCompleteTime) + " ms");
	WidgetManager::getInstance()->setString("drawcalls",
		"Instances :\n  " + std::to_string(Renderer::getInstance()->getNbDrawnInstances() + WidgetManager::getInstance()->getNbDrawnWidgets()) +
		"\n\nTriangles :\n  " + std::to_string(Renderer::getInstance()->getNbDrawnTriangles() + WidgetManager::getInstance()->getNbDrawnTriangles()));
	WidgetManager::getInstance()->update((float)elapseTime, EventHandler::getInstance()->isActivated(USE1));

	//	Move avatar if needed
	if (avatar && currentCamera->getParentEntity() == avatar)
	{
		//	compute direction and speed
		glm::vec3 direction = glm::vec3(0.f);
		if (EventHandler::getInstance()->isActivated(FORWARD))
			direction += glm::normalize(glm::vec3(currentCamera->getForward().x, currentCamera->getForward().y, 0.f));
		else if (EventHandler::getInstance()->isActivated(BACKWARD))
			direction -= glm::normalize(glm::vec3(currentCamera->getForward().x, currentCamera->getForward().y, 0.f));
		if (EventHandler::getInstance()->isActivated(LEFT))
			direction -= glm::normalize(glm::vec3(currentCamera->getRight().x, currentCamera->getRight().y, 0.f));
		else if (EventHandler::getInstance()->isActivated(RIGHT))
			direction += glm::normalize(glm::vec3(currentCamera->getRight().x, currentCamera->getRight().y, 0.f));
		if(direction.x != 0.f && direction.y != 0.f)
			direction = glm::normalize(direction);

		float speed = 0.f;
		if (Animator::getInstance()->isAnimationRunning(avatar, "run"))
			speed = 0.1f;
		else if (Animator::getInstance()->isAnimationRunning(avatar, "walk"))
			speed = 0.025f;

		//	physics
		Capsule avatarCollider(*static_cast<const Capsule*>(avatar->getGlobalBoundingShape()));
		avatarCollider.transform(speed * direction, glm::vec3(1.f), glm::fquat());

		/*if (DEBUG)
		{
			debugShape->setPosition(speed * direction + avatarCollider.p1);
			debugShape->setScale(glm::vec3(avatarCollider.radius));
			world.updateObject(debugShape);
			debugShape2->setPosition(speed * direction + avatarCollider.p2);
			debugShape2->setScale(glm::vec3(avatarCollider.radius));
			world.updateObject(debugShape2);
		}*/

		//Physics::getInstance()->moveEntity(avatar, elapseTime / 1000.f, speed * direction);
		
		/*
		glm::vec3 s = glm::vec3(avatar->getGlobalBoundingShape()->toSphere().radius);
		DefaultSceneManagerBoxTest sceneNodeTest(avatar->getPosition() + deltaPosition - s, avatar->getPosition() + deltaPosition + s);
		DefaultBoxCollector collector;
		world.getSceneManager().getObjects(collector, sceneNodeTest);
		std::vector<Entity*>& entities = collector.getObjectInBox();
		std::set<unsigned int> collisionIndex;
		std::vector<glm::vec3> collisionNormal;

		for (unsigned int k = 0; k < entities.size(); k++)
		{
			if (entities[k] == avatar) continue;
			if (entities[k] == debugShape) continue;
			if (entities[k] == debugShape2) continue;
			Entity* e = entities[k];
			glm::mat4 model2 = e->getMatrix();

			// pass 1 : box vs avatar sphere
			if(!Collision::collide(*e->getGlobalBoundingShape(), avatarCollider))
				continue;
			
			// pass 2 : avatar capsule vs mesh
			DrawableComponent* drawableComp = e->getComponent<DrawableComponent>();
			if (drawableComp && drawableComp->isValid())
			{
				const std::vector<glm::vec3>& vertices = *drawableComp->getMesh()->getVertices();
				const std::vector<unsigned short>& faces = *drawableComp->getMesh()->getFaces();
				for (unsigned int j = 0; j < faces.size(); j += 3)
				{
					glm::vec3 p1 = glm::vec3(model2 * glm::vec4(vertices[faces[j]], 1.f));
					glm::vec3 p2 = glm::vec3(model2 * glm::vec4(vertices[faces[j + 1]], 1.f));
					glm::vec3 p3 = glm::vec3(model2 * glm::vec4(vertices[faces[j + 2]], 1.f));

					if (Collision::collide(Triangle(p1, p2, p3), avatarCollider))
					{
						Intersection::Result result = Intersection::intersect(Triangle(p1, p2, p3), avatarCollider);
						glm::vec3 n = -result.normal2;// glm::vec3(-result.normal2.x, -result.normal2.y, -result.normal2.z);
						if(n != glm::vec3(0.f))
							n = glm::normalize(n);
						if (glm::dot(n, avatar->getPosition() - result.contact2) < 0.f)
							n *= -1.f;

						collisionNormal.push_back(n);
						collisionIndex.insert(k);
					}
				}
			}
		}
		

		//	debug
		if (false && DEBUG && !collisionNormal.empty())
		{
			Renderer::RenderOption option = Renderer::getInstance()->getRenderOption();
			Renderer::getInstance()->setRenderOption(option == Renderer::DEFAULT ? Renderer::BOUNDING_BOX : Renderer::DEFAULT);
			glm::mat4 view = currentCamera->getGlobalViewMatrix();
			glm::mat4 projection = glm::perspective(glm::radians(currentCamera->getFrustrumAngleVertical()), (float)width / height, 0.1f, 1500.f);
			Renderer::getInstance()->drawObject(avatar, &view[0][0], &projection[0][0]);
			for (std::set<unsigned int>::iterator it = collisionIndex.begin(); it != collisionIndex.end(); ++it)
				Renderer::getInstance()->drawObject(entities[*it], &view[0][0], &projection[0][0]);
			Renderer::getInstance()->setRenderOption(option);
		}

		//	update
		if (deltaPosition.x != 0.f && deltaPosition.y != 0.f)
			avatar->setOrientation(glm::toQuat(glm::rotate(glm::pi<float>() / 2.f + atan2(deltaPosition.y, deltaPosition.x), glm::vec3(0.f, 0.f, 1.f))));
		bool grounded = false;
		if (!collisionNormal.empty())
		{
			for (unsigned int i = 0; i < collisionNormal.size(); i++)
			{
				if (collisionNormal[i].z > 0.5f)
					grounded = true;
				if (glm::dot(deltaPosition, collisionNormal[i]) < -0.01f)
					deltaPosition -= glm::dot(deltaPosition, collisionNormal[i]) * collisionNormal[i];

				//std::cout << collisionNormal[i].z << std::endl;
			}
		}


		glm::vec3 p = avatar->getPosition() + deltaPosition;
		if (p.z <= avatarZeroHeight)
		{
			grounded = true;
			p.z = avatarZeroHeight;
		}
		avatarspeed.x = deltaPosition.x;
		avatarspeed.y = deltaPosition.y;
		if (grounded)
		{
			deltaPosition.z = 0.f;
			avatarspeed.z = 0.f;
		}
		else 
		{
			avatarspeed.z -= elapseTime * 0.0005f;
			deltaPosition.z = avatarspeed.z;
			p.z += avatarspeed.z;
		}
		avatarspeed.z = glm::clamp(avatarspeed.z, -0.7f, 0.7f);
		std::cout << (grounded ? "grounded " : ". ") << avatarspeed.z << std::endl;// << std::endl;
*/

		avatar->setPosition(avatar->getPosition() + speed * direction);
		if (direction.x != 0.f && direction.y != 0.f)
			avatar->setOrientation(glm::toQuat(glm::rotate(glm::pi<float>() / 2.f + atan2(direction.y, direction.x), glm::vec3(0.f, 0.f, 1.f))));
		world.updateObject(avatar);
	}

	// animate camera
	if (avatar && currentCamera->getParentEntity() == avatar)
	{
		if (!EventHandler::getInstance()->getCursorMode())
		{
			SkeletonComponent* skeletonComp = avatar->getComponent<SkeletonComponent>();
			if (skeletonComp && skeletonComp->isValid())
			{

				float sensitivity = 0.2f;
				float yaw = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x);
				float pitch = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y);
				cameraInfos.radius = cameraInfos.radius - sensitivity * EventHandler::getInstance()->getScrollingRelative().y;
				cameraInfos.radius = glm::clamp(cameraInfos.radius, 0.5f, 10.f);

				CameraComponent* tbCam = avatar->getComponent<CameraComponent>();
				tbCam->rotateAround(cameraInfos.target, pitch, yaw, cameraInfos.radius / avatar->getScale()[0]);
			}
		}
	}
	else
	{
		CameraComponent* ffCam = freeflyCamera->getComponent<CameraComponent>();
		//CameraComponent* tbCam = avatar->getComponent<CameraComponent>();
		
		// Rotation
		if (!EventHandler::getInstance()->getCursorMode())
		{
			float sensitivity = 0.2f;
			float yaw = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x);
			float pitch = glm::radians(-sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y);
			ffCam->rotate(pitch, yaw);
		}

		// Translation
		glm::vec3 direction(0., 0., 0.);
		glm::vec3 forward = ffCam->getForward(); // global because free rotations
		glm::vec3 right = ffCam->getRight(); // global because free rotations

		float speed = 0.003f;
		if (EventHandler::getInstance()->isActivated(FORWARD)) direction += forward;
		if (EventHandler::getInstance()->isActivated(BACKWARD)) direction -= forward;
		if (EventHandler::getInstance()->isActivated(LEFT)) direction -= right;
		if (EventHandler::getInstance()->isActivated(RIGHT)) direction += right;
		if (EventHandler::getInstance()->isActivated(SNEAKY)) speed /= 10.f;
		if (EventHandler::getInstance()->isActivated(RUN)) speed *= 10.f;

		if (direction.x || direction.y || direction.z)
		{
			freeflyCamera->setPosition(freeflyCamera->getPosition() + glm::normalize(direction)*elapseTime*speed);
			world.updateObject(freeflyCamera);
		}

		// FOV
		float angle = ffCam->getFieldOfView() + EventHandler::getInstance()->getScrollingRelative().y;
		if (angle > 160.f) angle = 160.f;
		else if (angle < 3.f) angle = 3.f;

		//tbCam->setFieldOfView(angle);
		ffCam->setFieldOfView(angle);
	}

	if (WidgetManager::getInstance()->getBoolean("syncCamera"))
	{
		CameraComponent* cam = frustrumCamera->getComponent<CameraComponent>();
		cam->setOrientation(currentCamera->getOrientation());
		frustrumCamera->setPosition(currentCamera->getGlobalPosition());
		world.updateObject(frustrumCamera);
	}

	// Map streaming
	world.getMapPtr()->update(freeflyCamera->getPosition());
}
//
