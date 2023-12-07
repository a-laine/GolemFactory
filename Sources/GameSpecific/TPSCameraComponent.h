#pragma once

#include <Renderer/CameraComponent.h>


class TPSCameraComponent : public CameraComponent
{
	GF_DECLARE_COMPONENT_CLASS(TPSCameraComponent, CameraComponent)

	public:
		TPSCameraComponent();

		void setTargetCharacter(Entity* _targetCharacter);

		void update(float _dt);

		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;

	protected:
		Entity* m_targetCharacter;
		float m_radius;
		vec4f m_targetOffset;

#ifdef USE_IMGUI
		bool  m_drawDebug = true;
		vec4f m_targetColor = vec4f(1, 0, 0, 1);
#endif
};
