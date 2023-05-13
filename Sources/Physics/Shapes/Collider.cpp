#include "Collider.h"
#include <Physics/BoundingVolume.h>
#include <Resources/Mesh.h>

#include <sstream>
#include <Utiles/Debug.h>



//	Default
Collider::Collider(Shape* _shape) : m_shape(_shape)
{}

Collider::~Collider()
{
	if (m_shape)
		delete m_shape;
}
//

void Collider::onAddToEntity(Entity* entity)
{
	entity->setFlags((uint64_t)Entity::Flags::Fl_Collision);
}

void Collider::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0, 1, 0, 1);
	std::ostringstream unicName;
	unicName << (m_shape ? m_shape->getTypeStr() : "INVALID") << " collider##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_shape)
		{
			bool edited = false;
			switch (m_shape->type)
			{
				case Shape::ShapeType::POINT:
					{
						Point* point = static_cast<Point*>(m_shape);
						edited |= ImGui::DragFloat3("Local point", &point->p[0]);
					}
					break;
				case Shape::ShapeType::SEGMENT:
					{
						Segment* segment = static_cast<Segment*>(m_shape);
						edited |= ImGui::DragFloat3("Local p1", &segment->p1[0]);
						edited |= ImGui::DragFloat3("Local p2", &segment->p2[0]);
					}
					break;
				case Shape::ShapeType::TRIANGLE:
					{
						Triangle* triangle = static_cast<Triangle*>(m_shape);
						edited |= ImGui::DragFloat3("Local p1", &triangle->p1[0]);
						edited |= ImGui::DragFloat3("Local p2", &triangle->p2[0]);
						edited |= ImGui::DragFloat3("Local p2", &triangle->p3[0]);
					}
					break;
				case Shape::ShapeType::SPHERE:
					{
						Sphere* sphere = static_cast<Sphere*>(m_shape);
						edited |= ImGui::DragFloat3("Local center", &sphere->center[0]);
						edited |= ImGui::DragFloat("Radius", &sphere->radius, 0.01f, 0.f, 1000.f);
					}
					break;
				case Shape::ShapeType::AXIS_ALIGNED_BOX:
					{
						AxisAlignedBox* aabb = static_cast<AxisAlignedBox*>(m_shape);
						vec4f min = aabb->min;
						vec4f max = aabb->min;
						edited |= ImGui::DragFloat3("Local min", &min[0]);
						edited |= ImGui::DragFloat3("Local max", &max[0]);
						if (edited)
						{
							aabb->min = vec4f::min(min, max);
							aabb->max = vec4f::max(min, max);
						}
					}
					break;
				case Shape::ShapeType::ORIENTED_BOX:
					{
						constexpr float floatMax = std::numeric_limits<float>::max();
						OrientedBox* box = static_cast<OrientedBox*>(m_shape);
						vec4f position = box->base[3];
						vec3f euler = (float)RAD2DEG * glm::eulerAngles(quatf(box->base));
						edited |= ImGui::DragFloat3("Local position", &position[0], 0.01f, -floatMax, floatMax, "%.3f");
						edited |= ImGui::DragFloat3("Local orientation", &euler[0], 0.1f, -180.f, 180.f, "%.3f");
						if (edited)
						{
							box->base = mat4f::TRS(position, quatf((float)DEG2RAD * euler), vec4f(1.f));
						}
					}
					break;
				case Shape::ShapeType::CAPSULE:
					{
						Capsule* capsule = static_cast<Capsule*>(m_shape);
						edited |= ImGui::DragFloat3("Local p1", &capsule->p1[0]);
						edited |= ImGui::DragFloat3("Local p2", &capsule->p2[0]);
						edited |= ImGui::DragFloat("Radius", &capsule->radius, 0.01f, 0.f, 1000.f);
					}
					break;
				case Shape::ShapeType::HULL:
					{
						const Hull* hull = static_cast<const Hull*>(m_shape);
						ImGui::Text("mesh name : %s", hull->mesh->name.c_str());
						ImGui::Text("vertices count : %d", hull->mesh->getNumberVertices());
						ImGui::Text("faces count : %d", hull->mesh->getNumberFaces());
					}
					break;
				default:
					break;
			}


			ImGui::Checkbox("Draw shape", &m_drawShape);
		}
		ImGui::TreePop();
	}

	if (m_drawShape)
	{
		drawDebug(vec4f(componentColor.x, componentColor.y, componentColor.z, componentColor.w));
	}
#endif // USE_IMGUI
}

void Collider::drawDebug(vec4f color) const
{
	if (m_shape)
	{
		Debug::color = color;
		mat4f transform = getParentEntity()->getWorldTransformMatrix();
		float scale = getParentEntity()->getWorldScale();
		switch (m_shape->type)
		{
			case Shape::ShapeType::POINT:
				{
					const Point* point = static_cast<const Point*>(m_shape);
					Debug::drawPoint(transform * point->p);
				}
				break;
			case Shape::ShapeType::SEGMENT:
				{
					const Segment* segment = static_cast<const Segment*>(m_shape);
					Debug::drawLine(transform * segment->p1, transform * segment->p2);
				}
				break;
			case Shape::ShapeType::TRIANGLE:
				{
					const Triangle* triangle = static_cast<const Triangle*>(m_shape);
					Debug::drawLine(transform * triangle->p1, transform * triangle->p2);
					Debug::drawLine(transform * triangle->p2, transform * triangle->p3);
					Debug::drawLine(transform * triangle->p3, transform * triangle->p1);
				}
				break;
			case Shape::ShapeType::SPHERE:
				{
					const Sphere* sphere = static_cast<const Sphere*>(m_shape);
					Debug::drawWiredSphere(transform * sphere->center, scale * sphere->radius);
				}
				break;
			case Shape::ShapeType::AXIS_ALIGNED_BOX:
				{
					const AxisAlignedBox* aabb = static_cast<const AxisAlignedBox*>(m_shape);
					Debug::drawLineCube(transform, aabb->min, aabb->max);
				}
				break;
			case Shape::ShapeType::ORIENTED_BOX:
				{
					const OrientedBox* box = static_cast<const OrientedBox*>(m_shape);
					Debug::drawLineCube(transform * box->base, box->min, box->max);
				}
				break;
			case Shape::ShapeType::CAPSULE:
				{
					const Capsule* capsule = static_cast<const Capsule*>(m_shape);
					Debug::drawWiredCapsule(transform * capsule->p1, transform * capsule->p2, scale * capsule->radius);
				}
				break;
			case Shape::ShapeType::HULL:
				{
					const Hull* hull = static_cast<const Hull*>(m_shape);
					Debug::drawWiredMesh(hull->mesh, transform * hull->base);
				}
				break;
			default:
				break;
		}
	}
}