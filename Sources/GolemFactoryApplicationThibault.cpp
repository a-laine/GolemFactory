// Golem Factory 1.0.cpp�: define console application entry point
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
#include "Animation/Animator.h"
#include "Generators/HouseGenerator.h"
#include "World/World.h"
#include "Scene/SceneQueryTests.h"
#include "Resources/Loader/SkeletonSaver.h"
#include "EntityComponent/Entity.hpp"
#include "Renderer/DrawableComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/AnimationComponent.h"

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

#include "Physics/Collision.h"

#define GRID_SIZE 100
#define GRID_ELEMENT_SIZE 5.f
#define DEBUG 0



//	global attributes
GLFWwindow* window = nullptr;
World world;
Camera camera, camera2;
Entity* avatar = nullptr;

double completeTime = 16.;
double averageCompleteTime = 16.;
//


// prototypes
GLFWwindow* initGLFW();
void initializeForestScene(bool emptyPlace = false);
std::string checkResourcesDirectory();


void initManagers();
void picking(int width, int height);
void events();
void updates(float elapseTime, int width, int height);
//


// program
int main()
{
	// init window and opengl
	window = initGLFW();
	Renderer::getInstance()->initGLEW(0);
	initManagers();

	//	Collision test
		WidgetManager::getInstance()->setActiveHUD("debug");
		Collision::debugUnitaryTest(2);

	//	Test scene
		Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("wired"));
		initializeForestScene(false);

		avatar = world.getEntityFactory().createObject("peasant", [](Entity* object)
		{
            DrawableComponent* drawable = object->getComponent<DrawableComponent>();
			OrientedBox box = object->getBoundingVolume();
			float scale = 1.7f / (box.max - box.min).z;
			object->setScale(glm::vec3(scale));
			glm::vec3 pos = glm::vec3(20.f, 20.f, -scale * drawable->getMeshBBMin().z);
			object->setPosition(pos);
			//camera.setMode(Camera::TRACKBALL);
			camera.setRadius(4);
		});

	// init loop time tracking
	double averageTime = 0;
	long samples = 0;
	double elapseTime = 16.;
	double dummy = 0;
	int width, height;

	//	game loop
	std::cout << "game loop initiated" << std::endl;
	while (!glfwWindowShouldClose(window))
	{
		// begin loop
		double startTime = glfwGetTime();
		glfwGetWindowSize(window, &width, &height);

		// Render scene & picking
		if (WidgetManager::getInstance()->getBoolean("BBrendering"))
			Renderer::getInstance()->setRenderOption(Renderer::BOUNDING_BOX);
		else Renderer::getInstance()->setRenderOption(Renderer::DEFAULT);
		Renderer::getInstance()->render(&camera);
		picking(width, height);
		Renderer::getInstance()->renderHUD(&camera2);

		//  handle events
		events();
		updates((float)elapseTime, width, height);

		//	clear garbages
		world.clearGarbage();
		ResourceManager::getInstance()->clearGarbage();

		// End loop
		completeTime = 1000.0*(glfwGetTime() - startTime);
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		elapseTime = 1000.0*(glfwGetTime() - startTime);
	}

	//	end
	std::cout << "ending game" << std::endl;
	world.clearGarbage();
	ResourceManager::getInstance()->clearGarbage();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
//


//	functions implementation
static void errorCallback(int error, const char* description) { std::cerr << "GLFW ERROR : " << description << std::endl; }
GLFWwindow* initGLFW()
{
	if (!glfwInit()) exit(EXIT_FAILURE);

	glfwSetErrorCallback(errorCallback);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow*window = glfwCreateWindow(1600, 900, "Golem Factory v1.0", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	return window;
}


void initializeForestScene(bool emptyPlace)
{
	// blue sky & green grass!!
	glClearColor(0.6f, 0.85f, 0.91f, 0.f);
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));

	// init instance placement
	int fail = 0;
	std::string objectType;
	float sDispersion;
	float sOffset;
	HouseGenerator hg;
	srand(3);// (unsigned int)time(NULL));

	//	center tree in place
	if (!emptyPlace)
	{
		world.getEntityFactory().createObject("tree", glm::vec3(0), glm::vec3(5.f, 5.f, 5.f));
	}

	// village
	int vilageHouseCount = 0;
	float villageRadius[] = {20.f, 30.f, 40.f};
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
				glm::vec3 p = glm::vec3(radius * cos(angle), radius * sin(angle), house->getBoundingVolume().toSphere().radius);
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
				house->setOrientation(glm::rotate(glm::mat4(1.0), 1.57f + angle, glm::vec3(0, 0, 1)));
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
				sDispersion = 0.f;// 2f;
				sOffset = 1.7f;
			}
			else continue;

			float s = sOffset + sDispersion * ((rand() % 100) / 50.f - 1.f);
			glm::quat a = glm::rotate(glm::quat(), glm::radians((rand() % 3600) / 10.f), glm::vec3(0, 0, 1));

			world.getEntityFactory().createObject(objectType, p, glm::vec3(s, s, s), a);
		}

	//	debug
	if (DEBUG)
	{
		std::cout << "Instance count : " << world.getObjectCount() << std::endl;
		std::cout << "House count : " << vilageHouseCount  << std::endl;
		std::cout << "Insert fail : " << fail << std::endl;
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
	if (DEBUG) std::cout << "Found resources folder at : " << resourceRepository << std::endl;

	// Init Event handler
	EventHandler::getInstance()->addWindow(window);
	EventHandler::getInstance()->setRepository(resourceRepository);
	EventHandler::getInstance()->loadKeyMapping("RPG Key mapping");
	EventHandler::getInstance()->setCursorMode(false);
	EventHandler::getInstance()->setResizeCallback(WidgetManager::resizeCallback);

	// Init Resources manager
	ResourceVirtual::logVerboseLevel = ResourceVirtual::ALL;
	ResourceManager::getInstance()->setRepository(resourceRepository);
    Texture::setDefaultName("10points.png");
    Font::setDefaultName("Comic Sans MS");
    Shader::setDefaultName("default");
    Mesh::setDefaultName("cube2.obj");
    Skeleton::setDefaultName("human");
    Animation::setDefaultName("human");
    ResourceManager::getInstance()->addNewResourceLoader(".animation", new AnimationLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".font", new FontLoader());
    ResourceManager::getInstance()->addNewResourceLoader("assimp", new AssimpLoader(AssimpLoader::MESH));
    ResourceManager::getInstance()->addNewResourceLoader(".mesh", new MeshLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".shader", new ShaderLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".skeleton", new SkeletonLoader());
    ResourceManager::getInstance()->addNewResourceLoader("image", new ImageLoader());
    ResourceManager::getInstance()->addNewResourceLoader(".texture", new TextureLoader());

	// Init world
	const glm::vec3 worldHalfSize = glm::vec3(GRID_SIZE*GRID_ELEMENT_SIZE, GRID_SIZE*GRID_ELEMENT_SIZE, 50) * 0.5f;
	const glm::vec3 worldPos = glm::vec3(0, 0, worldHalfSize.z - 5);
	world.getSceneManager().init(worldPos - worldHalfSize, worldPos + worldHalfSize, glm::ivec3(4, 4, 1), 2);
	world.setMaxObjectCount(4000000);

	//	Renderer
	camera.setMode(Camera::FREEFLY);
	camera.setAllRadius(2.f, 0.5f, 10.f);
	camera.setPosition(glm::vec3(0, 20, 20));
	camera2.setMode(Camera::FREEFLY);
	camera2.setAllRadius(2.f, 0.5f, 10.f);
	camera2.setPosition(glm::vec3(0, 20, 20));
	Renderer::getInstance()->setWindow(window);
	Renderer::getInstance()->setWorld(&world);
	Renderer::getInstance()->initializeGrid(GRID_SIZE, GRID_ELEMENT_SIZE, glm::vec3(24 / 255.f, 202 / 255.f, 230 / 255.f));	// blue tron
	Renderer::getInstance()->setCamera(&camera2);
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_DRAWABLE_BB, ResourceManager::getInstance()->getResource<Shader>("wired"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_ANIMATABLE_BB, ResourceManager::getInstance()->getResource<Shader>("skeletonBB"));

	// Animator
	Animator::getInstance();

	//	HUD
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	WidgetManager::getInstance()->setInitialWindowSize(width, height);
	WidgetManager::getInstance()->loadHud("default");
}
void picking(int width, int height)
{
	DefaultSceneManagerRayTest sceneNodeTest(camera.getPosition(), camera.getForward(), 100);
	DefaultRayPickingCollector collector(camera.getPosition(), camera.getForward(), 100);
	world.getSceneManager().getObjects(collector, sceneNodeTest);

	if (!collector.getObjects().empty())
	{
		std::string type;
		AnimationComponent* compAnim = collector.getNearestObject()->getComponent<AnimationComponent>();
		DrawableComponent* compDraw = collector.getNearestObject()->getComponent<DrawableComponent>();
		if(compAnim)       type = "animatable";
		else if(compDraw)  type = "drawable";
		else            type = "empty entity";
		glm::vec3 p = camera.getPosition() + collector.getNearestDistance() * camera.getForward();

		WidgetManager::getInstance()->setString("interaction", "Distance : " + ToolBox::to_string_with_precision(collector.getNearestDistance(), 5) +
			" m\nPosition : (" + ToolBox::to_string_with_precision(p.x, 5) + " , " + ToolBox::to_string_with_precision(p.y, 5) + " , " + ToolBox::to_string_with_precision(p.z, 5) +
			")\nInstance on ray : " + std::to_string(collector.getObjects().size()) +
			"\nFirst instance pointed id : " + std::to_string(collector.getNearestObject()->getId()) +
			"\n  type : " + type);

		if (WidgetManager::getInstance()->getBoolean("BBpicking"))
		{
			Renderer::RenderOption option = Renderer::getInstance()->getRenderOption();
			Renderer::getInstance()->setRenderOption(option == Renderer::DEFAULT ? Renderer::BOUNDING_BOX : Renderer::DEFAULT);
			glm::mat4 projection = glm::perspective(glm::radians(camera.getFrustrumAngleVertical()), (float)width / height, 0.1f, 1500.f);
			Renderer::getInstance()->drawObject(collector.getNearestObject(), &camera.getViewMatrix()[0][0], &projection[0][0]);
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

	for (unsigned int i = 0; i < v.size(); i++)
	{
		//	micselenious
		if (v[i] == QUIT) glfwSetWindowShouldClose(window, GL_TRUE);
		else if (v[i] == CHANGE_CURSOR_MODE) EventHandler::getInstance()->setCursorMode(!EventHandler::getInstance()->getCursorMode());
		else if (v[i] == ACTION)
		{
			if (camera.getMode() == Camera::TRACKBALL) camera.setMode(Camera::FREEFLY);
			else if (camera.getMode() == Camera::FREEFLY) camera.setMode(Camera::TRACKBALL);
		}

		//	avatar related
		else if (v[i] == SLOT1) Animator::getInstance()->launchAnimation(avatar, "hello");
		else if (v[i] == SLOT2) Animator::getInstance()->launchAnimation(avatar, "yes");
		else if (v[i] == SLOT3) Animator::getInstance()->launchAnimation(avatar, "no");
		else if (v[i] == FORWARD)
		{
			if (EventHandler::getInstance()->isActivated(FORWARD) && camera.getMode() == Camera::TRACKBALL)
			{
				if (EventHandler::getInstance()->isActivated(RUN)) Animator::getInstance()->launchAnimation(avatar, "run");
				else Animator::getInstance()->launchAnimation(avatar, "walk");
			}
			else
			{
				Animator::getInstance()->stopAnimation(avatar, "run");
				Animator::getInstance()->stopAnimation(avatar, "walk");
			}
		}
		else if (v[i] == RUN)
		{
			if (EventHandler::getInstance()->isActivated(RUN))
			{
				if (EventHandler::getInstance()->isActivated(FORWARD) && camera.getMode() == Camera::TRACKBALL)
					Animator::getInstance()->launchAnimation(avatar, "run");
				else
					Animator::getInstance()->stopAnimation(avatar, "run");
			}
			else
			{
				Animator::getInstance()->stopAnimation(avatar, "run");
				if (EventHandler::getInstance()->isActivated(FORWARD) && camera.getMode() == Camera::TRACKBALL)
					Animator::getInstance()->launchAnimation(avatar, "walk");
			}
		}

		//	debug action
		else if (v[i] == HELP) WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "help" ? "" : "help"));
		else if (v[i] == F9)   WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "debug" ? "" : "debug"));
		else if (v[i] == F10)  WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "rendering" ? "" : "rendering"));
	}
}
void updates(float elapseTime, int width, int height)
{
	//	animate avatar
	Animator::getInstance()->animate(avatar, elapseTime);

	//	Compute HUD picking parameters
	if (EventHandler::getInstance()->getCursorMode())
	{
		glm::vec2 cursor = EventHandler::getInstance()->getCursorNormalizedPosition();
		glm::vec4 ray_eye = glm::inverse(glm::perspective(glm::radians(ANGLE_VERTICAL_HUD_PROJECTION), (float)width / height, 0.01f, 150.f)) * glm::vec4(cursor.x, cursor.y, -1.f, 1.f);
		WidgetManager::getInstance()->setPickingParameters(
			camera.getViewMatrix() * glm::translate(glm::mat4(1.f), DISTANCE_HUD_CAMERA * camera.getForward()) * camera.getModelMatrix(),
			glm::normalize(glm::vec3(ray_eye.x, ray_eye.y, ray_eye.z)),
			camera.getPosition());
	}
	else
	{
		WidgetManager::getInstance()->setPickingParameters(glm::mat4(1.f), glm::vec3(0.f), camera.getPosition());
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
	if (camera.getMode() == Camera::TRACKBALL)
	{
		glm::vec3 forward = camera.getForward();
		float speed = 0.f;
		if (Animator::getInstance()->isAnimationRunning(avatar, "walk")) speed = 0.025f;
		if (Animator::getInstance()->isAnimationRunning(avatar, "run")) speed = 0.1f;
		avatar->setPosition(avatar->getPosition() + speed * glm::normalize(glm::vec3(forward.x, forward.y, 0.f)));
		if (speed != 0.f) avatar->setOrientation(glm::rotate(glm::pi<float>() / 2.f + atan2(forward.y, forward.x), glm::vec3(0.f, 0.f, 1.f)));
		world.updateObject(avatar);
	}

	//Animate camera
	if (camera.getMode() == Camera::TRACKBALL)
	{
        SkeletonComponent* skeletonComp = avatar->getComponent<SkeletonComponent>();
        if(skeletonComp && skeletonComp->isValid())
        {
            glm::vec4 headPosition = avatar->getMatrix() * glm::vec4(skeletonComp->getJointPosition("Head"), 1);
            camera.setTarget(headPosition);
        }
	}
	camera.animate((float)elapseTime,
		EventHandler::getInstance()->isActivated(FORWARD), EventHandler::getInstance()->isActivated(BACKWARD),
		EventHandler::getInstance()->isActivated(LEFT), EventHandler::getInstance()->isActivated(RIGHT),
		EventHandler::getInstance()->isActivated(RUN), EventHandler::getInstance()->isActivated(SNEAKY));
	if (camera.getMode() == Camera::TRACKBALL)
		camera.setRadius(camera.getRadius() + camera.getSensitivity() * EventHandler::getInstance()->getScrollingRelative().y);
	else
	{
		float angle = camera.getFrustrumAngleVertical() + EventHandler::getInstance()->getScrollingRelative().y;
		if (angle > 70.f) angle = 70.f;
		else if (angle < 3) angle = 3.f;
		camera.setFrustrumAngleVertical(angle);
		camera.setFrustrumAngleHorizontalFromScreenRatio((float)width / height);
	}
	if (WidgetManager::getInstance()->getBoolean("syncCamera"))
		camera2 = camera;
}
//
