#pragma once

#include <EntityComponent/Component.hpp>
#include <Math/TMath.h>
#include <Physics/Shapes/AxisAlignedBox.h>


class LightComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(LightComponent, Component)

public:
	friend class Renderer;

	explicit LightComponent();

	bool load(Variant& jsonObject, const std::string& objectName) override;
	void save(Variant& jsonObject) override;

	void onAddToEntity(Entity* entity) override;
	AxisAlignedBox getBoundingBox() const;

	// setter geter
	void setPointLight(bool _isPointLight);
	void setCastShadow(bool _enableShadow);
	void setRange(float _range);
	void setIntensity(float _intensity);
	void setColor(vec4f _color);
	void setInnerCutOffAngle(float _angleRad);
	void setOuterCutOffAngle(float _angleRad);


	vec4f getDirection();
	vec4f getPosition() const;
	bool isPointLight() const;
	bool castShadow() const;
	float getRange() const;
	float getIntensity() const;
	float getInnerCutOffAngle() const;
	float getOuterCutOffAngle() const;
	//

	//	Debug
	void onDrawImGui() override;
	//

protected:
	bool m_isPointLight;
	bool m_castShadow;

	vec4f m_color;
	float m_range;
	float m_intensity;
	float m_innerCutoffAngle;
	float m_outerCutoffAngle;

#ifdef USE_IMGUI
	bool  m_drawRange = false;
#endif
};







