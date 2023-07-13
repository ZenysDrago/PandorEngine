#include "pch.h"
#include <Render\EditorIcon.h>

#include <Components/Transform.h>
#include <Resources/Model.h>
#include <Resources/ResourcesManager.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>


Render::EditorIcon::EditorIcon()
{
	
}

Render::EditorIcon::~EditorIcon()
{
	delete m_material;
	m_material = nullptr;
}

void Render::EditorIcon::Initialize()
{
	Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(ENGINEPATH"Models/Plane.obj");
	m_material = new Resources::Material("EditorMaterial", ResourcesType::Material);
	m_material->SetShader(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(ENGINEPATH"Shaders/BillboardShader/billboard"));
}

void Render::EditorIcon::SetIcon(Resources::Texture* tex)
{
	if (m_material)
	{
		m_material->SetTexture(tex);
	}
}

void Render::EditorIcon::Draw(const Math::Vector3& position, bool outline)
{
	if (!Core::SceneManager::Get()->GetCurrentScene()->drawIcons)
		return;
	if (!m_plane)
	{
		m_plane = Resources::ResourcesManager::Get()->Find<Resources::Mesh>(ENGINEPATH"Models/Plane.obj:Plane");
	}
	if (m_plane)
	{
		m_plane->Render(GetMVP(Core::App::Get().sceneManager->GetCurrentScene()->GetVP(), position), Math::Matrix4(), { m_material }, false, true, outline);
	}
}

void Render::EditorIcon::DrawPicking(const Math::Vector3& position, int ID)
{
	if (!Core::SceneManager::Get()->GetCurrentScene()->drawIcons)
		return;
	if (m_plane)
	{
		m_plane->PickingResource(GetMVP(Core::App::Get().sceneManager->GetCurrentScene()->GetVP(), position), { m_material }, ID);
	}
}

Math::Matrix4 Render::EditorIcon::GetMVP(const Math::Matrix4& VP, const Math::Vector3& position)
{

	return Math::GetTranslationMatrix(position) * VP;
}

