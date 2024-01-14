// Golem Factory 2.0.cpp: define console application entry point
//




#include "Utiles/System.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Utiles/ProfilerConfig.h>

#include <iostream>
#include <list>
#include <time.h>
#include <sys/types.h>

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
#include "Events/EventEnum.h"

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

#include <EntityComponent/ComponentUpdater.h>

#include "Physics/Physics.h"
#include "Physics/RigidBody.h"
#include "Physics/Shapes/Collider.h"

#include "Resources/Loader/MeshSaver.h"
#include "Scene/RayEntityCollector.h"
#include "Scene/RaySceneQuerry.h"

#include "Utiles/Debug.h"
#include "Physics/GJK.h"
#include <Utiles/ConsoleColor.h>


#include <GameSpecific/TPSCameraComponent.h>
#include <GameSpecific/PlayerMovement.h>

#define GRID_SIZE 512
#define GRID_ELEMENT_SIZE 1.f
#define DEBUG_LEVEL 1



//	global attributes
RenderContext* context = nullptr;
World world;
Entity* avatar = nullptr;
Entity* freeflyCamera = nullptr;
Entity* tpsCamera = nullptr;
//Entity* frustrumCamera = nullptr;

float physicsTimeSpeed = 1.f;

double completeTime = 16.;
double averageCompleteTime = 16.;

float avatarZeroHeight;
vec4f avatarspeed(0.f);

CameraComponent* currentCamera = nullptr;
CameraComponent* debugFreeflyCam = nullptr;
struct {
	float radius = 2.f;
	vec4f target;
} cameraInfos;
//


// prototypes
void initializeForestScene(bool emptyPlace = false);
void initializePhysicsScene(int testCase = -1);
void initializeSyntyScene();
std::string checkResourcesDirectory();

void initManagers();
void picking();
void events();
void updates(float elapseTime);

void ImGuiMenuBar();
void ImGuiSystemDraw();
//


Entity* testEntity = nullptr;

// program
int main()
{
	std::cout << "Application start" << std::endl;
	Application application;
	context = application.createFullscreenWindow("Thibault test", 1600, 900);
	//context = application.createFullscreenWindow("Thibault test", glfwGetPrimaryMonitor(), 1600, 900);
	context->makeCurrent();
	context->setVSync(false);
	application.initGLEW(1);
	THREAD_MARKER("MainThread");
	initManagers();

	//AnimationGraph* animgraph = ResourceManager::getInstance()->getResource<AnimationGraph>("humanoid");

	//	Test scene
		EventHandler::getInstance()->setCursorMode(true);

		initializeSyntyScene();

		Renderer::getInstance()->setGridVisible(true);
		Entity* player = world.getSceneManager().searchEntity("CharacterTest999");
		
		if (player)
		{
			player->setFlags((uint64_t)Entity::Flags::Fl_Player);

			Capsule* capsule = new Capsule();
			capsule->radius = 0.4f;
			capsule->p1 = vec4f(0, capsule->radius, 0, 1);
			capsule->p2 = vec4f(0, 1.8f - capsule->radius, 0, 1);
			player->addComponent(new Collider(capsule));

			RigidBody* rb = new RigidBody(RigidBody::KINEMATICS);
			rb->setBouncyness(0.f);
			rb->setFriction(0.2f);
			rb->setDamping(0.001f);
			rb->setGravityFactor(1.f);
			rb->setAngularVelocity(vec4f::zero);
			player->addComponent(rb);
			world.getPhysics().addMovingEntity(player);
			rb->initialize(80.f);

			PlayerMovement* playerController = new PlayerMovement();
			player->addComponent(playerController);
		}

		freeflyCamera = world.getEntityFactory().createObject([](Entity* object)
			{
				object->setName("FreeFlyCam");
				object->setWorldPosition(vec4f(0, 7, -30, 1));

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
		Entity* freeflyCamera2 = world.getEntityFactory().createObject([](Entity* object)
			{
				object->setName("FreeFlyCam2");
				object->setWorldPosition(vec4f(0, 7, -30, 1));


				CameraComponent* cam = new CameraComponent(true);
				object->addComponent(cam);
				cam->setDirection(vec4f(-1, 0, 0, 0));
				//currentCamera = cam;
				debugFreeflyCam = cam;
			});

		/*tpsCamera = world.getEntityFactory().createObject([&](Entity* object)
			{
				object->setName("TPSCam");

				Collider* collider = new Collider(new Sphere(vec4f(0.f), 0.01f));
				object->addComponent(collider);
				object->recomputeBoundingBox();

				TPSCameraComponent* cam = new TPSCameraComponent();
				object->addComponent(cam);
				Renderer::getInstance()->setCamera(cam);
				cam->setDirection(vec4f(-1, 0, 0, 0));
				cam->setTargetCharacter(player);
				playerController->setCamera(cam);
				object->setWorldPosition(vec4f(0, 7, -30, 1));

				//cam->setOrientation(currentCamera->getOrientation());
				//cam->setPosition(currentCamera->getPosition());
				world.setMainCamera(cam);
				currentCamera = cam;
			});*/

		world.getSceneManager().addToRootList(freeflyCamera);
		world.getSceneManager().addToRootList(freeflyCamera2);

		WidgetManager::getInstance()->setBoolean("BBpicking", false);
		WidgetManager::getInstance()->setBoolean("wireframe", false);

		

	// init loop time tracking
	double averageTime = 0;
	long samples = 0;
	double elapseTime = 16.;
	double dummy = 0;


	//WidgetManager::getInstance()->setBoolean("syncCamera", false);


	//	game loop
	unsigned long frameCount = 0;
	std::cout << "game loop initiated" << std::endl;
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
		Debug::view = debugFreeflyCam->getViewMatrix();
		Debug::projection = mat4f::perspective(debugFreeflyCam->getVerticalFieldOfView(), context->getViewportRatio(), 0.1f, 30000.f);  //far = 1500.f

		//	physics
		world.getPhysics().stepSimulation(physicsTimeSpeed * 0.016f, &world.getSceneManager());

		// Render scene & picking
		/*if (WidgetManager::getInstance()->getBoolean("BBrendering"))
			Renderer::getInstance()->setRenderOption(Renderer::RenderOption::BOUNDING_BOX);
		else if (WidgetManager::getInstance()->getBoolean("wireframe"))
			Renderer::getInstance()->setRenderOption(Renderer::RenderOption::WIREFRAME);
		else Renderer::getInstance()->setRenderOption(Renderer::RenderOption::DEFAULT);*/
		Renderer::getInstance()->render(debugFreeflyCam);
		 
		//Renderer::getInstance()->drawMap(world.getMapPtr(), normalShader);
		
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

		// End loop
		completeTime = 1000.0*(glfwGetTime() - startTime);

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
		{
			SCOPED_CPU_MARKER("Swap buffers and clear");

			Renderer::getInstance()->swap();
			context->swapBuffers();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}


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
	Renderer::getInstance()->setEnvBackgroundColor(vec4f(0.6f, 0.85f, 0.91f, 0.f));
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("greenGrass"));

	// init instance placement
	std::string objectType;
	float sDispersion;
	float sOffset;
	HouseGenerator hg;
	srand(3);

	//	center tree in place
	if (!emptyPlace)
	{
		unsigned int N = 7;
		unsigned int N2 = 2;
		const float pi2 = 2.f * (float)PI;
		for (unsigned int j = 0; j < N2; j++)
		{
			for (unsigned int i = 0; i < N; i++)
			{
				vec4f p = vec4f(5.f * cos((float)i / N * pi2), 5.f * sin((float)i / N * pi2), 2.f*(j + 1), 1);
				p += N2 * 0.5f * p.getNormal();
				world.getEntityFactory().createObject("sphere", [&p, &j](Entity* object)
					{
						object->setName("PhysicSphere");
						quatf q = quatf::identity;
						object->setWorldTransformation(p, 1.f, q);
						RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
						rb->setMass(1.f);
						rb->setGravityFactor(1.f);
						object->addComponent(rb);
					});
			}
		}
		return;
	}

	// village
	int vilageHouseCount = 0;
	float villageRadius[] = {20.f, 35.f, 50.f};
	std::vector<vec4f> houseCircle;
	for (int i = 0; i < 3; i++)
	{
		int houseCount = 5 + i * 7;
		vilageHouseCount += houseCount;
		float angleOffset = 2 * (float)PI * ((rand() % 100) / 100.f);

		for (int j = 0; j < houseCount; j++)
		{
			float radius = villageRadius[i] + 3.f * (((rand() % 100) / 50.f) - 1.f);
			float angle = angleOffset + 6.28f * j / houseCount + ((((rand() % 100) / 50.f) - 1.f)) / houseCount;
			
			mat4f a = mat4f::identity;
			if(rand() % 2)
				a = mat4f::rotate(a, quatf(3.14f + angle + 0.4f * ((((rand() % 100) / 50.f) - 1.f)), vec3f(0, 0, 1)));
			else
				a = mat4f::rotate(a, quatf(angle + 0.4f * ((((rand() % 100) / 50.f) - 1.f)), vec3f(0, 0, 1)));
			
			world.getEntityFactory().createObject([radius, angle, &houseCircle, &hg](Entity* house)
				{
					int seed = rand();
					hg.getHouse(house, seed, 20, 30);
					house->setName("House_" + std::to_string(seed));
					vec4f p = vec4f(radius * cos(angle), radius * sin(angle), house->getBoundingBox().toSphere().radius, 1.f);
					for (unsigned int k = 0; k < houseCircle.size(); k++)
					{
						float delta = vec4f(houseCircle[k].x - p.x, houseCircle[k].y - p.y, 0.f, 0.f).getNorm();
						if (delta < houseCircle[k].z + p.z - 0.5f)
						{
							p += (1.f + houseCircle[k].z + p.z - delta) * vec4f(p.x - houseCircle[k].x, p.y - houseCircle[k].y, 0.f, 0.f).getNormal();
							k = 0;
						}
					}

					houseCircle.push_back(p);
					house->setWorldPosition(vec4f(p.x, p.y, 0, 1));
					house->setWorldOrientation(quatf(1.57f + angle, vec3f(0, 0, 1)));
				});
		}
	}


	// forest
	for (int i = 0; i < GRID_SIZE; i++)
		for (int j = 0; j < GRID_SIZE; j++)
		{
			vec4f p(GRID_ELEMENT_SIZE*i - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + GRID_ELEMENT_SIZE * ((rand() % 10) / 20.f - 0.25f),
				GRID_ELEMENT_SIZE*j - (GRID_SIZE * GRID_ELEMENT_SIZE) / 2 + GRID_ELEMENT_SIZE * ((rand() % 10) / 20.f - 0.25f),
				0, 1);

			if(p.getNorm() < villageRadius[2] + 5.f * GRID_ELEMENT_SIZE)
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
			quatf a = quatf((float)DEG2RAD *(rand() % 3600) * 0.1f, vec3f(0, 0, 1));

			world.getEntityFactory().createObject(objectType, p, s, a, "Firtree");
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
	Renderer::getInstance()->setEnvBackgroundColor(vec4f(0.6f, 0.85f, 0.91f, 0.f));
	Renderer::getInstance()->setShader(Renderer::GRID, ResourceManager::getInstance()->getResource<Shader>("wired"));

	if (testCase == 0)
	{
		unsigned int N = 40;
		for (unsigned int k = 0; k < 1; k++)
			for (unsigned int i = 0; i < N; i++)
				for (unsigned int j = 0; j < N; j++)
				{
					float r =  0.01f *(rand() % 100) + 0.5f;
					vec4f p = vec4f(10.f * (i - N / 2.f), 10.f * (j - N / 2.f), 3.2f * k + 2.f, 1);
					world.getEntityFactory().createObject("sphere", [&p, &r](Entity* object)
						{
							object->setName("PhysicsSphere");
							quatf q = quatf(1.f, 0.f, 0.5f * ((rand() % 200) - 100), 0.5f * ((rand() % 200) - 100));
							q.normalize();
							object->setWorldTransformation(p, r, q);
							RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
							rb->setMass(r * r * r);
							rb->setBouncyness(0.05f);
							rb->setFriction(0.2f);
							rb->setDamping(0.001f);
							rb->setGravityFactor(1.f);
							rb->setLinearVelocity(vec4f(0.1f *((rand() % 200) - 100), 0.1f *((rand() % 200) - 100), 0, 0));
							rb->setAngularVelocity(vec4f(0.1f * ((rand() % 200) - 100), 0.1f * ((rand() % 200) - 100), 0.1f * ((rand() % 200) - 100), 0));
							object->addComponent(rb);
						});
				}
	}
	else if (testCase == 1)
	{
		float r = 1.f;
		vec4f p = vec4f(0, 0, r + 5.5f, 1);

		world.getEntityFactory().createObject("cube", [&p, &r](Entity* object)
			{
				object->setName("PhysicsCube 1");
				quatf q = quatf(1.f, 0.f, 0.5f * ((rand() % 200) - 100), 0.5f * ((rand() % 200) - 100));
				q.normalize();
				object->setWorldTransformation(p, r, q);
				object->setWorldTransformation(p, r, quatf::identity);
				RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
				rb->setMass(5.f);
				rb->setBouncyness(0.f);
				rb->setFriction(0.2f);
				rb->setDamping(0.001f);
				rb->setGravityFactor(1.f);
				object->addComponent(rb);
			});

		p = vec4f(0, 0, r + 2.f, 1);
		world.getEntityFactory().createObject("cube", [&p, &r](Entity* object)
			{
				object->setName("PhysicsCube 2");
				object->setWorldTransformation(p, r, quatf::identity);
				RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
				rb->setMass(5.f);
				rb->setBouncyness(0.1f);
				rb->setFriction(1.f);
				rb->setGravityFactor(1);
				object->addComponent(rb);
			});

		p = vec4f(0, 3, r + 2.f, 1);
		world.getEntityFactory().createObject("sphere", [&p, &r](Entity* object)
			{
				object->setName("PhysicsSphere");
				object->setWorldTransformation(p, r, quatf::identity);
				RigidBody* rb = new RigidBody(RigidBody::DYNAMIC);
				rb->setMass(5.f);
				rb->setBouncyness(0.f);
				rb->setFriction(0.2f);
				rb->setDamping(0.001f);
				rb->setGravityFactor(1.f);
				rb->setAngularVelocity(vec4f(-10.f, 0.f, 0.f, 0.f));
				object->addComponent(rb);
			});
	}
}
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
						else if(!loaded)
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

	//	check for work repository
	directory = "C:/Users/Thibault-SED/Documents/Github/GolemFactory/Resources/";
	if (ToolBox::isPathExist(directory)) return directory;
	
	//	return the default resource path for portable applications
	std::cout << "FATAL WARRNING : Fail to find ressource repo" << std::endl;
	return "Resources/";
}


void initManagers()
{
	SCOPED_CPU_MARKER("initManagers");

	std::string resourceRepository = checkResourcesDirectory();
	if (DEBUG_LEVEL) std::cout << "Found resources folder at : " << resourceRepository << std::endl;

	// Init Event handler
	EventHandler::getInstance()->addWindow(context->getParentWindow());
	EventHandler::getInstance()->setRepository(resourceRepository);
	EventHandler::getInstance()->loadKeyMapping("RPG Key mapping", "");
	EventHandler::getInstance()->setCursorMode(false);
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

	world.getTerrainVirtualTexture().initialize(2048);
	
	if(false)
	{
		Entity* groundAndWalls = world.getEntityFactory().createObject([](Entity* object)
			{
				object->setName("GroundAndWalls");
				object->setWorldPosition(vec4f(0.f, 0.f, -10.f, 1));
			});
		world.getSceneManager().addToRootList(groundAndWalls);

		world.getEntityFactory().createObject([&](Entity* object)
			{
				object->setName("Ground");
				vec4f s = vec4f(0.5f*GRID_SIZE*GRID_ELEMENT_SIZE, 10.f, 0.5f*GRID_SIZE*GRID_ELEMENT_SIZE, 0.f);

				Collider* collider = new Collider(new AxisAlignedBox(-s, s));
				object->addComponent(collider);
				object->recomputeBoundingBox();
				groundAndWalls->addChild(object);
				object->setLocalPosition(vec4f(0.f, -s.y, 0.f, 1));
			});
		world.getEntityFactory().createObject("cube", [&](Entity* object)
			{
				object->setName("WallX");
				groundAndWalls->addChild(object);
				object->setLocalTransformation(
					vec4f(0.5f * GRID_SIZE * GRID_ELEMENT_SIZE, 0.f, 3.f, 1), 
					1.f,//vec4f(3.f, 0.5f * GRID_SIZE * GRID_ELEMENT_SIZE + 3.f, 6.f, 1.f),
					quatf::identity);
			});
		world.getEntityFactory().createObject("cube", [&](Entity* object)
			{
				object->setName("WallX");
				groundAndWalls->addChild(object);
				object->setLocalTransformation(
					vec4f(-0.5f * GRID_SIZE * GRID_ELEMENT_SIZE, 0.f, 3.f, 1),
					1.f, //vec4f(3.f, 0.5f * GRID_SIZE * GRID_ELEMENT_SIZE + 3.f, 6.f, 1.f),
					quatf::identity);
			});
		world.getEntityFactory().createObject("cube", [&](Entity* object)
			{
				object->setName("WallY");
				groundAndWalls->addChild(object);
				object->setLocalTransformation(
					vec4f(0.f, 3.f, 0.5f * GRID_SIZE * GRID_ELEMENT_SIZE, 1),
					1.f, //vec4f(0.5f * GRID_SIZE * GRID_ELEMENT_SIZE + 3.f, 3.f, 6.f, 1.f),
					quatf::identity);
			});
		world.getEntityFactory().createObject("cube", [&](Entity* object)
			{
				object->setName("WallY");
				groundAndWalls->addChild(object);
				object->setLocalTransformation(
					vec4f(0.f, 3.f,-0.5f * GRID_SIZE * GRID_ELEMENT_SIZE,  1),
					1.f, //vec4f(0.5f * GRID_SIZE * GRID_ELEMENT_SIZE + 3.f, 3.f, 6.f, 1.f),
					quatf::identity);
			});
	}

	//world.getMap().setShader(ResourceManager::getInstance()->getResource<Shader>("map"));
	//world.getMap().loadFromHeightmap(resourceRepository + "Textures/", "mountains512.png"); /// >> CREATE BUGS WITH TRANSPARENT ?

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

	// Animator
	//Animator::getInstance();

	//	HUD
	WidgetManager::getInstance()->setInitialViewportRatio(context->getViewportRatio());
	WidgetManager::getInstance()->loadHud("default");
}
void picking()
{
	SCOPED_CPU_MARKER("Picking");

	vec4f cameraPos = debugFreeflyCam->getPosition();
	vec4f direction = debugFreeflyCam->getForward(); // no rotations

	if (EventHandler::getInstance()->getCursorMode())
	{
		vec2f cursor = EventHandler::getInstance()->getCursorPositionAbsolute();
		vec2i vpsize = context->getViewportSize();
		vec4f up = debugFreeflyCam->getUp();
		vec4f right = debugFreeflyCam->getRight();
		float tanfov = tan(0.5f * debugFreeflyCam->getVerticalFieldOfView());
		float ratio = (float)vpsize.x / vpsize.y;
		float nearPlane = 0.1f;

		direction = nearPlane * debugFreeflyCam->getForward() + (nearPlane * tanfov * ratio * ( 2.f * cursor.x / vpsize.x - 1.f)) * right + (nearPlane * tanfov * (1.f - 2.f * cursor.y / vpsize.y)) * up;
		direction.normalize();

		//std::cout << c.x << ' ' << c.y << std::endl;
	}

	RaySceneQuerry test(cameraPos, direction, 10000);
	RayEntityCollector collector(cameraPos, direction, 10000);
	world.getSceneManager().getEntities(&test, &collector);

	//std::sort(collector.getSortedResult().begin(), collector.getSortedResult().end(), [](std::pair<float, unsigned int> a, std::pair<float, unsigned int> b) { return a.first > b.first; });

	if (!collector.getSortedResult().empty())
	{
		std::sort(collector.getSortedResult().begin(), collector.getSortedResult().end(), [](std::pair<float, unsigned int> a, std::pair<float, unsigned int> b) { return a.first < b.first; });
		float distance = collector.getSortedResult().front().first;
		Entity* entity = collector.getResult()[collector.getSortedResult().front().second];

		std::string type;
		AnimationComponent* compAnim = entity->getComponent<AnimationComponent>();
		DrawableComponent* compDraw = entity->getComponent<DrawableComponent>();
		if(compAnim)       type = "animatable";
		else if(compDraw)  type = "drawable";
		else               type = "empty entity";
		vec4f p = cameraPos + distance * direction;

		Debug::color = Debug::red;
		Debug::drawSphere(p, 0.03f);
		
		WidgetManager::getInstance()->setString("interaction", "Distance : " + ToolBox::to_string_with_precision(distance, 5) +
			" m\nPosition : (" + ToolBox::to_string_with_precision(p.x, 5) + " , " + ToolBox::to_string_with_precision(p.y, 5) + " , " + ToolBox::to_string_with_precision(p.z, 5) +
			")\nInstance on ray : " + std::to_string(collector.getResult().size()) +
			"\nFirst entity : " + entity->getName() +
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

	bool isTrackBall = avatar != nullptr && currentCamera->getParentEntity() == avatar;

	for (unsigned int i = 0; i < v.size(); i++)
	{
		//	debug
		/*if (v[i] == SLOT8) GJK::max_iteration++;
		else if (v[i] == SLOT9) GJK::max_iteration--;

		//	micselenious
		else */
		if (v[i] == QUIT) glfwSetWindowShouldClose(context->getParentWindow(), GL_TRUE);
		else if (v[i] == CHANGE_CURSOR_MODE)
		{
			EventHandler::getInstance()->setCursorMode(!EventHandler::getInstance()->getCursorMode());
		}
		/*else if (v[i] == ACTION && avatar != nullptr)
		{
			if (isTrackBall)
			{
				CameraComponent* tbCam = avatar->getComponent<CameraComponent>();
				CameraComponent* ffCam = freeflyCamera->getComponent<CameraComponent>();
				freeflyCamera->setWorldPosition(tbCam->getPosition());
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
		}*/

		else if (v[i] == PAUSE)
		{
			if (physicsTimeSpeed != 0.f)
				physicsTimeSpeed = 0.f;
			else
				physicsTimeSpeed = 1.f;
		}

		//	avatar related
		/*else if (v[i] == SLOT1 && avatar) Animator::getInstance()->launchAnimation(avatar, "hello");
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
		}*/

		//	debug action
		else if (v[i] == HELP) WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "help" ? "" : "help"));
		else if (v[i] == F9)   WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "debug" ? "" : "debug"));
		else if (v[i] == F10)  WidgetManager::getInstance()->setActiveHUD((WidgetManager::getInstance()->getActiveHUD() == "rendering" ? "" : "rendering"));

		else if (v[i] == F3)   WidgetManager::getInstance()->setBoolean("syncCamera", !WidgetManager::getInstance()->getBoolean("syncCamera"));


		else if (v[i] == F4)   WidgetManager::getInstance()->setBoolean("wireframe", !WidgetManager::getInstance()->getBoolean("wireframe"));
	}
}
void updates(float elapseTime)
{
	SCOPED_CPU_MARKER("Updates");

	//	animate avatar
	//if(avatar)
	//	Animator::getInstance()->animate(avatar, elapseTime);

	//	Compute HUD picking parameters
	if (EventHandler::getInstance()->getCursorMode())
	{
		vec2f cursor = EventHandler::getInstance()->getCursorNormalizedPosition();
		mat4f projection = mat4f::perspective((float)DEG2RAD * ANGLE_VERTICAL_HUD_PROJECTION, context->getViewportRatio(), 0.1f, 1500.f);
		vec4f ray_eye = mat4f::inverse(projection) * vec4f(cursor.x, cursor.y, -1.f, 1.f);
		ray_eye.normalize();
		WidgetManager::getInstance()->setPickingParameters(
			mat4f::translate(mat4f::identity, vec4f(0.f, 0.f, -DISTANCE_HUD_CAMERA, 1.f)) * mat4f::eulerAngleZX((float)PI, (float)PI*0.5f),
			ray_eye);// ,
			//currentCamera->getGlobalPosition());
	}
	else
	{
		WidgetManager::getInstance()->setPickingParameters(mat4f::identity, vec4f::zero);// , currentCamera->getGlobalPosition());
	}

	//	Update widgets
	int GPUdt = (int)Renderer::getInstance()->getElapsedTime();
	int GPUavg = (int)Renderer::getInstance()->getAvgElapsedTime();
	averageCompleteTime = 0.95f * averageCompleteTime + 0.05f * completeTime;
	WidgetManager::getInstance()->setString("runtime speed",
		"CPU: " + std::to_string((int)(averageCompleteTime)) + "ms (" + std::to_string((int)(completeTime)) +
		")\nGPU: " + std::to_string(GPUavg) + "ms (" + std::to_string(GPUdt) + ")\n\n\n");
	WidgetManager::getInstance()->setString("drawcalls",
		"Instances :\n  " + std::to_string(Renderer::getInstance()->getNbDrawnInstances() + WidgetManager::getInstance()->getNbDrawnWidgets()) +
		"\nDrawCalls :\n  " + std::to_string(Renderer::getInstance()->getNbDrawCalls()) +
		"\nTriangles :\n  " + std::to_string(Renderer::getInstance()->getNbDrawnTriangles() + WidgetManager::getInstance()->getNbDrawnTriangles()));
	WidgetManager::getInstance()->update((float)elapseTime, EventHandler::getInstance()->isActivated(USE1));

	//	Move avatar if needed
	/*if (avatar && currentCamera->getParentEntity() == avatar)
	{
		//	compute direction and speed
		vec4f direction = vec4f(0.f);
		if (EventHandler::getInstance()->isActivated(FORWARD))
			direction += vec4f(currentCamera->getForward().x, currentCamera->getForward().y, 0, 0).getNormal();
		else if (EventHandler::getInstance()->isActivated(BACKWARD))
			direction -= vec4f(currentCamera->getForward().x, currentCamera->getForward().y, 0, 0).getNormal();
		if (EventHandler::getInstance()->isActivated(LEFT))
			direction -= vec4f(currentCamera->getRight().x, currentCamera->getRight().y, 0, 0).getNormal();
		else if (EventHandler::getInstance()->isActivated(RIGHT))
			direction += vec4f(currentCamera->getRight().x, currentCamera->getRight().y, 0, 0).getNormal();
		if(direction.x != 0.f && direction.y != 0.f)
			direction.normalize();

		float speed = 0.f;

		avatar->setWorldPosition(avatar->getWorldPosition() + speed * direction);
		if (direction.x != 0.f && direction.y != 0.f)
			avatar->setWorldOrientation(quatf(atan2(direction.y, direction.x), vec3f(0.f, 0.f, 1.f)));
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
				float yaw = -(float)DEG2RAD * sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x;
				float pitch = -(float)DEG2RAD * sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y;
				cameraInfos.radius = cameraInfos.radius - sensitivity * EventHandler::getInstance()->getScrollingRelative().y;
				cameraInfos.radius = clamp(cameraInfos.radius, 0.5f, 10.f);

				CameraComponent* tbCam = avatar->getComponent<CameraComponent>();
				tbCam->rotateAround(cameraInfos.target, pitch, yaw, cameraInfos.radius / avatar->getWorldScale());
			}
		}
	}
	else*/
	{
		CameraComponent* ffCam = freeflyCamera->getComponent<CameraComponent>();
		
		// Rotation
		if (!EventHandler::getInstance()->getCursorMode() && WidgetManager::getInstance()->getBoolean("syncCamera"))
		{
			float sensitivity = 0.2f;
			float yaw = -(float)DEG2RAD * sensitivity * EventHandler::getInstance()->getCursorPositionRelative().x;
			float pitch = -(float)DEG2RAD * sensitivity * EventHandler::getInstance()->getCursorPositionRelative().y;
			ffCam->rotate(pitch, yaw);

			// FOV
			float angle = ffCam->getVerticalFieldOfView() + (float)DEG2RAD * EventHandler::getInstance()->getScrollingRelative().y;
			if (angle > 1.5f) angle = 1.5f;
			else if (angle < 0.05f) angle = 0.05f;

			ffCam->setVerticalFieldOfView(angle);
		}

		// Translation
		vec4f direction(0., 0., 0., 0.);
		vec4f forward = ffCam->getForward();
		vec4f right = ffCam->getRight();

		float speed = 0.003f;
		if (EventHandler::getInstance()->isActivated(FORWARD)) direction += forward;
		if (EventHandler::getInstance()->isActivated(BACKWARD)) direction -= forward;
		if (EventHandler::getInstance()->isActivated(LEFT)) direction -= right;
		if (EventHandler::getInstance()->isActivated(RIGHT)) direction += right;
		if (EventHandler::getInstance()->isActivated(SNEAKY)) speed /= 10.f;
		if (EventHandler::getInstance()->isActivated(RUN)) speed *= 10.f;

		if (direction.x || direction.y || direction.z)
		{
			direction.normalize();
			freeflyCamera->setWorldPosition(freeflyCamera->getWorldPosition() + direction*elapseTime*speed);
			world.updateObject(freeflyCamera);
		}
	}

	if (WidgetManager::getInstance()->getBoolean("syncCamera"))
	{
		//CameraComponent* cam = frustrumCamera->getComponent<CameraComponent>();
		debugFreeflyCam->setOrientation(currentCamera->getOrientation());
		debugFreeflyCam->setVerticalFieldOfView(currentCamera->getVerticalFieldOfView());
		debugFreeflyCam->getParentEntity()->setWorldPosition(currentCamera->getPosition());
		world.updateObject(debugFreeflyCam->getParentEntity());
	}

	if (physicsTimeSpeed != 0.f)
	{
		float dt = 0.001f * elapseTime * physicsTimeSpeed;
		ComponentUpdater::getInstance()->update(dt);

		/*float dt = 0.001f * elapseTime;
		extern std::vector<AnimationComponent*> g_allAnimations;
		for (AnimationComponent* comp : g_allAnimations)
		{
			if (comp && comp->isValid())
				comp->update(dt);
		}
		extern std::vector<Animator*> g_allAnimator;
		for (Animator* comp : g_allAnimator)
		{
			if (comp && comp->isValid())
				comp->update(dt);
		}*/
	}


	// Map streaming
	//world.getMapPtr()->update(freeflyCamera->getWorldPosition());
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
				ImGui::MenuItem("Physics", NULL, &PhysicDebugWindowEnable);
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
		world.getSceneManager().drawImGuiHierarchy(world, true);
	if (SpatialPartitionningWindowEnable)
		world.getSceneManager().drawImGuiSpatialPartitioning(world);
	if (RenderingWindowEnable)
		Renderer::getInstance()->drawImGui(world); 
	if (ResourcesWindowEnable)
		ResourceManager::getInstance()->drawImGui(world);
#endif
}
//
