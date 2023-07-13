#include "pch.h"

#include <Components/UI/UIImage.h>
#include <Components/UI/RectTransform.h>
#include <Core/GameObject.h>
#include <Resources/Mesh.h>
#include <Resources/Material.h>
#include <Resources/Texture.h>
#include <Core/App.h>
#include <Render/Camera.h>

Component::UI::UIImage::~UIImage()
{
}

void Component::UI::UIImage::Initialize()
{
	UIElement::Initialize();
}

void Component::UI::UIImage::ShowInInspector()
{
	UIElement::ShowInInspector();
	if (WrapperUI::ChangeTextureButton(m_texture))
		m_material->SetTexture(GetImage());
	auto color = m_material->GetDiffuse();
	if (WrapperUI::ColorEdit4("Color", &color.x))
	{
		m_material->SetDiffuse(color);
	}
}

std::ostream& Component::UI::UIImage::operator<<(std::ostream& os)
{
	if (m_material->GetTexture())
		os << m_material->GetTexture()->GetPath() << '\n';
	else
		os << "nullptr" << '\n';
	return os;
}

void Component::UI::UIImage::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
		{
			SetImage(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(line));
		}

	while (getline(sceneFile, line) && line != "end") {}
}

void Component::UI::UIImage::Update()
{
	UIElement::Update();
	if (m_material->GetTexture() != m_texture)
		SetImage(m_texture);
}

void Component::UI::UIImage::SetImage(Resources::Texture* tex)
{
	m_texture = tex;
	if (m_material)
		m_material->SetTexture(tex);
}
