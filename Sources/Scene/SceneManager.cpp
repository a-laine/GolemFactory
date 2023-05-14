#include <iostream>
#include <algorithm>
//#include <glm/gtc/matrix_access.hpp>

#include "SceneManager.h"
#include <Utiles/Assert.hpp>
#include <Utiles/Debug.h>
#include <Utiles/ImguiConfig.h>
#include <Scene/FrustrumSceneQuerry.h>


#ifdef USE_IMGUI
bool HierarchyWindowEnable = true;
bool SpatialPartitionningWindowEnable = false;
#endif // USE_IMGUI


SceneManager::SceneManager()
{}
SceneManager::SceneManager(SceneManager&& other) : world(std::move(other.world)), instanceTracking(std::move(other.instanceTracking))
{
	other.world.clear();
	other.instanceTracking.clear();
}
SceneManager::~SceneManager()
{
	for (unsigned int i = 0; i < world.size(); i++)
		delete world[i];
}


SceneManager& SceneManager::operator=(SceneManager&& other)
{
	if (&other == this) return *this;
	for(unsigned int i = 0; i < world.size(); i++)
		delete world[i];
	world = std::move(other.world);
	instanceTracking = std::move(other.instanceTracking);
	other.world.clear();
	other.instanceTracking.clear();
	return *this;
}


void SceneManager::init(const vec4f& bbMin, const vec4f& bbMax, const vec3i& nodeDivision, unsigned int depth)
{
	GF_ASSERT(world.empty());
	NodeVirtual* n = new NodeVirtual();
	n->init(bbMin, bbMax, nodeDivision, depth);
	world.push_back(n);
}
void SceneManager::clear()
{
	for(unsigned int i = 0; i < world.size(); i++)
		delete world[i];
	world.clear();
	instanceTracking.clear();
}
void SceneManager::reserveInstanceTrack(const unsigned int& count) { instanceTracking.reserve(count); }
unsigned int SceneManager::getObjectCount() const { return (unsigned int)instanceTracking.size(); }


bool SceneManager::addObject(Entity* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) != 0)
		return false;

	const AxisAlignedBox& box = object->m_worldBoundingBox;
	const vec4f objSize = 0.5f * (box.max - box.min);
	const vec4f objPos = 0.5f * (box.max + box.min);
	NodeVirtual* node = world[0];
	NodeVirtual* pNode = node;
	{
		vec4f p = objPos - world[0]->getCenter();
		vec4f s = 0.5f * world[0]->getSize() - vec4f(EPSILON);
		if (!(std::abs(p.x) < s.x && std::abs(p.y) < s.y && std::abs(p.z) < s.z))
			return false;
	}

	while (!node->isLeaf() && !node->isTooBig(objSize))
	{
		pNode = node;
		node = node->getChildAt(objPos);
	}

	NodeVirtual* finalNode;
	if (node->isTooBig(objSize))
		finalNode = pNode;
	else
		finalNode = node;

	finalNode->addObject(object);
	instanceTracking[object] = { objPos, finalNode };

	return true;
}

bool SceneManager::removeObject(Entity* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;
	instanceTracking.erase(track);
	
	return node->removeObject(object);
}

bool SceneManager::updateObject(Entity* object)
{
	GF_ASSERT(object);
	if (world.empty() || instanceTracking.count(object) == 0)
		return false;

	const AxisAlignedBox& box = object->m_worldBoundingBox;
	const vec4f objSize = 0.5f * (box.max - box.min);
	const vec4f objPos = 0.5f * (box.max + box.min);
	auto track = instanceTracking.find(object);
	NodeVirtual* node = track->second.owner;

	if (!node->isInside(objPos) || node->isTooBig(objSize) || node->isTooSmall(objSize))
	{
		node->removeObject(object);

		if (!world[0]->isInside(objPos))
		{
			instanceTracking.erase(track);
			return false;
		}
		else
		{
			node = world[0];
			NodeVirtual* pNode = node;
			while (!node->isLeaf() && !node->isTooBig(objSize))
			{
				pNode = node;
				node = node->getChildAt(objPos);
			}

			NodeVirtual* finalNode;
			if (node->isTooBig(objSize))
				finalNode = pNode;
			else
				finalNode = node;

			finalNode->addObject(object);
			track->second.owner = finalNode;
		}
	}
	track->second.position = objPos;
	return true;
}

void SceneManager::addToRootList(Entity* object)
{
	roots.push_back(object);
}


std::vector<Entity*> SceneManager::getAllObjects()
{
	VirtualSceneQuerry getAllTest;
	VirtualEntityCollector result;

	return std::vector<Entity*>(result.getResult());
}

std::vector<Entity*> SceneManager::getObjectsOnRay(const vec4f& position, const vec4f& direction, float maxDistance)
{
	/*if(world.empty())
		return;

	RaySceneQuerry test(position, direction, maxDistance);
	getObjects(world[0], result, test);*/
	return std::vector<Entity*>();
}

std::vector<Entity*> SceneManager::getObjectsInBox(const vec4f& bbMin, const vec4f& bbMax)
{
	/*if(world.empty())
		return;

	BoxSceneQuerry test(bbMin, bbMax);
	getObjectsInBox(world[0], result, test);*/
	return std::vector<Entity*>();
}


void SceneManager::getSceneNodes(VirtualSceneQuerry* collisionTest)
{
	//	initialize and test root
	NodeVirtual* node = world[0];
	VirtualSceneQuerry::CollisionType collision = (VirtualSceneQuerry::CollisionType)(*collisionTest)(node);
	if (collision == VirtualSceneQuerry::CollisionType::NONE)
		return;

	//	init path and iterate on tree
	std::vector<NodeVirtual*> path;
	std::vector<bool> parentIsInsideStack;

	if (!node->isLeaf())
		node->getChildren(path);
	for (int i = 0; i < node->getChildrenCount(); i++)
		parentIsInsideStack.push_back(collision == VirtualSceneQuerry::CollisionType::INSIDE);

	while (!path.empty())
	{
		// process node
		node = path.back();
		bool parentIsInside = parentIsInsideStack.back();
		path.pop_back();
		parentIsInsideStack.pop_back();

		// perform collision if needed
		if (parentIsInside)
		{
			collision = VirtualSceneQuerry::CollisionType::INSIDE;
			collisionTest->addNodeToResult(node);
		}
		else
			collision = (VirtualSceneQuerry::CollisionType)(*collisionTest)(node);

		//	iterate
		if (!node->isLeaf() && collision != VirtualSceneQuerry::CollisionType::NONE)
		{
			node->getChildren(path);
			for (int i = 0; i < node->getChildrenCount(); i++)
				parentIsInsideStack.push_back(collision == VirtualSceneQuerry::CollisionType::INSIDE);
		}
	}
}
void SceneManager::getEntities(VirtualSceneQuerry* collisionTest, VirtualEntityCollector* entityCollector)
{
	getSceneNodes(collisionTest);
	std::vector<const NodeVirtual*>& nodeList = collisionTest->getResult();

	for (unsigned int i = 0; i < nodeList.size(); i++)
	{
		const std::vector<Entity*>& entities = nodeList[i]->getEntitiesList();

		for (unsigned int j = 0; j < entities.size(); j++)
		{
			(*entityCollector)(entities[j]);
		}
	}
}






void SceneManager::drawSceneNodes()
{
	Debug::color = Debug::black;
	VirtualSceneQuerry getAllTest;
	getSceneNodes(&getAllTest);
	const std::vector<const NodeVirtual*>& nodes = getAllTest.getResult();
	for (const NodeVirtual* node : nodes)
	{
		if (node->getObjectCount() != 0)
			node->draw();
	}
}

void SceneManager::drawImGuiHierarchy(World& world)
{
#ifdef USE_IMGUI
	ImGui::Begin("Scene hierarchy");
	ImGui::PushID(this);
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Options");

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Filters");
	m_nameFilter.Draw();

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hierarchy");

	for (int i = 0; i < roots.size(); i++)
	{
		ImGui::PushID(roots[i]);
		drawRecursiveImGuiEntity(world, roots[i], 0);
		ImGui::PopID();
	}

	std::set<Entity*> toremove;
	for (auto& entity : m_selectedEntities)
	{
		if (entity->drawImGui(world))
			toremove.insert(entity);
	}
	for (auto& entity : toremove)
	{
		if (m_selectedEntities.erase(entity) > 0)
			world.releaseOwnership(entity);
	}
	ImGui::PopID();
	ImGui::End();
#endif // USE_IMGUI
}



void SceneManager::drawImGuiSpatialPartitioning(World& _world)
{
#ifdef USE_IMGUI
	class DebugQuery : public FrustrumSceneQuerry
	{
		public:
			DebugQuery(): FrustrumSceneQuerry(vec4f(0), vec4f(0), vec4f(0), vec4f(0), 0, 0)
			{};

			DebugQuery(const vec4f& position, const vec4f& direction, const vec4f& verticalDir, const vec4f& leftDir, float verticalAngle, float horizontalAngle) :
				FrustrumSceneQuerry(position, direction, verticalDir, leftDir, verticalAngle, horizontalAngle)
			{};

			VirtualSceneQuerry::CollisionType operator() (const NodeVirtual* node) override
			{
				auto collision = FrustrumSceneQuerry::operator()(node);
				collisionResults[node] = collision;
				return collision;
			}

			std::map<const NodeVirtual*, VirtualSceneQuerry::CollisionType> collisionResults;
	};
	DebugQuery sceneTest;

	ImGui::Begin("Spatial partitioning");
	ImGui::PushID(this);

	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Gizmos");
	ImGui::Checkbox("Show empty nodes", &m_showEmptyNodes);
	ImGui::Checkbox("Show fail collision nodes", &m_showFailTestNodes);
	ImGui::Checkbox("Print entities", &m_printEntities);
	ImGui::Checkbox("Print empty nodes", &m_printEmptyNodes);
	ImGui::Checkbox("Show frustrum", &m_showfrustrum);

	CameraComponent* mainCamera = _world.getMainCamera(); 
	if (mainCamera && ImGui::Button("Select camera entity"))
		selectEntity(_world, mainCamera->getParentEntity());

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Hierarchy");
	m_openAll = ImGui::Button("Open all");

	if (mainCamera)
	{
		float camFovVert = mainCamera->getVerticalFieldOfView();
		vec4f camPos, camFwd, camUp, camRight;
		mainCamera->getFrustrum(camPos, camFwd, camRight, camUp);
		sceneTest = DebugQuery(camPos, camFwd, camUp, -camRight, camFovVert, Debug::viewportRatio);
		getSceneNodes(&sceneTest);

		if (m_showfrustrum)
			mainCamera->drawDebug(Debug::viewportRatio, 30.f);
	}

	drawRecursiveImGuiSceneNode(_world, sceneTest.collisionResults, world[0], vec3i(-1), 0);

	m_openAll = false;
	ImGui::PopID();
	ImGui::End();
#endif // USE_IMGUI
}

void SceneManager::selectEntity(World& world, Entity* entity)
{
	const auto& it = m_selectedEntities.insert(entity);
	if (it.second)
		world.getOwnership(entity);
	entity->m_isDebugSelected = true;
}

void SceneManager::drawRecursiveImGuiEntity(World& world, Entity* entity, int depth)
{
	const auto ChildrenFilter = [&](Entity* e)
	{
		return m_nameFilter.PassFilter(e->getName().c_str());
	};

	bool passFilter = m_nameFilter.PassFilter(entity->getName().c_str());
	if (!passFilter)
		passFilter = entity->recursiveChildVisitor(ChildrenFilter);
	if (!passFilter)
		return;

	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (entity->m_isDebugSelected)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	const auto SelectionCheck = [&]()
	{
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			entity->m_isDebugSelected = !entity->m_isDebugSelected;
			if (entity->m_isDebugSelected)
			{
				const auto& it = m_selectedEntities.insert(entity);
				if (it.second)
					world.getOwnership(entity);
			}
			else
			{
				if (m_selectedEntities.erase(entity) > 0)
					world.releaseOwnership(entity);
			}
		}
	};
	const auto HoverCheck = [&]()
	{
		if (ImGui::IsItemHovered())
		{
			Debug::setDepthTest(false);
			Debug::color = Debug::magenta;
			Debug::drawLineCube(mat4f::identity, entity->m_worldBoundingBox.min, entity->m_worldBoundingBox.max);
			Debug::setDepthTest(true);
		}
	};

	std::vector<Entity*>& childs = entity->getChilds();
	if (childs.size() > 0)
	{
		bool openNode = ImGui::TreeNodeEx((void*)entity, nodeFlags, entity->getName().c_str());
		SelectionCheck();
		HoverCheck();

		if (openNode)
		{
			for (int i = 0; i < childs.size(); i++)
				drawRecursiveImGuiEntity(world, childs[i], depth + 1);
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::Selectable(entity->getName().c_str(), entity->m_isDebugSelected);
		SelectionCheck();
		HoverCheck();
	}
}

void SceneManager::drawRecursiveImGuiSceneNode(World& _world, std::map<const NodeVirtual*, VirtualSceneQuerry::CollisionType>& collisionResults, NodeVirtual* node, vec3i nodeIndex, int depth)
{
	// fast reject
	bool isEmpty = isEmptyNode(*node);
	if (!m_printEmptyNodes && isEmpty)
		return;

	// helpers
	bool hovered = false;
	const ImVec4 sectionColor = ImVec4(1, 0.9, 0.5, 1);
	const auto HoveredSelectedDrawing = [&]() {
		Debug::color = Debug::magenta;
		Debug::drawLineCube(mat4f::identity, node->getBBMin(), node->getBBMax());
		Debug::color = Debug::yellow;
		vec4f margin(node->getAllowanceSize());
		margin.w = 0.f;
		Debug::drawLineCube(mat4f::identity, node->getBBMin() - margin, node->getBBMax() + margin);
	};
	const auto HoverCheck = [&]()
	{
		if (ImGui::IsItemHovered())
		{
			HoveredSelectedDrawing();
			return true;
		}
		return false;
	};
	const auto SceneNodeSelectionCheck = [&]()
	{
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			m_selectedSceneNode = node;
	};
	const auto ColoredTreeNode = [](ImVec4 color, const char* label, void* id)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		bool open = ImGui::TreeNodeEx(id, ImGuiTreeNodeFlags_None, label);
		ImGui::PopStyleColor();
		return open;
	};
	const auto DrawNodeContent = [&]()
	{
		if (node->getObjectCount() != 0)
		{
			std::ostringstream label;
			label << "Objects (" << node->getObjectCount() <<  ")";

			if (m_printEntities)
			{
				if (ColoredTreeNode(sectionColor, label.str().c_str(), (void*)(intptr_t)(node + 1)))
				{
					for (int i = 0; i < node->objectList.size(); i++)
					{
						Entity* e = node->objectList[i];
						ImGui::Selectable(e->getName().c_str(), &e->m_isDebugSelected);

						if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
						{
							e->m_isDebugSelected = !e->m_isDebugSelected;
							if (e->m_isDebugSelected)
							{
								const auto& it = m_selectedEntities.insert(e);
								if (it.second)
									_world.getOwnership(e);
							}
							else
							{
								if (m_selectedEntities.erase(e) > 0)
									_world.releaseOwnership(e);
							}
						}
						if (ImGui::IsItemHovered())
						{
							Debug::color = Debug::magenta;
							Debug::drawLineCube(mat4f::identity, e->m_worldBoundingBox.min, e->m_worldBoundingBox.max);
						}
					}
					ImGui::TreePop();
				}
			}
			else
			{
				ImGui::TextColored(sectionColor, "   %s", label.str().c_str());
			}
		}
	};

	// node name
	std::ostringstream nodeName;
	if (nodeIndex.x >= 0)
		nodeName << "Node " << nodeIndex.x << ' ' << nodeIndex.y << ' ' << nodeIndex.z;
	else nodeName << "Root";

	bool isSelected = node == m_selectedSceneNode;
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (isSelected)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;

	if (node->children.size() > 0)
	{
		if (m_openAll)
			ImGui::SetNextItemOpen(true, ImGuiCond_None);
		bool openNode = ImGui::TreeNodeEx((void*)node, nodeFlags, nodeName.str().c_str());
		SceneNodeSelectionCheck();
		hovered = HoverCheck();

		if (openNode)
		{
			DrawNodeContent();

			if (m_openAll)
				ImGui::SetNextItemOpen(true, ImGuiCond_None);
			if (ColoredTreeNode(sectionColor, "Children", (void*)(intptr_t)(node + 2)))
			{
				vec3i division = node->getDivision();
				for (int x = 0; x < division.x; x++)
					for (int y = 0; y < division.y; y++)
						for (int z = 0; z < division.z; z++)
						{
							int index = x * division.z * division.y + y * division.z + z;
							drawRecursiveImGuiSceneNode(_world, collisionResults, &node->children[index], vec3i(x, y, z), depth + 1);
						}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::Selectable(nodeName.str().c_str(), &isSelected);
		SceneNodeSelectionCheck();
		hovered = HoverCheck();

		ImGui::Indent();
		DrawNodeContent();
		ImGui::Unindent();
	}

	// debug draw
	if (!isEmpty || m_showEmptyNodes)
	{
		VirtualSceneQuerry::CollisionType colresult = VirtualSceneQuerry::CollisionType::NONE;
		const auto& it = collisionResults.find(node);
		if (it != collisionResults.end())
			colresult = it->second;
		if (colresult != VirtualSceneQuerry::CollisionType::NONE || m_showFailTestNodes)
		{
			switch (colresult)
			{
				case VirtualSceneQuerry::CollisionType::INSIDE:  Debug::color = Debug::green; break;
				case VirtualSceneQuerry::CollisionType::OVERLAP: Debug::color = Debug::red;   break;
				default: Debug::color = Debug::grey; break;
			}

			vec4f hs = (float)pow(0.99f, depth) * node->getHalfSize();
			Debug::drawLineCube(mat4f::identity, node->getCenter() - hs, node->getCenter() + hs);
		}
	}
	if (isSelected)
		HoveredSelectedDrawing();
}

bool SceneManager::isEmptyNode(const NodeVirtual& _node)
{
	if (_node.getObjectCount() > 0)
		return false;
	for (const NodeVirtual& n : _node.children)
		if (!isEmptyNode(n))
			return false;
	return true;
}