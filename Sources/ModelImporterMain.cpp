
#include <iostream>
#include <list>
#include <time.h>
#include <sys/types.h>

#include "Utiles/System.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
#include <Resources/AnimationClip.h>
#include <Resources/Loader/AnimationLoader.h>
#include <Resources/Loader/FontLoader.h>
#include <Resources/Loader/AssimpLoader.h>
#include <Resources/Loader/MeshLoader.h>
#include <Resources/Loader/ShaderLoader.h>
#include <Resources/Loader/SkeletonLoader.h>
#include <Resources/Loader/ImageLoader.h>
#include <Resources/Loader/TextureLoader.h>
#include <Resources/Loader/AnimationGraphLoader.h>
#include <Resources/Loader/MeshSaver.h>

#include <Physics/Physics.h>
#include <Physics/RigidBody.h>
#include <Physics/Shapes/Collider.h>
#include <Physics/GJK.h>

#include <Scene/RayEntityCollector.h>
#include <Scene/RaySceneQuerry.h>

#include <Utiles/Debug.h>
#include <Utiles/OpenSaveFileDialog.h>
#include <Utiles/ConsoleColor.h>
#include <Utiles/DirectoryWatcher.h>

#include <Terrain/Terrain.h>

enum EventEnum
{
	#include <ModelImporterEventEnum.enum>
};

#include <imgui_internal.h>


#define GRID_SIZE 2048
#define GRID_ELEMENT_SIZE 1.f
#define DEBUG_LEVEL 0



//	global attributes
RenderContext* context = nullptr;
World world;
Entity* editorCamera = nullptr;
DirectoryWatcher* watcher = nullptr;
Terrain terrain;

double completeTime = 16.;
double averageCompleteTime = 16.;

float avatarZeroHeight;
vec4f avatarspeed(0.f);

CameraComponent* currentCamera = nullptr;
float editorCameraSensibility = 0.001f;
struct {
	float radius = 2.f;
	vec4f target;
} cameraInfos;

bool TerrainCreatorWindowEnable = true;
//


// prototypes
void initializeSyntyScene();
std::string checkResourcesDirectory();

void initManagers();
void picking();
void events();
void updates(float elapseTime);
void Import();
void Export();
void ImGuiMenuBar();
void ImGuiSystemDraw();
//


// program
int main()
{
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::GREEN) << "Application start";
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
	Application application;
	context = application.createWindow("GolemFactory : Editor", 1600, 900);
	context->makeCurrent();
	context->setVSync(true);
	application.initGLEW(1);
	initManagers();
	application.changeIcon(ResourceManager::getInstance()->getRepository() + "Textures/cubeIcon.png");
	application.maximizeMainWindow();

	watcher = new DirectoryWatcher(ResourceManager::getInstance()->getRepository() + "GUI");
	watcher->createNewFileWatcher("ModelImporter.gui");


	//	Collision test
	/*world.getEntityFactory().createObject("cube", [](Entity* object) // ground collider
		{
			object->setWorldTransformation(vec4f(0.f, 0.f, -10.f, 1.f), 1.f, quatf()); // vec4f(1000, 1000, 10, 1)
			object->removeComponent(object->getComponent<DrawableComponent>());
		});*/

	//	Scene
	Renderer::getInstance()->setGridVisible(true);
	//initializeSyntyScene();

	editorCamera = world.getEntityFactory().createObject([](Entity* object)
		{
			object->setName("EditorCamera");
			object->setWorldPosition(vec4f(-5, 3, -3, 1));

			Collider* collider = new Collider(new Sphere(vec4f(0.f), 0.01f));
			object->addComponent(collider);
			object->recomputeBoundingBox();

			CameraComponent* cam = new CameraComponent(true);
			object->addComponent(cam);
			cam->setDirection(vec4f(-1, 0, 0, 0));
			currentCamera = cam;
			world.setMainCamera(cam);
			Renderer::getInstance()->setCamera(cam);
		});
	world.getSceneManager().addToRootList(editorCamera);

	// init loop time tracking
	double averageTime = 0;
	long samples = 0;
	double elapseTime = 16.;
	double dummy = 0;

	//	game loop
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::GREEN) << "Game loop initiated";
	std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

	unsigned long frameCount = 0;
	while (!application.shouldExit())
	{
		// begin loop
		double startTime = glfwGetTime();

		std::ostringstream unicFrameName;
		unicFrameName << "Frame " << frameCount;
		FRAME_MARKER(unicFrameName.str().c_str());

#ifdef USE_IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		events();						// handle event before draw for text input and scrolling to work
		ImGui::NewFrame();
#else 
		events();
#endif

		updates((float)elapseTime);

		Debug::viewportRatio = context->getViewportRatio();
		Debug::view = currentCamera->getViewMatrix();
		Debug::projection = mat4f::perspective(currentCamera->getVerticalFieldOfView(), context->getViewportRatio(), 0.1f, 30000.f);  //far = 1500.f

		// Render scene & picking
		if (WidgetManager::getInstance()->getBoolean("wireframe"))
			Renderer::getInstance()->setRenderOption(Renderer::RenderOption::WIREFRAME);
		else Renderer::getInstance()->setRenderOption(Renderer::RenderOption::DEFAULT);
		Renderer::getInstance()->render(currentCamera);

		// gizmos and hud
#ifdef USE_IMGUI
		ImGuiMenuBar();
		ImGuiSystemDraw();
#endif
		picking();
		Renderer::getInstance()->renderHUD();

		//	clear garbages
		world.clearGarbage();
		ResourceManager::getInstance()->clearGarbage();

		completeTime = 1000.0 * (glfwGetTime() - startTime);
#ifdef USE_IMGUI
		{
			SCOPED_CPU_MARKER("ImGui rendering");

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
		}
#endif

		// End loop
		{
			SCOPED_CPU_MARKER("Swap buffers and clear");

			Renderer::getInstance()->swap();
			context->swapBuffers();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}
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
void initializeSyntyScene()
{
	Renderer::getInstance()->setEnvBackgroundColor(vec4f(0.f, 0.f, 0.f, 0.f));
	Renderer::getInstance()->setShader(Renderer::GRID, nullptr);//ResourceManager::getInstance()->getResource<Shader>("wired"));//

#if 0
	Entity* newObject = world.getEntityFactory().createEntity();
	newObject->setName("aaa");
	DrawableComponent* drawable = new DrawableComponent("PolygonDungeon/SM_Env_Grass_03.fbx", "defaultTextured");
	newObject->addComponent(drawable);
	newObject->recomputeBoundingBox();
	world.addToScene(newObject);
	return;
#endif


	const auto TryLoadAsVec4f = [](Variant& variant, vec4f& destination)
	{
		int sucessfullyParsed = 0;
		if (variant.getType() == Variant::ARRAY)
		{
			auto varray = variant.getArray();
			vec4f parsed = destination;
			for (int i = 0; i < 4 && i < varray.size(); i++)
			{
				auto& element = varray[i];
				if (element.getType() == Variant::FLOAT)
				{
					parsed[i] = element.toFloat();
					sucessfullyParsed++;
				}
				else if (element.getType() == Variant::DOUBLE)
				{
					parsed[i] = (float)element.toDouble();
					sucessfullyParsed++;
				}
				else if (element.getType() == Variant::INT)
				{
					parsed[i] = (float)element.toInt();
					sucessfullyParsed++;
				}
			}
			destination = parsed;
		}
		return sucessfullyParsed;
	};



	// load file and parse JSON
	std::string repository = ResourceManager::getInstance()->getRepository();
	std::string packageName = "PolygonDungeon";
	std::string sceneName = "Demo";
	std::string fullFileName = repository + "Scenes/" + packageName + "/" + sceneName + ".json";
	Variant v; Variant* tmp = nullptr;
	try
	{
		std::ifstream strm(fullFileName.c_str());
		if (!strm.is_open())
			throw std::invalid_argument("Reader::parseFile : Cannot opening file");

		Reader reader(&strm);
		reader.parse(v);
		tmp = &(v.getMap().begin()->second);
	}
	catch (std::exception&)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			std::cerr << "ERROR : loading scene : " << sceneName << " : fail to open or parse file" << std::endl;
		return;
	}
	Variant& sceneMap = *tmp;
	if (sceneMap.getType() != Variant::MAP)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			std::cerr << "ERROR : loading scene : " << sceneName << " : wrong file formating" << std::endl;
		return;
	}

	std::map<int, Entity*> sceneEntities;
	std::vector<Entity*> parentStack;
	try
	{
		if (sceneMap["sceneEntities"].getType() == Variant::ARRAY)
		{
			SCOPED_CPU_MARKER("initializeSyntyScene");

			// load all prefabs and instanciate
			for (auto it = sceneMap["sceneEntities"].getArray().begin(); it != sceneMap["sceneEntities"].getArray().end(); it++)
			{
				if (it->getType() == Variant::MAP)
				{

					// get id and name
					int id = (*it)["id"].toInt();
					std::string prefabName = "";
					auto prefabNameVariant = it->getMap().find("prefabName");
					if (prefabNameVariant != it->getMap().end() && prefabNameVariant->second.getType() == Variant::STRING)
						prefabName = prefabNameVariant->second.toString();

					// pop parent now in case of loading error
					Entity* newObject = nullptr;
					Entity* parent = nullptr;
					if (!parentStack.empty())
					{
						parent = parentStack.back();
						parentStack.pop_back();
					}

					// load a new object
					if (!prefabName.empty())
					{
						// load prefab
						bool contain = world.getEntityFactory().containPrefab(prefabName);
						bool loaded = world.getEntityFactory().loadPrefab(repository, packageName, prefabName);
						if (!contain && loaded)
						{
							//std::cout << ConsoleColor::getColorString(ConsoleColor::Color::GREEN) << prefabName << " loaded" << std::flush;
							//std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
						}
						else if (!loaded)
						{
							if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
								std::cerr << "ERROR : loading scene : " << sceneName << " : fail loading prefab : " << prefabName << std::endl;
							continue;
						}

						// instantiate
						newObject = world.getEntityFactory().instantiatePrefab(prefabName);
						newObject->setName((*it)["name"].toString());
					}
					else
					{
						// create and set transform
						newObject = world.getEntityFactory().createEntity();
						newObject->setName((*it)["name"].toString());
						world.getEntityFactory().tryLoadComponents(newObject, &(*it), packageName);
					}
					newObject->recomputeBoundingBox();

					// transform load
					vec4f position, tmp;
					TryLoadAsVec4f((*it)["position"], position);
					position.w = 1.f;
					TryLoadAsVec4f((*it)["rotation"], tmp);
					quatf rotation = quatf(tmp.w, tmp.x, tmp.y, tmp.z);
					rotation.normalize();

					float scale = 1.f;
					auto& scaleVariant = (*it)["scale"];
					if (scaleVariant.getType() == Variant::INT)
						scale = (float)scaleVariant.toInt();
					else if (scaleVariant.getType() == Variant::FLOAT)
						scale = scaleVariant.toFloat();
					else if (scaleVariant.getType() == Variant::DOUBLE)
						scale = (float)scaleVariant.toDouble();

					if (!prefabName.empty())
						scale *= newObject->getWorldScale();

					if (scale < 0.f)
					{
						std::cout << ConsoleColor::getColorString(ConsoleColor::Color::MAGENTA) << "WARNING : object " << newObject->getName() << " has a negative scaling" << std::flush;
						std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

						scale = -scale;
					}

					// set parent and local transform
					if (parent)
					{
						parent->addChild(newObject);
						newObject->setLocalTransformation(position, scale, rotation);
					}
					else
					{
						newObject->setLocalTransformation(position, scale, rotation);
						world.getSceneManager().addToRootList(newObject);
					}

					std::vector<Entity*> allObjectHierarchy;
					newObject->recursiveHierarchyCollect(allObjectHierarchy);
					for (Entity* e : allObjectHierarchy)
						world.addToScene(e);

					// push to stack if newObject is a parent
					auto childVariant = (*it).getMap().find("childs");
					if (childVariant != (*it).getMap().end() && childVariant->second.getType() == Variant::ARRAY)
					{
						auto childrenArray = childVariant->second.getArray();
						if (childrenArray.size() > 0)
						{
							for (auto it2 = childrenArray.begin(); it2 != childrenArray.end(); it2++)
								parentStack.push_back(newObject);
						}
					}
				}
			}
		}
	}
	catch (std::exception&)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			std::cerr << "ERROR : loading scene : " << sceneName << " : fail parsing file" << std::endl;
		return;
	}
}
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
	AnimationClip::setDefaultName("male_idle_breath");
	AnimationGraph::setDefaultName("humanoid");

	ResourceManager::getInstance()->addNewResourceLoader(".animation", new AnimationLoader());
	ResourceManager::getInstance()->addNewResourceLoader(".font", new FontLoader());
	ResourceManager::getInstance()->addNewResourceLoader("assimpMesh", new AssimpLoader(AssimpLoader::ResourceType::MESH));
	ResourceManager::getInstance()->addNewResourceLoader("assimpSkel", new AssimpLoader(AssimpLoader::ResourceType::SKELETON));
	ResourceManager::getInstance()->addNewResourceLoader(".mesh", new MeshLoader());
	ResourceManager::getInstance()->addNewResourceLoader(".shader", new ShaderLoader());
	ResourceManager::getInstance()->addNewResourceLoader(".skeleton", new SkeletonLoader());
	ResourceManager::getInstance()->addNewResourceLoader("image", new ImageLoader());
	ResourceManager::getInstance()->addNewResourceLoader(".texture", new TextureLoader());
	ResourceManager::getInstance()->addNewResourceLoader(".animGraph", new AnimationGraphLoader());

	// Init world
	const vec4f worldHalfSize = vec4f(GRID_SIZE * GRID_ELEMENT_SIZE, 128.f, GRID_SIZE * GRID_ELEMENT_SIZE, 0) * 0.5f;
	const vec4f worldPos = vec4f(0, 0.5f * worldHalfSize.y, 0, 1);
	NodeVirtual::debugWorld = &world;
	world.getSceneManager().init(worldPos - worldHalfSize, worldPos + worldHalfSize, vec3i(4, 1, 4), 2);
	world.setMaxObjectCount(400000);

	//	Renderer
	Renderer::getInstance()->setContext(context);
	Renderer::getInstance()->setWorld(&world);
	Renderer::getInstance()->setShader(Renderer::DEFAULT, ResourceManager::getInstance()->getResource<Shader>("default"));
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));
	Renderer::getInstance()->setShader(Renderer::INSTANCE_ANIMATABLE_BB, ResourceManager::getInstance()->getResource<Shader>("skeletonBB"));
	Renderer::getInstance()->normalViewer = ResourceManager::getInstance()->getResource<Shader>("normalViewer");
	Renderer::getInstance()->initializeGrid(GRID_SIZE, GRID_ELEMENT_SIZE, vec4f(24 / 255.f, 202 / 255.f, 230 / 255.f, 1.f));	// blue tron

	Renderer::getInstance()->initializeLightClusterBuffer(64, 36, 128);
	Renderer::getInstance()->initializeOcclusionBuffers(256, 144);
	Renderer::getInstance()->initializeShadows(1024, 1024, 1024, 1024);
	Renderer::getInstance()->initializeOverviewRenderer(512, 512);

	// Debug
	Debug::getInstance()->initialize("Shapes/box", "Shapes/sphere.obj", "Shapes/capsule", "default", "wired", "debug", "textureReinterpreter");

	//	HUD
	WidgetManager::getInstance()->setInitialViewportRatio(context->getViewportRatio());
	WidgetManager::getInstance()->loadHud("default");
	WidgetManager::getInstance()->loadHud("ModelImporter");
	WidgetManager::getInstance()->disableAllHUD();

	// Terrain
	terrain.initializeClipmaps();


	// default layout loading
#ifdef USE_IMGUI
	extern bool PhysicDebugWindowEnable;
	extern bool HierarchyWindowEnable;
	extern bool SpatialPartitionningWindowEnable;
	extern bool RenderingWindowEnable;
	extern bool ResourcesWindowEnable;

	PhysicDebugWindowEnable = false;
	HierarchyWindowEnable = true;
	SpatialPartitionningWindowEnable = false;
	RenderingWindowEnable = false;
	ResourcesWindowEnable = true;
#endif
}
void picking()
{
	SCOPED_CPU_MARKER("Picking");
	vec4f origin = currentCamera->getPosition();
	vec4f direction = currentCamera->getForward(); // no rotations

	RaySceneQuerry test(origin, direction, 10000);
	RayEntityCollector collector(origin, direction, 10000);
	world.getSceneManager().getEntities(&test, &collector);

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
		vec4f p = origin + distance * direction;

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
	SCOPED_CPU_MARKER("Events");
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
		{
			if (files[i].find_first_of("ModelImporter.gui") != std::string::npos)
			{
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CYAN) << "ModelImporter GUI hot reload" << std::flush;
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

				WidgetManager::getInstance()->deleteHud("ModelImporter");
				WidgetManager::getInstance()->loadHud("ModelImporter");
				//WidgetManager::getInstance()->setActiveHUD("ModelImporter");
			}
		}
	}
}
void updates(float elapseTime)
{
	SCOPED_CPU_MARKER("Updates");

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
		if (EventHandler::getInstance()->isActivated(CLICK_RIGHT))
		{
			float sensitivity = 0.2f;
			float yaw = (float)DEG2RAD * -sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x;
			float pitch = (float)DEG2RAD * -sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y;
			currentCamera->rotate(pitch, yaw);
		}

		// Translation
		vec4f direction(0., 0., 0., 0.);
		float speed = currentCamera->getVerticalFieldOfView();
		if (EventHandler::getInstance()->isActivated(CLICK_MIDDLE))
		{
			vec2f delta = EventHandler::getInstance()->getCursorPositionRelative();

			direction = currentCamera->getUp() * delta.y - currentCamera->getRight()* delta.x;
			direction *= editorCameraSensibility;
		}

		if (direction.x || direction.y || direction.z)
		{
			editorCamera->setWorldPosition(editorCamera->getWorldPosition() + direction * (elapseTime * speed));
			world.updateObject(editorCamera);
		}

		// FOV
		float angle = currentCamera->getVerticalFieldOfView() + (float)DEG2RAD * EventHandler::getInstance()->getScrollingRelative().y;
		if (angle > 1.5f) angle = 1.5f;
		else if (angle < 0.05f) angle = 0.05f;

		currentCamera->setVerticalFieldOfView(angle);
	}

}
void ImGuiMenuBar()
{
#ifdef USE_IMGUI
	extern bool PhysicDebugWindowEnable;
	extern bool HierarchyWindowEnable;
	extern bool SpatialPartitionningWindowEnable;
	extern bool RenderingWindowEnable;
	extern bool ResourcesWindowEnable;


	if (EventHandler::getInstance()->isActivated(ALT))
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Level"))
			{
				//ImGui::MenuItem("Physics", NULL, &PhysicDebugWindowEnable);
				ImGui::MenuItem("Rendering settings", NULL, &RenderingWindowEnable);
				ImGui::MenuItem("Resources", NULL, &ResourcesWindowEnable);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scene"))
			{
				ImGui::MenuItem("Hierarchy", NULL, &HierarchyWindowEnable);
				ImGui::MenuItem("Spatial partitionning", NULL, &SpatialPartitionningWindowEnable);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Creation tools"))
			{
				ImGui::MenuItem("Terrain", NULL, &TerrainCreatorWindowEnable);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
#endif
}
void ImGuiSystemDraw()
{
#ifdef USE_IMGUI
	SCOPED_CPU_MARKER("ImGui pass");

	extern bool PhysicDebugWindowEnable;
	extern bool HierarchyWindowEnable;
	extern bool SpatialPartitionningWindowEnable;
	extern bool RenderingWindowEnable;
	extern bool ResourcesWindowEnable;

	if (PhysicDebugWindowEnable)
	{
		world.getPhysics().drawImGui(world);
		world.getPhysics().debugDraw();
	}
	if (HierarchyWindowEnable)
		world.getSceneManager().drawImGuiHierarchy(world, false);
	if (SpatialPartitionningWindowEnable)
		world.getSceneManager().drawImGuiSpatialPartitioning(world);
	if (RenderingWindowEnable)
		Renderer::getInstance()->drawImGui(world);
	if (ResourcesWindowEnable)
		ResourceManager::getInstance()->drawImGui(world);

	// special creation tools
	if (TerrainCreatorWindowEnable)
	{
		static vec2i terrainSize = vec2i(16, 16);
		static int seed = 0;

		ImGui::Begin("Terrain Creator");
		
		ImGui::DragInt("Seed", &seed);
		ImGui::DragInt2("TerrainSize", &terrainSize.x, 1, 1, 4096, "%d", ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::Button("Generate"))
		{

		}

		ImGui::End();
	}

	// Editor camera parameters
	{
		SCOPED_CPU_MARKER("Editor camera");

		ImGui::Begin("Editor camera");
		ImGui::DragFloat("Move sensibility", &editorCameraSensibility, 0.0001f, 0.000001f, 1.f, "%.5f");
		constexpr float rangeMin = (float)RAD2DEG * 0.05f;
		constexpr float rangeMax = (float)RAD2DEG * 1.5f;
		float fov = (float)RAD2DEG * currentCamera->getVerticalFieldOfView();
		if (ImGui::SliderFloat("Vertical Fov", &fov, rangeMin, rangeMax, "%.3frad"))
			currentCamera->setVerticalFieldOfView((float)RAD2DEG * fov);


		Entity* selected = world.getSceneManager().getLastSelectedEntity();
		if (!selected)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button("Fit selected") && selected)
		{
			Sphere sphere = selected->getBoundingBox().toSphere(); 
			float tanA = tanf(0.5f * currentCamera->getVerticalFieldOfView());
			currentCamera->setPosition(sphere.center - ((1.5f * sphere.radius) / tanA) * currentCamera->getForward());
		}
		if (!selected)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::End();
	}

	// Default Entity inspector
	{
		SCOPED_CPU_MARKER("Inspector");

		ImGui::Begin("Inspector");

		Entity* selected = world.getSceneManager().getLastSelectedEntity();
		if (selected)
			selected->drawImGui(world, true);

		ImGui::End();
	}
#endif
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