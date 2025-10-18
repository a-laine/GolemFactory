#include "RigidBody.h"
#include "Shapes/Collider.h"
#include <EntityComponent/Entity.hpp>

#include <iostream>
#include <sstream>
#include <Utiles/Debug.h>
#include <Utiles/ConsoleColor.h>

//	Default
RigidBody::RigidBody(const RigidBodyType& type, const SolverType& solver) : 
	m_type(type), m_solver(solver),
	m_gravityFactor(1.f), m_bouncyness(0.5f), m_friction(0.1f), m_damping(0.f),
	m_mass(1.f), m_inverseMass(1.f), m_volumicMass(800.f), m_inertia(1.f), m_inverseInertia(1.f),
	m_externalForces(0.f), m_externalTorques(0.f), m_linearVelocity(0.f), m_linearAcceleration(0.f), m_angularVelocity(0.f), m_angularAcceleration(0.f)
{

}
RigidBody::~RigidBody()
{}

void RigidBody::initialize(bool explicitMass, float _mass)
{
	m_colliders.clear();
	auto colliderVisitor = [&](Component* componentCollider)
	{
		const Collider* collider = static_cast<const Collider*>(componentCollider);
		if (collider && !collider->m_isTrigger)
			m_colliders.push_back(componentCollider);
		return false;
	};
	getParentEntity()->componentsVisitor(Collider::getStaticClassID(), colliderVisitor);

	if (!m_colliders.empty())
	{
		float volume = 0.f;
		m_inertia = mat4f(0.f);
		for (int i = 0; i < m_colliders.size(); i++)
		{
			Collider* collider = static_cast<Collider*>(m_colliders[i]);
			m_inertia += collider->m_shape->computeInertiaMatrix();
			volume += collider->m_shape->computeVolume();
		}

		m_mass = std::max(explicitMass ? _mass : _mass * volume, 0.001f);
		m_volumicMass = volume > 0.001f ? m_mass / volume : m_mass;
		m_inverseMass = 1.f / m_mass;
		m_inertia *= m_mass;
		m_inverseInertia = mat4f::inverse(m_inertia);
	}
	else
	{
		m_mass = std::max(_mass, 0.001f);
		m_inverseMass = 1.f / m_mass;
		m_inertia = mat4f(m_mass);
		m_inverseInertia = mat4f(m_inverseMass);
	}
}
bool RigidBody::load(Variant& jsonObject, const std::string& objectName)
{
	const auto PrintWarning = [&](const char* msg)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
		{
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "WARNING : " << objectName <<
				" : ColliderComponent loading : " << msg << std::flush;
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	};
	const auto TryLoadAsVec4f = [](Variant& variant, const char* label, vec4f& destination)
	{
		int sucessfullyParsed = 0;
		auto it0 = variant.getMap().find(label);
		if (it0 != variant.getMap().end() && it0->second.getType() == Variant::ARRAY)
		{
			auto varray = it0->second.getArray();
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
	const auto TryLoadAsFloat = [](Variant& variant, const char* label, float& destination)
	{
		if (variant.getMap().find(label) != variant.getMap().end())
		{
			auto& v = variant[label];
			if (v.getType() == Variant::FLOAT)
				destination = v.toFloat();
			else if (v.getType() == Variant::DOUBLE)
				destination = (float)v.toDouble();
			else if (v.getType() == Variant::INT)
				destination = (float)v.toInt();
			else
				return false;
			return true;
		}
		return false;
	};


	if (jsonObject.getType() == Variant::MAP)
	{
		std::string rbType;
		auto it1 = jsonObject.getMap().find("type");
		if (it1 != jsonObject.getMap().end() && it1->second.getType() == Variant::STRING)
			rbType = it1->second.toString();
		if (!rbType.empty())
		{
			if (rbType == "kinematic")
				m_type = RigidBody::KINEMATICS;
			else if (rbType == "static")
				m_type = RigidBody::STATIC;
			else if (rbType == "dynamic")
				m_type = RigidBody::DYNAMIC;
			else
			{
				m_type = RigidBody::DYNAMIC;
				PrintWarning("unknown rigidbody type");
			}
		}

		TryLoadAsFloat(jsonObject, "bouncyness", m_bouncyness);
		TryLoadAsFloat(jsonObject, "friction", m_friction);
		TryLoadAsFloat(jsonObject, "damping", m_damping);
		TryLoadAsFloat(jsonObject, "gravityFactor", m_gravityFactor);

		m_mass = 0.f;
		it1 = jsonObject.getMap().find("mass");
		if (it1 != jsonObject.getMap().end())
			TryLoadAsFloat(jsonObject, "mass", m_mass);

		m_volumicMass = 800.f;
		it1 = jsonObject.getMap().find("volumicMass");
		if (it1 != jsonObject.getMap().end())
			TryLoadAsFloat(jsonObject, "volumicMass", m_volumicMass);
		return true;
	}
	return false;
}

void RigidBody::computeWorldShapes()
{
	for (Shape* shape : m_worldShapes)
		delete shape;
	m_worldShapes.clear();

	vec4f position = getPosition();
	vec4f scale(getParentEntity()->getWorldScale());
	quatf orientation = getOrientation();

	for (Component* colliderComponent : m_colliders)
	{
		Collider* collider = static_cast<Collider*>(colliderComponent);
		m_worldShapes.push_back(collider->m_shape->duplicate());
		m_worldShapes.back()->transform(position, scale, orientation);
	}
}
//

//	Set / get / test
void RigidBody::setType(const RigidBodyType& t) { m_type = t; }
void RigidBody::setMass(const float& m)
{
	m_inertia *= m / m_mass;
	m_inverseInertia *= m_mass / m;
	m_mass = m;
	m_inverseMass = 1.f / m_mass;
}
void RigidBody::setGravityFactor(const float& f) { m_gravityFactor = f; }
void RigidBody::setBouncyness(const float& b) { m_bouncyness = b; }
void RigidBody::setFriction(const float& f) { m_friction = f; }
void RigidBody::setDamping(const float& f) { m_damping = f; }

void RigidBody::setExternalForces(const vec4f& f) { m_externalForces = f; }
void RigidBody::setExternalTorques(const vec4f& t) { m_externalTorques = t; }
void RigidBody::setLinearAcceleration(const vec4f& a) { m_linearAcceleration = a; }
void RigidBody::setAngularAcceleration(const vec4f& a) { m_angularAcceleration = a; }
void RigidBody::setLinearVelocity(const vec4f& v) { m_linearVelocity = v; }
void RigidBody::setAngularVelocity(const vec4f& v) { m_angularVelocity = v; }
void RigidBody::setPosition(const vec4f& p)
{
	Entity* parent = getParentEntity();
	if (parent)
		parent->setWorldPosition(p);
}
void RigidBody::setOrientation(const quatf& q)
{
	Entity* parent = getParentEntity();
	if (parent)
		parent->setWorldOrientation(q);
}


RigidBody::RigidBodyType RigidBody::getType() const { return m_type; }
float RigidBody::getMass() const { return m_mass; }
float RigidBody::getInverseMass() const { return m_inverseMass; }
const mat4f& RigidBody::getInertia() const { return m_inertia; }
const mat4f& RigidBody::getInverseInertia() const { return m_inverseInertia; }
float RigidBody::getGravityFactor() const { return m_gravityFactor; }
float RigidBody::getFriction() const { return m_friction; }
float RigidBody::getDamping() const { return m_damping; }

vec4f RigidBody::getExternalForces() const { return m_externalForces; }
vec4f RigidBody::getExternalTorques() const { return m_externalTorques; }
vec4f RigidBody::getLinearAcceleration() const { return m_linearAcceleration; }
vec4f RigidBody::getAngularAcceleration() const { return m_angularAcceleration; }
vec4f RigidBody::getLinearVelocity() const { return m_linearVelocity; }
vec4f RigidBody::getAngularVelocity() const { return m_angularVelocity; }
vec4f RigidBody::getPosition() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getWorldPosition();
	else
		return vec4f(0.f);
}
quatf RigidBody::getOrientation() const
{
	Entity* parent = getParentEntity();
	if (parent)
		return parent->getWorldOrientation();
	else
		return quatf::identity;
}



/*glm::vec3 RigidBody::getDeltaPosition() const { return deltaPosition; }
glm::fquat RigidBody::getDeltaRotation() const { return deltaRotation; }*/


/*void RigidBody::addForce(const glm::vec3& force) { forces.push_back(force); }
void RigidBody::addForce(const glm::vec3& force, const glm::vec3& contactPoint)
{
	forces.push_back(force);
	torques.push_back(glm::cross(contactPoint, force));
}
void RigidBody::addTorque(const glm::vec3& torque) { torques.push_back(torque); }*/


bool RigidBody::isResting() const
{
	/*return velocity == glm::vec3(0.f) &&
		angularVelocity == glm::vec3(0.f) &&
		forces.empty() && torques.empty() &&
		acceleration == glm::vec3(0.f) &&
		angularAcceleration == glm::vec3(0.f);*/
	return false;
}
vec4f RigidBody::computeLocalPoint(vec4f worldPoint)
{
	return getParentEntity()->getInverseWorldTransformMatrix() * worldPoint;
}
vec4f RigidBody::computeLocalDirection(vec4f worldDirection)
{
	return getParentEntity()->getInverseWorldTransformMatrix() * worldDirection;
}
vec4f RigidBody::computeWorldDirection(vec4f localDirection)
{
	return getParentEntity()->getWorldTransformMatrix() * localDirection;
}
//


void RigidBody::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Physics);

	World* world = entity->getParentWorld();
	if (world)
		world->getPhysics().addMovingEntity(entity);
	
	bool explicitmass = m_mass > 0.001f;
	initialize(explicitmass, explicitmass ? m_mass : m_volumicMass);
}
void RigidBody::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0, 1, 0, 1);
	std::ostringstream unicName;
	unicName << "RigidBody component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Attributes");
		ImGui::Indent();

		switch (m_type)
		{
			case RigidBody::DYNAMIC: ImGui::Text("Type : DYNAMIC"); break;
			case RigidBody::STATIC: ImGui::Text("Type : STATIC"); break;
			case RigidBody::KINEMATICS: ImGui::Text("Type : KINEMATICS"); break;
			default: ImGui::Text("Type : ??"); break;
		}
		switch (m_solver)
		{
			case SolverType::DISCRETE: ImGui::Text("Type : DISCRETE"); break;
			case SolverType::CONTINUOUS: ImGui::Text("Type : CONTINUOUS"); break;
			case SolverType::SUPERSAMPLING: ImGui::Text("Type : SUPERSAMPLING"); break;
			default: ImGui::Text("Type : ??"); break;
		}

		float m = m_mass;
		if (ImGui::DragFloat("Mass", &m_mass, 0.1f, 0.01f, 3000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic) && !ImGui::IsItemActive())
		{
			m_inertia *= m_mass / m;
			m_inverseInertia = mat4f::inverse(m_inertia);
		}

		ImGui::DragFloat("GravityFactor", &m_gravityFactor, 0.1f);
		ImGui::SliderFloat("Bouncyness", &m_bouncyness, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Friction", &m_friction, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
		ImGui::SliderFloat("Damping", &m_damping, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "State");
		ImGui::Indent();
		ImGui::DragFloat3("Linear velocity", &m_linearVelocity.x);
		ImGui::DragFloat3("Angular velocity", &m_angularVelocity.x);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw colliders", &m_drawColliders);
		ImGui::Combo("Draw cluster collision cache", &m_drawClusterCollisionCache, "None\0Wireframe\0Wireframe+NoZtest\0Solid\0Solid+NoZtest\0\0");
		ImGui::ColorEdit3("Cache base color", &m_cacheColor.x, ImGuiColorEditFlags_NoAlpha);
		ImGui::Unindent();

		ImGui::TreePop();
	}

	if (m_drawColliders)
	{
		drawColliders(vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w));
	}
	if (m_drawClusterCollisionCache && m_clusterIndex >=0)
	{
		bool wireframe = m_drawClusterCollisionCache == 1 || m_drawClusterCollisionCache == 2;
		bool ztest = m_drawClusterCollisionCache == 1 || m_drawClusterCollisionCache == 3;
		const Physics::Cluster* cluster = getParentEntity()->getParentWorld()->getPhysics().getCLuster(m_clusterIndex);
		if (cluster)
		{
			Debug::setDepthTest(ztest);
			cluster->cache.debugDraw(wireframe, m_cacheColor);

			Debug::setDepthTest(false);

			int bufferSize = 0;
			Debug::Vertex tmpBuffer[64 * 3];
			constexpr float ptssize = 0.015f;
			for (const Constraint& c : cluster->constraints)
			{
				tmpBuffer[bufferSize].m_position = c.worldPoint + vec4f(ptssize, 0, 0, 0);             tmpBuffer[bufferSize].m_color = Debug::yellow;
				tmpBuffer[bufferSize + 2].m_position = c.worldPoint + vec4f(-ptssize, 0, ptssize, 0);  tmpBuffer[bufferSize + 2].m_color = Debug::yellow;
				tmpBuffer[bufferSize + 1].m_position = c.worldPoint - vec4f(ptssize, 0, ptssize, 0);   tmpBuffer[bufferSize + 1].m_color = Debug::yellow;
				bufferSize += 3;
				Debug::color = Debug::red;
				Debug::drawLine(c.worldPoint, c.worldPoint + c.axis[0]);
				if (bufferSize == 64 * 3)
				{
					Debug::drawMultiplePrimitive(tmpBuffer, bufferSize, mat4f::identity, GL_TRIANGLES);
					bufferSize = 0;
				}
			}
			if (bufferSize)
			{
				Debug::drawMultiplePrimitive(tmpBuffer, bufferSize, mat4f::identity, GL_TRIANGLES);
				bufferSize = 0;
			}
			Debug::setDepthTest(true);
		}
	}
#endif // USE_IMGUI
}
void RigidBody::drawColliders(vec4f color) const
{
	for (const Component* comp : m_colliders)
	{
		const Collider* collider = static_cast<const Collider*>(comp);
		collider->drawDebug(color, true);
	}
}