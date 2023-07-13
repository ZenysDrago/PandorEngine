#pragma once
#include "PandorAPI.h"

#include <vector>

#include <Components/BaseComponent.h>
#include <Render/Camera.h>
#include <Core/Wrappers/WrapperRHI.h>

namespace Render
{
	class EditorIcon;
}

namespace Component
{
	class PANDOR_API CameraComponent : public IComponent<CameraComponent>, public Render::Camera
	{
	private:
		bool m_showPreviewCamera = false;
		bool m_mainCamera = false;
#ifndef PANDOR_GAME
		Render::EditorIcon* m_icon = nullptr;
#endif

	public:
		CameraComponent() : Render::Camera() {}
		CameraComponent(int width, int height, Math::Vector3 position);
		~CameraComponent();

		void Initialize() override;

		std::string GetComponentName() { return "Camera Component"; }

		void SetAsMainCamera() override;

		void Update() override {}

		void DrawPicking(int ID) override;

		void EditorDraw() override;

		void ShowInInspector() override;

		void OnDestroy() override;

		Component::Transform* GetTransform() const override;

		Math::Vector2 GetScreenResolution() override;

		bool IsVisible() override;

		void ReadComponent(std::fstream& sceneFile) override;

		std::ostream& operator<<(std::ostream& os) override;

	

	};
}