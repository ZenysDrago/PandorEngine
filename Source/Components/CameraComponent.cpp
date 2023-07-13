#include "pch.h"
#include <Components\CameraComponent.h>

#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Core/GameObject.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/GameWindow.h>
#include <Render/Framebuffer.h>
#ifndef PANDOR_GAME
#include <Render/EditorIcon.h>
#endif

Component::CameraComponent::CameraComponent(int width, int height, Math::Vector3 position) : Render::Camera(width, height, position)
{
}

Component::CameraComponent::~CameraComponent()
{
}

void Component::CameraComponent::Initialize()
{
	Camera::Init();
	Core::App::Get().sceneManager->GetCurrentScene()->AddCamera(this);
#ifndef PANDOR_GAME
	m_icon = new Render::EditorIcon();
	m_icon->Initialize();
	m_icon->SetIcon(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(ENGINEPATH"Textures/icons/Camera.png"));
#endif
}

void Component::CameraComponent::SetAsMainCamera()
{
	if (Core::App::Get().sceneManager->GetCurrentScene()->mainCamera)
		dynamic_cast<Component::CameraComponent*>(Core::App::Get().sceneManager->GetCurrentScene()->mainCamera)->m_mainCamera = false;
	Core::App::Get().sceneManager->GetCurrentScene()->mainCamera = this;
	m_mainCamera = true;
}

void Component::CameraComponent::DrawPicking(int ID)
{
#ifndef PANDOR_GAME
	m_icon->DrawPicking(GetTransform()->GetWorldPosition(), ID);
#endif
}

void Component::CameraComponent::EditorDraw()
{
#ifndef PANDOR_GAME
	if (Core::SceneManager::Get()->GetCurrentScene()->drawIcons)
		m_icon->Draw(GetTransform()->GetWorldPosition(), gameObject->IsSelected());
	WrapperRHI::Line::Get().Color = 1;
	if (gameObject->IsSelected()) {
		Math::Vector3 topleftnear = UnProject(Math::Vector3{ 0,0, p_near });
		Math::Vector3 topleftfar = UnProject(Math::Vector3{ 0,0, p_far });

		Math::Vector3 toprightnear = UnProject(Math::Vector3{ GetScreenResolution().x,0, p_near });
		Math::Vector3 toprightfar = UnProject(Math::Vector3{ GetScreenResolution().x,0, p_far });

		Math::Vector3 bottomleftnear = UnProject(Math::Vector3{ 0, GetScreenResolution().y, p_near });
		Math::Vector3 bottomleftfar = UnProject(Math::Vector3{ 0, GetScreenResolution().y, p_far });

		Math::Vector3 bottomrightnear = UnProject(Math::Vector3{ GetScreenResolution(), p_near });
		Math::Vector3 bottomrightfar = UnProject(Math::Vector3{ GetScreenResolution(), p_far });

		auto pos = topleftnear;
		auto direction = bottomleftnear;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = bottomleftnear;
		direction = bottomrightnear;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = bottomrightnear;
		direction = toprightnear;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = toprightnear;
		direction = topleftnear;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = topleftnear;
		direction = topleftfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = bottomleftnear;
		direction = bottomleftfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = bottomrightnear;
		direction = bottomrightfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = toprightnear;
		direction = toprightfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = topleftfar;
		direction = bottomleftfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = bottomleftfar;
		direction = bottomrightfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = bottomrightfar;
		direction = toprightfar;
		WrapperRHI::Line::Get().Draw(pos, direction);

		pos = toprightfar;
		direction = topleftfar;
		WrapperRHI::Line::Get().Draw(pos, direction);
	}
#endif
}

void Component::CameraComponent::ShowInInspector()
{
	if (m_showPreviewCamera) {
		if (WrapperUI::Begin("Camera Preview", &m_showPreviewCamera, (WindowFlags)((int)WindowFlags::Modal | (int)WindowFlags::NoFocusOnAppearing)))
		{
			auto size = WrapperUI::GetWindowSize();
			WrapperUI::Image(framebuffer->GetFrameBufferTexture(), Math::Vector2(size.x - 16, size.y - 38.f));
		}
		WrapperUI::End();
	}
	WrapperUI::BeginDisabled(this->m_mainCamera);
	if (WrapperUI::Button("Set Main Camera")) { SetAsMainCamera(); }
	WrapperUI::EndDisabled();
	WrapperUI::Checkbox("Post-Processing", framebuffer->EnablePostProcessing());
	WrapperUI::BeginDisabled(!*framebuffer->EnablePostProcessing());
	if (WrapperUI::Button("Screen Shader"))
	{
		WrapperUI::OpenPopup("ScreenShaderPopup");
	}
	if (auto shad = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Shader>("ScreenShaderPopup"))
	{
		framebuffer->SetShader(shad);
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(framebuffer->GetShader()->GetName().c_str());
	WrapperUI::EndDisabled();

	DrawCameraSettings();
	WrapperUI::Checkbox("Show Preview", &m_showPreviewCamera);
}

void Component::CameraComponent::OnDestroy()
{
	if (!gameObject->GetScene()->IsPrefabScene())
		Core::App::Get().sceneManager->GetCurrentScene()->RemoveCamera(this);
#ifndef PANDOR_GAME
	delete m_icon;
	m_icon = nullptr;
#endif
}

Component::Transform* Component::CameraComponent::GetTransform() const
{
	return Component::BaseComponent::gameObject->transform;
}

Math::Vector2 Component::CameraComponent::GetScreenResolution()
{
#ifndef PANDOR_GAME
	return Core::App::Get().GetEditorUIManager().GetGameWindow().GetSize();
#else
	return Core::App::Get().window->GetSize();
#endif
}

bool Component::CameraComponent::IsVisible()
{
#ifndef PANDOR_GAME
	return (Component::BaseComponent::gameObject->IsSelected() && m_showPreviewCamera) || (m_mainCamera && Core::App::Get().GetEditorUIManager().GetGameWindow().IsOpen() && Core::App::Get().GetEditorUIManager().GetGameWindow().IsVisible());
#else
	return m_mainCamera;
#endif
}

void Component::CameraComponent::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (!framebuffer)
	{
		framebuffer = new Render::Framebuffer();
		framebuffer->Initialize({ 800, 600 });
	}

	if (getline(sceneFile, line) && line != "end")
		p_enable = (bool)std::atoi(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		m_mainCamera = (bool)std::atoi(line.c_str());
	if (m_mainCamera)
		SetAsMainCamera();

	if (getline(sceneFile, line) && line != "end")
		framebuffer->m_enablePostProcessing = (bool)std::atoi(line.c_str());

	if (framebuffer->m_enablePostProcessing)
	{
		framebuffer->m_postProcess = new Render::Framebuffer();
		framebuffer->m_postProcess->Initialize({ 800, 600 });
	}

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			framebuffer->m_material->SetShader(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(line));

	if (getline(sceneFile, line) && line != "end")
		fov = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		p_near = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		p_far = std::stof(line.c_str());

	if (getline(sceneFile, line) && line != "end")
		ClearColor = Math::ParseVector4(line);

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			skybox = Resources::ResourcesManager::Get()->GetOrLoad<Resources::CubeMap>(line);

	while (getline(sceneFile, line) && line != "end") {}

}

std::ostream& Component::CameraComponent::operator<<(std::ostream& os)
{
	os << p_enable << "\n";
	os << m_mainCamera << "\n";

	os << framebuffer->m_enablePostProcessing << "\n";
	if (framebuffer->m_postProcess)
		os << framebuffer->m_material->GetShader()->GetPath() << "\n";
	else
		os << "nullptr" << "\n";
	os << fov << "\n";
	os << p_near << "\n";
	os << p_far << "\n";
	os << ClearColor << "\n";
	if (skybox)
		os << skybox->GetPath() << "\n";
	else
		os << "nullptr" << '\n';

	return os;
}
