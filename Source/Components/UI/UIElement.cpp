#include "pch.h"

#include <Components/UI/UIElement.h>
#include <Components/UI/RectTransform.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Mesh.h>
#include <Resources/Shader.h>
#include <Resources/Material.h>
#include <Core/GameObject.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Render/Camera.h>

Component::UI::UIElement::~UIElement()
{
	delete m_material;
	m_material = nullptr;
	delete m_buffer;
	m_buffer = nullptr;
}

void Component::UI::UIElement::Initialize()
{
	m_buffer = new WrapperRHI::Buffer();
	m_buffer->InitializePlane();

	auto shader = Resources::ResourcesManager::Get()->Find<Resources::Shader>(ENGINEPATH"Shaders/UIShader");
	//TODO: Move material to resourceManager
	m_material = new Resources::Material("UIMaterial", ResourcesType::Material);
	m_material->SetShader(shader);
	m_rectTransform = gameObject->GetComponent<Component::UI::RectTransform>();
}

void Component::UI::UIElement::Update()
{
	if (!m_rectTransform)
	{
		m_rectTransform = gameObject->GetOrAddComponent<Component::UI::RectTransform>();
	}
}

void Component::UI::UIElement::GameDraw()
{
	if (!m_material || !m_material->GetShader() || !m_material->GetShader()->HasBeenSent())
		return;
	m_material->GetShader()->Use();
	auto mat = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetOrtho();
	auto depth = Arithmetics::Clamp(-(gameObject->transform->GetWorldPosition().z - (gameObject->index / 1000.f)) / 10000.f, -1.f, 1.f);
	WrapperRHI::ShaderSendMat4(m_material->GetShader()->GetLocation("projection"), &mat[0][0]);
	WrapperRHI::ShaderSendInt(m_material->GetShader()->GetLocation("enableTexture"), m_material->GetTexture() != nullptr);
	WrapperRHI::ShaderSendFloat(m_material->GetShader()->GetLocation("depth"), depth);
	WrapperRHI::ShaderSendVec4(m_material->GetShader()->GetLocation("ourColor"), m_material->GetDiffuse());

	Math::Vector2 pos = GetRectTransform()->GetFinalPosition();
	pos.y = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetScreenResolution().y - pos.y;
	Math::Vector2 size = GetRectTransform()->GetSize();

	m_buffer->Bind();

	float vertices[6][4] = {
			{ pos.x			, pos.y + size.y, 0.0f, 0.0f },
			{ pos.x			, pos.y			, 0.0f, 1.0f },
			{ pos.x + size.x, pos.y			, 1.0f, 1.0f },

			{ pos.x			, pos.y + size.y, 0.0f, 0.0f },
			{ pos.x + size.x, pos.y			, 1.0f, 1.0f },
			{ pos.x + size.x, pos.y + size.y, 1.0f, 0.0f }
	};
	// render glyph texture over quad
	if (m_material->GetTexture())
		m_material->GetTexture()->Bind();
	// update content of VBO memory
	m_buffer->BindVertexBuffer();
	m_buffer->BufferSubData(0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

	m_buffer->UnbindVertexBuffer();
	// render quad
	WrapperRHI::DrawArrays(0, 6, false, false);


	m_buffer->Unbind();
	if (m_material->GetTexture())
		m_material->GetTexture()->UnBind();
}

Component::UI::RectTransform* Component::UI::UIElement::GetRectTransform()
{
	if (!m_rectTransform)
	{
		m_rectTransform = gameObject->GetOrAddComponent<Component::UI::RectTransform>();
	}
	return m_rectTransform;
}
