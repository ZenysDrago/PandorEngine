#include "pch.h"
#include <Components/TextMesh.h>

#include <Core/Wrappers/WrapperRHI.h>

#include <Resources/Material.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Font.h>

#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Core/GameObject.h>

Component::TextMesh::TextMesh()
{

}

Component::TextMesh::~TextMesh()
{
	delete m_material;
	m_material = nullptr;
}

void Component::TextMesh::Initialize()
{
	m_buffer = new WrapperRHI::Buffer();
	m_buffer->InitializePlane();
	if (!m_material) {
		m_material = new Resources::Material("Text3DMaterial", Resources::ResourcesType::Material);
		auto shader = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(ENGINEPATH"Shaders/FontShader/Font");
		m_material->SetShader(shader);
	}
	if (!m_font)
		m_font = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Font>(ENGINEPATH"Fonts/arial.ttf");
}

void Component::TextMesh::Draw()
{
	if (!m_material || !m_material->GetShader() || !m_material->GetShader()->HasBeenSent() || !m_font->HasBeenSent())
		return;
	auto shader = m_material->GetShader();
	shader->Use();
	// Send Value to shader
	auto mat = gameObject->transform->GetModelMatrix() * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
	WrapperRHI::ShaderSendVec4(shader->GetLocation("textColor"), m_material->GetDiffuse());
	WrapperRHI::ShaderSendMat4(shader->GetLocation("projection"), &mat[0][0], false);

	Vector3 pos(0);
	Vector2 scale = Vector2(gameObject->transform->GetLocalScale().x, gameObject->transform->GetLocalScale().y) * 0.015f;

	m_buffer->Bind();
	std::string::const_iterator c;
	auto Characters = m_font->GetCharacters();
	float maxHeight = 0.f;
	// iterate through all characters
	for (c = m_text.begin(); c != m_text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = pos.x + ch.Bearing.x * scale.x;
		float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale.y;

		float w = ch.Size.x * scale.x;
		float h = ch.Size.y * scale.y;
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
		WrapperRHI::TextureBind(ch.TextureID, PR_TEXTURE2D);
		// update content of VBO memory
		m_buffer->BindVertexBuffer();
		m_buffer->BufferSubData(0, sizeof(vertices), vertices);

		m_buffer->UnbindVertexBuffer();
		// render quad
		WrapperRHI::DrawArrays(0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		pos.x += (ch.Advance >> 6) * scale.x; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}

	m_buffer->Unbind();
	WrapperRHI::TextureUnBind(PR_TEXTURE2D);
}

void Component::TextMesh::DrawPicking(int ID)
{
	int r = (ID & 0x000000FF) >> 0;
	int g = (ID & 0x0000FF00) >> 8;
	int b = (ID & 0x00FF0000) >> 16;

	auto shaderData = m_material->GetPickingShader();
	if (!shaderData)
		return;

	shaderData->Use();
	m_buffer->Bind();
	if (!shaderData->HasBeenSent())
		return;
	auto mat = gameObject->transform->GetModelMatrix() * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
	WrapperRHI::ShaderSendVec4(shaderData->GetLocation("PickingColor"), { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f });
	WrapperRHI::ShaderSendMat4(shaderData->GetLocation("projection"), &mat[0][0], false);

	Vector3 pos(0);
	Vector2 scale = Vector2(gameObject->transform->GetLocalScale().x, gameObject->transform->GetLocalScale().y) * 0.015f;

	m_buffer->Bind();
	std::string::const_iterator c;
	auto Characters = m_font->GetCharacters();
	float maxHeight = 0.f;
	// iterate through all characters
	for (c = m_text.begin(); c != m_text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = pos.x + ch.Bearing.x * scale.x;
		float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale.y;

		float w = ch.Size.x * scale.x;
		float h = ch.Size.y * scale.y;
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
		WrapperRHI::TextureBind(ch.TextureID, PR_TEXTURE2D);
		// update content of VBO memory
		m_buffer->BindVertexBuffer();
		m_buffer->BufferSubData(0, sizeof(vertices), vertices);

		m_buffer->UnbindVertexBuffer();
		// render quad
		WrapperRHI::DrawArrays(0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		pos.x += (ch.Advance >> 6) * scale.x; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	m_buffer->Unbind();
	WrapperRHI::TextureUnBind(PR_TEXTURE2D);
}

void Component::TextMesh::ShowInInspector()
{
	char text[128];
	strcpy_s(text, 128, m_text.c_str());
	if (WrapperUI::InputText("Text", text, 128))
	{
		m_text = text;
	}
	if (WrapperUI::Button("Change Font"))
	{
		WrapperUI::OpenPopup("FontPopup");
	}
	if (auto font = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Font>("FontPopup"))
	{
		m_font = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Font>(font->GetPath());
	}
	auto diffuse = m_material->GetDiffuse();
	WrapperUI::ColorEdit4("Color", &diffuse.x);
	m_material->SetDiffuse(diffuse);
}

void Component::TextMesh::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		m_text = line;

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
		{
			m_font = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Font>(line);
		}


	if (getline(sceneFile, line) && line != "end")
	{
		m_material = new Resources::Material("Text3DMaterial", Resources::ResourcesType::Material);
		auto shader = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(ENGINEPATH"Shaders/FontShader/Font");
		m_material->SetShader(shader);
		m_material->SetDiffuse(ParseVector4(line));
	}
	while (getline(sceneFile, line) && line != "end") {}
}

std::ostream& Component::TextMesh::operator<<(std::ostream& os)
{
	os << m_text << '\n';
	if (m_font)
		os << m_font->GetPath().c_str() << '\n';
	else
		os << "nullptr" << '\n';
	os << m_material->GetDiffuse() << '\n';
	return os;
}
