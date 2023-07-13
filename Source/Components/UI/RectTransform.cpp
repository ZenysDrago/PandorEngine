#include "pch.h"

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


void Component::UI::Rect::ToViewport(Math::Vector2 screenSize)
{
	Min = { (float)(2.0f * Min.x) / (float)screenSize.x - 1.0f, (float)1.0f - (2.0f * Min.y) / (float)screenSize.y };
	Max = { (float)(2.0f * Max.x) / (float)screenSize.x - 1.0f, (float)1.0f - (2.0f * Max.y) / (float)screenSize.y };
}

void Component::UI::Rect::ToScreen(Math::Vector2 screenSize)
{
	Min = { (Min.x + 1.0f) * (screenSize.x / 2.0f), screenSize.y - ((Min.y + 1.0f) * (screenSize.y / 2.0f)) };
	Max = { (Max.x + 1.0f) * (screenSize.x / 2.0f), screenSize.y - ((Max.y + 1.0f) * (screenSize.y / 2.0f)) };
}


Component::UI::RectTransform::~RectTransform()
{
}

void Component::UI::RectTransform::Initialize()
{
}

void Component::UI::RectTransform::Update()
{
	auto pos = gameObject->transform->GetLocalPosition().ToVector2() + GetAnchorPosition();
	m_position = pos + GetPivot();
	pos += -Math::Vector2(-(m_size.x) * (-m_pivot.x), +(m_size.y) * (1 - m_pivot.y));
	m_rect = Rect{ pos, pos + GetSize() };
}

void Component::UI::RectTransform::OnChangeAnchor()
{
	Vector2 MaxSize = GetParentSize();
	switch (m_anchor)
	{
	case Component::UI::Anchor::TOP_LEFT:
		m_anchorPosition = { 0, 0 };
		m_pivot = { 0, 1 };
		break;
	case Component::UI::Anchor::STRETCH_TOP:
		m_size.x = MaxSize.x;
	case Component::UI::Anchor::TOP_MIDDLE:
		m_anchorPosition = { 0.5f, 0 };
		m_pivot = { 0.5, 1 };
		break;
	case Component::UI::Anchor::TOP_RIGHT:
		m_anchorPosition = { 1, 0 };
		m_pivot = { 1, 1 };
		break;
	case Component::UI::Anchor::STRETCH_LEFT:
		m_size.y = MaxSize.y;
	case Component::UI::Anchor::CENTER_LEFT:
		m_anchorPosition = { 0, 0.5f };
		m_pivot = { 0, 0.5 };
		break;
	case Component::UI::Anchor::STRETCH_MIDDLE:
		m_size.x = MaxSize.x;
	case Component::UI::Anchor::CENTER_MIDDLE:
		m_anchorPosition = { 0.5f, 0.5f };
		m_pivot = { 0.5, 0.5 };
		break;
	case Component::UI::Anchor::STRETCH_RIGHT:
		m_size.y = MaxSize.y;
	case Component::UI::Anchor::CENTER_RIGHT:
		m_anchorPosition = { 1.f, 0.5f };
		m_pivot = { 1, 0.5 };
		break;
	case Component::UI::Anchor::BOTTOM_LEFT:
		m_anchorPosition = { 0.f, 1.f };
		m_pivot = { 0, 0 };
		break;
	case Component::UI::Anchor::STRETCH_BOTTOM:
		m_size.x = MaxSize.x;
	case Component::UI::Anchor::BOTTOM_MIDDLE:
		m_anchorPosition = { 0.5f, 1.f };
		m_pivot = { 0.5, 0 };
		break;
	case Component::UI::Anchor::BOTTOM_RIGHT:
		m_anchorPosition = { 1.f, 1.f };
		m_pivot = { 1, 0 };
		break;
	case Component::UI::Anchor::STRETCH_CENTER:
		m_size.y = MaxSize.y;
		m_anchorPosition = { 0.5f, 0.5f };
		m_pivot = { 0.5, 0.5 };
		break;
	case Component::UI::Anchor::STRETCH_ALL:
		m_size = MaxSize;
		m_anchorPosition = { 0.5f, 0.5f };
		m_pivot = { 0.5, 0.5 };
		break;
	default:
		break;
	}
}

void Component::UI::RectTransform::ShowInInspector()
{
	if (WrapperUI::Combo("Anchor", (int*)&m_anchor, "Top Left\0Top Middle\0Top Right\0Center Left\0Center Middle\0Center Right\0Bottom Left\0Bottom Middle\0Bottom Right\0Stretch Top\0Stretch Middle\0Stretch Bottom\0Stretch Left\0Stretch Center\0Stretch Right\0Stretch All"))
	{
		OnChangeAnchor();
	}
	WrapperUI::DragFloat2("Size", &m_size.x);
	WrapperUI::InputFloat2("Anchor Position", &m_anchorPosition.x);
	WrapperUI::InputFloat2("Pivot", &m_pivot.x);
	WrapperUI::Checkbox("Draw Anchor", &m_drawAnchor);
}

Math::Vector2 Component::UI::RectTransform::GetScreenResolution()
{
	if (Core::App::Get().sceneManager->GetCurrentScene()->currentCamera)
		return Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetScreenResolution();
	return 0;
}
Math::Vector2 Component::UI::RectTransform::GetParentPosition()
{
	if (auto parent = gameObject->GetParent())
		if (auto rectTransform = parent->GetComponent<RectTransform>())
			return rectTransform->GetFinalPosition();
	return { 0,0 };
}


Math::Vector2 Component::UI::RectTransform::GetParentSize()
{
	if (auto parent = gameObject->GetParent())
		if (auto rectTransform = parent->GetComponent<RectTransform>())
			return rectTransform->GetSize();
	return GetScreenResolution();
}


Math::Vector2 Component::UI::RectTransform::GetSize()
{
	return m_size;
}

Math::Vector2 Component::UI::RectTransform::GetFinalPosition()
{
	return m_position;
}

Math::Vector2 Component::UI::RectTransform::GetAnchorPosition()
{
	auto max = GetParentSize();
	Math::Vector2 pos = { m_anchorPosition.x * max.x, m_anchorPosition.y * max.y };
	if (auto parent = gameObject->GetParent())
	{
		if (auto ParentRect = parent->GetComponent<RectTransform>())
			pos += ParentRect->GetRectangle().Min;
	}
	return pos;
}

void Component::UI::RectTransform::EditorDraw()
{
}

Math::Vector2 Component::UI::RectTransform::GetPivot()
{
	return Math::Vector2(-(m_size.x) * (m_pivot.x), +(m_size.y) * (m_pivot.y));
}

Component::UI::Rect Component::UI::RectTransform::GetRectangle()
{
	return m_rect;
}

std::ostream& Component::UI::RectTransform::operator<<(std::ostream& os)
{
	os << (int)m_anchor << '\n';
	os << m_size << '\n';
	os << m_anchorPosition << '\n';
	os << m_pivot << '\n';
	return os;
}

void Component::UI::RectTransform::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		m_anchor = (Anchor)std::stoi(line);

	if (getline(sceneFile, line) && line != "end")
		m_size = Math::ParseVector2(line);

	if (getline(sceneFile, line) && line != "end")
		m_anchorPosition = Math::ParseVector2(line);

	if (getline(sceneFile, line) && line != "end")
		m_pivot = Math::ParseVector2(line);

	while (getline(sceneFile, line) && line != "end") {}
}
