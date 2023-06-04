
#include <iostream>
#include <list>
#include <time.h>
#include <sys/types.h>

#include "Utiles/System.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
/*#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>*/

#include "Utiles/ToolBox.h"
#include "HUD/WidgetManager.h"
#include "Events/EventHandler.h"
#include "Renderer/Renderer.h"
#include "Renderer/DrawableComponent.h"
#include "Renderer/CameraComponent.h"
#include "Animation/Animator.h"
#include "Generators/HouseGenerator.h"
#include "Resources/Loader/SkeletonSaver.h"
#include "Animation/AnimationComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Core/Application.h"

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

#include <Physics/Physics.h>
#include <Physics/RigidBody.h>

#include <Resources/Loader/MeshSaver.h>
#include <Scene/RayEntityCollector.h>
#include <Scene/RaySceneQuerry.h>

#include <Utiles/Debug.h>
#include <Utiles/OpenSaveFileDialog.h>
#include <Physics/GJK.h>
#include <Utiles/ConsoleColor.h>

enum EventEnum
{
	#include <ModelImporterEventEnum.enum>
};

#include <Utiles/DirectoryWatcher.h>
#include <Physics/Shapes/Collider.h>



#define GRID_SIZE 10
#define GRID_ELEMENT_SIZE 1.f
#define DEBUG_LEVEL 0



//	global attributes
RenderContext* context = nullptr;
World world;
Entity* freeflyCamera = nullptr;
DirectoryWatcher* watcher = nullptr;

//Shader* normalShader = nullptr;

double completeTime = 16.;
double averageCompleteTime = 16.;

float avatarZeroHeight;
vec4f avatarspeed(0.f);

CameraComponent* currentCamera = nullptr;
struct {
	float radius = 2.f;
	vec4f target;
} cameraInfos;
//


// prototypes
std::string checkResourcesDirectory();

void initManagers();
void picking();
void events();
void updates(float elapseTime);
void Import();
void Export();
//


// program
int main()
{
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::GREEN) << "Application start";
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
	Application application;
	context = application.createWindow("GolemFactory : Model Importer", 1600, 900);
	context->makeCurrent();
	context->setVSync(true);
	application.initGLEW(1);
	initManagers();
	application.changeIcon(ResourceManager::getInstance()->getRepository() + "Textures/cubeIcon.png");

	watcher = new DirectoryWatcher(ResourceManager::getInstance()->getRepository() + "GUI");
	watcher->createNewFileWatcher("ModelImporter.gui");

	//	Collision test
	world.getEntityFactory().createObject("cube", [](Entity* object) // ground collider
		{
			object->setWorldTransformation(vec4f(0.f, 0.f, -10.f, 1.f), 1.f, quatf()); // vec4f(1000, 1000, 10, 1)
			object->removeComponent(object->getComponent<DrawableComponent>());
		});

	//normalShader = ResourceManager::getInstance()->getResource<Shader>("normalViewer");

	//	Empty scene
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("wired"));
	Renderer::getInstance()->setGridVisible(true);
	Renderer::getInstance()->normalViewer = ResourceManager::getInstance()->getResource<Shader>("normalViewer");

	freeflyCamera = world.getEntityFactory().createObject([](Entity* object)
		{
			vec4f p(-5, -3, 3, 1);
			object->setWorldPosition(p);

			Collider* collider = new Collider(new Sphere(vec4f(0.f), 0.1f));
			object->addComponent(collider);
			object->recomputeBoundingBox();

			CameraComponent* ffCam = new CameraComponent(true);
			ffCam->lookAt(-p);
			object->addComponent(ffCam);
			Renderer::getInstance()->setCamera(ffCam);

			currentCamera = ffCam;
		});

	// init loop time tracking
	double averageTime = 0;
	long samples = 0;
	double elapseTime = 16.;
	double dummy = 0;

	//	game loop
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::GREEN) << "Game loop initiated";
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

	while (!application.shouldExit())
	{
		// begin loop
		double startTime = glfwGetTime();

		//  handle events
		events();
		updates((float)elapseTime);

		// Render scene & picking
		if (WidgetManager::getInstance()->getBoolean("wireframe"))
			Renderer::getInstance()->setRenderOption(Renderer::RenderOption::WIREFRAME);
		else Renderer::getInstance()->setRenderOption(Renderer::RenderOption::DEFAULT);
		Renderer::getInstance()->render(currentCamera);

		// picking & HUD
		picking();
		Renderer::getInstance()->renderHUD();

		//	clear garbages
		world.clearGarbage();
		ResourceManager::getInstance()->clearGarbage();

		// End loop
		completeTime = 1000.0 * (glfwGetTime() - startTime);
		context->swapBuffers();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		elapseTime = 1000.0 * (glfwGetTime() - startTime);
	}

	//	end
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::GREEN) << "Ending game";
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

	delete watcher;
	world.clearGarbage();
	ResourceManager::getInstance()->clearGarbage();
	return 0;
}
//


//	functions implementation
std::string checkResourcesDirectory()
{
	//	check for home repository
	std::string directory = "C:/Users/Thibault/Documents/Github/GolemFactory/Resources/";
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
	EventHandler::getInstance()->loadKeyMapping("ModelImporter", "ModelImporterEventEnum");
	EventHandler::getInstance()->setCursorMode(true);
	EventHandler::getInstance()->addResizeCallback(WidgetManager::resizeCallback);

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
	const vec4f worldHalfSize = vec4f(GRID_SIZE * GRID_ELEMENT_SIZE, GRID_SIZE * GRID_ELEMENT_SIZE, 64, 0) * 0.5f;
	const vec4f worldPos = vec4f(0, 0, worldHalfSize.z - 5, 1.f);
	NodeVirtual::debugWorld = &world;
	world.getSceneManager().init(worldPos - worldHalfSize, worldPos + worldHalfSize, vec3i(4, 4, 1), 3);
	world.setMaxObjectCount(4000000);
	world.getEntityFactory().createObject([](Entity* object)
		{
			vec4f s = vec4f(0.5f * GRID_SIZE * GRID_ELEMENT_SIZE, 0.5f * GRID_SIZE * GRID_ELEMENT_SIZE, 1.f, 0.f);

			Collider* collider = new Collider(new AxisAlignedBox(-s, s));
			object->addComponent(collider);
			object->recomputeBoundingBox();
			object->setWorldPosition(vec4f(0.f, 0.f, -1.f, 1.f));
		});

	//	Renderer
	Renderer::getInstance()->setContext(context);
	Renderer::getInstance()->setWorld(&world);
	Renderer::getInstance()->initializeGrid(GRID_SIZE, GRID_ELEMENT_SIZE, vec4f(24 / 255.f, 202 / 255.f, 230 / 255.f, 1.f));	// blue tron
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_DRAWABLE_BB, ResourceManager::getInstance()->getResource<Shader>("wired"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_ANIMATABLE_BB, ResourceManager::getInstance()->getResource<Shader>("skeletonBB"));

	// Debug
	Debug::getInstance()->initialize("Shapes/point", "Shapes/box", "Shapes/sphere", "Shapes/capsule", "Shapes/point", "Shapes/segment", "default", "wired", "Shapes/multipleSegment");

	// Animator
	Animator::getInstance();

	//	HUD
	WidgetManager::getInstance()->setInitialViewportRatio(context->getViewportRatio());
	WidgetManager::getInstance()->loadHud("default");
	WidgetManager::getInstance()->loadHud("ModelImporter");
	WidgetManager::getInstance()->setActiveHUD("ModelImporter");
}
void picking()
{
	vec4f cameraPos = currentCamera->getPosition();
	vec4f cameraForward = currentCamera->getForward(); // no rotations

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
		if (compAnim)       type = "animatable";
		else if (compDraw)  type = "drawable";
		else               type = "empty entity";
		vec4f p = cameraPos + distance * cameraForward;

		WidgetManager::getInstance()->setString("interaction", "Distance : " + ToolBox::to_string_with_precision(distance, 5) +
			" m\nPosition : (" + ToolBox::to_string_with_precision(p.x, 5) + " , " + ToolBox::to_string_with_precision(p.y, 5) + " , " + ToolBox::to_string_with_precision(p.z, 5) +
			")\nInstance on ray : " + std::to_string(collector.getResult().size()) +
			"\nFirst instance pointed id : " + std::to_string(entity->getId()) +
			"\n  type : " + type);

		if (WidgetManager::getInstance()->getBoolean("BBpicking"))
		{
			Renderer::RenderOption option = Renderer::getInstance()->getRenderOption();
			Renderer::getInstance()->setRenderOption(option == Renderer::RenderOption::DEFAULT ? Renderer::RenderOption::BOUNDING_BOX : Renderer::RenderOption::DEFAULT);
			for (auto it = collector.getResult().begin(); it != collector.getResult().end(); ++it)
				Renderer::getInstance()->drawObject((*it));
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
		//	debug
		if (v[i] == QUIT) glfwSetWindowShouldClose(context->getParentWindow(), GL_TRUE);

		//else if (v[i] == F10)  EventHandler::getInstance()->getCursorPositionRelative();
		
		else if (v[i] == HELP) WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "help" ? "" : "help"));
		else if (v[i] == F9)   WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "debug" ? "" : "debug"));
		else if (v[i] == F10)  WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "rendering" ? "" : "rendering"));
		else if (v[i] == F11)  WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "ModelImporter" ? "" : "ModelImporter"));

		else if (v[i] == F4)   WidgetManager::getInstance()->setBoolean("wireframe", !WidgetManager::getInstance()->getBoolean("wireframe"));

		else if (v[i] == CLICK_LEFT && !EventHandler::getInstance()->isActivated(CLICK_LEFT)) // released
		{
			auto widgets = WidgetManager::getInstance()->getActiveWidgets();
			for (auto it = widgets.begin(); it != widgets.end(); ++it)
			{
				auto associations = WidgetManager::getInstance()->getWidgetAssociations(*it);
				if (associations)
				{
					for (unsigned int i = 0; i < associations->size(); i++)
					{
						if (associations->at(i) == "ImportButton")
							Import();
						else if (associations->at(i) == "ExportButton")
							Export();
					}
				}
			}
		}
	}

	if (watcher->hasChanges())
	{
		std::vector<std::string> files = watcher->getAllChanges();
		for (unsigned int i = 0; i < files.size(); i++)
			if (files[i].find_first_of("ModelImporter") != std::string::npos)
			{
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CYAN) << "ModelImporter GUI hot reload" << std::flush;
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

				WidgetManager::getInstance()->deleteHud("ModelImporter");
				WidgetManager::getInstance()->loadHud("ModelImporter");
				//WidgetManager::getInstance()->setActiveHUD("ModelImporter");
			}
	}
}
void updates(float elapseTime)
{
	//	Compute HUD picking parameters
	if (EventHandler::getInstance()->getCursorMode())
	{
		vec2f cursor = EventHandler::getInstance()->getCursorNormalizedPosition();
		mat4f projection = mat4f::perspective((float)DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION, context->getViewportRatio(), 0.1f, 1500.f);
		vec4f ray_eye = mat4f::inverse(projection) * vec4f(cursor.x, cursor.y, -1.f, 0.f);
		WidgetManager::getInstance()->setPickingParameters(
			mat4f::translate(mat4f::identity, vec4f(0.f, 0.f, -DISTANCE_HUD_CAMERA, 1.f)) * mat4f::eulerAngleZX((float)PI, (float)PI * 0.5f), ray_eye);
	}
	else
	{
		WidgetManager::getInstance()->setPickingParameters(mat4f::identity, vec4f::zero);
	}

	//	Update widgets
	averageCompleteTime = 0.99f * averageCompleteTime + 0.01f * completeTime;
	WidgetManager::getInstance()->setString("runtime speed",
		"FPS : " + std::to_string((int)(1000.f / completeTime)) + "\navg : " + std::to_string((int)(1000.f / averageCompleteTime)) +
		"\n\nTime : " + ToolBox::to_string_with_precision(completeTime) + " ms\navg : " + ToolBox::to_string_with_precision(averageCompleteTime) + " ms");
	WidgetManager::getInstance()->setString("drawcalls",
		"Instances :\n  " + std::to_string(Renderer::getInstance()->getNbDrawnInstances() + WidgetManager::getInstance()->getNbDrawnWidgets()) +
		"\n\nTriangles :\n  " + std::to_string(Renderer::getInstance()->getNbDrawnTriangles() + WidgetManager::getInstance()->getNbDrawnTriangles()));
	WidgetManager::getInstance()->update((float)elapseTime, EventHandler::getInstance()->isActivated(CLICK_LEFT));

	// animate camera
	if (!WidgetManager::getInstance()->isUnderMouse())
	{
		// Rotation
		CameraComponent* ffCam = freeflyCamera->getComponent<CameraComponent>();
		if (EventHandler::getInstance()->isActivated(CLICK_RIGHT))
		{
			float sensitivity = 0.2f;
			float yaw = (float)DEG2RAD * -sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x;
			float pitch = (float)DEG2RAD * -sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y;
			ffCam->rotate(pitch, yaw);
		}

		// Translation
		vec4f direction(0., 0., 0., 0.);
		float speed = ffCam->getVerticalFieldOfView();
		if (EventHandler::getInstance()->isActivated(CLICK_MIDDLE))
		{
			vec2f delta = EventHandler::getInstance()->getCursorPositionRelative();

			float sensitivity = 0.00001f;
			direction = ffCam->getUp() * delta.y - ffCam->getRight()* delta.x;
			direction *= sensitivity;
		}

		if (direction.x || direction.y || direction.z)
		{
			freeflyCamera->setWorldPosition(freeflyCamera->getWorldPosition() + direction * (elapseTime * speed));
			world.updateObject(freeflyCamera);
		}

		// FOV
		float angle = ffCam->getVerticalFieldOfView() + (float)DEG2RAD * EventHandler::getInstance()->getScrollingRelative().y;
		if (angle > 1.5f) angle = 1.5f;
		else if (angle < 0.05f) angle = 0.05f;

		ffCam->setVerticalFieldOfView(angle);
	}

}
//

// 
void Import()
{
	std::cout << "Import" << std::endl;

	std::string filename;
	bool success = OpenSaveFileDialog::OpenFile(filename);
	if (success)
	{
		std::cout << filename << std::endl;
	}


}
void Export()
{
	std::cout << "Export" << std::endl;
}
//