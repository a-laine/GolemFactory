#include "LightComponent.h"

#include <EntityComponent/Entity.hpp>
#include <Utiles/Debug.h>
#include <Renderer/Renderer.h>
#include <Utiles/ConsoleColor.h>


LightComponent::LightComponent()
{
	m_isPointLight = true;
	m_castShadow = false;
	m_range = 20;

	m_color = vec4f(1.f);
	m_intensity = 1.f;
	m_innerCutoffAngle = (float)DEG2RAD * 28.f;
	m_outerCutoffAngle = (float)DEG2RAD * 35.f;
}

bool LightComponent::load(Variant& jsonObject, const std::string& objectName)
{
	// helpers
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
	const auto TryLoadAsVec4f = [](Variant& variant, const char* label, vec4f& destination)
	{
		int sucessfullyParsed = 0;
		auto it0 = variant.getMap().find(label);
		if (it0 != variant.getMap().end() && it0->second.getType() == Variant::ARRAY)
		{
			auto& varray = it0->second.getArray();
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
	const auto TryLoadInt = [](Variant& variant, const char* label, int& destination)
	{
		auto it0 = variant.getMap().find(label);
		if (it0 != variant.getMap().end() && it0->second.getType() == Variant::INT)
		{
			destination = it0->second.toInt();
			return true;
		}
		return false;
	};
	const auto TryLoadBool = [](Variant& variant, const char* label, bool& destination)
	{
		auto it0 = variant.getMap().find(label);
		if (it0 != variant.getMap().end() && it0->second.getType() == Variant::BOOL)
		{
			destination = it0->second.toBool();
			return true;
		}
		return false;
	};
	const auto PrintWarning = [](std::string header, const char* msg)
	{
		if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::WARNINGS)
		{
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << header << msg << std::flush;
			std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
		}
	};

	if (jsonObject.getType() == Variant::MAP)
	{
		m_color = vec4f(1.f, 1.f, 1.f, 1.f);
		m_range = 10;
		m_intensity = 1;
		float inCutOff = 28;
		float outCutOff = 32;
		int type = 0;
		std::string warningHeader = "WARNING : " + objectName + " : LightComponent loading : ";

		bool rangeOk = TryLoadAsFloat(jsonObject, "range", m_range);
		bool intensityOk = TryLoadAsFloat(jsonObject, "intensity", m_intensity);
		int colorOk = TryLoadAsVec4f(jsonObject, "color", m_color);
		bool typeOk = TryLoadInt(jsonObject, "type", type);
		bool inCutoffOk = TryLoadAsFloat(jsonObject, "inCutOff", inCutOff);
		bool outCutoffOk = TryLoadAsFloat(jsonObject, "outCutOff", outCutOff);

		if (!typeOk)
		{
			if (ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
			{
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "ERROR   : " << objectName << 
					" : LightComponent loading : fail parsing light type" << std::flush;
				std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;
			}
		}
		else if ((type != 0) && (type != 1))
		{
			PrintWarning(warningHeader, "invalid light type, valid are 0=point, 1=spot. Was set to 0");
			type = 0;
		}
		else if (type == 1)
		{
			if (!inCutoffOk)
				PrintWarning(warningHeader, "fail parsing spot in cutoff angle, Was set to 28");
			if (!outCutoffOk)
				PrintWarning(warningHeader, "fail parsing spot in cutoff angle, Was set to 32");
		}

		if (!rangeOk)
			PrintWarning(warningHeader, "fail parsing light range, was set to 10");
		if (!intensityOk)
			PrintWarning(warningHeader, "fail parsing light intensity, was set to 1");
		if (colorOk < 3)
			PrintWarning(warningHeader, "fail parsing light color, was set to white");
		else
		{
			float maxValue = std::max(m_color.x, std::max(m_color.y, m_color.z));
			if (maxValue > 1.f)
				m_color *= 1.f / 255.f;
			m_color.w = 1.f;
		}

		if (typeOk)
		{
			m_isPointLight = (type == 0);
			TryLoadBool(jsonObject, "castShadow", m_castShadow);

			if (type == 1)
			{
				m_innerCutoffAngle = std::min(inCutOff, outCutOff);
				m_outerCutoffAngle = std::max(inCutOff, outCutOff);
			}

			return true;
		}
	}
	return false;
}
void LightComponent::save(Variant& jsonObject)
{

}

void LightComponent::onAddToEntity(Entity* entity)
{
	Component::onAddToEntity(entity);
	entity->setFlags((uint64_t)Entity::Flags::Fl_Light);
}

AxisAlignedBox LightComponent::getBoundingBox() const
{
	vec4f center = getPosition();
	vec4f range = vec4f(m_range, m_range, m_range, 0);
	return AxisAlignedBox(center - range, center + range);
}

// setter geter
void LightComponent::setPointLight(bool _isPointLight) { m_isPointLight = _isPointLight; }
void LightComponent::setCastShadow(bool _enableShadow) { m_castShadow = _enableShadow; };
void LightComponent::setRange(float _range) { m_range = _range; }
void LightComponent::setColor(vec4f _color) { m_color = _color; }
void LightComponent::setIntensity(float _intensity) { m_intensity = _intensity; }
void LightComponent::setInnerCutOffAngle(float _angleRad) { m_innerCutoffAngle = _angleRad; }
void LightComponent::setOuterCutOffAngle(float _angleRad) { m_outerCutoffAngle = _angleRad; }


vec4f LightComponent::getDirection()
{
	return getParentEntity()->getWorldOrientation() * vec4f(0, 0, 1, 0);
}
vec4f LightComponent::getPosition() const
{
	return getParentEntity()->getWorldPosition();
}
bool LightComponent::isPointLight() const { return m_isPointLight; }
bool LightComponent::castShadow() const { return m_castShadow; }
float LightComponent::getRange() const { return m_range; }
float LightComponent::getIntensity() const { return m_color.w; }
float LightComponent::getInnerCutOffAngle() const { return m_innerCutoffAngle; }
float LightComponent::getOuterCutOffAngle() const { return m_outerCutoffAngle; }
//

//	Debug
void LightComponent::onDrawImGui()
{
#ifdef USE_IMGUI
	const ImVec4 componentColor = ImVec4(0.7f, 0.7f, 0.5f, 1.f);
	std::ostringstream unicName;
	unicName << "Light component##" << (uintptr_t)this;
	if (ImGui::TreeNodeEx(unicName.str().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextColored(componentColor, "Parameters");
		ImGui::Indent();

		static int typeCurrent = (m_isPointLight ? 0 : 1);
		ImGui::Combo("Type", &typeCurrent, "Point\0Spot\0\0");
		m_isPointLight = (typeCurrent == 0);

		ImGui::DragFloat("Range", &m_range, 0.01f, 0.f, 300.f, "%.3fm");
		if (!m_isPointLight)
			ImGui::DragFloatRange2("CutOff angles", &m_innerCutoffAngle, &m_outerCutoffAngle, 0.1f, 0.f, 180.f, "%.3fdeg");
		ImGui::ColorEdit3("Color", &m_color[0]);
		ImGui::DragFloat("Intensity", &m_intensity, 0.01f, 0.0001f, 100.f);
		ImGui::Checkbox("Cast shadow", &m_castShadow);
		
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::TextColored(componentColor, "Gizmos");
		ImGui::Indent();
		ImGui::Checkbox("Draw range", &m_drawRange);
		ImGui::Unindent();

		ImGui::TreePop();
	}

	if (m_drawRange)
	{
		Debug::color = m_color;
		Debug::drawWiredSphere(getPosition(), m_range);
	}
#endif // USE_IMGUI
}
//

