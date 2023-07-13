#include "pch.h"
#include <Components/UI/Text.h>
#include <Components/UI/RectTransform.h>

#include <Resources/ResourcesManager.h>
#include <Resources/Font.h>
#include <Resources/Material.h>

#include <Core/App.h>
#include <Core/GameObject.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Render/Camera.h>
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/SceneWindow.h>
#endif

Component::UI::Text::Text()
{
}

Component::UI::Text::~Text()
{
}

void Component::UI::Text::ShowInInspector()
{
	char Content[513];
	strcpy_s(Content, 512, m_text.c_str());
	WrapperUI::InputText("Text", Content, 512);
	SetText(Content);
	if (WrapperUI::Button("Change Font"))
	{
		WrapperUI::OpenPopup("FontPopup");
	}
	if (auto font = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Font>("FontPopup"))
	{
		m_font = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Font>(font->GetPath());
	}
	WrapperUI::InputFloat("Scale", &m_scale);
	WrapperUI::ColorEdit4("Color", &m_color.x);
	WrapperUI::Checkbox("Set Size to Content", &m_sizeToContent);
}

void Component::UI::Text::GameDraw()
{
	if (!m_material || !m_material->GetShader() || !m_material->GetShader()->HasBeenSent() || !m_font->HasBeenSent())
		return;
	auto shader = m_material->GetShader();
	shader->Use();
	// Send Value to shader
	auto mat = Core::App::Get().sceneManager->GetCurrentScene()->GetOrtho();
	WrapperRHI::ShaderSendVec4(shader->GetLocation("textColor"), m_color);
	auto depth = Arithmetics::Clamp(-(gameObject->transform->GetWorldPosition().z - (gameObject->index / 1000.f)) / 10000.f, -1.f, 1.f);
	WrapperRHI::ShaderSendFloat(m_material->GetShader()->GetLocation("depth"), depth);
	WrapperRHI::ShaderSendMat4(shader->GetLocation("projection"), &mat[0][0]);
	// Get the position
	Math::Vector2 pos = GetRectTransform()->GetFinalPosition();
	pos.y = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetScreenResolution().y - pos.y;
	Math::Vector2 size = GetRectTransform()->GetSize();

	m_buffer->Bind();
	std::string::const_iterator c;
	auto Characters = m_font->GetCharacters();
	auto startPosition = pos;
	float maxHeight = 0.f;
	// iterate through all characters
	for (c = m_text.begin(); c != m_text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = pos.x + ch.Bearing.x * m_scale;
		float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * m_scale;

		float w = ch.Size.x * m_scale;
		float h = ch.Size.y * m_scale;
		if (h > maxHeight)
			maxHeight = h;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		WrapperRHI::ActivateTexture(0);
		WrapperRHI::TextureBind(ch.TextureID, PR_TEXTURE2D);
		WrapperRHI::ShaderSendInt(shader->GetLocation("text"), 0);

		// update content of VBO memory
		m_buffer->BindVertexBuffer();
		m_buffer->BufferSubData(0, sizeof(vertices), vertices);

		m_buffer->UnbindVertexBuffer();
		// render quad
		WrapperRHI::DrawArrays(0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		pos.x += (ch.Advance >> 6) * m_scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}

	if (m_sizeToContent)
		GetRectTransform()->SetSize(Math::Vector2((pos.x - startPosition.x), m_font->maxHeight) * m_scale);

	m_buffer->Unbind();
	WrapperRHI::TextureUnBind(PR_TEXTURE2D);
}

void Component::UI::Text::Initialize()
{
	UIElement::Initialize();
	auto shader = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(ENGINEPATH"Shaders/FontShader/Font");
	m_material->SetShader(shader);
	if (!m_font)
		m_font = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Font>(ENGINEPATH"Fonts/arial.ttf");
}

void Component::UI::Text::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		SetText(line);

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_font = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Font>(line);

	if (getline(sceneFile, line) && line != "end")
		m_color = Math::ParseVector4(line);

	if (getline(sceneFile, line) && line != "end")
		SetScale(std::stof(line));

	if (getline(sceneFile, line) && line != "end")
		m_sizeToContent = std::stoi(line);

	while (getline(sceneFile, line) && line != "end") {}
}

std::ostream& Component::UI::Text::operator<<(std::ostream& os)
{
	os << m_text << '\n';
	if (m_font)
		os << m_font->GetPath().c_str() << '\n';
	else
		os << "nullptr" << '\n';
	os << m_color << '\n';
	os << m_scale << '\n';
	os << m_sizeToContent << '\n';

	return os;
}
