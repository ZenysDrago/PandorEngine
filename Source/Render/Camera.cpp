#include "pch.h"

#include <Components/Transform.h>
#include <Resources/Shader.h>
#include <Resources/ResourcesManager.h>
#ifndef PANDOR_GAME
#include <EditorUI/SceneWindow.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/GameWindow.h>
#include <EditorUI/Inspector.h>
#include <Render/Gizmo.h>
#include <EditorUI/PrefabWindow.h>
#endif
#include <Render/Camera.h>
#include <Render/Framebuffer.h>
#include <Core/App.h>
#include <Core/Scene.h>

#include <Core/SceneManager.h>
#include <Core/GameObject.h>
#include <Core/Scene.h>

using namespace Render;
using namespace Core::Wrapper;

Camera::Camera(int width, int height, Vector3 position)
{
}

Camera::Camera()
{

}

void Camera::SetAsMainCamera()
{
}

Render::Camera::~Camera()
{
	delete framebuffer;
	framebuffer = nullptr;
	delete transform;
	transform = nullptr;
}

void Render::Camera::Init()
{
	if (!transform)
		transform = new Component::Transform();
	if (!framebuffer)
	{
		framebuffer = new Framebuffer();
		framebuffer->Initialize({ 800, 600 });
	}

	skybox = Resources::ResourcesManager::Get()->GetOrLoad<Resources::CubeMap>(ENGINEPATH"CubeMaps/Space1.cbm");
	CreateFrustum();
}

Math::Matrix4 Rotate(float angle, const Math::Vector3& axis)
{
	Math::Matrix4 result(1.0f);
	float cosA = std::cos(angle);
	float sinA = std::sin(angle);

	Math::Vector3 axisN = axis;
	axisN.Normalize();

	float oneMinusCosA = 1.0f - cosA;

	result[0][0] = cosA + oneMinusCosA * axisN.x * axisN.x;
	result[0][1] = oneMinusCosA * axisN.x * axisN.y - sinA * axisN.z;
	result[0][2] = oneMinusCosA * axisN.x * axisN.z + sinA * axisN.y;
	result[0][3] = 0.0f;

	result[1][0] = oneMinusCosA * axisN.x * axisN.y + sinA * axisN.z;
	result[1][1] = cosA + oneMinusCosA * axisN.y * axisN.y;
	result[1][2] = oneMinusCosA * axisN.y * axisN.z - sinA * axisN.x;
	result[1][3] = 0.0f;

	result[2][0] = oneMinusCosA * axisN.x * axisN.z - sinA * axisN.y;
	result[2][1] = oneMinusCosA * axisN.y * axisN.z + sinA * axisN.x;
	result[2][2] = cosA + oneMinusCosA * axisN.z * axisN.z;
	result[2][3] = 0.0f;

	result[3][0] = 0.0f;
	result[3][1] = 0.0f;
	result[3][2] = 0.0f;
	result[3][3] = 1.0f;

	return result;
}

#ifndef PANDOR_GAME
void Render::Camera::ReadPickingID(unsigned char data[4], bool sceneWindow /*= true*/)
{
	pickedID =
		data[0] +
		data[1] * 256 +
		data[2] * 256 * 256;

	if (WrapperUI::IsMouseClicked(MouseButton::Left) &&
		(sceneWindow && Core::App::Get().GetEditorUIManager().GetSceneWindow().IsHovered()
			|| !sceneWindow && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsHovered()))
	{
		picked = Core::App::Get().sceneManager->GetCurrentNode()->GetWithPickingIndex(pickedID);
		if (picked != nullptr)
		{
			if (picked->isPrefab)
			{
				picked = picked->GetRootPrefab();
			}
			Core::App::Get().GetEditorUIManager().GetInspector().SetGameObjectSelected(picked);
		}

		if (Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->GetGizmo())
		{
			if (pickedGizmo = Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->GetGizmo()->GetWithPickingIndex(pickedID))
			{
				Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->clickedGizmo = true;
			}
			if (picked == nullptr && pickedGizmo == nullptr)
			{
				Core::App::Get().GetEditorUIManager().GetInspector().SetGameObjectSelected(picked);
			}
		}

		WrapperRHI::ClearColorAndBuffer(Core::SceneManager::Get()->GetCurrentScene()->GetEditorCamera()->ClearColor);
		Core::SceneManager::Get()->GetCurrentScene()->GetEditorCamera()->DrawSkybox();
	}
	else if (Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->clickedGizmo && WrapperUI::IsMouseReleased(MouseButton::Left))
	{
		Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->clickedGizmo = false;
	}

}
#endif

void Camera::CameraWithMouse(Vector2 vec)
{
	if (firstClick) {
		moveX = vec.x;
		moveY = vec.y;
		firstClick = false;
	}

	float offsetX = moveX - vec.x;
	float offsetY = vec.y - moveY;
	moveX = vec.x;
	moveY = vec.y;

	offsetX *= sensibility;
	offsetY *= sensibility;

	angle -= offsetX;
	pitch += offsetY;

	if (pitch > 89)
		pitch = 89;

	if (pitch < -89)
		pitch = -89;

	Quaternion newRot = Math::Vector3(pitch, -angle, 0).ToQuaternion();
	GetTransform()->SetLocalRotation(newRot);
}

Math::Matrix4 Camera::GetProjectionMatrix()
{
	float s = 1.0f / ((p_aspectRatio)*atanf((fov / 2.0f) * DEG2RAD));
	float s2 = 1.0f / atanf((fov / 2.0f) * DEG2RAD);
	float param1 = -(p_far + p_near) / (p_far - p_near);
	float param2 = -(2 * p_near * p_far) / (p_far - p_near);
	Math::Matrix4 out;
	out[0][0] = s;
	out[1][1] = s2;
	out[2][2] = param1;
	out[2][3] = -1;
	out[3][2] = param2;
	return out;
}

Math::Matrix4 Render::Camera::GetViewMatrix()
{
	Math::Matrix4 temp;
	const Math::Vector3 z = -GetTransform()->GetForwardVector();
	const Math::Vector3 x = -GetTransform()->GetRightVector();
	const Math::Vector3 y = GetTransform()->GetUpVector();
	const Math::Vector3 delta = Math::Vector3(-x.Dot(this->GetTransform()->GetWorldPosition() - z), -y.Dot(this->GetTransform()->GetWorldPosition() - z), -z.Dot(this->GetTransform()->GetWorldPosition() - z));
	for (int i = 0; i < 3; i++)
	{
		temp[i][0] = x[i];
		temp[i][1] = y[i];
		temp[i][2] = z[i];
		temp[3][i] = delta[i];
	}
	temp[3][3] = 1;
	return temp;
}

Math::Matrix4 Render::Camera::GetViewNoTranslate()
{
	Math::Matrix4 temp;
	const Math::Vector3 z = -GetTransform()->GetForwardVector();
	const Math::Vector3 x = -GetTransform()->GetRightVector();
	const Math::Vector3 y = GetTransform()->GetUpVector();
	for (int i = 0; i < 3; i++)
	{
		temp[i][0] = x[i];
		temp[i][1] = y[i];
		temp[i][2] = z[i];
		temp[3][i] = 0.f;
	}
	temp[3][3] = 0.f;
	return temp;
}

Math::Matrix4 Render::Camera::GetOrtho()
{
	Math::Matrix4 Result;
	Result[0][0] = 2.f / (GetScreenResolution().x);
	Result[1][1] = 2.f / (GetScreenResolution().y);
	Result[2][2] = -1.f;
	Result[0][3] = -(GetScreenResolution().x) / (GetScreenResolution().x);
	Result[1][3] = -(GetScreenResolution().y) / (GetScreenResolution().y);
	return Result;
}

#ifndef	PANDOR_GAME
void Render::Camera::Inputs(bool sceneWindow /*= true*/)
{
	if (!WrapperUI::IsMouseDown(MouseButton::Right)
		|| (sceneWindow && !Core::App::Get().GetEditorUIManager().GetSceneWindow().IsHovered())
		|| (!sceneWindow && !Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsHovered())) {
		firstClick = true;
		if(!Core::App::Get().GetEditorUIManager().GetGameWindow().IsVisible())
			WrapperUI::SetMouseCursor(MouseCursor::Arrow);
		return;
	}

	auto Position = GetTransform()->GetLocalPosition();

	if (WrapperUI::IsKeyDown(Key::Key_W))
		Position += GetTransform()->GetForwardVector() * speed * WrapperUI::GetDeltaTime();

	if (WrapperUI::IsKeyDown(Key::Key_A)) {
		Position += GetTransform()->GetRightVector() * speed * WrapperUI::GetDeltaTime();
	}

	if (WrapperUI::IsKeyDown(Key::Key_S))
		Position += -GetTransform()->GetForwardVector() * speed * WrapperUI::GetDeltaTime();

	if (WrapperUI::IsKeyDown(Key::Key_D)) {
		Position += -GetTransform()->GetRightVector() * speed * WrapperUI::GetDeltaTime();
	}

	if (WrapperUI::IsKeyDown(Key::Key_Space))
		Position += up * speed * WrapperUI::GetDeltaTime();

	if (WrapperUI::IsKeyDown(Key::Key_LeftCtrl))
		Position += -up * speed * WrapperUI::GetDeltaTime();

	if (WrapperUI::IsKeyDown(Key::Key_LeftShift))
		speed = 70.f;

	else if (WrapperUI::IsKeyReleased(Key::Key_LeftShift))
		speed = 35.f;

	GetTransform()->SetLocalPosition(Position);
	if (WrapperUI::IsMouseDown(MouseButton::Right))
	{
		WrapperUI::SetMouseCursor(MouseCursor::None);
		POINT mouse = { (long)0.0 };
		GetCursorPos(&mouse);
		static POINT prevMouse = mouse;
		Vector2 mouseDelta = { (float)(mouse.x - prevMouse.x), (float)(mouse.y - prevMouse.y) };
		SetCursorPos(prevMouse.x, prevMouse.y);

		if (!WrapperUI::IsMouseClicked(MouseButton::Right)) {
			crntpos += mouseDelta;
			CameraWithMouse(crntpos);
		}
		else {
			prevMouse = mouse;
		}
	}

	else if (WrapperUI::IsMouseReleased(MouseButton::Right)) {
		firstClick = true;
		WrapperUI::SetMouseCursor(MouseCursor::Arrow);
	}
}
#endif

Math::Matrix4 Camera::GetVPMatrix()
{
	return GetViewMatrix() * GetProjectionMatrix();
}

void Render::Camera::PreUpdate(float aspect)
{
	framebuffer->PreUpdate();
	SetAspectRatio(aspect);
}

void Camera::PostUpdate()
{
	framebuffer->Draw();
}

void Render::Camera::DrawSkybox()
{
	if (skybox)
		skybox->Render(GetViewNoTranslate() * GetProjectionMatrix());
}

Math::Vector3 Camera::GetForward()
{
	return GetTransform()->GetForwardVector();
}

Math::Vector3 Render::Camera::UnProject(const Math::Vector3& point)
{
	Math::Vector4 mousePosition = { ToViewPort(point.ToVector2()) , 1.f, 1.f };
	auto invVP = GetVPMatrix().GetTransposed().GetInverse();
	auto position = GetTransform()->GetWorldPosition();
	return position + (invVP * mousePosition) * point.z;
}

Math::Vector2 Render::Camera::ToViewPort(const Math::Vector2& pos)
{
	return { (float)(2.0f * pos.x) / (float)GetScreenResolution().x - 1.0f, (float)1.0f - (2.0f * pos.y) / (float)GetScreenResolution().y };
}

Math::Vector2 Render::Camera::ToScreenCoordinates(const Math::Vector2& pos)
{
	return { (pos.x + 1.0f) * (GetScreenResolution().x / 2.0f), GetScreenResolution().y - ((pos.y + 1.0f) * (GetScreenResolution().y / 2.0f)) };
}

void Render::Camera::DrawCameraSettings()
{
	WrapperUI::TextUnformatted("General:");
	WrapperUI::DragFloat("FOV", &fov);
	WrapperUI::DragFloatRange2("Near/Far", &p_near, &p_far);
	WrapperUI::ColorEdit4("Clear Color", &ClearColor.x);
	WrapperUI::Dummy(Vector2(2, 2));
	WrapperUI::TextUnformatted("Skybox:");
	if (WrapperUI::Button(skybox ? skybox->GetName().c_str() : "None", Math::Vector2(157, 0)))
	{
		WrapperUI::OpenPopup("Skybox List");
	}
	if (Resources::Skybox* sky = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Skybox>("Skybox List"))
	{
		skybox = Resources::ResourcesManager::Get()->GetOrLoad<Resources::CubeMap>(sky->GetPath());
	}
	WrapperUI::SameLine();
	bool value = false;
	if (WrapperUI::Button("Reset")) {
		skybox = nullptr;
		value = true;
	}
}

#ifndef	PANDOR_GAME
bool Render::Camera::PickingUpdate(bool sceneWindow /*= true*/)
{
	if (WrapperUI::IsMouseClicked(MouseButton::Left) && 
		(sceneWindow && Core::App::Get().GetEditorUIManager().GetSceneWindow().IsHovered() 
		|| !sceneWindow && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsHovered()))
	{
		WrapperRHI::ClearColorAndBuffer({ 1.0f, 1.0f, 1.0f, 1.0f });
		int ID = 0;
		Core::App::Get().sceneManager->GetCurrentScene()->GetSceneNode()->DrawPicking(ID);
		if (Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->GetGizmo() && Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected())
		{
			WrapperRHI::DepthRange({ 0.f, 0.1f });
			Core::App::Get().sceneManager->GetCurrentScene()->drawGizmo->GetGizmo()->DrawPicking(ID);
			WrapperRHI::DepthRange({ 0.2f, 1.f });
		}
		return true;
	}
	return false;

}
#endif

void Render::Camera::CreateFrustum()
{
	float fovY = -fov * DEG2RAD;
	float halfVSide = p_far * 5.f * tanf(fovY * .5f);
	float halfHSide = halfVSide * p_aspectRatio;
	auto camPos = GetTransform()->GetWorldPosition();
	auto camFront = GetTransform()->GetForwardVector();
	auto camUp = GetTransform()->GetUpVector();
	auto camRight = GetTransform()->GetRightVector();

	Vector3 frontMultFar = camFront * p_far * 5.f;

	frustum.planes[0] = MathUtils::Plane(camPos + camFront * p_near, camFront);
	frustum.planes[1] = MathUtils::Plane(camPos + frontMultFar, -camFront);
	frustum.planes[2] = MathUtils::Plane(camPos, (frontMultFar - camRight * halfHSide).Cross(camUp));
	frustum.planes[3] = MathUtils::Plane(camPos, camUp.Cross(frontMultFar + camRight * halfHSide));
	frustum.planes[4] = MathUtils::Plane(camPos, camRight.Cross(frontMultFar - camUp * halfVSide));
	frustum.planes[5] = MathUtils::Plane(camPos, (frontMultFar + camUp * halfVSide).Cross(camRight));
}

Math::Vector2 Camera::GetScreenResolution()
{
#ifndef	PANDOR_GAME
	return Core::App::Get().GetEditorUIManager().GetSceneWindow().GetWindowSize();
#else
	return Core::App::Get().window->GetSize();
#endif
}

std::ostream& Render::Camera::operator<<(std::ostream& os)
{
	os << "===" << "\n";
	os << "Camera" << "\n";
	os << "===" << "\n";
	os << transform->GetComponentName() << '\n';
	transform->operator<<(os);
	os << "end" << '\n';
	os << x << "\n";
	os << y << "\n";
	os << moveX << "\n";
	os << moveY << "\n";
	os << sensibility << "\n";
	os << speed << "\n";
	os << angle << "\n";
	os << pitch << "\n";
	os << fov << "\n";
	os << p_near << "\n";
	os << p_far << "\n";
	os << fullScreen << "\n";
	os << firstClick << "\n";
	os << up << "\n";
	os << ClearColor << "\n";

	framebuffer->operator<<(os);

	os << "end" << "\n";
	return os;
}

void Render::Camera::ReadCameraFromFile(Render::Camera* cam, std::fstream& sceneFile, Component::Transform* tr)
{
	std::string line;

	if (cam->transform)
		delete cam->transform;
	cam->transform = tr;

	if (getline(sceneFile, line) && line != "end")
		cam->x = std::stod(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->y = std::stod(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->moveX = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->moveY = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->sensibility = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->speed = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->angle = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->pitch = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->fov = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->p_near = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->p_far = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->fullScreen = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->firstClick = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		cam->up = Math::ParseVector3(line);

	if (getline(sceneFile, line) && line != "end")
		cam->ClearColor = Math::ParseVector4(line);

	if (getline(sceneFile, line) && line != "end")
	{
		if (line == "===")
			Render::Framebuffer::ReadFrameBufferFromFile(cam->framebuffer, sceneFile);
		else
			cam->framebuffer = nullptr;
	}

	while (getline(sceneFile, line) && line != "end") {}
}

bool Camera::IsVisible()
{
#ifndef	PANDOR_GAME
	return Core::App::Get().GetEditorUIManager().GetSceneWindow().IsOpen() || Core::App::Get().GetEditorUIManager().GetSceneWindow().IsVisible();
#else
	return false;
#endif
}

bool Render::Camera::IsVisiblePrefab()
{
#ifndef	PANDOR_GAME
	return Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsOpen() && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsVisible();
#else
	return false;
#endif
}

void Frustum::Draw()
{
}
