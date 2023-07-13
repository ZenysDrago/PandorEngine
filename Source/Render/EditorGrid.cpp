#include "pch.h"
#include <Render\EditorGrid.h>
#include <Render\Camera.h>

#include <Resources/Material.h>
#include <Resources/Mesh.h>
#include <Resources/ResourcesManager.h>

#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>


Render::EditorGrid::EditorGrid()
{
}

Render::EditorGrid::~EditorGrid()
{
	delete m_buffer;
	m_buffer = nullptr;
	delete m_material;
	m_material = nullptr;
}

void Render::EditorGrid::Initialize()
{
	m_material = new Resources::Material("GridMaterial", ResourcesType::Material);
	auto gridShader = ResourcesManager::Get()->Create<Resources::Shader>(ENGINEPATH"Shaders/grid");
	m_material->SetShader(gridShader);
	m_buffer = new WrapperRHI::Buffer();
	m_buffer->InitializePlane();
}

void Render::EditorGrid::Draw()
{
	if (!m_material->GetShader() || !m_material->GetShader()->HasBeenSent())
		return;
	m_buffer->Bind();
	m_material->GetShader()->Use();

	WrapperRHI::ShaderSendMat4(m_material->GetShader()->GetLocation("view"), Core::SceneManager::Get()->GetCurrentScene()->currentCamera->GetViewMatrix(), false);
	WrapperRHI::ShaderSendMat4(m_material->GetShader()->GetLocation("proj"), Core::SceneManager::Get()->GetCurrentScene()->currentCamera->GetProjectionMatrix(), false);
	WrapperRHI::ShaderSendFloat(m_material->GetShader()->GetLocation("near"), Core::SceneManager::Get()->GetCurrentScene()->currentCamera->GetNear());
	WrapperRHI::ShaderSendFloat(m_material->GetShader()->GetLocation("far"), Core::SceneManager::Get()->GetCurrentScene()->currentCamera->GetFar());

	WrapperRHI::DrawArrays(0, 6, false, false);

	m_buffer->Unbind();

}
