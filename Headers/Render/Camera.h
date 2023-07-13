#pragma once
#include "PandorAPI.h"

#include <Math/Maths.h>

namespace Resources
{
	class Shader;
	class Skybox;
}

namespace Core
{
	class GameObject;
}

namespace Component {
	class Transform;
}

namespace Render
{
	struct Frustum
	{
		Math::MathUtils::Plane planes[6];

		Frustum() {}

		void Draw();
	};

	class PANDOR_API Camera
	{
	private:
		Component::Transform* transform;

	protected:
		double x;
		double y;

		float moveX;
		float moveY;
		float sensibility = 0.1f;
		float speed = 35.f;
		float angle = 90;
		float pitch = 0;

		float p_near = 0.1f;
		float p_far = 100.0f;
		float p_aspectRatio;

		bool fullScreen = false;
		bool firstClick = true;

		Math::Vector3 up = Math::Vector3(0.0f, 1.0f, 0.0f);

	public:
		Frustum frustum = Frustum();
	public:
		float fov = 45.f;
		float GetNear() { return p_near; }
		float GetFar() { return p_far; }
		Math::Vector4 ClearColor = Math::Vector4(0.2f, 0.3f, 0.3f, 1.0f);
		virtual void SetAsMainCamera();
		Core::GameObject* pickedGizmo = nullptr;
		Core::GameObject* picked = nullptr;

		class Framebuffer* framebuffer;
		Vector2 crntpos;
		int pickedID;

		Resources::Skybox* skybox = nullptr;

		Camera();
		Camera(int width, int height, Math::Vector3 position);
		~Camera();

		void Init();

		void PreUpdate(float aspect);
		void PostUpdate();
		void DrawSkybox();

		void SetAspectRatio(float aspect) { p_aspectRatio = aspect; }
#ifndef PANDOR_GAME
		void ReadPickingID(unsigned char data[4], bool sceneWindow = true);
#endif

		void CameraWithMouse(Math::Vector2 vec);
		Math::Matrix4 GetVPMatrix();
		Math::Matrix4 GetViewMatrix();
		Math::Matrix4 GetViewNoTranslate();
		Math::Matrix4 GetProjectionMatrix();
		Math::Matrix4 GetOrtho();
#ifndef	PANDOR_GAME
		void Inputs(bool sceneWindow = true);
#endif

		Math::Vector3 GetForward();

		Math::Vector3 UnProject(const Math::Vector3& screenPos);

		Math::Vector2 ToViewPort(const Math::Vector2& pos);
		Math::Vector2 ToScreenCoordinates(const Math::Vector2& pos);
		Math::Matrix4 LookAt(Math::Vector3 cameraPosition, Math::Vector3 targetPosition, Math::Vector3 upVector);

		void DrawCameraSettings();

		std::ostream& operator<<(std::ostream& os);

		virtual Math::Vector2 GetScreenResolution();

		static void ReadCameraFromFile(Render::Camera* cam, std::fstream& sceneFile, Component::Transform* tr);

		virtual bool IsVisible();
		virtual bool IsVisiblePrefab();
		virtual Component::Transform* GetTransform() const { return transform; }

#ifndef	PANDOR_GAME
		bool PickingUpdate(bool sceneWindow = true);
#endif

		void CreateFrustum();
	};
}
